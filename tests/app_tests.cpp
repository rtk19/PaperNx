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
    static void finger(PaperNxApp &app, Uint32 type, float x, float y, SDL_FingerID id = 41) {
        SDL_Event e{}; e.type = type; e.tfinger.fingerId = id; e.tfinger.x = x; e.tfinger.y = y;
        bool running = true; app.handleEvent(e, running); assert(running);
    }
    static void tap(PaperNxApp &app, float x, float y) {
        finger(app, SDL_FINGERDOWN, x, y); finger(app, SDL_FINGERUP, x, y);
    }
    static void modalTap(PaperNxApp &app, int x, int y) {
        int w, h; app.getWindowSize(w, h);
        tap(app, float((w - 600) / 2 + x) / w, float((h - 460) / 2 + y) / h);
    }
    static void snapshot(PaperNxApp &app, const char *name) {
        for (int i = 0; i < 15; ++i) { app.render(); SDL_Delay(10); app.processCompletedThumbnails(); }
        app.render(); int w, h; app.getWindowSize(w, h);
        SDL_Surface *surface = SDL_CreateRGBSurfaceWithFormat(0, w, h, 32, SDL_PIXELFORMAT_ARGB8888);
        assert(surface);
        assert(SDL_RenderReadPixels(app.renderer_, nullptr, surface->format->format, surface->pixels, surface->pitch) == 0);
        fs::create_directories("verification/screenshots");
        assert(SDL_SaveBMP(surface, (std::string("verification/screenshots/") + name + ".bmp").c_str()) == 0);
        SDL_FreeSurface(surface);
    }
    static void touchOnly(const fs::path &root) {
        const auto start = std::chrono::steady_clock::now();
        std::string epubId, pdfId;
        int savedPage;
        {
            PaperNxApp app(root); assert(app.init()); assert(!app.controller_);
            auto epub = *std::find_if(app.books_.begin(), app.books_.end(), [](const Book &b) { return b.format == Format::Epub; });
            // Open first library card using only finger events.
            tap(app, .06f, .18f); assert(app.doc_ && app.screen_ == ScreenState::Reader);
            if (!px_document_is_epub(app.doc_)) { app.closeBook(); app.openBook(epub); }
            epubId = app.currentBookId_;
            for (int i = 0; i < 10; ++i) tap(app, .75f, .5f);
            assert(app.currentPage_ == 10);
            tap(app, .25f, .5f); assert(app.currentPage_ == 9);
            finger(app, SDL_FINGERDOWN, .7f, .5f); SDL_Delay(900);
            finger(app, SDL_FINGERMOTION, .45f, .51f); finger(app, SDL_FINGERUP, .4f, .51f);
            assert(app.currentPage_ == 10); // Slow swipe used to be rejected.
            finger(app, SDL_FINGERDOWN, .25f, .5f); SDL_Delay(550); finger(app, SDL_FINGERUP, .27f, .51f);
            assert(app.currentPage_ == 9); // Long tap and ordinary finger drift.
            finger(app, SDL_FINGERDOWN, .3f, .5f); finger(app, SDL_FINGERUP, .65f, .5f);
            assert(app.currentPage_ == 8);
            // Two-finger input cannot also turn a page.
            finger(app, SDL_FINGERDOWN, .8f, .5f, 1); finger(app, SDL_FINGERDOWN, .7f, .5f, 2);
            finger(app, SDL_FINGERUP, .8f, .5f, 1); finger(app, SDL_FINGERUP, .7f, .5f, 2);
            assert(app.currentPage_ == 8);
            // Synthesized mouse event arriving after the native finger is ignored.
            SDL_Event mouse{}; mouse.type = SDL_MOUSEBUTTONDOWN; mouse.button.which = SDL_TOUCH_MOUSEID;
            mouse.button.button = SDL_BUTTON_LEFT; mouse.button.x = 1100; mouse.button.y = 350;
            bool running = true; app.handleEvent(mouse, running); mouse.type = SDL_MOUSEBUTTONUP; app.handleEvent(mouse, running);
            assert(app.currentPage_ == 8);
            // A mouse-only SDL backend remains usable, on release (not down).
            app.sawFinger_ = false;
            mouse.type = SDL_MOUSEBUTTONDOWN; app.handleEvent(mouse, running); assert(app.currentPage_ == 8);
            mouse.type = SDL_MOUSEBUTTONUP; app.handleEvent(mouse, running); assert(app.currentPage_ == 9);
            tap(app, .5f, .5f); assert(app.toolbarVisible_);
            tap(app, .96f, .04f); assert(app.screen_ == ScreenState::AaModal);
            const auto mark = px_document_make_bookmark(app.doc_, app.currentPage_);
            modalTap(app, 370, 135); assert(app.currentFontSize_ == 24);
            modalTap(app, 280, 205); assert(app.currentFontFamily_ == 1);
            modalTap(app, 225, 65); assert(app.aaTab_ == 1);
            modalTap(app, 280, 310); assert(app.extraStyle_.paragraphSpacing == 120);
            modalTap(app, 280, 370); assert(app.extraStyle_.indentation == 0);
            modalTap(app, 375, 65); assert(app.aaTab_ == 2);
            for (int i = 0; i < 6; ++i) { modalTap(app, 60 + i % 3 * 180, 140 + i / 3 * 65); assert(app.currentTheme_ == i); }
            assert(app.commitColor("123456", 0)); assert(app.commitColor("#EEDDBB", 1));
            assert(!app.commitColor("GGGGGG", 0));
            assert(app.extraStyle_.textColor == 0x123456 && app.extraStyle_.backgroundColor == 0xeeddbb);
            assert(app.currentPage_ == px_document_lookup_bookmark(app.doc_, mark));
            snapshot(app, "epub-settings");
            // Global defaults are explicit; current book retains an independent override.
            app.storeDefaults();
            modalTap(app, 575, 20); assert(app.screen_ == ScreenState::Reader);
            savedPage = app.currentPage_;
            tap(app, .86f, .04f); assert(app.screen_ == ScreenState::BookmarksModal);
            modalTap(app, 250, 430); assert(app.progressStore_.get(epubId).bookmarks.size() == 1);
            modalTap(app, 575, 20); assert(app.screen_ == ScreenState::Reader);
            tap(app, .04f, .04f); assert(app.screen_ == ScreenState::Library && !app.progressDirty_);
            app.category_ = Category::Completed; app.updateVisibleBooks();
            assert(app.libraryEmptyMessage() == "Henüz tamamladığınız bir kitap yok.");
            snapshot(app, "library-empty-completed");
            app.category_ = Category::All; app.updateVisibleBooks();
            app.books_[0].title = "İstanbul'da çok uzun bir kitap başlığı — Türkçe karakterler ve çok satırlı kitap kartı";
            app.updateVisibleBooks(); snapshot(app, "library");
            for (auto size : {std::pair<int,int>{960,540}, {1920,1080}, {640,480}}) {
                SDL_SetWindowSize(app.window_, size.first, size.second); app.render();
            }
            SDL_SetWindowSize(app.window_, 1280, 720);
            auto pdf = *std::find_if(app.books_.begin(), app.books_.end(), [](const Book &b) { return b.format == Format::Pdf; });
            app.openBook(pdf); pdfId = pdf.id;
            assert(app.currentFontFamily_ == 1); // New book inherits global defaults.
            tap(app, .5f, .5f); tap(app, .96f, .04f);
            modalTap(app, 120, 195); assert(app.extraStyle_.pdfViewMode == 1 && app.pdfZoom_ > 1);
            modalTap(app, 220, 275); assert(app.extraStyle_.pdfColorMode == 1);
            snapshot(app, "pdf-settings");
            modalTap(app, 575, 20); tap(app, .5f, .5f); // Hide toolbar.
            const int page = app.currentPage_;
            finger(app, SDL_FINGERDOWN, .5f, .6f); finger(app, SDL_FINGERMOTION, .5f, .3f); finger(app, SDL_FINGERUP, .5f, .3f);
            assert(app.currentPage_ == page && app.pdfPanY_ > 0);
            app.handleDirectional(100000, 100000); int maxPan = app.pdfPanY_;
            app.handleDirectional(100000, 100000); assert(app.pdfPanY_ == maxPan);
            snapshot(app, "pdf-fit-width-filter");
            app.setPdfView(0); assert(app.pdfZoom_ == 1 && app.pdfPanY_ == 0);
            tap(app, .75f, .5f); assert(app.currentPage_ == 1);
            SDL_Event background{}; background.type = SDL_APP_WILLENTERBACKGROUND;
            app.handleEvent(background, running); assert(!app.progressDirty_);
            background.type = SDL_APP_DIDENTERFOREGROUND; app.handleEvent(background, running);
            app.closeBook();
        }
        {
            PaperNxApp restart(root); assert(restart.init());
            auto epub = *std::find_if(restart.books_.begin(), restart.books_.end(), [&](const Book &b) { return b.id == epubId; });
            restart.openBook(epub); assert(restart.currentPage_ == savedPage);
            assert(restart.extraStyle_.textColor == 0x123456 && restart.extraStyle_.backgroundColor == 0xeeddbb);
            assert(restart.currentTheme_ == 5 && restart.currentFontFamily_ == 1);
            assert(restart.progressStore_.get(epubId).bookmarks.size() == 1);
            auto pdf = *std::find_if(restart.books_.begin(), restart.books_.end(), [&](const Book &b) { return b.id == pdfId; });
            restart.openBook(pdf); assert(restart.currentPage_ == 1 && restart.extraStyle_.pdfColorMode == 1);
        }
        std::cout << "[PASS] Touch-only workflow elapsed_ms=" << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count() << "\n";
    }
    static void checkRestart(const fs::path &root) {
        PaperNxApp app(root); assert(app.init());
        assert(app.books_.size() == 3);
        auto epub = std::find_if(app.books_.begin(), app.books_.end(), [](const Book &b) { return b.format == Format::Epub; });
        assert(epub != app.books_.end() && epub->title.find("İstanbul") != std::string::npos);
        app.openBook(*epub);
        assert(app.currentTheme_ == 4 && app.currentFontFamily_ == 1);
        assert(app.progressStore_.get(app.currentBookId_).bookmarks.size() == 1);
    }
    static void run(const fs::path &root) {
        PaperNxApp app(root);
        assert(app.init());
        assert(app.books_.size() == 2);
        int first = SDL_JoystickAttachVirtual(SDL_JOYSTICK_TYPE_GAMECONTROLLER, 6, 21, 0);
        int second = SDL_JoystickAttachVirtual(SDL_JOYSTICK_TYPE_GAMECONTROLLER, 6, 21, 0);
        assert(first >= 0 && second >= 0);
        SDL_Event hotplug{}; hotplug.type = SDL_CONTROLLERDEVICEADDED; hotplug.cdevice.which = first;
        bool keepRunning = true; app.handleEvent(hotplug, keepRunning);
        assert(app.controller_);
        SDL_JoystickID primaryId = SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(app.controller_));
        hotplug.type = SDL_CONTROLLERDEVICEREMOVED; hotplug.cdevice.which = primaryId + 1000;
        app.handleEvent(hotplug, keepRunning);
        assert(app.controller_ && SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(app.controller_)) == primaryId);
        assert(SDL_JoystickDetachVirtual(first) == 0);
        hotplug.cdevice.which = primaryId; app.handleEvent(hotplug, keepRunning);
        assert(app.controller_ && SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(app.controller_)) != primaryId);
        SDL_JoystickID secondaryId = SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(app.controller_));
        assert(SDL_JoystickDetachVirtual(0) == 0);
        hotplug.cdevice.which = secondaryId; app.handleEvent(hotplug, keepRunning);
        assert(!app.controller_);
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
        pad(app, SDL_CONTROLLER_BUTTON_Y);
        pad(app, SDL_CONTROLLER_BUTTON_DPAD_DOWN);
        assert(app.currentFontFamily_ == 1);
        pad(app, SDL_CONTROLLER_BUTTON_DPAD_RIGHT);
        const int oldLineHeight = app.currentLineHeight_;
        pad(app, SDL_CONTROLLER_BUTTON_DPAD_UP);
        assert(app.currentLineHeight_ > oldLineHeight);
        pad(app, SDL_CONTROLLER_BUTTON_Y);
        const int oldMargin = app.currentMargin_;
        pad(app, SDL_CONTROLLER_BUTTON_DPAD_UP);
        assert(app.currentMargin_ > oldMargin);
        pad(app, SDL_CONTROLLER_BUTTON_Y);
        pad(app, SDL_CONTROLLER_BUTTON_DPAD_DOWN);
        assert(app.currentTextAlign_ == 1);
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
        SDL_Event pinch{}; pinch.type = SDL_MULTIGESTURE; pinch.mgesture.numFingers = 2; pinch.mgesture.dDist = 0.1f;
        app.handleEvent(pinch, running); assert(app.pdfZoom_ > 1);
        pad(app, SDL_CONTROLLER_BUTTON_RIGHTSTICK);
        pad(app, SDL_CONTROLLER_BUTTON_START);
        int w, h; app.getWindowSize(w, h);
        app.handleAaModalTap((w - 600) / 2 + 200, (h - 460) / 2 + 130, w, h);
        assert(app.pdfZoom_ == 1.25f);
        pad(app, SDL_CONTROLLER_BUTTON_Y); assert(app.pdfZoom_ == 1);
        pad(app, SDL_CONTROLLER_BUTTON_B);
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
        app.openBrowser();
        auto external = std::find_if(app.browserEntries_.begin(), app.browserEntries_.end(), [](const BrowserEntry &e) { return e.path.filename() == "external.pdf"; });
        assert(external != app.browserEntries_.end());
        app.browserSelected_ = static_cast<int>(external - app.browserEntries_.begin());
        app.handleBrowserSelect();
        assert(app.doc_ && app.books_.size() == 3);
        app.closeBook();
        assert(app.progressStore_.save());
        // A real failed periodic write must keep pending progress for retry.
        const auto blockedTemp = root / "data" / "progress.txt.tmp";
        fs::create_directory(blockedTemp);
        app.progressDirty_ = true;
        app.lastProgressSave_ = std::chrono::steady_clock::now() - std::chrono::seconds(4);
        SDL_Event quit{}; quit.type = SDL_QUIT; assert(SDL_PushEvent(&quit) == 1);
        app.run();
        assert(app.progressDirty_);
        fs::remove(blockedTemp);
        assert(app.progressStore_.save());
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
    {
        const int index = SDL_JoystickAttachVirtual(SDL_JOYSTICK_TYPE_UNKNOWN, 4, 28, 0);
        assert(index >= 0);
        char guid[33] = {};
        SDL_JoystickGetGUIDString(SDL_JoystickGetDeviceGUID(index), guid, sizeof guid);
        assert(SDL_GameControllerAddMapping(switchControllerMapping(guid).c_str()) >= 0);
        SDL_GameController *pad = SDL_GameControllerOpen(index);
        assert(pad);
        SDL_Joystick *joystick = SDL_GameControllerGetJoystick(pad);
        const std::pair<int, SDL_GameControllerButton> buttons[] = {
            {0, SDL_CONTROLLER_BUTTON_A}, {1, SDL_CONTROLLER_BUTTON_B},
            {2, SDL_CONTROLLER_BUTTON_X}, {3, SDL_CONTROLLER_BUTTON_Y},
            {6, SDL_CONTROLLER_BUTTON_LEFTSHOULDER}, {7, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER},
            {10, SDL_CONTROLLER_BUTTON_START}, {11, SDL_CONTROLLER_BUTTON_BACK},
            {12, SDL_CONTROLLER_BUTTON_DPAD_LEFT}, {13, SDL_CONTROLLER_BUTTON_DPAD_UP},
            {14, SDL_CONTROLLER_BUTTON_DPAD_RIGHT}, {15, SDL_CONTROLLER_BUTTON_DPAD_DOWN}};
        for (const auto &button : buttons) {
            assert(SDL_JoystickSetVirtualButton(joystick, button.first, SDL_PRESSED) == 0);
            SDL_GameControllerUpdate();
            assert(SDL_GameControllerGetButton(pad, button.second) == SDL_PRESSED);
            assert(SDL_JoystickSetVirtualButton(joystick, button.first, SDL_RELEASED) == 0);
            SDL_GameControllerUpdate();
        }
        for (int trigger = 8; trigger <= 9; ++trigger) {
            assert(SDL_JoystickSetVirtualButton(joystick, trigger, SDL_PRESSED) == 0);
            SDL_GameControllerUpdate();
            assert(SDL_GameControllerGetAxis(pad, trigger == 8 ? SDL_CONTROLLER_AXIS_TRIGGERLEFT : SDL_CONTROLLER_AXIS_TRIGGERRIGHT) == 32767);
        }
        SDL_GameControllerClose(pad);
        assert(SDL_JoystickDetachVirtual(index) == 0);
    }
    const auto root = fs::temp_directory_path() / "papernx-app-regressions";
    fs::remove_all(root); fs::create_directories(root / "books");
    fs::copy_file("tests/fixtures/sample_book.epub", root / "books" / "sample.epub");
    fs::copy_file("tests/fixtures/sample_doc.pdf", root / "books" / "sample.pdf");
    fs::create_directory_symlink(fs::absolute("fonts"), root / "fonts");
    fs::copy_file("tests/fixtures/sample_doc.pdf", root / "external.pdf");
    AppTest::run(root);
    AppTest::checkRestart(root);
    const auto touchRoot = root / "touch-only";
    fs::create_directories(touchRoot / "books");
    fs::copy_file("tests/fixtures/sample_book.epub", touchRoot / "books" / "a.epub");
    fs::copy_file("tests/fixtures/sample_doc.pdf", touchRoot / "books" / "b.pdf");
    fs::create_directory_symlink(fs::absolute("fonts"), touchRoot / "fonts");
    AppTest::touchOnly(touchRoot);
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
    assert(px_document_open_limited("tests/fixtures/sample_book.epub", 600, 600, 22, "", err, sizeof err, 1024) == nullptr);
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
        assert(image.rgb[0] == g_palettes[theme].bg.r && image.rgb[1] == g_palettes[theme].bg.g && image.rgb[2] == g_palettes[theme].bg.b);
        themeHashes.insert(hashImage(image));
        px_image_free(&image);
    }
    assert(themeHashes.size() == 6);
    // Exercise the reader's V4 CSS model, not just the legacy palette helper.
    for (int preset = 0; preset < 6; ++preset) {
        ReaderPreferences style; style.theme = preset; style.fontFamily = preset % 3;
        const auto css = generateEpubCss(style);
        const auto layoutStart = std::chrono::steady_clock::now();
        assert(px_document_relayout(doc, 600, 600, 22, css.c_str(), err, sizeof err));
        const auto renderStart = std::chrono::steady_clock::now();
        PxImage image{}; assert(px_document_render(doc, 0, 600, 600, &image, err, sizeof err));
        assert(image.rgb[0] == g_palettes[preset].bg.r && image.rgb[1] == g_palettes[preset].bg.g);
        std::cout << "BENCH preset=" << preset << " relayout_us=" << std::chrono::duration_cast<std::chrono::microseconds>(renderStart - layoutStart).count()
            << " render_us=" << std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - renderStart).count() << " viewport=600x600\n";
        px_image_free(&image);
    }
    std::set<uint64_t> fontHashes;
    for (int family = 0; family < 2; ++family) {
        ReaderPreferences style; style.fontFamily = family;
        const auto css = generateEpubCss(style);
        assert(px_document_relayout(doc, 600, 600, 22, css.c_str(), err, sizeof err));
        PxImage image{}; assert(px_document_render(doc, 0, 600, 600, &image, err, sizeof err));
        fontHashes.insert(hashImage(image)); px_image_free(&image);
    }
    assert(fontHashes.size() == 2);
    ReaderPreferences custom; custom.backgroundColor = 0xeeddbb; custom.textColor = 0x123456;
    auto customCss = generateEpubCss(custom);
    assert(px_document_relayout(doc, 600, 600, 22, customCss.c_str(), err, sizeof err));
    PxImage customImage{}; assert(px_document_render(doc, 0, 600, 600, &customImage, err, sizeof err));
    assert(customImage.rgb[0] == 0xee && customImage.rgb[1] == 0xdd && customImage.rgb[2] == 0xbb);
    const auto customHash = hashImage(customImage); px_image_free(&customImage);
    custom.textColor = 0x884422; customCss = generateEpubCss(custom);
    assert(px_document_relayout(doc, 600, 600, 22, customCss.c_str(), err, sizeof err));
    assert(px_document_render(doc, 0, 600, 600, &customImage, err, sizeof err));
    assert(hashImage(customImage) != customHash && customImage.rgb[0] == 0xee); px_image_free(&customImage);
    const int validPages = px_document_pages(doc);
    assert(!px_document_relayout(doc, 100000, 600, 22, customCss.c_str(), err, sizeof err));
    assert(px_document_pages(doc) == validPages);
    assert(px_document_render(doc, 0, 600, 600, &customImage, err, sizeof err)); px_image_free(&customImage);
    assert(readingContrast({255,255,255,255}, {255,255,255,255}) == 1);
    assert(readingContrast({255,255,255,255}, {0,0,0,255}) > 20);
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
