#define main papernx_application_entry
#include "../src/main.cpp"
#undef main
#include "papernx/switch_compat.h"
#include <cassert>
#include <cstring>
#include <iostream>
#ifdef NDEBUG
#error Assertions must remain enabled
#endif

namespace {
struct AppTest {
    static void key(PaperNxApp &app, SDL_Keycode code) {
        SDL_Event e{}; e.type = SDL_KEYDOWN; e.key.keysym.sym = code;
        bool running = true; app.handleEvent(e, running); assert(running);
    }
    static void pad(PaperNxApp &app, Uint8 button) {
        SDL_Event e{}; e.type = SDL_CONTROLLERBUTTONDOWN; e.cbutton.button = button;
        bool running = true; app.handleEvent(e, running); assert(running);
    }
    static void run(const fs::path &root) {
        PaperNxApp app(root);
        assert(app.init());
        assert(app.books_.size() == 2);
        app.render();
        key(app, SDLK_x);
        assert(app.searchActive_);
        SDL_Event text{}; text.type = SDL_TEXTINPUT;
        std::strcpy(text.text.text, "İıI iŞşĞğÜüÖöÇç");
        bool running = true;
        app.handleEvent(text, running);
        assert(app.searchQuery_ == "İıI iŞşĞğÜüÖöÇç");
        key(app, SDLK_BACKSPACE);
        assert(app.searchQuery_ == "İıI iŞşĞğÜüÖöÇ");
        key(app, SDLK_ESCAPE);
        assert(!app.searchActive_ && app.visibleBooks_.size() == 2);
        key(app, SDLK_MINUS); assert(app.viewMode_ == ViewMode::List);
        key(app, SDLK_MINUS); assert(app.viewMode_ == ViewMode::Grid);
        key(app, SDLK_f);
        assert(app.progressStore_.get(app.visibleBooks_[app.selectedBookIdx_].id).favorite);
        auto epub = *std::find_if(app.books_.begin(), app.books_.end(), [](const Book &b) { return b.format == Format::Epub; });
        app.openBook(epub); assert(app.doc_ && app.screen_ == ScreenState::Reader);
        assert(app.tocEntries_.size() == 2 && app.pageCount_ > 2);
        app.render();
        pad(app, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER); assert(app.currentPage_ == 1);
        pad(app, SDL_CONTROLLER_BUTTON_Y);
        assert(app.progressStore_.get(epub.id).bookmarks.size() == 1);
        pad(app, SDL_CONTROLLER_BUTTON_X); assert(app.screen_ == ScreenState::TocModal);
        pad(app, SDL_CONTROLLER_BUTTON_DPAD_DOWN);
        pad(app, SDL_CONTROLLER_BUTTON_A);
        assert(app.screen_ == ScreenState::Reader && app.currentPage_ > 1);
        const auto stableMark = px_document_make_bookmark(app.doc_, app.currentPage_);
        pad(app, SDL_CONTROLLER_BUTTON_START); assert(app.screen_ == ScreenState::AaModal);
        int page = app.currentPage_;
        pad(app, SDL_CONTROLLER_BUTTON_B); assert(app.currentPage_ == page);
        app.currentFontFamily_ = 1;
        app.reflowEpub(30, 4);
        assert(app.currentPage_ == px_document_lookup_bookmark(app.doc_, stableMark));
        assert(app.pageCache_.count() == 0);
        app.render();
        assert(app.progressStore_.save());
        app.closeBook();
        app.openBook(epub);
        assert(app.currentTheme_ == 4 && app.currentFontSize_ == 30 && app.currentFontFamily_ == 1);
        pad(app, SDL_CONTROLLER_BUTTON_BACK); assert(app.screen_ == ScreenState::BookmarksModal);
        pad(app, SDL_CONTROLLER_BUTTON_A); assert(app.screen_ == ScreenState::Reader);
        auto pdf = *std::find_if(app.books_.begin(), app.books_.end(), [](const Book &b) { return b.format == Format::Pdf; });
        app.openBook(pdf); assert(app.doc_ && !px_document_is_epub(app.doc_));
        key(app, SDLK_EQUALS); assert(app.pdfZoom_ == 1.25f);
        SDL_Event axis{}; axis.type = SDL_CONTROLLERAXISMOTION;
        axis.caxis.axis = SDL_CONTROLLER_AXIS_TRIGGERRIGHT; axis.caxis.value = 30000;
        app.handleEvent(axis, running); assert(app.pdfZoom_ == 1.5f);
        app.zoomPdf(5); app.handleDirectional(100000, 100000);
        assert(app.pdfPanX_ >= 0 && app.pdfPanY_ > 0);
        int oldPan = app.pdfPanY_;
        SDL_Event touch{}; touch.type = SDL_FINGERDOWN; touch.tfinger.fingerId = 1;
        touch.tfinger.x = 0.5f; touch.tfinger.y = 0.5f; app.handleEvent(touch, running);
        touch.type = SDL_FINGERMOTION; touch.tfinger.y = 0.6f; app.handleEvent(touch, running);
        assert(app.pdfPanY_ < oldPan);
        touch.type = SDL_FINGERUP; app.handleEvent(touch, running);
        pad(app, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER);
        assert(app.currentPage_ == 1 && app.pdfPanX_ == 0 && app.pdfPanY_ == 0);
        pad(app, SDL_CONTROLLER_BUTTON_RIGHTSTICK); assert(app.pdfZoom_ == 1);
        app.render();
        app.closeBook();
        // Worker completion must release pending IDs and permit reload after eviction.
        app.getOrRenderThumbnail(epub);
        bool uploaded = false;
        for (int i = 0; i < 500; ++i) {
            SDL_Delay(10); app.processCompletedThumbnails();
            if (app.thumbCache_.get("thumb_" + epub.id)) { uploaded = true; break; }
        }
        assert(uploaded);
        { std::lock_guard<std::mutex> lock(app.queueMutex_); assert(!app.queuedThumbIds_.count(epub.id)); }
        app.thumbCache_.clear();
        app.getOrRenderThumbnail(epub);
        { std::lock_guard<std::mutex> lock(app.queueMutex_); assert(app.queuedThumbIds_.count(epub.id)); }
    }
};
}

static uint64_t hashImage(const PxImage &image) {
    uint64_t hash = 14695981039346656037ull;
    for (size_t i = 0; i < static_cast<size_t>(image.height) * image.stride; ++i)
        hash = (hash ^ image.rgb[i]) * 1099511628211ull;
    return hash;
}

int main() {
    assert(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | SDL_INIT_EVENTS) == 0);
    assert(TTF_Init() == 0);
    const auto root = fs::temp_directory_path() / "papernx-app-regressions";
    fs::remove_all(root); fs::create_directories(root / "books");
    fs::copy_file("tests/fixtures/sample_book.epub", root / "books" / "sample.epub");
    fs::copy_file("tests/fixtures/sample_doc.pdf", root / "books" / "sample.pdf");
    fs::create_directory_symlink(fs::absolute("fonts"), root / "fonts");
    AppTest::run(root);
    SDL_Window *window = SDL_CreateWindow("cache", 0, 0, 128, 128, SDL_WINDOW_HIDDEN);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    assert(renderer);
    {
        PageCache cache(1024);
        auto texture = [&] { return SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STATIC, 16, 16); };
        assert(cache.put("first", texture(), 16, 16));
        assert(cache.put("second", texture(), 16, 16));
        assert(!cache.get("first") && cache.currentBytes() == 1024);
        assert(!cache.put("oversized", SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STATIC, 32, 32), 32, 32));
        assert(cache.currentBytes() == 1024);
        assert(cache.put("second", texture(), 16, 16));
        assert(cache.count() == 1);
        auto serif = PageCache::makeEpubKey("doc", 0, 22, 0, 140, 40, 0, 1280, 720, 0);
        auto sans = PageCache::makeEpubKey("doc", 0, 22, 0, 140, 40, 0, 1280, 720, 1);
        assert(serif != sans);
    }
    SDL_DestroyRenderer(renderer); SDL_DestroyWindow(window);
    char err[256] = {};
    auto doc = px_document_open("tests/fixtures/sample_book.epub", 600, 600, 22, "", err, sizeof err);
    assert(doc);
    PxImage cover{};
    assert(px_document_render_thumbnail(doc, 96, 144, &cover, err, sizeof err));
    assert(cover.width == 96 && cover.height == 144);
    assert(cover.rgb[0] == 24 && cover.rgb[1] == 80 && cover.rgb[2] == 180);
    px_image_free(&cover);
    std::set<uint64_t> themeHashes;
    for (int theme = 0; theme < 6; ++theme) {
        const auto css = generateEpubCss(theme, 0, 22, 140, 20, 0);
        assert(px_document_relayout(doc, 600, 600, 22, css.c_str(), err, sizeof err));
        PxImage image{}; assert(px_document_render(doc, 0, 600, 600, &image, err, sizeof err));
        themeHashes.insert(hashImage(image));
        px_image_free(&image);
    }
    assert(themeHashes.size() == 6);
    auto mark = px_document_make_bookmark(doc, px_document_pages(doc) - 1);
    px_document_close(doc);
    for (int i = 0; i < 20; ++i) {
        doc = px_document_open("tests/fixtures/sample_book.epub", 400 + i * 10, 500, 26, "", err, sizeof err);
        assert(doc && px_document_lookup_bookmark(doc, mark) == px_document_pages(doc) - 1);
        assert(px_document_lookup_bookmark(doc, 9876543210ll) == -1);
        px_document_close(doc);
    }
    struct tm epoch{}; epoch.tm_year = 70; epoch.tm_mday = 1;
    assert(papernx_timegm(&epoch) == 0);
    epoch.tm_year = 100; epoch.tm_mon = 1; epoch.tm_mday = 29;
    assert(papernx_timegm(&epoch) == 951782400);
    fs::remove_all(root);
    TTF_Quit(); SDL_Quit();
    std::cout << "[PASS] SDL input, worker reload, EPUB themes/cover/reopen, PDF controls, texture budget and UTC regressions\n";
}
