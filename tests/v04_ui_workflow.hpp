// Actual SDL application interaction and output regressions, no mocked screenshots.
static void v04UiWorkflow(const fs::path &parent) {
    FocusManager focus;
    focus.begin("library");focus.add("book",{0,60,100,100});focus.add("menu",{120,0,56,56});focus.add("disabled",{0,0,56,56},false);focus.finish();
    assert(focus.focused("book"));assert(focus.move(1,0) && focus.focused("menu"));
    focus.begin("modal");focus.add("cancel",{0,0,56,56});focus.add("confirm",{80,0,56,56});focus.finish();
    assert(focus.focused("cancel"));assert(focus.move(1,0) && focus.focused("confirm"));
    focus.begin("library");focus.add("book",{0,60,100,100});focus.add("menu",{120,0,56,56});focus.finish();assert(focus.focused("menu"));
    assert(focus.touch(20,100) && focus.focused("book"));
    const auto root=parent/"v04-ui";fs::create_directories(root/"books");
    fs::create_directory_symlink(fs::absolute("fonts"),root/"fonts");
    fs::copy_file("tests/fixtures/sample_book.epub",root/"books"/"paper.epub");
    fs::copy_file("tests/fixtures/sample_doc.pdf",root/"books"/"pdf.pdf");
    {
        PaperNxApp app(root);assert(app.init());
        app.openSettings(true);auto old=app.currentStyle();
        pad(app,SDL_CONTROLLER_BUTTON_DPAD_DOWN);assert(app.aaRow_==1 && app.currentFontFamily_==old.fontFamily);
        pad(app,SDL_CONTROLLER_BUTTON_DPAD_UP);pad(app,SDL_CONTROLLER_BUTTON_A);assert(app.settingsEditing_);
        pad(app,SDL_CONTROLLER_BUTTON_DPAD_RIGHT);assert(app.currentFontSize_==old.fontSize+2);
        pad(app,SDL_CONTROLLER_BUTTON_DPAD_LEFT);assert(app.currentFontSize_==old.fontSize);
        pad(app,SDL_CONTROLLER_BUTTON_DPAD_RIGHT);pad(app,SDL_CONTROLLER_BUTTON_B);assert(!app.settingsEditing_ && app.currentFontSize_==old.fontSize);
        pad(app,SDL_CONTROLLER_BUTTON_A);pad(app,SDL_CONTROLLER_BUTTON_DPAD_RIGHT);pad(app,SDL_CONTROLLER_BUTTON_A);
        assert(app.currentFontSize_==old.fontSize+2 && !app.settingsEditing_);
        snapshot(app,"v04-settings-focus-landscape");
        app.editColor(1);pad(app,SDL_CONTROLLER_BUTTON_DPAD_RIGHT);assert(app.extraStyle_.backgroundColor==app.swatchColors_[app.swatchFocus_]);
        snapshot(app,"v04-color-swatches");pad(app,SDL_CONTROLLER_BUTTON_B);assert(app.extraStyle_.backgroundColor==old.backgroundColor);
        closeSettings(app);
        auto epub=*std::find_if(app.books_.begin(),app.books_.end(),[](const Book &b){return b.format==Format::Epub;});
        app.openBook(epub);app.currentTheme_=2;app.extraStyle_.backgroundColor=app.extraStyle_.textColor=-1;app.queueStyleChange();v03WaitLayout(app);
        app.openSettings();assert(app.palette().bg.r==g_palettes[2].bg.r);snapshot(app,"v04-sepia-epub-settings");closeSettings(app);
        app.currentTheme_=4;app.extraStyle_.backgroundColor=app.extraStyle_.textColor=-1;app.queueStyleChange();v03WaitLayout(app);
        app.screen_=ScreenState::TocModal;snapshot(app,"v04-dark-epub-toc");app.screen_=ScreenState::Reader;
        app.currentTheme_=5;app.queueStyleChange();v03WaitLayout(app);app.quickMenu_=true;snapshot(app,"v04-oled-quick-menu");app.quickMenu_=false;
        app.closeBook();
        for(int orientation=1;orientation<=2;++orientation) {
            auto a=app.appSettings_.get();a.displayOrientation=orientation;app.appSettings_.put(a);app.persistAppSettings();
            int w,h;app.getWindowSize(w,h);assert(w==720 && h==1280);app.render();
            const char *name=orientation==1?"v04-portrait-cw-library":"v04-portrait-ccw-library";snapshot(app,name);
            // Tap physical point corresponding to logical +; assert inverse input mapping.
            DisplayTransform transform{1280,720,orientation};auto point=transform.toPhysical(float(w-48),40);
            tap(app,point.first/1280,point.second/720);assert(app.quickMenu_);app.quickMenu_=false;
            app.openBook(epub);assert(app.layoutWidth_<=720);app.render();
            snapshot(app,orientation==1?"v04-portrait-cw-epub":"v04-portrait-ccw-epub");app.closeBook();
        }
        auto a=app.appSettings_.get();a.displayOrientation=0;app.appSettings_.put(a);app.persistAppSettings();
        auto pdf=*std::find_if(app.books_.begin(),app.books_.end(),[](const Book &b){return b.format==Format::Pdf;});app.openBook(pdf);
        app.extraStyle_.pdfColorMode=1;app.pageCache_.clear();assert(app.palette().bg.r<48);app.toolbarVisible_=true;snapshot(app,"v04-dark-pdf-toolbar");
        app.openSettings();snapshot(app,"v04-dark-pdf-settings");closeSettings(app);app.toolbarVisible_=false;
        app.extraStyle_.pdfColorMode=2;app.pageCache_.clear();snapshot(app,"v04-sepia-pdf-canvas");
        app.extraStyle_.pdfColorMode=0;app.pageCache_.clear();snapshot(app,"v04-original-pdf");
        app.zoomPdf(2);app.currentPage_=0;pad(app,SDL_CONTROLLER_BUTTON_DPAD_RIGHT);assert(app.currentPage_==1 && app.pdfZoom_==2);
        pad(app,SDL_CONTROLLER_BUTTON_DPAD_LEFT);assert(app.currentPage_==0 && app.pdfZoom_==2);
        finger(app,SDL_FINGERDOWN,.8f,.5f);finger(app,SDL_FINGERMOTION,.2f,.5f);finger(app,SDL_FINGERUP,.2f,.5f);assert(app.currentPage_==1);
        finger(app,SDL_FINGERDOWN,.2f,.5f);finger(app,SDL_FINGERMOTION,.8f,.5f);finger(app,SDL_FINGERUP,.8f,.5f);assert(app.currentPage_==0);
        finger(app,SDL_FINGERDOWN,.5f,.7f);app.pdfGestureStart_=SDL_GetTicks()-600;finger(app,SDL_FINGERMOTION,.3f,.7f);finger(app,SDL_FINGERUP,.3f,.7f);assert(app.currentPage_==0);
        app.closeBook();
        for(auto size:{std::pair<int,int>{640,480},{1920,1080}}){SDL_SetWindowSize(app.window_,size.first,size.second);snapshot(app,size.first==640?"v04-library-640":"v04-library-1920");}
    }
    {AppSettingsStore store(root/"data"/"settings.txt");assert(store.load() && store.get().displayOrientation==0);}
}
