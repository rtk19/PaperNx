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
        romfsExit();
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
        progressStore_.load();
        books_ = scan(root_ / "books");

        window_ = SDL_CreateWindow("PaperNX", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                   1280, 720, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
        if (!window_) {
            logMessage("Failed to create SDL window");
            return false;
        }

        renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
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
            SDL_Event e;
            // Event-driven loop: wait up to 100ms for events, then flush and draw if dirty
            if (SDL_WaitEventTimeout(&e, 100)) {
                handleEvent(e, running);
                while (SDL_PollEvent(&e)) {
                    handleEvent(e, running);
                }
            }

            // Periodic auto-save if progress changed
            if (progressDirty_) {
                auto now = std::chrono::steady_clock::now();
                if (now - lastProgressSave_ > std::chrono::seconds(3)) {
                    progressStore_.save();
                    progressDirty_ = false;
                    lastProgressSave_ = now;
                }
            }

            // Process any newly rendered background thumbnails
            processCompletedThumbnails();

            if (dirty_ && running) {
                render();
                dirty_ = false;
            }
        }
        closeBook();
        progressStore_.save();
    }

private:
    fs::path root_;
    ProgressStore progressStore_;
    PageCache pageCache_;
    PageCache thumbCache_;
    std::unique_ptr<Typography> typography_;

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
    std::string currentErrorMessage_;

    // Table of contents and bookmarks
    std::vector<PxTocEntry> tocEntries_;
    int tocSelected_{0};
    int tocOffset_{0};
    int bmSelected_{0};
    int bmOffset_{0};

    // Modal navigation
    int aaTab_{0}; // 0: Tipografi, 1: Düzen, 2: Temalar, 3: Kontroller

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
    std::chrono::steady_clock::time_point lastProgressSave_{std::chrono::steady_clock::now()};

    // Background thumbnail worker
    std::thread thumbWorker_;
    std::atomic<bool> workerRunning_{false};
    std::mutex queueMutex_;
    std::condition_variable queueCv_;
    std::deque<Book> thumbQueue_;
    std::set<std::string> queuedThumbIds_;
    std::set<std::string> failedThumbIds_;

    std::mutex completedMutex_;
    std::vector<LoadedThumb> completedThumbs_;

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
                failedThumbIds_.insert(b.id);
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
                failedThumbIds_.insert(b.id);
            }
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
            std::string key = "thumb_" + lt.id;
            SDL_Texture *tex = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STATIC, lt.w, lt.h);
            if (tex) {
                SDL_UpdateTexture(tex, nullptr, lt.rgb.data(), lt.stride);
                thumbCache_.put(key, tex, lt.w, lt.h);
                dirty_ = true;
            }
        }
    }

    void getWindowSize(int &w, int &h) {
        SDL_GetRendererOutputSize(renderer_, &w, &h);
    }

    const ThemePalette &palette() const {
        int idx = std::clamp(currentTheme_, 0, g_paletteCount - 1);
        return g_palettes[idx];
    }

    std::string buildEpubCss() const {
        return generateEpubCss(currentTheme_, currentFontFamily_, currentFontSize_,
                               currentLineHeight_, currentMargin_, currentTextAlign_);
    }

    void updateVisibleBooks() {
        visibleBooks_ = searchEx(books_, progressStore_, searchQuery_, sort_, category_);
        if (selectedBookIdx_ >= static_cast<int>(visibleBooks_.size())) {
            selectedBookIdx_ = std::max(0, static_cast<int>(visibleBooks_.size()) - 1);
        }
        scrollOffset_ = 0;
        dirty_ = true;
    }

    void openBook(const Book &b) {
        closeBook();
        int w = 1280, h = 720;
        getWindowSize(w, h);

        auto p = progressStore_.get(b.id);
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
        doc_ = px_document_open(b.path.c_str(), w - (currentMargin_ * 2), h - 100,
                                static_cast<float>(currentFontSize_),
                                buildEpubCss().c_str(), err, sizeof(err));

        if (!doc_) {
            currentErrorMessage_ = std::string("Kitap açılamadı: ") + err;
            logMessage(currentErrorMessage_);
            screen_ = ScreenState::Library;
            dirty_ = true;
            return;
        }

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
        if (p.muBookmark != -1) {
            restored = px_document_lookup_bookmark(doc_, p.muBookmark);
        }
        if (restored < 0 || restored >= pageCount_) {
            restored = restorePage(p, pageCount_);
        }
        currentPage_ = std::clamp(restored, 0, std::max(0, pageCount_ - 1));

        // Load TOC
        loadToc();

        screen_ = ScreenState::Reader;
        toolbarVisible_ = false;
        dirty_ = true;
    }

    void closeBook() {
        if (doc_) {
            saveCurrentPosition();
            px_document_close(doc_);
            doc_ = nullptr;
        }
        tocEntries_.clear();
        screen_ = ScreenState::Library;
        toolbarVisible_ = false;
        dirty_ = true;
    }

    void loadToc() {
        tocEntries_.clear();
        if (!doc_) return;
        PxTocEntry buffer[128];
        int count = px_document_toc(doc_, buffer, 128);
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

    void turnPage(int delta) {
        if (!doc_ || pageCount_ <= 0) return;
        int target = std::clamp(currentPage_ + delta, 0, pageCount_ - 1);
        if (target != currentPage_) {
            currentPage_ = target;
            saveCurrentPosition();
            dirty_ = true;
        }
    }

    void reflowEpub(int newFontSize, int newTheme = -1) {
        if (!doc_ || !px_document_is_epub(doc_)) return;

        int64_t bookmark = px_document_make_bookmark(doc_, currentPage_);

        int w = 1280, h = 720;
        getWindowSize(w, h);
        currentFontSize_ = std::clamp(newFontSize, 14, 44);
        if (newTheme >= 0) currentTheme_ = newTheme;

        char err[256] = {};
        if (px_document_relayout(doc_, w - (currentMargin_ * 2), h - 100,
                                 static_cast<float>(currentFontSize_),
                                 buildEpubCss().c_str(), err, sizeof(err))) {
            pageCount_ = px_document_pages(doc_);
            int restored = px_document_lookup_bookmark(doc_, bookmark);
            if (restored < 0 || restored >= pageCount_) {
                auto p = progressStore_.get(currentBookId_);
                restored = restorePage(p, pageCount_);
            }
            currentPage_ = std::clamp(restored, 0, std::max(0, pageCount_ - 1));
            saveCurrentPosition();
        } else {
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
                                         currentLineHeight_, currentMargin_, currentTextAlign_, w, h);
        } else {
            key = PageCache::makePdfKey(currentBookId_, pageIdx, pdfZoom_, pdfPanX_, pdfPanY_, w, h);
        }

        SDL_Texture *tex = pageCache_.get(key);
        if (tex) return tex;

        PxImage img{};
        char err[256] = {};
        int ok = 0;

        if (px_document_is_epub(doc_)) {
            ok = px_document_render(doc_, pageIdx, w - (currentMargin_ * 2), h - 80, &img, err, sizeof(err));
        } else {
            // PDF: support viewport zoom and panning
            if (pdfZoom_ > 1.05f) {
                ok = px_document_render_view(doc_, pageIdx, pdfZoom_, pdfPanX_, pdfPanY_,
                                             w - 80, h - 80, &img, err, sizeof(err));
            } else {
                ok = px_document_render(doc_, pageIdx, w - 80, h - 80, &img, err, sizeof(err));
            }
        }

        if (ok && img.rgb) {
            tex = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STATIC, img.width, img.height);
            if (tex) {
                SDL_UpdateTexture(tex, nullptr, img.rgb, img.stride);
                pageCache_.put(key, tex, img.width, img.height);
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
        if (failedThumbIds_.find(b.id) == failedThumbIds_.end() &&
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

        for (const auto &entry : fs::directory_iterator(browserPath_, ec)) {
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
            b.size = entry.size;
            openBook(b);
        }
    }

    // Input & Event Handling
    void handleEvent(const SDL_Event &e, bool &running) {
        if (e.type == SDL_QUIT) {
            running = false;
            return;
        }

        // Touch Input
        if (e.type == SDL_FINGERDOWN) {
            gestureTracker_.onFingerDown(e.tfinger.x, e.tfinger.y, e.tfinger.fingerId);
            return;
        }
        if (e.type == SDL_FINGERMOTION) {
            gestureTracker_.onFingerMotion(e.tfinger.x, e.tfinger.y, e.tfinger.fingerId);
            return;
        }
        if (e.type == SDL_FINGERUP) {
            auto res = gestureTracker_.onFingerUp(e.tfinger.x, e.tfinger.y, e.tfinger.fingerId);
            handleGesture(res);
            return;
        }

        // Mouse click emulation for testing
        if (e.type == SDL_MOUSEBUTTONDOWN) {
            if (e.button.which == SDL_TOUCH_MOUSEID) return;
            int w = 1280, h = 720;
            getWindowSize(w, h);
            float normX = static_cast<float>(e.button.x) / w;
            float normY = static_cast<float>(e.button.y) / h;
            handleTap(normX, normY);
            return;
        }

        // Controller Hotplugging
        if (e.type == SDL_CONTROLLERDEVICEADDED) {
            if (!controller_) {
                controller_ = SDL_GameControllerOpen(e.cdevice.which);
                if (controller_) logMessage("Controller attached dynamically");
            }
            return;
        }
        if (e.type == SDL_CONTROLLERDEVICEREMOVED) {
            if (controller_) {
                SDL_GameControllerClose(controller_);
                controller_ = nullptr;
                logMessage("Controller detached dynamically");
                for (int i = 0; i < SDL_NumJoysticks(); ++i) {
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
        if (screen_ == ScreenState::Reader) {
            if (res.type == GestureType::SwipeLeft) {
                toolbarVisible_ = false;
                turnPage(1);
            } else if (res.type == GestureType::SwipeRight) {
                toolbarVisible_ = false;
                turnPage(-1);
            } else if (res.type == GestureType::TapLeft || res.type == GestureType::TapRight || res.type == GestureType::TapCenter) {
                handleTap(res.tapX, res.tapY);
            }
        } else {
            handleTap(res.tapX, res.tapY);
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
                        float frac = std::clamp(static_cast<float>(px - 60) / (w - 120), 0.0f, 1.0f);
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
            if (x < 0.22f) turnPage(-1);
            else if (x > 0.78f) turnPage(1);
            else { toolbarVisible_ = true; dirty_ = true; }
        } else if (screen_ == ScreenState::AaModal) {
            handleAaModalTap(px, py, w, h);
        } else if (screen_ == ScreenState::TocModal) {
            handleTocModalTap(px, py, w, h);
        } else if (screen_ == ScreenState::BookmarksModal) {
            handleBookmarksModalTap(px, py, w, h);
        } else if (screen_ == ScreenState::Library) {
            // Header category buttons
            if (py >= 55 && py <= 90) {
                int catW = 110;
                int startX = 48;
                for (int c = 0; c <= 5; ++c) {
                    if (px >= startX + c * catW && px < startX + (c + 1) * catW) {
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
                    searchActive_ = !searchActive_;
                    if (searchActive_) SDL_StartTextInput();
                    else { SDL_StopTextInput(); searchQuery_.clear(); updateVisibleBooks(); }
                    return;
                }
            }
            // Card grid click
            if (viewMode_ == ViewMode::Grid) {
                int cardW = 160, spacingX = 28, startX = 36;
                int cols = std::max(1, (w - startX * 2) / (cardW + spacingX));
                int topY = 96;
                int cardH = 220, spacingY = 24;
                if (py >= topY) {
                    int col = (px - startX) / (cardW + spacingX);
                    int row = (py - topY) / (cardH + spacingY);
                    if (col >= 0 && col < cols && row >= 0) {
                        int idx = (scrollOffset_ + row) * cols + col;
                        if (idx >= 0 && idx < static_cast<int>(visibleBooks_.size())) {
                            selectedBookIdx_ = idx;
                            openBook(visibleBooks_[idx]);
                        }
                    }
                }
            } else {
                // List view click
                int topY = 96;
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
            if (idx >= 0 && idx < static_cast<int>(tocEntries_.size())) {
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

        auto p = progressStore_.get(currentBookId_);
        int rowH = 42;
        int topY = my + 65;
        if (py >= topY && py <= topY + 8 * rowH) {
            int clickedRow = (py - topY) / rowH;
            int idx = bmOffset_ + clickedRow;
            if (idx >= 0 && idx < static_cast<int>(p.bookmarks.size())) {
                bmSelected_ = idx;
                currentPage_ = std::clamp(p.bookmarks[idx].page, 0, std::max(0, pageCount_ - 1));
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

        // Tabs (0: Tipografi, 1: Düzen, 2: Temalar, 3: Kontroller)
        if (py >= my + 45 && py <= my + 85) {
            int tabW = modalW / 4;
            aaTab_ = std::clamp((px - mx) / tabW, 0, 3);
            dirty_ = true;
            return;
        }

        // Tab 0: Tipografi (Font Size + Font Family)
        if (aaTab_ == 0) {
            if (py >= my + 115 && py <= my + 165) {
                if (px >= mx + 190 && px <= mx + 245) { reflowEpub(currentFontSize_ - 2); return; }
                if (px >= mx + 345 && px <= mx + 400) { reflowEpub(currentFontSize_ + 2); return; }
            }
            if (py >= my + 180 && py <= my + 230) {
                if (px >= mx + 190 && px <= mx + 400) {
                    currentFontFamily_ = (currentFontFamily_ == 0) ? 1 : 0;
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
        // Tab 2: Temalar (6 renk teması)
        else if (aaTab_ == 2) {
            if (py >= my + 130 && py <= my + 340) {
                int swatchW = 160, swatchH = 80;
                for (int i = 0; i < g_paletteCount; ++i) {
                    int sx = mx + 36 + (i % 3) * 180;
                    int sy = my + 130 + (i / 3) * 95;
                    if (px >= sx && px <= sx + swatchW && py >= sy && py <= sy + swatchH) {
                        reflowEpub(currentFontSize_, i);
                        return;
                    }
                }
            }
        }
    }

    void handleDirectional(int dx, int dy) {
        if (screen_ == ScreenState::Reader) {
            if (!px_document_is_epub(doc_) && pdfZoom_ > 1.05f) {
                // PDF Pan
                pdfPanX_ = std::clamp(pdfPanX_ + dx * 40, -1000, 1000);
                pdfPanY_ = std::clamp(pdfPanY_ + dy * 40, -1000, 1000);
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
            if (dx != 0) {
                aaTab_ = std::clamp(aaTab_ + dx, 0, 3);
                dirty_ = true;
            } else if (dy != 0) {
                if (aaTab_ == 0) {
                    reflowEpub(currentFontSize_ + (dy < 0 ? 2 : -2));
                } else if (aaTab_ == 2) {
                    int nextTheme = std::clamp(currentTheme_ + (dy > 0 ? 1 : -1), 0, g_paletteCount - 1);
                    reflowEpub(currentFontSize_, nextTheme);
                }
            }
        } else if (screen_ == ScreenState::Library) {
            int w = 1280, h = 720;
            getWindowSize(w, h);
            int cardW = 160, spacingX = 28, startX = 36;
            int cols = (viewMode_ == ViewMode::Grid) ? std::max(1, (w - startX * 2) / (cardW + spacingX)) : 1;
            int total = static_cast<int>(visibleBooks_.size());
            if (total == 0) return;

            int target = selectedBookIdx_ + dx + dy * cols;
            selectedBookIdx_ = std::clamp(target, 0, total - 1);

            // Ensure focused card or row is scrolled into view
            if (viewMode_ == ViewMode::Grid) {
                int topY = 96, cardH = 220, spacingY = 24;
                int maxVisibleRows = std::max(1, (h - topY - 40) / (cardH + spacingY));
                int selectedRow = selectedBookIdx_ / cols;
                if (selectedRow < scrollOffset_) {
                    scrollOffset_ = selectedRow;
                } else if (selectedRow >= scrollOffset_ + maxVisibleRows) {
                    scrollOffset_ = selectedRow - maxVisibleRows + 1;
                }
            } else {
                int rowH = 48, topY = 96;
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
                if (!tocEntries_.empty() && tocSelected_ >= 0 && tocSelected_ < static_cast<int>(tocEntries_.size())) {
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
                    currentPage_ = std::clamp(p.bookmarks[bmSelected_].page, 0, std::max(0, pageCount_ - 1));
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
            if (match(SDLK_x, SDL_CONTROLLER_BUTTON_X)) {
                searchActive_ = !searchActive_;
                if (searchActive_) SDL_StartTextInput();
                else { SDL_StopTextInput(); searchQuery_.clear(); updateVisibleBooks(); }
            }
            if (match(SDLK_y, SDL_CONTROLLER_BUTTON_Y)) openBrowser();
            if (match(SDLK_t, -1)) { currentTheme_ = (currentTheme_ + 1) % g_paletteCount; dirty_ = true; }
            if (match(SDLK_MINUS, SDL_CONTROLLER_BUTTON_BACK)) {
                viewMode_ = (viewMode_ == ViewMode::Grid) ? ViewMode::List : ViewMode::Grid;
                dirty_ = true;
            }
        }

        if (!pad && key == SDLK_BACKSPACE && searchActive_ && !searchQuery_.empty()) {
            searchQuery_.pop_back();
            updateVisibleBooks();
        }
    }

    // Rendering Screens
    void render() {
        int w = 1280, h = 720;
        getWindowSize(w, h);
        const auto &pal = palette();

        SDL_SetRenderDrawColor(renderer_, pal.bg.r, pal.bg.g, pal.bg.b, 255);
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

    void renderLibraryScreen(int w, int h, const ThemePalette &pal) {
        // Top Header
        typography_->drawBox(renderer_, {0, 0, w, 52}, pal.header, pal.cardBorder);
        typography_->renderText(renderer_, typography_->titleFont(), "PaperNX", 36, 10, pal.fg);

        std::string modeText = (viewMode_ == ViewMode::Grid) ? "[Kılavuz]" : "[Liste]";
        typography_->renderText(renderer_, typography_->smallFont(), modeText, w - 190, 16, pal.muted);
        typography_->renderText(renderer_, typography_->smallFont(), "[SD Gözat]", w - 105, 16, pal.accent);

        if (searchActive_) {
            typography_->renderText(renderer_, typography_->uiFont(), "Ara: " + searchQuery_ + "_", 200, 12, pal.accent, 400);
        }

        // Category bar
        const char *cats[] = {"Tümü", "Okunanlar", "Son Eklenenler", "Favoriler", "Okunmamış", "Bitenler"};
        int startX = 36;
        for (int i = 0; i < 6; ++i) {
            bool active = (static_cast<int>(category_) == i);
            SDL_Rect catRect{startX + i * 115, 58, 105, 26};
            typography_->drawBox(renderer_, catRect, active ? pal.accent : pal.header, pal.cardBorder);
            typography_->renderText(renderer_, typography_->smallFont(), cats[i], catRect.x + 8, catRect.y + 4,
                                   active ? pal.bg : pal.fg);
        }

        // View content
        if (visibleBooks_.empty()) {
            typography_->renderText(renderer_, typography_->uiFont(), "Kitaplığınızda henüz kitap bulunmuyor.", 48, 160, pal.fg);
            typography_->renderText(renderer_, typography_->smallFont(),
                                   "sdmc:/switch/PaperNX/books/ klasörüne .epub veya .pdf dosyaları ekleyin.",
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
                               "A: Aç   X: Ara   Y: SD Gözat   -: Görünüm   T: Tema",
                               36, h - 24, pal.muted);
    }

    void renderGrid(int w, int h, const ThemePalette &pal) {
        int cardW = 160, cardH = 220;
        int spacingX = 28, spacingY = 24;
        int topY = 96;
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
                SDL_Rect imgRect{x + 4, y + 4, cardW - 8, cardH - 52};
                SDL_RenderCopy(renderer_, thumb, nullptr, &imgRect);
            } else {
                SDL_Rect fillRect{x + 4, y + 4, cardW - 8, cardH - 52};
                typography_->drawBox(renderer_, fillRect, pal.header, pal.cardBorder);
                typography_->renderText(renderer_, typography_->smallFont(),
                                       visibleBooks_[i].format == Format::Pdf ? "PDF" : "EPUB",
                                       x + 12, y + 16, pal.muted);
                typography_->renderText(renderer_, typography_->uiFont(), visibleBooks_[i].title,
                                       x + 12, y + 48, pal.fg, cardW - 24);
            }

            // Book info footer
            auto p = progressStore_.get(visibleBooks_[i].id);
            typography_->drawProgressBar(renderer_, x + 4, y + cardH - 42, cardW - 8, 4,
                                        static_cast<float>(p.fraction), pal.cardBorder, pal.accent);

            typography_->renderText(renderer_, typography_->smallFont(), visibleBooks_[i].title,
                                   x + 6, y + cardH - 32, pal.fg, cardW - 12);
        }
    }

    void renderList(int w, int h, const ThemePalette &pal) {
        int rowH = 48;
        int topY = 96;
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
            SDL_RenderCopy(renderer_, pageTex, nullptr, &dst);
        }

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
            typography_->renderText(renderer_, typography_->uiFont(), "Aa", w - 75, 12, pal.accent);

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
        typography_->renderText(renderer_, typography_->titleFont(), "SD Dosya Gezgini", 36, 10, pal.fg);
        typography_->renderText(renderer_, typography_->smallFont(), browserPath_.string(), 36, 62, pal.muted, w - 80);

        int topY = 96;
        int rowH = 44;
        for (int i = browserOffset_; i < static_cast<int>(browserEntries_.size()) && i < browserOffset_ + 12; ++i) {
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

        // Tab Content
        if (aaTab_ == 0) {
            // Font Size
            typography_->renderText(renderer_, typography_->uiFont(), "Yazı Boyutu", mx + 36, my + 120, pal.fg);
            typography_->drawBox(renderer_, {mx + 190, my + 115, 48, 38}, pal.bg, pal.cardBorder);
            typography_->renderText(renderer_, typography_->uiFont(), "-", mx + 208, my + 118, pal.fg);

            char szStr[16];
            std::snprintf(szStr, sizeof(szStr), "%d pt", currentFontSize_);
            typography_->renderText(renderer_, typography_->uiFont(), szStr, mx + 265, my + 120, pal.fg);

            typography_->drawBox(renderer_, {mx + 345, my + 115, 48, 38}, pal.bg, pal.cardBorder);
            typography_->renderText(renderer_, typography_->uiFont(), "+", mx + 361, my + 118, pal.fg);

            // Font Family
            typography_->renderText(renderer_, typography_->uiFont(), "Yazı Tipi", mx + 36, my + 190, pal.fg);
            SDL_Rect fontToggle{mx + 190, my + 185, 203, 38};
            typography_->drawBox(renderer_, fontToggle, pal.bg, pal.accent);
            std::string fontName = (currentFontFamily_ == 0) ? "Serif (Noto Serif)" : "Sans (Noto Sans)";
            typography_->renderText(renderer_, typography_->smallFont(), fontName, fontToggle.x + 16, fontToggle.y + 10, pal.fg);
        } else if (aaTab_ == 1) {
            // Satır Aralığı
            typography_->renderText(renderer_, typography_->smallFont(), "Satır Aralığı", mx + 36, my + 115, pal.fg);
            int lhVals[] = {120, 140, 160};
            for (int i = 0; i < 3; ++i) {
                bool sel = (currentLineHeight_ == lhVals[i]);
                SDL_Rect r{mx + 180 + i * 70, my + 110, 60, 32};
                typography_->drawBox(renderer_, r, sel ? pal.accent : pal.bg, pal.cardBorder);
                typography_->renderText(renderer_, typography_->smallFont(), std::to_string(lhVals[i]) + "%", r.x + 8, r.y + 6, sel ? pal.bg : pal.fg);
            }

            // Kenar Boşluğu
            typography_->renderText(renderer_, typography_->smallFont(), "Kenar Boşluğu", mx + 36, my + 175, pal.fg);
            int mgVals[] = {20, 40, 60};
            for (int i = 0; i < 3; ++i) {
                bool sel = (currentMargin_ == mgVals[i]);
                SDL_Rect r{mx + 180 + i * 70, my + 170, 60, 32};
                typography_->drawBox(renderer_, r, sel ? pal.accent : pal.bg, pal.cardBorder);
                typography_->renderText(renderer_, typography_->smallFont(), std::to_string(mgVals[i]) + "px", r.x + 8, r.y + 6, sel ? pal.bg : pal.fg);
            }

            // Hizalama
            typography_->renderText(renderer_, typography_->smallFont(), "Hizalama", mx + 36, my + 235, pal.fg);
            const char *alNames[] = {"Sola", "İki Yana"};
            for (int i = 0; i < 2; ++i) {
                bool sel = (currentTextAlign_ == i);
                SDL_Rect r{mx + 180 + i * 105, my + 230, 95, 32};
                typography_->drawBox(renderer_, r, sel ? pal.accent : pal.bg, pal.cardBorder);
                typography_->renderText(renderer_, typography_->smallFont(), alNames[i], r.x + 16, r.y + 6, sel ? pal.bg : pal.fg);
            }
        } else if (aaTab_ == 2) {
            typography_->renderText(renderer_, typography_->uiFont(), "Tema Seçimi", mx + 36, my + 100, pal.fg);
            for (int i = 0; i < g_paletteCount; ++i) {
                int sx = mx + 36 + (i % 3) * 180;
                int sy = my + 130 + (i / 3) * 95;
                const auto &p = g_palettes[i];

                bool cur = (currentTheme_ == i);
                typography_->drawBox(renderer_, {sx, sy, 160, 80}, p.bg, cur ? pal.accent : pal.cardBorder);
                typography_->renderText(renderer_, typography_->smallFont(), p.name, sx + 14, sy + 14, p.fg);
            }
        } else if (aaTab_ == 3) {
            typography_->renderText(renderer_, typography_->smallFont(), "Sol %22 Ekran: Önceki Sayfa", mx + 36, my + 115, pal.fg);
            typography_->renderText(renderer_, typography_->smallFont(), "Sağ %22 Ekran: Sonraki Sayfa", mx + 36, my + 150, pal.fg);
            typography_->renderText(renderer_, typography_->smallFont(), "Orta %56 Ekran: Araç Çubuğu / Menü", mx + 36, my + 185, pal.fg);
            typography_->renderText(renderer_, typography_->smallFont(), "D-Pad Sol/Sağ veya L/R: Sayfa Çevirme", mx + 36, my + 220, pal.fg);
            typography_->renderText(renderer_, typography_->smallFont(), "+ Tuşu: Aa Ayarları   X: İçindekiler   Y: Yer İmi", mx + 36, my + 255, pal.fg);
            typography_->renderText(renderer_, typography_->smallFont(), "B Tuşu: Geri / Kitaplık   A Tuşu: Seçim Onayla", mx + 36, my + 290, pal.fg);
        }
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

    {
        PaperNxApp app(root);
        if (app.init()) {
            app.run();
        }
    }

    TTF_Quit();
    SDL_Quit();
    return 0;
}
