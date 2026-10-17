#include "papernx/core.hpp"
#include "papernx/mupdf_bridge.h"
#include "papernx/theme.hpp"
#include "papernx/typography.hpp"
#include "papernx/gestures.hpp"
#include "papernx/page_cache.hpp"

#ifdef __SWITCH__
#include <switch.h>
#endif

#include <SDL.h>
#include <SDL_ttf.h>
#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdio>
#include <cmath>
#include <ctime>
#include <deque>
#include <filesystem>
#include <fstream>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <thread>
#include <vector>

using namespace papernx;

namespace {

// devkitPro SDL2's built-in Switch mapping swaps A/B and X/Y. Keep labels
// consistent with the native pad_mapping_default[] button indices.
std::string switchControllerMapping(const char *guid) {
    return std::string(guid) + ",PaperNX Switch,a:b0,b:b1,x:b2,y:b3,"
        "leftstick:b4,rightstick:b5,leftshoulder:b6,rightshoulder:b7,"
        "lefttrigger:b8,righttrigger:b9,start:b10,back:b11,"
        "dpleft:b12,dpup:b13,dpright:b14,dpdown:b15,"
        "leftx:a0,lefty:a1,rightx:a2,righty:a3,";
}

void prepareControllerMapping(int index) {
#ifdef __SWITCH__
    const char *name = SDL_JoystickNameForIndex(index);
    if (name && std::string(name) == "Switch Controller") {
        char guid[33] = {};
        SDL_JoystickGetGUIDString(SDL_JoystickGetDeviceGUID(index), guid, sizeof guid);
        SDL_GameControllerAddMapping(switchControllerMapping(guid).c_str());
    }
#else
    (void)index;
#endif
}

enum class ScreenState {
    Library,
    Reader,
    FileBrowser,
    AaModal,
    TocModal,
    BookmarksModal
};

enum class ViewMode {
    Grid,
    List
};

struct BrowserEntry {
    fs::path path;
    bool isDirectory{false};
    std::uintmax_t size{0};
};

struct LoadedThumb {
    std::string id;
    int w{0};
    int h{0};
    int stride{0};
    std::vector<unsigned char> rgb;
};

class PaperNxApp {
public:
    explicit PaperNxApp(fs::path rootPath)
        : root_(std::move(rootPath)),
          progressStore_(root_ / "data" / "progress.txt"),
          pageCache_(64 * 1024 * 1024),
          thumbCache_(32 * 1024 * 1024) {}

    ~PaperNxApp() {
        // 1. Stop background thumbnail worker thread safely
        workerRunning_ = false;
        queueCv_.notify_all();
        if (thumbWorker_.joinable()) {
            thumbWorker_.join();
        }

        // 2. Close active document
        closeBook();

        flushProgress();

        // 3. Crucial: Release all textures and fonts BEFORE destroying renderer
        pageCache_.clear();
        thumbCache_.clear();
        typography_.reset();

        if (controller_) {
            SDL_GameControllerClose(controller_);
            controller_ = nullptr;
        }
        if (renderer_) {
            SDL_DestroyRenderer(renderer_);
            renderer_ = nullptr;
        }
        if (window_) {
            SDL_DestroyWindow(window_);
            window_ = nullptr;
        }
#ifdef __SWITCH__
        if (romfsReady_) romfsExit();
#endif
    }

    void logMessage(const std::string &msg) {
        std::error_code ec;
        fs::path logDir = root_ / "logs";
        fs::create_directories(logDir, ec);
        fs::path logFile = logDir / "papernx.log";
        auto sz = fs::file_size(logFile, ec);
        std::ofstream out(logFile, (sz > 256 * 1024) ? (std::ios::trunc | std::ios::out) : (std::ios::app | std::ios::out));
        if (out) {
            auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            char timeBuf[32];
            std::strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
            out << "[" << timeBuf << "] " << msg << "\n";
        }
    }

    bool init() {
#ifdef __SWITCH__
        Result rc = romfsInit();
        romfsReady_ = R_SUCCEEDED(rc);
        if (R_FAILED(rc)) {
            std::fprintf(stderr, "Warning: romfsInit failed with rc=0x%x\n", rc);
        }
#endif
        std::error_code ec;
        fs::create_directories(root_ / "books", ec);
        fs::create_directories(root_ / "data", ec);
        fs::create_directories(root_ / "fonts", ec);
        fs::create_directories(root_ / "logs", ec);

        logMessage("Starting PaperNX application...");
        debugTouch_ = fs::exists(root_ / "data" / "touch-debug.flag");
        progressStore_.load();
        books_ = scan(root_ / "books");
        for (const auto &[id, p] : progressStore_.all()) {
            auto found = std::find_if(books_.begin(), books_.end(), [&](const Book &b) { return b.id == id; });
            if (found != books_.end()) {
                if (!p.bookTitle.empty()) found->title = p.bookTitle;
                found->author = p.bookAuthor;
            } else if (!p.bookPath.empty() && books_.size() < 10000) {
                auto bytes = fs::file_size(fs::u8path(p.bookPath), ec);
                if (!ec && detectFormat(fs::u8path(p.bookPath)) != Format::Unsupported && documentId(p.bookPath, bytes) == id) {
                    Book b; b.id = id; b.path = p.bookPath; b.bytes = bytes;
                    b.title = p.bookTitle.empty() ? defaultTitle(fs::u8path(b.path)) : p.bookTitle;
                    b.author = p.bookAuthor; b.format = detectFormat(fs::u8path(b.path));
                    books_.push_back(std::move(b));
                }
                ec.clear();
            }
        }

        SDL_SetHint(SDL_HINT_MOUSE_TOUCH_EVENTS, "0");
        window_ = SDL_CreateWindow("PaperNX", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                   1280, 720, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
        if (!window_) {
            logMessage("Failed to create SDL window");
            return false;
        }

        renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
        if (!renderer_) renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_SOFTWARE);
        if (!renderer_) {
            logMessage("Failed to create SDL renderer");
            return false;
        }

        typography_ = std::make_unique<Typography>(root_);
        if (!typography_->init()) {
            logMessage("Warning: Default typography missing, fallback active");
        }

        // Open first available game controller if connected
        for (int i = 0; i < SDL_NumJoysticks(); ++i) {
            prepareControllerMapping(i);
            if (SDL_IsGameController(i)) {
                controller_ = SDL_GameControllerOpen(i);
                if (controller_) {
                    logMessage("Primary controller attached");
                    break;
                }
            }
        }

        // Start background thumbnail worker thread
        workerRunning_ = true;
        thumbWorker_ = std::thread([this]() {
            thumbWorkerLoop();
        });

        updateVisibleBooks();
        dirty_ = true;
        return true;
    }

    void run() {
        bool running = true;
        while (running) {
#ifdef __SWITCH__
            if (!appletMainLoop()) break;
#endif
            SDL_Event e;
            // Event-driven loop: wait up to 100ms for events, then flush and draw if dirty
            if (SDL_WaitEventTimeout(&e, 100)) {
                handleEvent(e, running);
                while (SDL_PollEvent(&e)) {
                    handleEvent(e, running);
                }
            }

            if (controller_) {
                for (int axis = SDL_CONTROLLER_AXIS_LEFTX; axis <= SDL_CONTROLLER_AXIS_LEFTY; ++axis) {
                    int dx = 0, dy = 0;
                    if (controllerNav_.processAxis(axis, SDL_GameControllerGetAxis(controller_, static_cast<SDL_GameControllerAxis>(axis)), dx, dy))
                        handleDirectional(dx, dy);
                }
            }
            if (progressDirty_ && std::chrono::steady_clock::now() - lastProgressSave_ >
                    std::chrono::seconds(saveRetrySeconds_)) flushProgress();

            // Process any newly rendered background thumbnails
            processCompletedThumbnails();

            if (hintUntil_ && SDL_GetTicks() > hintUntil_) { hintUntil_ = 0; dirty_ = true; }
            if (dirty_ && running) {
                render();
                dirty_ = false;
            }
        }
        closeBook();
        flushProgress();
    }

private:
    friend struct AppTest;
    fs::path root_;
    ProgressStore progressStore_;
    PageCache pageCache_;
    PageCache thumbCache_;
    std::unique_ptr<Typography> typography_;

    bool romfsReady_{false};
    SDL_Window *window_{nullptr};
    SDL_Renderer *renderer_{nullptr};
    SDL_GameController *controller_{nullptr};

    ScreenState screen_{ScreenState::Library};
    ViewMode viewMode_{ViewMode::Grid};
    Category category_{Category::All};
    Sort sort_{Sort::Title};

    std::vector<Book> books_;
    std::vector<Book> visibleBooks_;
    int selectedBookIdx_{0};
    int scrollOffset_{0};
    std::string searchQuery_;
    bool searchActive_{false};

    // Reader state
    PxDocument *doc_{nullptr};
    std::string currentBookId_;
    int currentPage_{0};
    int pageCount_{0};
    ReaderPreferences extraStyle_;
    int currentTheme_{0};
    int currentFontSize_{22};
    int currentLineHeight_{140};
    int currentMargin_{40};
    int currentTextAlign_{0};
    int currentFontFamily_{0}; // 0: Serif, 1: Sans
    float pdfZoom_{1.0f};
    int pdfPanX_{0};
    int pdfPanY_{0};
    bool toolbarVisible_{false};
    bool pdfDragging_{false};
    bool pdfMoved_{false};
    SDL_FingerID pdfFinger_{0};
    float pdfTouchX_{0}, pdfTouchY_{0};
    std::string currentErrorMessage_;

    // Table of contents and bookmarks
    std::vector<PxTocEntry> tocEntries_;
    int tocSelected_{0};
    int tocOffset_{0};
    int bmSelected_{0};
    int bmOffset_{0};

    // Modal navigation
    int aaTab_{0};
    int aaRow_{0}; // 0: Tipografi, 1: Düzen, 2: Temalar, 3: Kontroller

    // File browser
    fs::path browserPath_;
    fs::path browserRoot_;
    std::vector<BrowserEntry> browserEntries_;
    int browserSelected_{0};
    int browserOffset_{0};

    // Gestures and input
    TouchGestureTracker gestureTracker_;
    ControllerNavigator controllerNav_;
    bool dirty_{true};
    bool progressDirty_{false};
    int saveRetrySeconds_{3};
    std::string lastSaveFailure_;
    std::set<SDL_FingerID> activeFingers_;
    bool multiTouch_{false};
    bool pointerActive_{false};
    Uint32 lastFingerTick_{0};
    bool sawFinger_{false};
    float pressX_{0}, pressY_{0};
    bool debugTouch_{false};
    Uint32 hintUntil_{0};
    std::string lastTouchDebug_;
    int colorEditing_{-1};
    std::string colorInput_;
    std::chrono::steady_clock::time_point lastProgressSave_{std::chrono::steady_clock::now()};

    // Background thumbnail worker
    std::thread thumbWorker_;
    std::atomic<bool> workerRunning_{false};
    std::mutex queueMutex_;
    std::condition_variable queueCv_;
    std::deque<Book> thumbQueue_;
    std::set<std::string> queuedThumbIds_;
    std::map<std::string, std::chrono::steady_clock::time_point> failedThumbIds_;

    std::mutex completedMutex_;
    std::vector<LoadedThumb> completedThumbs_;

    // Caller holds queueMutex_. Pending IDs include queued, processing and upload-ready work.
    void failThumbnail(const std::string &id) {
        queuedThumbIds_.erase(id);
        if (failedThumbIds_.size() >= 512) failedThumbIds_.erase(failedThumbIds_.begin());
        failedThumbIds_[id] = std::chrono::steady_clock::now() + std::chrono::seconds(30);
    }

    void thumbWorkerLoop() {
        while (workerRunning_) {
            Book b;
            {
                std::unique_lock<std::mutex> lock(queueMutex_);
                queueCv_.wait(lock, [this]() {
                    return !workerRunning_ || !thumbQueue_.empty();
                });
                if (!workerRunning_) break;
                b = thumbQueue_.front();
                thumbQueue_.pop_front();
            }

            char err[256] = {};
            PxDocument *thumbDoc = px_document_open(b.path.c_str(), 160, 220, 16.0f, "", err, sizeof(err));
            if (!thumbDoc) {
                std::lock_guard<std::mutex> lock(queueMutex_);
                failThumbnail(b.id);
                continue;
            }

            PxImage img{};
            if (px_document_render_thumbnail(thumbDoc, 150, 210, &img, err, sizeof(err)) && img.rgb && img.width > 0 && img.height > 0) {
                LoadedThumb lt;
                lt.id = b.id;
                lt.w = img.width;
                lt.h = img.height;
                lt.stride = img.stride;
                lt.rgb.assign(img.rgb, img.rgb + (size_t)img.height * img.stride);
                px_image_free(&img);

                {
                    std::lock_guard<std::mutex> lock(completedMutex_);
                    completedThumbs_.push_back(std::move(lt));
                }
            } else {
                std::lock_guard<std::mutex> lock(queueMutex_);
                failThumbnail(b.id);
            }
            px_image_free(&img);
            px_document_close(thumbDoc);
        }
    }

    void processCompletedThumbnails() {
        std::vector<LoadedThumb> toUpload;
        {
            std::lock_guard<std::mutex> lock(completedMutex_);
            if (completedThumbs_.empty()) return;
            toUpload.swap(completedThumbs_);
        }
        for (const auto &lt : toUpload) {
            {
                std::lock_guard<std::mutex> lock(queueMutex_);
                queuedThumbIds_.erase(lt.id);
            }
            std::string key = "thumb_" + lt.id;
            SDL_Texture *tex = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STATIC, lt.w, lt.h);
            if (tex) {
                if (SDL_UpdateTexture(tex, nullptr, lt.rgb.data(), lt.stride) == 0)
                    thumbCache_.put(key, tex, lt.w, lt.h);
                else SDL_DestroyTexture(tex);
                dirty_ = true;
            }
        }
    }

    void getWindowSize(int &w, int &h) {
        SDL_GetRendererOutputSize(renderer_, &w, &h);
    }

    ReaderPreferences currentStyle() const {
        auto p = extraStyle_;
        p.theme = currentTheme_; p.fontSize = currentFontSize_; p.fontFamily = currentFontFamily_;
        p.lineHeight = currentLineHeight_; p.margin = currentMargin_; p.textAlign = currentTextAlign_;
        return p;
    }
    void applyStyle(const ReaderPreferences &p) {
        extraStyle_ = p;
        currentTheme_ = p.theme; currentFontSize_ = p.fontSize; currentFontFamily_ = p.fontFamily;
        currentLineHeight_ = p.lineHeight; currentMargin_ = p.margin; currentTextAlign_ = p.textAlign;
    }
    ThemePalette palette() const { return g_palettes[std::clamp(currentTheme_, 0, g_paletteCount - 1)]; }
    std::string buildEpubCss() const { return generateEpubCss(currentStyle()); }
    void storeDefaults() {
        Progress p;
        static_cast<ReaderPreferences &>(p) = currentStyle();
        progressStore_.put("PAPERNX_DEFAULTS", p);
        progressDirty_ = true;
        dirty_ = true;
    }
    void resetBookStyle() {
        applyStyle(progressStore_.get("PAPERNX_DEFAULTS"));
        reflowEpub(currentFontSize_);
    }
    void toggleBookmark() {
        auto p = progressStore_.get(currentBookId_);
        auto it = std::find_if(p.bookmarks.begin(), p.bookmarks.end(), [&](const Bookmark &b) { return resolveBookmark(b) == currentPage_; });
        if (it != p.bookmarks.end()) p.bookmarks.erase(it);
        else p.bookmarks.push_back({currentPage_, "Sayfa " + std::to_string(currentPage_ + 1), std::time(nullptr), px_document_make_bookmark(doc_, currentPage_)});
        progressStore_.put(currentBookId_, p); progressDirty_ = true; dirty_ = true;
    }

    bool commitColor(std::string value, int target) {
        if (!value.empty() && value[0] == '#') value.erase(0, 1);
        if (value.size() != 6 || value.find_first_not_of("0123456789abcdefABCDEF") != std::string::npos) return false;
        const int rgb = int(std::strtol(value.c_str(), nullptr, 16));
        (target ? extraStyle_.backgroundColor : extraStyle_.textColor) = rgb;
        reflowEpub(currentFontSize_); return true;
    }
    void editColor(int target) {
#ifdef __SWITCH__
        SwkbdConfig config;
        if (R_SUCCEEDED(swkbdCreate(&config, 0))) {
            swkbdConfigMakePresetDefault(&config);
            swkbdConfigSetGuideText(&config, "RGB HEX: 6 karakter, örnek ECE0C6");
            swkbdConfigSetStringLenMax(&config, 6);
            char text[32] = {};
            Result rc = swkbdShow(&config, text, sizeof text); swkbdClose(&config);
            if (R_SUCCEEDED(rc) && !commitColor(text, target)) currentErrorMessage_ = "Renk için 6 HEX karakter girin.";
            gestureTracker_.reset(); activeFingers_.clear(); multiTouch_ = false; pdfDragging_ = false;
        }
#else
        colorEditing_ = target; colorInput_.clear(); SDL_StartTextInput();
#endif
        dirty_ = true;
    }

    void toggleSearch() {
#ifdef __SWITCH__
        SwkbdConfig config;
        if (R_SUCCEEDED(swkbdCreate(&config, 0))) {
            swkbdConfigMakePresetDefault(&config);
            swkbdConfigSetGuideText(&config, "Kitap, yazar veya dosya adı ara");
            swkbdConfigSetInitialText(&config, searchQuery_.c_str());
            swkbdConfigSetStringLenMax(&config, 256);
            char text[1024] = {};
            Result result = swkbdShow(&config, text, sizeof text);
            swkbdClose(&config);
            if (R_SUCCEEDED(result)) searchQuery_ = utf8SafeTruncate(text, 256);
            updateVisibleBooks();
        }
#else
        searchActive_ = !searchActive_;
        if (searchActive_) SDL_StartTextInput();
        else { SDL_StopTextInput(); searchQuery_.clear(); updateVisibleBooks(); }
#endif
        dirty_ = true;
    }

    void updateVisibleBooks() {
        visibleBooks_ = searchEx(books_, progressStore_, searchQuery_, sort_, category_);
        if (selectedBookIdx_ >= static_cast<int>(visibleBooks_.size())) {
            selectedBookIdx_ = std::max(0, static_cast<int>(visibleBooks_.size()) - 1);
        }
        scrollOffset_ = 0;
        dirty_ = true;
    }

    void openBook(Book b) {
        closeBook();
        int w = 1280, h = 720;
        getWindowSize(w, h);

        auto p = progressStore_.get(b.id);
        if (!progressStore_.all().count(b.id)) static_cast<ReaderPreferences &>(p) = progressStore_.get("PAPERNX_DEFAULTS");
        extraStyle_ = p;
        currentBookId_ = b.id;
        currentTheme_ = p.theme;
        currentFontSize_ = p.fontSize > 0 ? p.fontSize : 22;
        currentLineHeight_ = p.lineHeight > 0 ? p.lineHeight : 140;
        currentMargin_ = p.margin > 0 ? p.margin : 40;
        currentTextAlign_ = p.textAlign;
        currentFontFamily_ = p.fontFamily;
        pdfZoom_ = 1.0f;
        pdfPanX_ = 0;
        pdfPanY_ = 0;

        char err[256] = {};
        doc_ = px_document_open(b.path.c_str(), w - (currentMargin_ * 2), h - 144,
                                static_cast<float>(currentFontSize_),
                                buildEpubCss().c_str(), err, sizeof(err));

        if (!doc_) {
            currentBookId_.clear();
            currentErrorMessage_ = std::string("Kitap açılamadı: ") + err;
            logMessage(currentErrorMessage_);
            screen_ = ScreenState::Library;
            dirty_ = true;
            return;
        }

        hintUntil_ = p.lastRead == 0 ? SDL_GetTicks() + 8000 : 0;
        // Extract metadata if available
        char title[512] = {}, author[512] = {};
        if (px_document_metadata(doc_, "info:Title", title, sizeof(title)) > 0 && title[0]) {
            for (auto &bk : books_) {
                if (bk.id == b.id) { bk.title = title; break; }
            }
        }
        if (px_document_metadata(doc_, "info:Author", author, sizeof(author)) > 0 && author[0]) {
            for (auto &bk : books_) {
                if (bk.id == b.id) { bk.author = author; break; }
            }
        }

        pageCount_ = px_document_pages(doc_);

        // Restore position: prioritize MuPDF bookmark if valid, fallback to restorePage
        int restored = -1;
        // Only bridge-versioned chapter/fraction marks survive document reopen.
        if (p.muBookmark >= INT64_C(0x4000000000000000))
            restored = px_document_lookup_bookmark(doc_, p.muBookmark);
        if (restored < 0 || restored >= pageCount_) {
            restored = restorePage(p, pageCount_);
        }
        currentPage_ = std::clamp(restored, 0, std::max(0, pageCount_ - 1));

        if (!px_document_is_epub(doc_)) setPdfView(extraStyle_.pdfViewMode);

        // Load TOC
        loadToc();
        saveCurrentPosition();

        screen_ = ScreenState::Reader;
        toolbarVisible_ = false;
        dirty_ = true;
    }

    void closeBook() {
        pageCache_.clear();
        if (doc_) {
            saveCurrentPosition();
            flushProgress();
            px_document_close(doc_);
            doc_ = nullptr;
        }
        tocEntries_.clear();
        updateVisibleBooks();
        screen_ = ScreenState::Library;
        toolbarVisible_ = false;
        dirty_ = true;
    }

    void loadToc() {
        tocEntries_.clear();
        if (!doc_) return;
        PxTocEntry buffer[256];
        int count = px_document_toc(doc_, buffer, 256);
        for (int i = 0; i < count; ++i) {
            tocEntries_.push_back(buffer[i]);
        }
        tocSelected_ = 0;
        tocOffset_ = 0;
    }

    void saveCurrentPosition() {
        if (!doc_ || currentBookId_.empty()) return;
        auto p = progressStore_.get(currentBookId_);
        p.page = currentPage_;
        p.pageCount = pageCount_;
        p.fraction = (pageCount_ > 1) ? static_cast<double>(currentPage_) / (pageCount_ - 1) : 0.0;
        auto book = std::find_if(books_.begin(), books_.end(), [&](const Book &b) { return b.id == currentBookId_; });
        if (book != books_.end()) {
            p.bookPath = book->path; p.bookTitle = book->title; p.bookAuthor = book->author;
        }
        static_cast<ReaderPreferences &>(p) = currentStyle();
        p.fontSize = currentFontSize_;
        p.theme = currentTheme_;
        p.lineHeight = currentLineHeight_;
        p.margin = currentMargin_;
        p.textAlign = currentTextAlign_;
        p.fontFamily = currentFontFamily_;
        p.muBookmark = px_document_make_bookmark(doc_, currentPage_);
        p.lastRead = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();

        int ch = -1, subP = -1;
        if (px_document_get_location(doc_, currentPage_, &ch, &subP)) {
            p.chapter = ch;
        }

        progressStore_.put(currentBookId_, p);
        progressDirty_ = true;
    }

    bool flushProgress() {
        if (!progressDirty_) return true;
        lastProgressSave_ = std::chrono::steady_clock::now();
        if (progressStore_.save()) {
            progressDirty_ = false;
            saveRetrySeconds_ = 3;
            if (!lastSaveFailure_.empty()) { currentErrorMessage_.clear(); dirty_ = true; }
            lastSaveFailure_.clear();
            return true;
        }
        saveRetrySeconds_ = std::min(60, saveRetrySeconds_ * 2);
        const auto &detail = progressStore_.saveDiagnostic();
        if (detail != lastSaveFailure_) {
            logMessage(detail);
            lastSaveFailure_ = detail;
            currentErrorMessage_ = "Kaydedilemedi. SD kartı kontrol edin; kayıt yeniden denenecek.";
            dirty_ = true;
        }
        return false;
    }

    void clampPdfPan() {
        if (!doc_ || px_document_is_epub(doc_)) return;
        float pw = 0, ph = 0;
        if (!px_document_page_size(doc_, currentPage_, &pw, &ph)) return;
        int w, h; getWindowSize(w, h);
        const int vw = std::max(1, w - 80), vh = std::max(1, h - 144);
        const float scale = std::min(vw / pw, vh / ph) * pdfZoom_;
        pdfPanX_ = std::clamp(pdfPanX_, 0, std::max(0, static_cast<int>(std::ceil(pw * scale)) - vw));
        pdfPanY_ = std::clamp(pdfPanY_, 0, std::max(0, static_cast<int>(std::ceil(ph * scale)) - vh));
    }

    void zoomPdf(float zoom, float focalX = .5f, float focalY = .5f) {
        if (!doc_ || px_document_is_epub(doc_)) return;
        const float previous = pdfZoom_;
        pdfZoom_ = std::round(std::clamp(zoom, 0.1f, 5.0f) * 100) / 100;
        int w, h; getWindowSize(w, h);
        pdfPanX_ = int((pdfPanX_ + (w - 80) * focalX) * pdfZoom_ / previous - (w - 80) * focalX);
        pdfPanY_ = int((pdfPanY_ + (h - 144) * focalY) * pdfZoom_ / previous - (h - 144) * focalY);
        clampPdfPan(); pageCache_.clear(); dirty_ = true;
    }
    void setPdfView(int mode) {
        if (!doc_ || px_document_is_epub(doc_)) return;
        float pw, ph; if (!px_document_page_size(doc_, currentPage_, &pw, &ph)) return;
        int w, h; getWindowSize(w, h);
        const float fit = std::min((w - 80) / pw, (h - 144) / ph);
        zoomPdf(mode == 1 ? ((w - 80) / pw) / fit : mode == 2 ? 1.0f / fit : 1.0f);
        pdfPanX_ = pdfPanY_ = 0;
        extraStyle_.pdfViewMode = mode;
        saveCurrentPosition(); dirty_ = true;
    }

    int resolveBookmark(const Bookmark &bm) {
        int resolved = px_document_lookup_bookmark(doc_, bm.muBookmark);
        return std::clamp(resolved >= 0 ? resolved : bm.page, 0, std::max(0, pageCount_ - 1));
    }

    void turnPage(int delta) {
        if (!doc_ || pageCount_ <= 0) return;
        int target = std::clamp(currentPage_ + delta, 0, pageCount_ - 1);
        if (target != currentPage_) {
            currentPage_ = target;
            pdfPanX_ = pdfPanY_ = 0;
            clampPdfPan();
            saveCurrentPosition();
            dirty_ = true;
        }
    }

    void reflowEpub(int newFontSize, int newTheme = -1) {
        if (!doc_) return;
        if (!px_document_is_epub(doc_)) {
            if (newTheme >= 0) { currentTheme_ = newTheme; saveCurrentPosition(); dirty_ = true; }
            return;
        }

        const auto previous = progressStore_.get(currentBookId_);
        int64_t bookmark = px_document_make_bookmark(doc_, currentPage_);

        int w = 1280, h = 720;
        getWindowSize(w, h);
        currentFontSize_ = std::clamp(newFontSize, 14, 44);
        if (newTheme >= 0) currentTheme_ = newTheme;

        char err[256] = {};
        if (px_document_relayout(doc_, w - (currentMargin_ * 2), h - 144,
                                 static_cast<float>(currentFontSize_),
                                 buildEpubCss().c_str(), err, sizeof(err))) {
            pageCache_.clear();
            loadToc();
            pageCount_ = px_document_pages(doc_);
            int restored = px_document_lookup_bookmark(doc_, bookmark);
            if (restored < 0 || restored >= pageCount_) {
                auto p = progressStore_.get(currentBookId_);
                restored = restorePage(p, pageCount_);
            }
            currentPage_ = std::clamp(restored, 0, std::max(0, pageCount_ - 1));
            saveCurrentPosition();
        } else {
            applyStyle(previous);
            currentErrorMessage_ = std::string("Yeniden boyutlandırma hatası: ") + err;
            logMessage(currentErrorMessage_);
        }
        dirty_ = true;
    }

    SDL_Texture *getOrRenderPageTexture(int pageIdx) {
        if (!doc_ || pageIdx < 0 || pageIdx >= pageCount_) return nullptr;

        int w = 1280, h = 720;
        getWindowSize(w, h);

        std::string key;
        if (px_document_is_epub(doc_)) {
            key = PageCache::makeEpubKey(currentBookId_, pageIdx, currentFontSize_, currentTheme_,
                                         currentLineHeight_, currentMargin_, currentTextAlign_, w, h, currentFontFamily_);
        } else {
            key = PageCache::makePdfKey(currentBookId_, pageIdx, pdfZoom_, pdfPanX_, pdfPanY_, w, h) + ":filter:" + std::to_string(extraStyle_.pdfColorMode);
        }

        SDL_Texture *tex = pageCache_.get(key);
        if (tex) return tex;

        PxImage img{};
        char err[256] = {};
        int ok = 0;

        if (px_document_is_epub(doc_)) {
            ok = px_document_render(doc_, pageIdx, w - (currentMargin_ * 2), h - 144, &img, err, sizeof(err));
        } else {
            // PDF: support viewport zoom and panning
            if (pdfZoom_ != 1.0f) {
                ok = px_document_render_view(doc_, pageIdx, pdfZoom_, pdfPanX_, pdfPanY_,
                                             w - 80, h - 144, &img, err, sizeof(err));
            } else {
                ok = px_document_render(doc_, pageIdx, w - 80, h - 144, &img, err, sizeof(err));
            }
        }

        if (ok && img.rgb) {
            if (!px_document_is_epub(doc_)) px_image_filter(&img, extraStyle_.pdfColorMode);
            tex = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STATIC, img.width, img.height);
            if (tex) {
                if (SDL_UpdateTexture(tex, nullptr, img.rgb, img.stride) != 0) {
                    SDL_DestroyTexture(tex); tex = nullptr;
                } else if (!pageCache_.put(key, tex, img.width, img.height)) tex = nullptr;
            }
            px_image_free(&img);
        }
        return tex;
    }

    SDL_Texture *getOrRenderThumbnail(const Book &b) {
        std::string key = "thumb_" + b.id;
        SDL_Texture *tex = thumbCache_.get(key);
        if (tex) return tex;

        // If not already in cache and not failed, enqueue for asynchronous background loading
        std::lock_guard<std::mutex> lock(queueMutex_);
        auto failed = failedThumbIds_.find(b.id);
        if (failed != failedThumbIds_.end() && failed->second <= std::chrono::steady_clock::now())
            failedThumbIds_.erase(failed);
        if (failedThumbIds_.find(b.id) == failedThumbIds_.end() && queuedThumbIds_.size() < 32 &&
            queuedThumbIds_.find(b.id) == queuedThumbIds_.end()) {
            queuedThumbIds_.insert(b.id);
            thumbQueue_.push_back(b);
            queueCv_.notify_one();
        }
        return nullptr;
    }

    void scanBrowserDirectory() {
        browserEntries_.clear();
        std::error_code ec;
        if (!fs::exists(browserPath_, ec)) return;

        fs::directory_iterator it(browserPath_, fs::directory_options::skip_permission_denied, ec), end;
        for (; !ec && it != end && browserEntries_.size() < 10000; it.increment(ec)) {
            const auto &entry = *it;
            if (entry.is_symlink(ec)) continue;
            if (entry.is_directory(ec)) {
                browserEntries_.push_back({entry.path(), true, 0});
            } else if (entry.is_regular_file(ec)) {
                auto fmt = detectFormat(entry.path().string());
                if (fmt == Format::Epub || fmt == Format::Pdf) {
                    browserEntries_.push_back({entry.path(), false, entry.file_size(ec)});
                }
            }
        }
        std::sort(browserEntries_.begin(), browserEntries_.end(), [](const BrowserEntry &a, const BrowserEntry &b) {
            if (a.isDirectory != b.isDirectory) return a.isDirectory > b.isDirectory;
            return a.path.filename() < b.path.filename();
        });
        browserSelected_ = 0;
        browserOffset_ = 0;
        dirty_ = true;
    }

    void openBrowser() {
#ifdef __SWITCH__
        browserRoot_ = "sdmc:/";
#else
        browserRoot_ = root_;
#endif
        browserPath_ = browserRoot_;
        scanBrowserDirectory();
        screen_ = ScreenState::FileBrowser;
        dirty_ = true;
    }

    void handleBrowserSelect() {
        if (browserSelected_ < 0 || browserSelected_ >= static_cast<int>(browserEntries_.size())) return;
        const auto &entry = browserEntries_[browserSelected_];
        if (entry.isDirectory) {
            browserPath_ = entry.path;
            scanBrowserDirectory();
        } else {
            // Open selected book directly
            Book b;
            b.path = entry.path.string();
            b.format = detectFormat(b.path);
            b.id = documentId(b.path, entry.size);
            b.title = entry.path.stem().string();
            b.bytes = entry.size;
            auto found = std::find_if(books_.begin(), books_.end(), [&](const Book &book) { return book.id == b.id; });
            if (found == books_.end()) books_.push_back(b);
            openBook(b);
        }
    }

    // Input & Event Handling
    void handleEvent(const SDL_Event &e, bool &running) {
        if (e.type == SDL_QUIT) {
            running = false;
            return;
        }
        if (colorEditing_ >= 0) {
            if (e.type == SDL_TEXTINPUT) { if (colorInput_.size() < 6) colorInput_ += e.text.text; dirty_ = true; return; }
            if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_BACKSPACE && !colorInput_.empty()) colorInput_.pop_back();
                if (e.key.keysym.sym == SDLK_RETURN && commitColor(colorInput_, colorEditing_)) { colorEditing_ = -1; SDL_StopTextInput(); }
                if (e.key.keysym.sym == SDLK_ESCAPE) { colorEditing_ = -1; SDL_StopTextInput(); }
                dirty_ = true; return;
            }
            if (e.type == SDL_MOUSEBUTTONDOWN || e.type == SDL_MOUSEBUTTONUP || e.type == SDL_FINGERDOWN || e.type == SDL_FINGERUP) return;
        }
        if (e.type == SDL_TEXTINPUT && searchActive_) {
            if (searchQuery_.size() + std::char_traits<char>::length(e.text.text) <= 256)
                searchQuery_ += e.text.text;
            updateVisibleBooks();
            return;
        }

        // Touch Input
        if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
            pageCache_.clear();
            if (doc_ && px_document_is_epub(doc_)) reflowEpub(currentFontSize_);
            else if (doc_) setPdfView(extraStyle_.pdfViewMode);
            dirty_ = true;
        }
        if (e.type == SDL_APP_WILLENTERBACKGROUND) {
            saveCurrentPosition();
            flushProgress();
            gestureTracker_.reset(); activeFingers_.clear(); multiTouch_ = false; pointerActive_ = pdfDragging_ = false;
        }
        if (e.type == SDL_APP_DIDENTERFOREGROUND) { gestureTracker_.reset(); activeFingers_.clear(); multiTouch_ = false; pointerActive_ = pdfDragging_ = false; pageCache_.clear(); dirty_ = true; }
        if (e.type == SDL_MULTIGESTURE && screen_ == ScreenState::Reader && e.mgesture.numFingers >= 2) {
            zoomPdf(pdfZoom_ * (1.0f + e.mgesture.dDist * 4), e.mgesture.x, e.mgesture.y);
            pdfMoved_ = true;
            gestureTracker_.reset();
            return;
        }
        if (e.type == SDL_FINGERDOWN || e.type == SDL_FINGERMOTION || e.type == SDL_FINGERUP) {
            if (e.tfinger.touchId != -42) { lastFingerTick_ = SDL_GetTicks(); sawFinger_ = true; }
            if (debugTouch_) logMessage("touch type=" + std::to_string(e.type) + " finger=" + std::to_string(e.tfinger.fingerId) +
                " x=" + std::to_string(e.tfinger.x) + " y=" + std::to_string(e.tfinger.y));
        }
        if (e.type == SDL_FINGERDOWN) {
            if (activeFingers_.empty()) { gestureTracker_.reset(); pdfDragging_ = false; multiTouch_ = false; }
            activeFingers_.insert(e.tfinger.fingerId);
            if (activeFingers_.size() > 1) { multiTouch_ = true; gestureTracker_.reset(); pdfDragging_ = false; return; }
            pressX_ = e.tfinger.x; pressY_ = e.tfinger.y;
            if (screen_ == ScreenState::Reader && !px_document_is_epub(doc_) && pdfZoom_ > 1.0f && !toolbarVisible_) {
                if (!pdfDragging_) {
                    pdfDragging_ = true; pdfMoved_ = false; pdfFinger_ = e.tfinger.fingerId;
                    pdfTouchX_ = e.tfinger.x; pdfTouchY_ = e.tfinger.y;
                }
                return;
            }
            gestureTracker_.onFingerDown(e.tfinger.x, e.tfinger.y, e.tfinger.fingerId);
            return;
        }
        if (e.type == SDL_FINGERMOTION) {
            if (multiTouch_) return;
#ifdef __SWITCH__
            if (activeFingers_.size() == 1 && !activeFingers_.count(e.tfinger.fingerId)) {
                SDL_Event corrected = e; corrected.tfinger.fingerId = *activeFingers_.begin();
                handleEvent(corrected, running); return;
            }
#endif
            if (pdfDragging_ && e.tfinger.fingerId == pdfFinger_) {
                int w, h; getWindowSize(w, h);
                pdfPanX_ -= static_cast<int>((e.tfinger.x - pdfTouchX_) * w);
                pdfPanY_ -= static_cast<int>((e.tfinger.y - pdfTouchY_) * h);
                pdfTouchX_ = e.tfinger.x; pdfTouchY_ = e.tfinger.y;
                if (std::hypot(e.tfinger.x - pressX_, e.tfinger.y - pressY_) > .015f) pdfMoved_ = true;
                clampPdfPan(); dirty_ = true;
                return;
            }
            gestureTracker_.onFingerMotion(e.tfinger.x, e.tfinger.y, e.tfinger.fingerId);
            return;
        }
        if (e.type == SDL_FINGERUP) {
            activeFingers_.erase(e.tfinger.fingerId);
            if (multiTouch_) { if (activeFingers_.empty()) multiTouch_ = false; return; }
            if (pdfDragging_) {
                if (e.tfinger.fingerId == pdfFinger_) {
                    pdfDragging_ = false;
                    if (!pdfMoved_) handleTap(e.tfinger.x, e.tfinger.y);
                }
                return;
            }
            auto res = gestureTracker_.onFingerUp(e.tfinger.x, e.tfinger.y, e.tfinger.fingerId);
            handleGesture(res);
            return;
        }

        if (e.type == SDL_MOUSEBUTTONDOWN || e.type == SDL_MOUSEBUTTONUP || e.type == SDL_MOUSEMOTION) {
            if ((sawFinger_ && SDL_GetTicks() - lastFingerTick_ < 750)) return;
            const bool motion = e.type == SDL_MOUSEMOTION;
            if (!motion && e.button.button != SDL_BUTTON_LEFT) return;
            if (motion && !pointerActive_) return;
            if (!motion && e.type == SDL_MOUSEBUTTONDOWN) pointerActive_ = true;
            if (!motion && e.type == SDL_MOUSEBUTTONUP && !pointerActive_) return;
            int w, h; SDL_GetWindowSize(window_, &w, &h);
            SDL_Event finger{};
            finger.type = motion ? SDL_FINGERMOTION : e.type == SDL_MOUSEBUTTONDOWN ? SDL_FINGERDOWN : SDL_FINGERUP;
            finger.tfinger.touchId = -42; // Internal mouse fallback; do not mark as native finger input.
            finger.tfinger.fingerId = -42;
            finger.tfinger.x = float(motion ? e.motion.x : e.button.x) / std::max(1, w);
            finger.tfinger.y = float(motion ? e.motion.y : e.button.y) / std::max(1, h);
            handleEvent(finger, running);
            if (!motion && e.type == SDL_MOUSEBUTTONUP) pointerActive_ = false;
            return;
        }

        // Controller Hotplugging
        if (e.type == SDL_CONTROLLERDEVICEADDED) {
            prepareControllerMapping(e.cdevice.which);
            if (!controller_) {
                controller_ = SDL_GameControllerOpen(e.cdevice.which);
                if (controller_) logMessage("Controller attached dynamically");
            }
            return;
        }
        if (e.type == SDL_CONTROLLERDEVICEREMOVED) {
            if (controller_ && SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(controller_)) == e.cdevice.which) {
                SDL_GameControllerClose(controller_);
                controller_ = nullptr;
                logMessage("Controller detached dynamically");
                for (int i = 0; i < SDL_NumJoysticks(); ++i) {
                    prepareControllerMapping(i);
                    if (SDL_IsGameController(i)) {
                        controller_ = SDL_GameControllerOpen(i);
                        if (controller_) {
                            logMessage("Switched to secondary controller");
                            break;
                        }
                    }
                }
            }
            return;
        }

        // Controller Axis Navigation
        if (e.type == SDL_CONTROLLERAXISMOTION) {
            if (screen_ == ScreenState::Reader && !px_document_is_epub(doc_)) {
                if (e.caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERLEFT && e.caxis.value > 24000) { zoomPdf(pdfZoom_ - 0.25f); return; }
                if (e.caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERRIGHT && e.caxis.value > 24000) { zoomPdf(pdfZoom_ + 0.25f); return; }
            }
            int dx = 0, dy = 0;
            if (controllerNav_.processAxis(e.caxis.axis, e.caxis.value, dx, dy)) {
                handleDirectional(dx, dy);
            }
            return;
        }

        // Controller & Keyboard Keydown
        if (e.type == SDL_KEYDOWN || e.type == SDL_CONTROLLERBUTTONDOWN) {
            handleButtons(e, running);
        }
    }

    void handleGesture(const GestureResult &res) {
        if (debugTouch_) {
            logMessage("touch action=" + std::to_string(static_cast<int>(res.type)));
            lastTouchDebug_ = "Touch action=" + std::to_string(static_cast<int>(res.type)) +
                " x=" + std::to_string(res.tapX) + " y=" + std::to_string(res.tapY);
        }
        if (screen_ == ScreenState::Reader) {
            if (toolbarVisible_ && (pressY_ < 0.1f || pressY_ > 0.9f) &&
                (res.type == GestureType::SwipeLeft || res.type == GestureType::SwipeRight)) return;
            if (res.type == GestureType::SwipeLeft) {
                toolbarVisible_ = false;
                turnPage(1);
            } else if (res.type == GestureType::SwipeRight) {
                toolbarVisible_ = false;
                turnPage(-1);
            } else if (res.type == GestureType::TapLeft || res.type == GestureType::TapRight || res.type == GestureType::TapCenter) {
                handleTap(res.tapX, res.tapY);
            }
        } else if (res.type == GestureType::TapLeft || res.type == GestureType::TapRight || res.type == GestureType::TapCenter) {
            handleTap(res.tapX, res.tapY);
        } else if (res.type == GestureType::SwipeUp || res.type == GestureType::SwipeDown) {
            handleDirectional(0, res.type == GestureType::SwipeUp ? 1 : -1);
        }
    }

    void handleTap(float x, float y) {
        int w = 1280, h = 720;
        getWindowSize(w, h);
        int px = static_cast<int>(x * w);
        int py = static_cast<int>(y * h);

        // Dismiss any active error banner on tap
        if (!currentErrorMessage_.empty()) {
            currentErrorMessage_.clear();
            dirty_ = true;
        }

        if (screen_ == ScreenState::Reader) {
            if (toolbarVisible_) {
                // Header buttons
                if (py < 65) {
                    if (px < 120) { closeBook(); return; } // Geri -> Kitaplık
                    if (px > w - 85) { screen_ = ScreenState::AaModal; dirty_ = true; return; } // Aa
                    if (px > w - 185) { screen_ = ScreenState::BookmarksModal; dirty_ = true; return; } // Yer İmleri
                    if (px > w - 285) { screen_ = ScreenState::TocModal; dirty_ = true; return; } // İçindekiler
                    return;
                }
                // Footer scrubber
                if (py > h - 65) {
                    if (pageCount_ > 1) {
                        float frac = std::clamp(static_cast<float>(px - 180) / std::max(1, w - 300), 0.0f, 1.0f);
                        currentPage_ = static_cast<int>(frac * (pageCount_ - 1));
                        saveCurrentPosition();
                        dirty_ = true;
                    }
                    return;
                }
                // Tap in middle dismisses toolbar without turning page
                toolbarVisible_ = false;
                dirty_ = true;
                return;
            }
            // Standard reading tap zones (when toolbar is not visible)
            if (x < extraStyle_.touchZone / 100.0f) turnPage(-1);
            else if (x > 1.0f - extraStyle_.touchZone / 100.0f) turnPage(1);
            else { toolbarVisible_ = true; dirty_ = true; }
        } else if (screen_ == ScreenState::AaModal) {
            handleAaModalTap(px, py, w, h);
        } else if (screen_ == ScreenState::TocModal) {
            handleTocModalTap(px, py, w, h);
        } else if (screen_ == ScreenState::BookmarksModal) {
            handleBookmarksModalTap(px, py, w, h);
        } else if (screen_ == ScreenState::Library) {
            // Header category buttons
            if (py >= 54 && py <= 98) {
                int catW = std::max(1, (w - 72) / 6);
                int startX = 36;
                for (int c = 0; c <= 5; ++c) {
                    if (px >= startX + c * catW && px < startX + c * catW + catW - 6) {
                        category_ = static_cast<Category>(c);
                        updateVisibleBooks();
                        return;
                    }
                }
            }
            // Search / View / Browse buttons on top right
            if (py < 50) {
                if (px > w - 100) { openBrowser(); return; }
                if (px > w - 190) { viewMode_ = (viewMode_ == ViewMode::Grid) ? ViewMode::List : ViewMode::Grid; dirty_ = true; return; }
                if (px > w - 280) {
                    toggleSearch();
                    return;
                }
            }
            // Card grid click
            if (viewMode_ == ViewMode::Grid) {
                int cardW = 200, spacingX = 28, startX = 36;
                int cols = std::max(1, (w - startX * 2) / (cardW + spacingX));
                int topY = 108;
                int cardH = 260, spacingY = 24;
                if (px >= startX && py >= topY && py < h - 30 && (px - startX) % (cardW + spacingX) < cardW && (py - topY) % (cardH + spacingY) < cardH) {
                    int col = (px - startX) / (cardW + spacingX);
                    int row = (py - topY) / (cardH + spacingY);
                    if (col >= 0 && col < cols && row >= 0) {
                        int idx = (scrollOffset_ + row) * cols + col;
                        if (idx >= 0 && idx < static_cast<int>(visibleBooks_.size())) {
                            selectedBookIdx_ = idx;
                            if (px >= startX + col * (cardW + spacingX) + cardW - 44 &&
                                (py - topY) % (cardH + spacingY) >= cardH - 44) {
                                auto p = progressStore_.get(visibleBooks_[idx].id); p.favorite = !p.favorite;
                                progressStore_.put(visibleBooks_[idx].id, p); progressDirty_ = true; updateVisibleBooks(); return;
                            }
                            openBook(visibleBooks_[idx]);
                        }
                    }
                }
            } else {
                // List view click
                int topY = 108;
                int rowH = 48;
                if (py >= topY) {
                    int idx = scrollOffset_ + (py - topY) / rowH;
                    if (idx >= 0 && idx < static_cast<int>(visibleBooks_.size())) {
                        selectedBookIdx_ = idx;
                        openBook(visibleBooks_[idx]);
                    }
                }
            }
        } else if (screen_ == ScreenState::FileBrowser) {
            if (py < 54) { screen_ = ScreenState::Library; dirty_ = true; return; }
            if (py >= 96) {
                int idx = browserOffset_ + (py - 96) / 44;
                if (idx >= 0 && idx < static_cast<int>(browserEntries_.size())) {
                    browserSelected_ = idx;
                    handleBrowserSelect();
                }
            }
        }
    }

    void handleTocModalTap(int px, int py, int w, int h) {
        int modalW = 600, modalH = 500;
        int mx = (w - modalW) / 2, my = (h - modalH) / 2;

        // Close button top right of modal or tap outside modal
        if (px < mx || px > mx + modalW || py < my || py > my + modalH ||
            (px >= mx + modalW - 45 && py <= my + 45)) {
            screen_ = ScreenState::Reader;
            dirty_ = true;
            return;
        }

        // Row clicks
        int rowH = 40;
        int topY = my + 65;
        if (py >= topY && py <= topY + 9 * rowH) {
            int clickedRow = (py - topY) / rowH;
            int idx = tocOffset_ + clickedRow;
            if (idx >= 0 && idx < static_cast<int>(tocEntries_.size()) && tocEntries_[idx].page >= 0) {
                tocSelected_ = idx;
                currentPage_ = std::clamp(tocEntries_[idx].page, 0, std::max(0, pageCount_ - 1));
                screen_ = ScreenState::Reader;
                toolbarVisible_ = false;
                saveCurrentPosition();
                dirty_ = true;
            }
        }
    }

    void handleBookmarksModalTap(int px, int py, int w, int h) {
        int modalW = 600, modalH = 460;
        int mx = (w - modalW) / 2, my = (h - modalH) / 2;

        // Close button or tap outside modal
        if (px < mx || px > mx + modalW || py < my || py > my + modalH ||
            (px >= mx + modalW - 45 && py <= my + 45)) {
            screen_ = ScreenState::Reader;
            dirty_ = true;
            return;
        }

        if (py >= my + 405 && py <= my + 455) { toggleBookmark(); return; }
        auto p = progressStore_.get(currentBookId_);
        int rowH = 42;
        int topY = my + 65;
        if (py >= topY && py <= topY + 8 * rowH) {
            int clickedRow = (py - topY) / rowH;
            int idx = bmOffset_ + clickedRow;
            if (idx >= 0 && idx < static_cast<int>(p.bookmarks.size())) {
                bmSelected_ = idx;
                currentPage_ = resolveBookmark(p.bookmarks[idx]);
                screen_ = ScreenState::Reader;
                toolbarVisible_ = false;
                saveCurrentPosition();
                dirty_ = true;
            }
        }
    }

    void handleAaModalTap(int px, int py, int w, int h) {
        int modalW = 600, modalH = 460;
        int mx = (w - modalW) / 2, my = (h - modalH) / 2;

        // Close button top right of modal or tap outside modal
        if (px < mx || px > mx + modalW || py < my || py > my + modalH ||
            (px >= mx + modalW - 45 && py <= my + 45)) {
            screen_ = ScreenState::Reader;
            dirty_ = true;
            return;
        }

        if (doc_ && !px_document_is_epub(doc_)) {
            if (py >= my + 110 && py <= my + 155) {
                if (px >= mx + 36 && px <= mx + 146) zoomPdf(pdfZoom_ - 0.25f);
                else if (px >= mx + 170 && px <= mx + 280) zoomPdf(pdfZoom_ + 0.25f);
                else if (px >= mx + 304 && px <= mx + 464) zoomPdf(1.0f);
            }
            if (py >= my + 175 && py <= my + 225) setPdfView(px < mx + 290 ? 1 : 2);
            if (py >= my + 250 && py <= my + 300) {
                extraStyle_.pdfColorMode = (extraStyle_.pdfColorMode + 1) % 4;
                pageCache_.clear(); saveCurrentPosition(); dirty_ = true;
            }
            return;
        }
        // Tabs (0: Tipografi, 1: Düzen, 2: Temalar, 3: Kontroller)
        if (py >= my + 45 && py <= my + 85) {
            int tabW = modalW / 4;
            aaTab_ = std::clamp((px - mx) / tabW, 0, 3);
            dirty_ = true;
            return;
        }

        if (aaTab_ == 0 && py >= my + 315 && py <= my + 365) {
            if (px < mx + 290) storeDefaults(); else resetBookStyle();
            return;
        }
        if (aaTab_ == 1 && py >= my + 285 && py <= my + 335) {
            extraStyle_.paragraphSpacing = (extraStyle_.paragraphSpacing + 40) % 201;
            reflowEpub(currentFontSize_); return;
        }
        if (aaTab_ == 1 && py >= my + 345 && py <= my + 395) {
            extraStyle_.indentation = extraStyle_.indentation ? 0 : 120;
            reflowEpub(currentFontSize_); return;
        }
        if (aaTab_ == 3 && py >= my + 310 && py <= my + 360) {
            extraStyle_.touchZone = extraStyle_.touchZone == 40 ? 20 : extraStyle_.touchZone + 10;
            saveCurrentPosition(); dirty_ = true; return;
        }
        if (aaTab_ == 3 && py >= my + 365 && py <= my + 410) {
            debugTouch_ = !debugTouch_; dirty_ = true; return;
        }
        if (aaTab_ == 0 && py >= my + 245 && py <= my + 295) {
            extraStyle_.publisherStyles = !extraStyle_.publisherStyles;
            reflowEpub(currentFontSize_); return;
        }
        // Tab 0: Tipografi (Font Size + Font Family)
        if (aaTab_ == 0) {
            if (py >= my + 115 && py <= my + 165) {
                if (px >= mx + 190 && px <= mx + 245) { reflowEpub(currentFontSize_ - 2); return; }
                if (px >= mx + 345 && px <= mx + 400) { reflowEpub(currentFontSize_ + 2); return; }
            }
            if (py >= my + 180 && py <= my + 230) {
                if (px >= mx + 190 && px <= mx + 400) {
                    currentFontFamily_ = (currentFontFamily_ + 1) % 3;
                    reflowEpub(currentFontSize_);
                    return;
                }
            }
        }
        // Tab 1: Düzen (Satır Aralığı + Kenar Boşluğu + Hizalama)
        else if (aaTab_ == 1) {
            if (py >= my + 110 && py <= my + 155) {
                if (px >= mx + 180 && px <= mx + 240) { currentLineHeight_ = 120; reflowEpub(currentFontSize_); return; }
                if (px >= mx + 250 && px <= mx + 310) { currentLineHeight_ = 140; reflowEpub(currentFontSize_); return; }
                if (px >= mx + 320 && px <= mx + 380) { currentLineHeight_ = 160; reflowEpub(currentFontSize_); return; }
            }
            if (py >= my + 170 && py <= my + 215) {
                if (px >= mx + 180 && px <= mx + 240) { currentMargin_ = 20; reflowEpub(currentFontSize_); return; }
                if (px >= mx + 250 && px <= mx + 310) { currentMargin_ = 40; reflowEpub(currentFontSize_); return; }
                if (px >= mx + 320 && px <= mx + 380) { currentMargin_ = 60; reflowEpub(currentFontSize_); return; }
            }
            if (py >= my + 230 && py <= my + 275) {
                if (px >= mx + 180 && px <= mx + 275) { currentTextAlign_ = 0; reflowEpub(currentFontSize_); return; }
                if (px >= mx + 285 && px <= mx + 380) { currentTextAlign_ = 1; reflowEpub(currentFontSize_); return; }
            }
        }
        else if (aaTab_ == 2) {
            if (py >= my + 115 && py < my + 245) {
                for (int i = 0; i < 6; ++i) {
                    int sx = mx + 36 + (i % 3) * 180, sy = my + 115 + (i / 3) * 65;
                    if (px >= sx && px < sx + 160 && py >= sy && py < sy + 55) {
                        extraStyle_.textColor = extraStyle_.backgroundColor = -1;
                        reflowEpub(currentFontSize_, i); return;
                    }
                }
            }
            const int colors[] = {0x222222, 0xebe7dc, 0xffffff, 0x000000, 0xece0c6, 0x413224};
            for (int row = 0; row < 2; ++row) {
                if (py >= my + 280 + row * 55 && py < my + 330 + row * 55) {
                    if (px >= mx + 36 && px < mx + 165) { editColor(row); return; }
                    int col = (px - mx - 175) / 62;
                    if (px >= mx + 175 && col >= 0 && col < 6) {
                        (row ? extraStyle_.backgroundColor : extraStyle_.textColor) = colors[col];
                        reflowEpub(currentFontSize_); return;
                    }
                }
            }
            if (py >= my + 390 && py < my + 430) {
                extraStyle_.textColor = extraStyle_.backgroundColor = -1; reflowEpub(currentFontSize_);
            }
        }
    }

    void handleDirectional(int dx, int dy) {
        if (screen_ == ScreenState::Reader) {
            if (!px_document_is_epub(doc_) && pdfZoom_ > 1.0f) {
                // PDF Pan
                pdfPanX_ += dx * 40;
                pdfPanY_ += dy * 40;
                clampPdfPan();
                dirty_ = true;
            } else {
                if (dx > 0) turnPage(1);
                else if (dx < 0) turnPage(-1);
            }
        } else if (screen_ == ScreenState::TocModal) {
            if (dy != 0 && !tocEntries_.empty()) {
                int total = static_cast<int>(tocEntries_.size());
                tocSelected_ = std::clamp(tocSelected_ + dy, 0, total - 1);
                if (tocSelected_ < tocOffset_) tocOffset_ = tocSelected_;
                if (tocSelected_ >= tocOffset_ + 9) tocOffset_ = tocSelected_ - 8;
                dirty_ = true;
            }
        } else if (screen_ == ScreenState::BookmarksModal) {
            auto p = progressStore_.get(currentBookId_);
            if (dy != 0 && !p.bookmarks.empty()) {
                int total = static_cast<int>(p.bookmarks.size());
                bmSelected_ = std::clamp(bmSelected_ + dy, 0, total - 1);
                if (bmSelected_ < bmOffset_) bmOffset_ = bmSelected_;
                if (bmSelected_ >= bmOffset_ + 8) bmOffset_ = bmSelected_ - 7;
                dirty_ = true;
            }
        } else if (screen_ == ScreenState::AaModal) {
            if (doc_ && !px_document_is_epub(doc_)) {
                if (dx) zoomPdf(pdfZoom_ + dx * 0.25f);
                if (dy) { extraStyle_.pdfColorMode = (extraStyle_.pdfColorMode + (dy > 0 ? 1 : 3)) % 4; pageCache_.clear(); saveCurrentPosition(); dirty_ = true; }
                return;
            }
            if (dx != 0) {
                aaTab_ = std::clamp(aaTab_ + dx, 0, 3);
                aaRow_ = 0;
                dirty_ = true;
            } else if (dy != 0) {
                if (aaTab_ == 0) {
                    if (aaRow_ == 0) reflowEpub(currentFontSize_ + (dy < 0 ? 2 : -2));
                    else if (aaRow_ == 1) { currentFontFamily_ = (currentFontFamily_ + 1) % 3; reflowEpub(currentFontSize_); }
                    else if (aaRow_ == 2) { extraStyle_.publisherStyles = !extraStyle_.publisherStyles; reflowEpub(currentFontSize_); }
                    else if (aaRow_ == 3) storeDefaults(); else resetBookStyle();
                } else if (aaTab_ == 1) {
                    if (aaRow_ == 0) currentLineHeight_ = std::clamp(currentLineHeight_ + (dy < 0 ? 20 : -20), 100, 220);
                    else if (aaRow_ == 1) currentMargin_ = std::clamp(currentMargin_ + (dy < 0 ? 20 : -20), 10, 120);
                    else if (aaRow_ == 2) currentTextAlign_ = 1 - currentTextAlign_;
                    else if (aaRow_ == 3) extraStyle_.paragraphSpacing = std::clamp(extraStyle_.paragraphSpacing + (dy > 0 ? -20 : 20), 0, 200);
                    else extraStyle_.indentation = extraStyle_.indentation ? 0 : 120;
                    reflowEpub(currentFontSize_);
                } else if (aaTab_ == 2) {
                    if (aaRow_ == 0) {
                        extraStyle_.textColor = extraStyle_.backgroundColor = -1;
                        int nextTheme = std::clamp(currentTheme_ + (dy > 0 ? 1 : -1), 0, g_paletteCount - 1);
                        reflowEpub(currentFontSize_, nextTheme);
                    } else if (aaRow_ < 3) editColor(aaRow_ - 1);
                    else { extraStyle_.textColor = extraStyle_.backgroundColor = -1; reflowEpub(currentFontSize_); }
                } else if (aaTab_ == 3) {
                    if (aaRow_ == 0) { extraStyle_.touchZone = extraStyle_.touchZone == 40 ? 20 : extraStyle_.touchZone + 10; saveCurrentPosition(); }
                    else debugTouch_ = !debugTouch_;
                    dirty_ = true;
                }
            }
        } else if (screen_ == ScreenState::Library) {
            int w = 1280, h = 720;
            getWindowSize(w, h);
            int cardW = 200, spacingX = 28, startX = 36;
            int cols = (viewMode_ == ViewMode::Grid) ? std::max(1, (w - startX * 2) / (cardW + spacingX)) : 1;
            int total = static_cast<int>(visibleBooks_.size());
            if (total == 0) return;

            int target = selectedBookIdx_ + dx + dy * cols;
            selectedBookIdx_ = std::clamp(target, 0, total - 1);

            // Ensure focused card or row is scrolled into view
            if (viewMode_ == ViewMode::Grid) {
                int topY = 108, cardH = 260, spacingY = 24;
                int maxVisibleRows = std::max(1, (h - topY - 40) / (cardH + spacingY));
                int selectedRow = selectedBookIdx_ / cols;
                if (selectedRow < scrollOffset_) {
                    scrollOffset_ = selectedRow;
                } else if (selectedRow >= scrollOffset_ + maxVisibleRows) {
                    scrollOffset_ = selectedRow - maxVisibleRows + 1;
                }
            } else {
                int rowH = 48, topY = 108;
                int maxRows = std::max(1, (h - topY - 40) / rowH);
                if (selectedBookIdx_ < scrollOffset_) {
                    scrollOffset_ = selectedBookIdx_;
                } else if (selectedBookIdx_ >= scrollOffset_ + maxRows) {
                    scrollOffset_ = selectedBookIdx_ - maxRows + 1;
                }
            }
            dirty_ = true;
        } else if (screen_ == ScreenState::FileBrowser) {
            int total = static_cast<int>(browserEntries_.size());
            if (total == 0) return;
            browserSelected_ = std::clamp(browserSelected_ + dy, 0, total - 1);
            if (browserSelected_ < browserOffset_) browserOffset_ = browserSelected_;
            if (browserSelected_ >= browserOffset_ + 10) browserOffset_ = browserSelected_ - 9;
            dirty_ = true;
        }
    }

    void handleButtons(const SDL_Event &e, bool &running) {
        bool pad = (e.type == SDL_CONTROLLERBUTTONDOWN);
        int key = pad ? e.cbutton.button : e.key.keysym.sym;
        auto match = [&](int k, int p) { return pad ? (key == p) : (key == k); };
        if (!pad && searchActive_ && key != SDLK_ESCAPE) {
            if (key == SDLK_BACKSPACE && !searchQuery_.empty()) {
                std::size_t pos = searchQuery_.size() - 1;
                while (pos > 0 && (static_cast<unsigned char>(searchQuery_[pos]) & 0xc0) == 0x80) --pos;
                searchQuery_.resize(pos);
                updateVisibleBooks();
            }
            return;
        }

        if (match(SDLK_ESCAPE, SDL_CONTROLLER_BUTTON_B)) {
            if (screen_ == ScreenState::AaModal || screen_ == ScreenState::TocModal || screen_ == ScreenState::BookmarksModal) {
                screen_ = ScreenState::Reader;
            } else if (screen_ == ScreenState::FileBrowser) {
                if (browserPath_ == browserRoot_) screen_ = ScreenState::Library;
                else { browserPath_ = browserPath_.parent_path(); scanBrowserDirectory(); }
            } else if (searchActive_) {
                searchActive_ = false;
                SDL_StopTextInput();
                searchQuery_.clear();
                updateVisibleBooks();
            } else if (screen_ == ScreenState::Reader) {
                closeBook();
            } else {
                running = false;
            }
            dirty_ = true;
            return;
        }

        if (match(SDLK_RETURN, SDL_CONTROLLER_BUTTON_A)) {
            if (screen_ == ScreenState::TocModal) {
                if (!tocEntries_.empty() && tocSelected_ >= 0 && tocSelected_ < static_cast<int>(tocEntries_.size()) && tocEntries_[tocSelected_].page >= 0) {
                    currentPage_ = std::clamp(tocEntries_[tocSelected_].page, 0, std::max(0, pageCount_ - 1));
                    screen_ = ScreenState::Reader;
                    toolbarVisible_ = false;
                    saveCurrentPosition();
                    dirty_ = true;
                }
                return;
            } else if (screen_ == ScreenState::BookmarksModal) {
                auto p = progressStore_.get(currentBookId_);
                if (!p.bookmarks.empty() && bmSelected_ >= 0 && bmSelected_ < static_cast<int>(p.bookmarks.size())) {
                    currentPage_ = resolveBookmark(p.bookmarks[bmSelected_]);
                    screen_ = ScreenState::Reader;
                    toolbarVisible_ = false;
                    saveCurrentPosition();
                    dirty_ = true;
                }
                return;
            } else if (screen_ == ScreenState::AaModal) {
                screen_ = ScreenState::Reader;
                dirty_ = true;
                return;
            } else if (screen_ == ScreenState::FileBrowser) {
                handleBrowserSelect();
            } else if (screen_ == ScreenState::Library) {
                if (selectedBookIdx_ >= 0 && selectedBookIdx_ < static_cast<int>(visibleBooks_.size())) {
                    openBook(visibleBooks_[selectedBookIdx_]);
                }
            } else if (screen_ == ScreenState::Reader) {
                toolbarVisible_ = !toolbarVisible_;
                dirty_ = true;
            }
            return;
        }

        if (screen_ == ScreenState::Reader) {
            if (match(SDLK_PAGEDOWN, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER)) { turnPage(1); return; }
            if (match(SDLK_PAGEUP, SDL_CONTROLLER_BUTTON_LEFTSHOULDER)) { turnPage(-1); return; }
            if (!px_document_is_epub(doc_)) {
                if (!pad && (key == SDLK_EQUALS || key == SDLK_PLUS)) { zoomPdf(pdfZoom_ + 0.25f); return; }
                if (!pad && key == SDLK_MINUS) { zoomPdf(pdfZoom_ - 0.25f); return; }
                if (match(SDLK_0, SDL_CONTROLLER_BUTTON_RIGHTSTICK)) { zoomPdf(1.0f); return; }
            }
            if (match(SDLK_b, SDL_CONTROLLER_BUTTON_BACK)) { screen_ = ScreenState::BookmarksModal; dirty_ = true; return; }
        }
        if (screen_ == ScreenState::AaModal && match(SDLK_y, SDL_CONTROLLER_BUTTON_Y)) {
            if (doc_ && !px_document_is_epub(doc_)) { zoomPdf(1.0f); return; }
            aaRow_ = (aaRow_ + 1) % (aaTab_ == 0 ? 5 : aaTab_ == 1 ? 5 : aaTab_ == 2 ? 4 : 2);
            dirty_ = true; return;
        }
        if (screen_ == ScreenState::AaModal && doc_ && !px_document_is_epub(doc_) && match(SDLK_f, SDL_CONTROLLER_BUTTON_X)) {
            setPdfView((extraStyle_.pdfViewMode + 1) % 3); return;
        }
        // Page turns
        if (match(SDLK_RIGHT, SDL_CONTROLLER_BUTTON_DPAD_RIGHT) || match(SDLK_PAGEDOWN, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER)) {
            handleDirectional(1, 0);
        } else if (match(SDLK_LEFT, SDL_CONTROLLER_BUTTON_DPAD_LEFT) || match(SDLK_PAGEUP, SDL_CONTROLLER_BUTTON_LEFTSHOULDER)) {
            handleDirectional(-1, 0);
        } else if (match(SDLK_DOWN, SDL_CONTROLLER_BUTTON_DPAD_DOWN)) {
            handleDirectional(0, 1);
        } else if (match(SDLK_UP, SDL_CONTROLLER_BUTTON_DPAD_UP)) {
            handleDirectional(0, -1);
        }

        // Shortcuts
        if (screen_ == ScreenState::Reader) {
            if (match(SDLK_x, SDL_CONTROLLER_BUTTON_X)) { screen_ = ScreenState::TocModal; dirty_ = true; }
            if (match(SDLK_y, SDL_CONTROLLER_BUTTON_Y)) {
                // Toggle bookmark
                auto p = progressStore_.get(currentBookId_);
                auto it = std::find_if(p.bookmarks.begin(), p.bookmarks.end(),
                                       [this](const Bookmark &bm) { return bm.page == currentPage_; });
                if (it != p.bookmarks.end()) {
                    p.bookmarks.erase(it);
                } else {
                    p.bookmarks.push_back({currentPage_, "Sayfa " + std::to_string(currentPage_ + 1),
                                           std::chrono::duration_cast<std::chrono::seconds>(
                                               std::chrono::system_clock::now().time_since_epoch()).count(),
                                           px_document_make_bookmark(doc_, currentPage_)});
                }
                progressStore_.put(currentBookId_, p);
                progressDirty_ = true;
                dirty_ = true;
            }
            if (match(SDLK_PLUS, SDL_CONTROLLER_BUTTON_START)) { screen_ = ScreenState::AaModal; dirty_ = true; }
            if (match(SDLK_EQUALS, -1)) reflowEpub(currentFontSize_ + 2);
            if (match(SDLK_MINUS, -1)) reflowEpub(currentFontSize_ - 2);
        } else if (screen_ == ScreenState::Library) {
            if (match(SDLK_f, SDL_CONTROLLER_BUTTON_LEFTSTICK) && !visibleBooks_.empty()) {
                auto p = progressStore_.get(visibleBooks_[selectedBookIdx_].id);
                p.favorite = !p.favorite;
                progressStore_.put(visibleBooks_[selectedBookIdx_].id, p);
                progressDirty_ = true; updateVisibleBooks();
            }
            if (match(SDLK_PAGEUP, SDL_CONTROLLER_BUTTON_LEFTSHOULDER)) {
                category_ = static_cast<Category>((static_cast<int>(category_) + 5) % 6); updateVisibleBooks();
            }
            if (match(SDLK_PAGEDOWN, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER)) {
                category_ = static_cast<Category>((static_cast<int>(category_) + 1) % 6); updateVisibleBooks();
            }
            if (match(SDLK_x, SDL_CONTROLLER_BUTTON_X)) {
                toggleSearch();
            }
            if (match(SDLK_y, SDL_CONTROLLER_BUTTON_Y)) openBrowser();
            if (match(SDLK_t, -1)) { currentTheme_ = (currentTheme_ + 1) % g_paletteCount; dirty_ = true; }
            if (match(SDLK_MINUS, SDL_CONTROLLER_BUTTON_BACK)) {
                viewMode_ = (viewMode_ == ViewMode::Grid) ? ViewMode::List : ViewMode::Grid;
                dirty_ = true;
            }
        }

        if (!pad && key == SDLK_BACKSPACE && searchActive_ && !searchQuery_.empty()) {
            std::size_t pos = searchQuery_.size() - 1;
            while (pos > 0 && (static_cast<unsigned char>(searchQuery_[pos]) & 0xc0) == 0x80) --pos;
            searchQuery_.resize(pos);
            updateVisibleBooks();
        }
    }

    // Rendering Screens
    void render() {
        int w = 1280, h = 720;
        getWindowSize(w, h);
        const auto &pal = palette();

        SDL_Color canvas = doc_ && px_document_is_epub(doc_) ? readingColor(extraStyle_.backgroundColor, pal.bg) : pal.bg;
        SDL_SetRenderDrawColor(renderer_, canvas.r, canvas.g, canvas.b, 255);
        SDL_RenderClear(renderer_);

        if (screen_ == ScreenState::Library) {
            renderLibraryScreen(w, h, pal);
        } else if (screen_ == ScreenState::Reader) {
            renderReaderScreen(w, h, pal);
        } else if (screen_ == ScreenState::FileBrowser) {
            renderFileBrowserScreen(w, h, pal);
        } else if (screen_ == ScreenState::AaModal || screen_ == ScreenState::TocModal || screen_ == ScreenState::BookmarksModal) {
            // Keep the reader page rendered in the background!
            renderReaderScreen(w, h, pal);

            // Draw dimming backdrop scrim
            SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 150);
            SDL_Rect fullscreen{0, 0, w, h};
            SDL_RenderFillRect(renderer_, &fullscreen);

            // Render modal on top
            if (screen_ == ScreenState::AaModal) {
                renderAaModal(w, h, pal);
            } else if (screen_ == ScreenState::TocModal) {
                renderTocModal(w, h, pal);
            } else if (screen_ == ScreenState::BookmarksModal) {
                renderBookmarksModal(w, h, pal);
            }
        }

        SDL_RenderPresent(renderer_);
    }

    std::string libraryEmptyMessage() const {
        std::error_code ec;
        if (!fs::is_directory(root_ / "books", ec)) return "SD kitap klasörü kullanılamıyor. SD kartı kontrol edin.";
        if (!searchQuery_.empty()) return "Aramanızla eşleşen kitap bulunamadı.";
        if (books_.empty()) return "Kitaplığınızda henüz kitap bulunmuyor.";
        switch (category_) {
            case Category::Completed: return "Henüz tamamladığınız bir kitap yok.";
            case Category::Favorites: return "Henüz favori kitabınız yok.";
            case Category::ContinueReading: return "Devam ettiğiniz bir kitap yok.";
            case Category::Recent: return "Henüz kitap açmadınız.";
            case Category::Unread: return "Okunmamış kitabınız yok.";
            default: return "Bu kategoride kitap bulunamadı.";
        }
    }
    void renderBookTitle(const std::string &title, SDL_Rect rect, SDL_Color color) {
        auto font = typography_->smallFont();
        if (!font) return;
        SDL_Surface *surface = TTF_RenderUTF8_Blended_Wrapped(font, title.c_str(), color, rect.w);
        if (!surface) return;
        SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer_, surface);
        if (texture) {
            SDL_Rect src{0, 0, std::min(rect.w, surface->w), std::min(rect.h, surface->h)};
            SDL_Rect dst{rect.x, rect.y, src.w, src.h};
            SDL_RenderCopy(renderer_, texture, &src, &dst); SDL_DestroyTexture(texture);
        }
        SDL_FreeSurface(surface);
    }

    void renderLibraryScreen(int w, int h, const ThemePalette &pal) {
        // Top Header
        typography_->drawBox(renderer_, {0, 0, w, 52}, pal.header, pal.cardBorder);
        typography_->renderText(renderer_, typography_->titleFont(), "PaperNX", 36, 10, pal.fg);

        std::string modeText = (viewMode_ == ViewMode::Grid) ? "[Izgara]" : "[Liste]";
        typography_->renderText(renderer_, typography_->smallFont(), modeText, w - 190, 16, pal.muted);
        typography_->renderText(renderer_, typography_->smallFont(), "[SD Gözat]", w - 105, 16, pal.accent);

        typography_->renderText(renderer_, typography_->smallFont(), "[Ara]", w - 275, 16, pal.accent);
        if (searchActive_) {
            typography_->renderText(renderer_, typography_->uiFont(), "Ara: " + searchQuery_ + "_", 200, 12, pal.accent, 400);
        }

        // Category bar
        const char *cats[] = {"Tümü", "Okunanlar", "Son okunan", "Favoriler", "Yeni", "Bitenler"};
        int startX = 36;
        for (int i = 0; i < 6; ++i) {
            bool active = (static_cast<int>(category_) == i);
            const int catW = std::max(1, (w - 72) / 6);
            SDL_Rect catRect{startX + i * catW, 54, catW - 6, 44};
            typography_->drawBox(renderer_, catRect, active ? pal.accent : pal.header, pal.cardBorder);
            typography_->renderText(renderer_, typography_->smallFont(), (w < 900 && i == 2 ? "Son" : w < 900 && i == 1 ? "Devam" : cats[i]), catRect.x + 8, catRect.y + 12,
                                   active ? pal.bg : pal.fg, catRect.w - 16);
        }

        // View content
        if (visibleBooks_.empty()) {
            typography_->renderText(renderer_, typography_->uiFont(), libraryEmptyMessage(), 48, 160, pal.fg);
            typography_->renderText(renderer_, typography_->smallFont(),
                                   books_.empty() ? "SD Gözat ile kitap açın veya books/ klasörüne EPUB/PDF ekleyin." : "Başka bir kategori seçebilir veya aramayı temizleyebilirsiniz.",
                                   48, 195, pal.muted);
        } else {
            if (viewMode_ == ViewMode::Grid) {
                renderGrid(w, h, pal);
            } else {
                renderList(w, h, pal);
            }
        }

        // Error message banner if present
        if (!currentErrorMessage_.empty()) {
            typography_->drawBox(renderer_, {36, h - 70, w - 72, 34}, pal.card, pal.accent);
            typography_->renderText(renderer_, typography_->smallFont(), currentErrorMessage_, 48, h - 62, pal.accent);
        }

        // Bottom shortcut guide
        typography_->drawBox(renderer_, {0, h - 30, w, 30}, pal.header, pal.cardBorder);
        typography_->renderText(renderer_, typography_->smallFont(),
                               visibleBooks_.empty() ? "SD Gözat ile kitap ekleyin" : visibleBooks_[selectedBookIdx_].title,
                               36, h - 24, pal.muted, w - 72);
    }

    void renderGrid(int w, int h, const ThemePalette &pal) {
        int cardW = 200, cardH = 260;
        int spacingX = 28, spacingY = 24;
        int topY = 108;
        int startX = 36;
        int cols = std::max(1, (w - startX * 2) / (cardW + spacingX));

        int startIndex = scrollOffset_ * cols;
        int maxVisibleRows = std::max(1, (h - topY - 40) / (cardH + spacingY));

        for (int i = startIndex; i < static_cast<int>(visibleBooks_.size()) && i < startIndex + maxVisibleRows * cols; ++i) {
            int col = (i - startIndex) % cols;
            int row = (i - startIndex) / cols;
            int x = startX + col * (cardW + spacingX);
            int y = topY + row * (cardH + spacingY);

            bool selected = (i == selectedBookIdx_);
            SDL_Rect cardRect{x, y, cardW, cardH};

            typography_->drawBox(renderer_, cardRect, pal.card, selected ? pal.accent : pal.cardBorder);

            // Thumbnail cover or styled book cover placeholder
            SDL_Texture *thumb = getOrRenderThumbnail(visibleBooks_[i]);
            if (thumb) {
                int tw, th; SDL_QueryTexture(thumb, nullptr, nullptr, &tw, &th);
                float scale = std::min(float(cardW - 16) / tw, float(cardH - 90) / th);
                int iw = std::max(1, int(tw * scale)), ih = std::max(1, int(th * scale));
                SDL_Rect imgRect{x + (cardW - iw) / 2, y + 6 + (cardH - 90 - ih) / 2, iw, ih};
                SDL_RenderCopy(renderer_, thumb, nullptr, &imgRect);
            } else {
                SDL_Rect fillRect{x + 4, y + 4, cardW - 8, cardH - 90};
                typography_->drawBox(renderer_, fillRect, pal.header, pal.cardBorder);
                typography_->renderText(renderer_, typography_->smallFont(),
                                       visibleBooks_[i].format == Format::Pdf ? "PDF" : "EPUB",
                                       x + 12, y + 16, pal.muted);
                typography_->renderText(renderer_, typography_->uiFont(), visibleBooks_[i].title,
                                       x + 12, y + 48, pal.fg, cardW - 24);
            }

            // Book info footer
            auto p = progressStore_.get(visibleBooks_[i].id);
            typography_->drawProgressBar(renderer_, x + 4, y + cardH - 82, cardW - 8, 4,
                                        static_cast<float>(p.fraction), pal.cardBorder, pal.accent);

            renderBookTitle(visibleBooks_[i].title, {x + 8, y + cardH - 72, cardW - 16, 44}, pal.fg);
            typography_->renderText(renderer_, typography_->smallFont(), visibleBooks_[i].author,
                x + 8, y + cardH - 25, pal.muted, cardW - 50);
            typography_->renderText(renderer_, typography_->smallFont(), p.favorite ? "F" : "+", x + cardW - 32, y + cardH - 25, pal.accent);
        }
    }

    void renderList(int w, int h, const ThemePalette &pal) {
        int rowH = 48;
        int topY = 108;
        int maxRows = std::max(1, (h - topY - 40) / rowH);

        for (int i = scrollOffset_; i < static_cast<int>(visibleBooks_.size()) && i < scrollOffset_ + maxRows; ++i) {
            int y = topY + (i - scrollOffset_) * rowH;
            bool selected = (i == selectedBookIdx_);

            SDL_Rect rowRect{36, y, w - 72, rowH - 4};
            typography_->drawBox(renderer_, rowRect, selected ? pal.card : pal.bg, selected ? pal.accent : pal.cardBorder);

            std::string fmtBadge = (visibleBooks_[i].format == Format::Pdf) ? "[PDF]" : "[EPUB]";
            typography_->renderText(renderer_, typography_->smallFont(), fmtBadge, 48, y + 12, pal.muted);
            typography_->renderText(renderer_, typography_->uiFont(), visibleBooks_[i].title, 110, y + 10, pal.fg, w - 450);

            auto p = progressStore_.get(visibleBooks_[i].id);
            char pct[32];
            std::snprintf(pct, sizeof(pct), "%d%%", static_cast<int>(p.fraction * 100));
            typography_->renderText(renderer_, typography_->smallFont(), pct, w - 160, y + 12, pal.muted);

            typography_->drawProgressBar(renderer_, w - 100, y + 18, 50, 6, static_cast<float>(p.fraction),
                                        pal.cardBorder, pal.accent);
        }
    }

    void renderReaderScreen(int w, int h, const ThemePalette &pal) {
        SDL_Texture *pageTex = getOrRenderPageTexture(currentPage_);
        if (pageTex) {
            int texW = 0, texH = 0;
            SDL_QueryTexture(pageTex, nullptr, nullptr, &texW, &texH);
            SDL_Rect dst{(w - texW) / 2, (h - texH) / 2, texW, texH};
            SDL_Rect viewport{0, 72, w, std::max(1, h - 144)};
            SDL_RenderSetClipRect(renderer_, &viewport);
            SDL_RenderCopy(renderer_, pageTex, nullptr, &dst);
            SDL_RenderSetClipRect(renderer_, nullptr);
        }

        if (!currentErrorMessage_.empty()) {
            settingsButton({36, h - 120, w - 72, 48}, currentErrorMessage_, pal);
        }
        if (!toolbarVisible_ && hintUntil_) settingsButton({w / 2 - 240, h - 62, 480, 48}, "Ortaya dokun: Araçlar ve Aa ayarları", pal);
        if (debugTouch_) settingsButton({36, 72, w - 72, 48}, lastTouchDebug_, pal);
        // Toolbar Overlays
        if (toolbarVisible_) {
            // Header bar
            typography_->drawBox(renderer_, {0, 0, w, 54}, pal.header, pal.cardBorder);
            typography_->renderText(renderer_, typography_->uiFont(), "< Kitaplık", 24, 14, pal.accent);

            std::string title = currentBookId_;
            for (const auto &b : books_) {
                if (b.id == currentBookId_) { title = b.title; break; }
            }
            typography_->renderText(renderer_, typography_->smallFont(), title, 150, 16, pal.fg, w - 450);

            typography_->renderText(renderer_, typography_->smallFont(), "[İçindekiler]", w - 275, 16, pal.fg);
            typography_->renderText(renderer_, typography_->smallFont(), "[Yer İmi]", w - 175, 16, pal.fg);
            settingsButton({w - 92, 4, 84, 48}, "Aa", pal);

            // Footer bar
            typography_->drawBox(renderer_, {0, h - 56, w, 56}, pal.header, pal.cardBorder);

            char pageBuf[64];
            std::snprintf(pageBuf, sizeof(pageBuf), "Sayfa %d / %d", currentPage_ + 1, std::max(1, pageCount_));
            typography_->renderText(renderer_, typography_->smallFont(), pageBuf, 36, h - 44, pal.fg);

            float prog = (pageCount_ > 1) ? static_cast<float>(currentPage_) / (pageCount_ - 1) : 1.0f;
            typography_->drawProgressBar(renderer_, 180, h - 34, w - 300, 8, prog, pal.cardBorder, pal.accent);
        }
    }

    void renderFileBrowserScreen(int w, int h, const ThemePalette &pal) {
        typography_->drawBox(renderer_, {0, 0, w, 54}, pal.header, pal.cardBorder);
        typography_->renderText(renderer_, typography_->titleFont(), "< Kitaplık — SD Dosya Gezgini", 36, 10, pal.fg);
        typography_->renderText(renderer_, typography_->smallFont(), browserPath_.string(), 36, 62, pal.muted, w - 80);

        int topY = 96;
        int rowH = 44;
        int visible = std::max(1, (h - topY - 30) / rowH);
        for (int i = browserOffset_; i < static_cast<int>(browserEntries_.size()) && i < browserOffset_ + visible; ++i) {
            int y = topY + (i - browserOffset_) * rowH;
            bool sel = (i == browserSelected_);
            SDL_Rect r{36, y, w - 72, rowH - 4};
            typography_->drawBox(renderer_, r, sel ? pal.accent : pal.card, pal.cardBorder);

            std::string prefix = browserEntries_[i].isDirectory ? "[KLASÖR] " : "[KİTAP]  ";
            typography_->renderText(renderer_, typography_->uiFont(),
                                   prefix + browserEntries_[i].path.filename().string(),
                                   48, y + 8, sel ? pal.bg : pal.fg, w - 120);
        }
    }

    void renderAaModal(int w, int h, const ThemePalette &pal) {
        int modalW = 600, modalH = 460;
        int mx = (w - modalW) / 2, my = (h - modalH) / 2;

        typography_->drawBox(renderer_, {mx, my, modalW, modalH}, pal.card, pal.accent);
        if (doc_ && !px_document_is_epub(doc_)) {
            typography_->renderText(renderer_, typography_->titleFont(), "PDF Görünüm", mx + 24, my + 14, pal.fg);
            typography_->renderText(renderer_, typography_->titleFont(), "X", mx + modalW - 36, my + 14, pal.muted);
            settingsButton({mx + 36, my + 110, 110, 48}, "-", pal);
            settingsButton({mx + 170, my + 110, 110, 48}, "+", pal);
            settingsButton({mx + 304, my + 110, 250, 48}, "Sayfaya sığdır", pal);
            settingsButton({mx + 36, my + 175, 245, 50}, "Genişliğe sığdır", pal);
            settingsButton({mx + 301, my + 175, 255, 50}, "100% (72 dpi)", pal);
            const char *modes[] = {"Orijinal", "Koyu", "Sepya", "Ters renkler"};
            settingsButton({mx + 36, my + 250, 520, 50}, std::string("Sayfa filtresi: ") + modes[extraStyle_.pdfColorMode], pal);
            float pw = 1, ph = 1; px_document_page_size(doc_, currentPage_, &pw, &ph);
            int rw, rh; getWindowSize(rw, rh);
            int actualPercent = int(100 * std::min((rw - 80) / pw, (rh - 144) / ph) * pdfZoom_);
            typography_->renderText(renderer_, typography_->smallFont(), "Ölçek: %" + std::to_string(actualPercent) + " (72 dpi)", mx + 36, my + 70, pal.fg);
            typography_->renderText(renderer_, typography_->smallFont(), "Filtreler fotoğraf ve çizimleri de değiştirir.", mx + 36, my + 325, pal.muted);
            typography_->renderText(renderer_, typography_->smallFont(), "Sürükle: kaydır  X: görünüm  Y: sayfaya sığdır", mx + 36, my + 365, pal.muted);
            return;
        }
        typography_->renderText(renderer_, typography_->titleFont(), "Aa Görünüm Ayarları", mx + 24, my + 14, pal.fg);
        typography_->renderText(renderer_, typography_->titleFont(), "X", mx + modalW - 36, my + 14, pal.muted);

        // Tab headers
        const char *tabNames[] = {"Tipografi", "Düzen", "Temalar", "Kontroller"};
        int tabW = modalW / 4;
        for (int t = 0; t < 4; ++t) {
            bool active = (aaTab_ == t);
            SDL_Rect tr{mx + t * tabW, my + 54, tabW, 36};
            typography_->drawBox(renderer_, tr, active ? pal.bg : pal.header, pal.cardBorder);
            typography_->renderText(renderer_, typography_->smallFont(), tabNames[t], tr.x + 14, tr.y + 8,
                                   active ? pal.accent : pal.fg);
        }

        const char *fields[][5] = {{"Boyut", "Yazı tipi", "Yayıncı düzeni", "Varsayılanı kaydet", "Varsayılanı yükle"},
            {"Satır aralığı", "Kenar", "Hizalama", "Paragraf aralığı", "Girinti"},
            {"Tema", "Yazı HEX", "Zemin HEX", "Renkleri sıfırla", ""}, {"Dokunmatik kenar", "Günlük", "", "", ""}};
        if (aaTab_ != 2) typography_->renderText(renderer_, typography_->smallFont(),
            std::string("Y: alan  D-pad: değiştir/sekme  B: kapat | ") + fields[aaTab_][aaRow_],
            mx + 24, my + modalH - 30, pal.muted, modalW - 48);
        // Tab Content
        if (aaTab_ == 0) {
            // Font Size
            typography_->renderText(renderer_, typography_->uiFont(), "Yazı Boyutu", mx + 36, my + 120, pal.fg);
            typography_->drawBox(renderer_, {mx + 190, my + 115, 48, 48}, pal.bg, pal.cardBorder);
            typography_->renderText(renderer_, typography_->uiFont(), "-", mx + 208, my + 118, pal.fg);

            char szStr[16];
            std::snprintf(szStr, sizeof(szStr), "%d pt", currentFontSize_);
            typography_->renderText(renderer_, typography_->uiFont(), szStr, mx + 265, my + 120, pal.fg);

            typography_->drawBox(renderer_, {mx + 345, my + 115, 48, 48}, pal.bg, pal.cardBorder);
            typography_->renderText(renderer_, typography_->uiFont(), "+", mx + 361, my + 118, pal.fg);

            // Font Family
            typography_->renderText(renderer_, typography_->uiFont(), "Yazı Tipi", mx + 36, my + 190, pal.fg);
            SDL_Rect fontToggle{mx + 190, my + 185, 203, 48};
            typography_->drawBox(renderer_, fontToggle, pal.bg, pal.accent);
            std::string fontName = currentFontFamily_ == 2 ? "Yayıncı" : currentFontFamily_ == 0 ? "Serif" : "Sans";
            typography_->renderText(renderer_, typography_->smallFont(), fontName, fontToggle.x + 16, fontToggle.y + 10, pal.fg);
            settingsButton({mx + 36, my + 245, 520, 48}, extraStyle_.publisherStyles ? "Yayıncı düzeni: Açık" : "Yayıncı düzeni: Kapalı", pal);
            settingsButton({mx + 36, my + 315, 245, 50}, "Varsayılan olarak kaydet", pal);
            settingsButton({mx + 301, my + 315, 255, 50}, "Kitabı varsayılana döndür", pal);
            typography_->renderText(renderer_, currentFontFamily_ == 0 ? typography_->readerFont() : typography_->uiFont(),
                "İ ı Ş ş Ğ ğ — kitap sayfaları", mx + 36, my + 380, pal.fg, 520);
        } else if (aaTab_ == 1) {
            // Satır Aralığı
            typography_->renderText(renderer_, typography_->smallFont(), "Satır Aralığı", mx + 36, my + 115, pal.fg);
            int lhVals[] = {120, 140, 160};
            for (int i = 0; i < 3; ++i) {
                bool sel = (currentLineHeight_ == lhVals[i]);
                SDL_Rect r{mx + 180 + i * 70, my + 110, 60, 45};
                typography_->drawBox(renderer_, r, sel ? pal.accent : pal.bg, pal.cardBorder);
                typography_->renderText(renderer_, typography_->smallFont(), std::to_string(lhVals[i]) + "%", r.x + 8, r.y + 6, sel ? pal.bg : pal.fg);
            }

            // Kenar Boşluğu
            typography_->renderText(renderer_, typography_->smallFont(), "Kenar Boşluğu", mx + 36, my + 175, pal.fg);
            int mgVals[] = {20, 40, 60};
            for (int i = 0; i < 3; ++i) {
                bool sel = (currentMargin_ == mgVals[i]);
                SDL_Rect r{mx + 180 + i * 70, my + 170, 60, 45};
                typography_->drawBox(renderer_, r, sel ? pal.accent : pal.bg, pal.cardBorder);
                typography_->renderText(renderer_, typography_->smallFont(), std::to_string(mgVals[i]) + "px", r.x + 8, r.y + 6, sel ? pal.bg : pal.fg);
            }

            // Hizalama
            typography_->renderText(renderer_, typography_->smallFont(), "Hizalama", mx + 36, my + 235, pal.fg);
            const char *alNames[] = {"İki Yana", "Sola"};
            for (int i = 0; i < 2; ++i) {
                bool sel = (currentTextAlign_ == i);
                SDL_Rect r{mx + 180 + i * 105, my + 230, 95, 45};
                typography_->drawBox(renderer_, r, sel ? pal.accent : pal.bg, pal.cardBorder);
                typography_->renderText(renderer_, typography_->smallFont(), alNames[i], r.x + 16, r.y + 6, sel ? pal.bg : pal.fg);
            }
            settingsButton({mx + 36, my + 285, 520, 50}, "Paragraf aralığı: " + std::to_string(extraStyle_.paragraphSpacing) + "%", pal);
            settingsButton({mx + 36, my + 345, 520, 50}, extraStyle_.indentation ? "Paragraf girintisi: Açık" : "Paragraf girintisi: Kapalı", pal);
        } else if (aaTab_ == 2) {
            for (int i = 0; i < 6; ++i) {
                int sx = mx + 36 + (i % 3) * 180, sy = my + 115 + (i / 3) * 65;
                const auto &p = g_palettes[i];
                typography_->drawBox(renderer_, {sx, sy, 160, 55}, p.bg, currentTheme_ == i ? pal.accent : pal.cardBorder);
                typography_->renderText(renderer_, typography_->smallFont(), p.name, sx + 10, sy + 16, p.fg);
            }
            typography_->renderText(renderer_, typography_->smallFont(), "Özel renkler", mx + 36, my + 250, pal.fg);
            const int colors[] = {0x222222, 0xebe7dc, 0xffffff, 0x000000, 0xece0c6, 0x413224};
            for (int row = 0; row < 2; ++row) {
                typography_->renderText(renderer_, typography_->smallFont(), row ? "Zemin HEX" : "Yazı HEX", mx + 36, my + 294 + row * 55, pal.fg);
                for (int c = 0; c < 6; ++c) {
                    auto col = readingColor(colors[c], pal.bg);
                    typography_->drawBox(renderer_, {mx + 175 + c * 62, my + 280 + row * 55, 50, 48}, col, pal.accent);
                }
            }
            if (colorEditing_ >= 0) typography_->renderText(renderer_, typography_->smallFont(),
                "HEX: " + colorInput_ + "_  Enter: uygula   Esc: vazgeç", mx + 36, my + 250, pal.accent);
            auto previewPalette = palette();
            previewPalette.bg = readingColor(extraStyle_.backgroundColor, previewPalette.bg);
            previewPalette.fg = readingColor(extraStyle_.textColor, previewPalette.fg);
            settingsButton({mx + 36, my + 390, 520, 40}, readingContrast(previewPalette.bg, previewPalette.fg) < 4.5 ?
                "Düşük kontrast — renkleri sıfırla" : "Renkleri sıfırla", pal);
        } else if (aaTab_ == 3) {
            typography_->renderText(renderer_, typography_->smallFont(), "Kenar dokunuşu / yatay kaydırma: Sayfa", mx + 36, my + 115, pal.fg);
            typography_->renderText(renderer_, typography_->smallFont(), "Orta dokunuş: Araçlar ve Aa ayarları", mx + 36, my + 160, pal.fg);
            typography_->renderText(renderer_, typography_->smallFont(), "L/R: Sayfa   X: İçindekiler   Y: Yer imi", mx + 36, my + 205, pal.fg);
            settingsButton({mx + 36, my + 310, 520, 50}, "Dokunmatik kenarlar: %" + std::to_string(extraStyle_.touchZone), pal);
            settingsButton({mx + 36, my + 365, 520, 45}, debugTouch_ ? "Dokunmatik günlük: Açık" : "Dokunmatik günlük: Kapalı", pal);
        }
    }

    void settingsButton(SDL_Rect r, const std::string &label, const ThemePalette &pal) {
        typography_->drawBox(renderer_, r, pal.bg, pal.cardBorder);
        typography_->renderText(renderer_, typography_->smallFont(), label, r.x + 12, r.y + 12, pal.fg, r.w - 24);
    }

    void renderTocModal(int w, int h, const ThemePalette &pal) {
        int modalW = 600, modalH = 500;
        int mx = (w - modalW) / 2, my = (h - modalH) / 2;

        typography_->drawBox(renderer_, {mx, my, modalW, modalH}, pal.card, pal.accent);
        typography_->renderText(renderer_, typography_->titleFont(), "İçindekiler", mx + 24, my + 16, pal.fg);
        typography_->renderText(renderer_, typography_->titleFont(), "X", mx + modalW - 36, my + 16, pal.muted);

        if (tocEntries_.empty()) {
            typography_->renderText(renderer_, typography_->smallFont(), "Bu belgede içindekiler tablosu bulunamadı.", mx + 24, my + 80, pal.muted);
            return;
        }

        int rowH = 40;
        for (int i = tocOffset_; i < static_cast<int>(tocEntries_.size()) && i < tocOffset_ + 9; ++i) {
            int y = my + 65 + (i - tocOffset_) * rowH;
            bool sel = (i == tocSelected_);
            SDL_Rect r{mx + 16, y, modalW - 32, rowH - 4};
            typography_->drawBox(renderer_, r, sel ? pal.accent : pal.bg, pal.cardBorder);

            int indent = std::min(40, tocEntries_[i].level * 16);
            typography_->renderText(renderer_, typography_->smallFont(), tocEntries_[i].title,
                                   mx + 28 + indent, y + 8, sel ? pal.bg : pal.fg, modalW - 120 - indent);

            std::string pageNum = std::to_string(tocEntries_[i].page + 1);
            typography_->renderText(renderer_, typography_->smallFont(), pageNum,
                                   mx + modalW - 55, y + 8, sel ? pal.bg : pal.muted);
        }
    }

    void renderBookmarksModal(int w, int h, const ThemePalette &pal) {
        int modalW = 600, modalH = 460;
        int mx = (w - modalW) / 2, my = (h - modalH) / 2;

        typography_->drawBox(renderer_, {mx, my, modalW, modalH}, pal.card, pal.accent);
        typography_->renderText(renderer_, typography_->titleFont(), "Yer İmleri", mx + 24, my + 16, pal.fg);
        typography_->renderText(renderer_, typography_->titleFont(), "X", mx + modalW - 36, my + 16, pal.muted);

        settingsButton({mx + 24, my + 405, 550, 50}, "Bu sayfanın yer imini ekle / kaldır", pal);
        auto p = progressStore_.get(currentBookId_);
        if (p.bookmarks.empty()) {
            typography_->renderText(renderer_, typography_->smallFont(),
                                   "Henüz yer imi eklenmedi. Okurken Y tuşuna basarak ekleyebilirsiniz.",
                                   mx + 24, my + 80, pal.muted);
            return;
        }

        int rowH = 42;
        for (int i = bmOffset_; i < static_cast<int>(p.bookmarks.size()) && i < bmOffset_ + 8; ++i) {
            int y = my + 65 + (i - bmOffset_) * rowH;
            bool sel = (i == bmSelected_);
            SDL_Rect r{mx + 16, y, modalW - 32, rowH - 4};
            typography_->drawBox(renderer_, r, sel ? pal.accent : pal.bg, pal.cardBorder);

            typography_->renderText(renderer_, typography_->smallFont(), p.bookmarks[i].title,
                                   mx + 28, y + 10, sel ? pal.bg : pal.fg, modalW - 60);
        }

    }
};

} // namespace

int main(int argc, char **argv) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | SDL_INIT_EVENTS) != 0) {
        std::fprintf(stderr, "Failed to initialize SDL2: %s\n", SDL_GetError());
        return 1;
    }
    if (TTF_Init() != 0) {
        std::fprintf(stderr, "Failed to initialize SDL2_ttf: %s\n", TTF_GetError());
        SDL_Quit();
        return 1;
    }

#ifdef __SWITCH__
    fs::path root = "sdmc:/switch/PaperNX";
#else
    fs::path root = (argc > 1) ? argv[1] : "./PaperNX-data";
#endif

    int result = 0;
    {
        PaperNxApp app(root);
        if (app.init()) app.run();
        else result = 1;
    }

    TTF_Quit();
    SDL_Quit();
    return result;
}
