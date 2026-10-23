    // Shared geometric focus registry: contexts isolate modals and remember their last item.
    FocusManager navigationFocus_;
    void rebuildFocus() {
        int w,h;getWindowSize(w,h);
        std::string context=catalogPanel_!=CatalogPanel::None?"catalog."+std::to_string(int(catalogPanel_)):quickMenu_?"quick":std::to_string(int(screen_));
        if(screen_==ScreenState::AaModal && !quickMenu_)context+=":"+std::to_string(aaTab_);
        if(screen_==ScreenState::TocModal && !quickMenu_)context+=":"+std::to_string(tocTab_);
        navigationFocus_.begin(context);
        auto add=[&](const std::string &id,SDL_Rect r){navigationFocus_.add(id,{r.x,r.y,r.w,r.h});};
        if(catalogPanel_!=CatalogPanel::None) {
            auto r=catalogPanelRect(w,h);
            if(catalogPanel_==CatalogPanel::Text){auto keys=catalogKeyboard();for(int i=0;i<int(keys.size());++i)add("key."+std::to_string(i),catalogKeyRect(i,w,h));
                add("text.save",{r.x+20,r.y+r.h-76,r.w/2-24,56});add("text.erase",{r.x+r.w/2+4,r.y+r.h-76,r.w/2-24,56});
            }else{auto rows=catalogRows();for(int i=0;i<int(rows.size());++i){bool manage=catalogPanel_==CatalogPanel::Values && catalogGroup_==CatalogGroup::Collections;
                auto rect=catalogRowRect(i,w,h);if(manage)rect.w-=132;add("catalog.row."+std::to_string(i),rect);if(manage)add("catalog.manage."+std::to_string(i),catalogManageRect(i,w,h));}}

            add("catalog.close",{r.x+r.w-152,r.y+12,56,56});
        } else if(quickMenu_) {
            auto r=quickSheet(w,h);auto actions=quickActions();
            for(int i=0;i<int(actions.size());++i)add("quick."+std::to_string(i),quickActionRect(i,w,h));
            add("quick.orientation",{r.x+r.w-80,r.y+20,56,56});
            add("quick.close",{w-80,8,64,64});
        } else if(screen_==ScreenState::AaModal) {
            ui::SettingsLayout l(w,h);
            // Initial focus is the first setting; selection is still the independent current preference.
            for(int slot=0;slot<settingsRows();++slot){int i=slot;for(int candidate=0;candidate<settingsRows();++candidate)if(settingsSlot(candidate)==slot){i=candidate;break;}if(aaTab_==2 && i<5)continue;if(aaTab_==4 && i==4 && (!doc_ || px_document_is_epub(doc_)))continue;if(aaTab_==5 && i==4 && browserPath_.empty())continue;auto r=settingsControl(i,w,h);r.y-=int(settingsScroll_);add("setting."+std::to_string(i),r);}
            if(aaTab_==2)for(int i=0;i<9;++i){auto r=l.control(i/2);r.y-=int(settingsScroll_);int half=(r.w-12)/2;r.x+=(i%2)*(half+12);r.w=half;add("theme."+std::to_string(i),r);}
            for(int i=0;i<6;++i)add("tab."+std::to_string(i),l.tab(i));
            if(doc_ && aaTab_!=4 && aaTab_!=5)add("scope",l.scope);
            add("close",l.close);
            add("save",l.footer);add("reset",l.reset);
        } else if(screen_==ScreenState::Library) {
            auto l=libraryLayout(w);
            for(int i=0;i<int(visibleBooks_.size());++i) {
                SDL_Rect r=viewMode_==ViewMode::Grid?SDL_Rect{l.left+(i%l.columns)*l.stepX,l.top+(i/l.columns)*l.stepY-int(listScroll_),l.cardWidth,l.cardHeight}:libraryListRow(i,w,listScroll_);
                add("book."+std::to_string(i),r);
            }
            if(l.home && books_.empty()){for(int i=0;i<3;++i)add("welcome."+std::to_string(i),welcomeCard(i,w,h));}
            else for(int i=0;i<6;++i)add("category."+std::to_string(i),categoryRect(i,w));
            add("search",librarySearch(w));add("library.add",libraryAdd(w));add("plus",{w-80,8,64,64});
            if(l.home)add("library.resume",l.hero(w));
            if(!openSeriesKey_.empty())add("series.back",{l.left,16,56,56});
            if(books_.empty() && !l.home)add("browse",emptyLibraryAction(w,h));
        } else if(screen_==ScreenState::FileBrowser) {
            ui::BrowserLayout l(w,h);
            for(int i=0;i<int(browserEntries_.size());++i)add("file."+std::to_string(i),l.row(i,browserScroll_));
            add("browser.folder",l.import);add("browser.back",l.back);if(browserPath_!=browserRoot_)add("browser.up",l.up);add("plus",{w-80,8,64,64});
        } else if(screen_==ScreenState::TocModal) {
            const int first=std::max(0,int(tocScroll_)/tocStep()-1)*tocColumns(w);
            const int last=std::min(tocCount(),first+(tocContent(w,h).h/tocStep()+4)*tocColumns(w));
            for(int i=first;i<last;++i)if(tocItem(i).page>=0 && tocItem(i).page<pageCount_)add("toc."+std::to_string(i),tocCell(i,w,h));
            if(tocSelected_>=0 && tocSelected_<tocCount() && (tocSelected_<first || tocSelected_>=last) && tocItem(tocSelected_).page>=0 && tocItem(tocSelected_).page<pageCount_)add("toc."+std::to_string(tocSelected_),tocCell(tocSelected_,w,h));
            for(int tab=0;tab<3;++tab)add("contents.tab."+std::to_string(tab),tocTabRect(tab,w));
            add("close",ui::NavigationLayout(w,h).close);
        } else if(screen_==ScreenState::BookmarksModal) {
            auto p=progressStore_.get(currentBookId_);
            for(int i=0;i<int(p.bookmarks.size());++i) {
                auto r=bookmarkRow(i,w,h);r.w-=104;add("bookmark."+std::to_string(i),r);
                auto full=bookmarkRow(i,w,h);add("bookmark.delete."+std::to_string(i),{full.x+full.w-96,full.y+8,88,56});
            }
            add("bookmark.add",bookmarkAdd(w,h));add("close",ui::NavigationLayout(w,h).close);
        } else if(screen_==ScreenState::Reader && (toolbarVisible_ || appSettings_.get().toolbar)) {
            add("reader.library",readerHeaderButton(0,w));
            add("reader.toc",readerHeaderButton(1,w));
            add("reader.bookmarks",readerHeaderButton(2,w));
            add("reader.settings",readerHeaderButton(3,w));
            add("reader.previous",readerPrev(w,h));add("reader.seek",readerSeekHit(w,h));add("reader.next",readerNext(w,h));add("reader.pages",readerPages(w,h));
            if(showZoomReset())add("reader.zoom-reset",zoomResetRect(w,h));
        }
        if(!quickMenu_ && catalogPanel_==CatalogPanel::None && (screen_==ScreenState::Library || screen_==ScreenState::FileBrowser || screen_==ScreenState::ReadingJournal)) {
            ui::ShellLayout shell(w,h);for(int i=0;i<5;++i)add("nav."+std::to_string(i),shell.nav(i));if(shell.rail)add("nav.settings",shell.settings());
        }
        navigationFocus_.finish();
    }
    void focusReveal() {
        const auto id=navigationFocus_.active();
        auto index=[&](const char *prefix){return std::stoi(id.substr(std::char_traits<char>::length(prefix)));};
        int w,h;getWindowSize(w,h);
        auto reveal=[](float &scroll,float top,float height,float viewport){if(top<scroll)scroll=top;if(top+height>scroll+viewport)scroll=std::max(0.f,top+height-viewport);};
        if(id.rfind("catalog.manage.",0)==0){catalogFocus_=index("catalog.manage.");auto r=catalogPanelRect(w,h);reveal(catalogScroll_,(catalogFocus_/catalogColumns())*80.f,72,float(r.y+r.h-16-catalogContentTop(r)));}
        else if(id.rfind("catalog.row.",0)==0){catalogFocus_=index("catalog.row.");auto r=catalogPanelRect(w,h);reveal(catalogScroll_,(catalogFocus_/catalogColumns())*80.f,72,float(r.y+r.h-16-catalogContentTop(r)));}
        else if(id=="catalog.close")catalogFocus_=-1;
        else if(id.rfind("bookmark.delete.",0)==0){bmSelected_=index("bookmark.delete.");reveal(bookmarkScroll_,bmSelected_*80.f,72,float(bookmarkContent(w,h).h));}
        else if(id.rfind("theme.",0)==0){aaRow_=index("theme.")/2;ui::SettingsLayout l(w,h);reveal(settingsScroll_,settingsSlot(aaRow_)*72.f,64,float(l.content.h));}
        else if(id.rfind("setting.",0)==0){aaRow_=index("setting.");ui::SettingsLayout l(w,h);reveal(settingsScroll_,settingsSlot(aaRow_)*72.f,64,float(l.content.h));}
        else if(id.rfind("book.",0)==0){selectedBookIdx_=index("book.");auto l=libraryLayout(w);int columns=viewMode_==ViewMode::Grid?l.columns:1;int step=viewMode_==ViewMode::Grid?l.stepY:112;reveal(listScroll_,float(selectedBookIdx_/columns*step),float(viewMode_==ViewMode::Grid?l.cardHeight:104),float(libraryBottom(w,h)-l.top));}
        else if(id.rfind("file.",0)==0){browserSelected_=index("file.");reveal(browserScroll_,browserSelected_*80.f,72,float(ui::BrowserLayout(w,h).content.h));}
        else if(id.rfind("toc.",0)==0){tocSelected_=index("toc.");revealContentsItem();}
        else if(id.rfind("bookmark.",0)==0 && id.rfind("bookmark.delete.",0)!=0 && id!="bookmark.add"){bmSelected_=index("bookmark.");reveal(bookmarkScroll_,bmSelected_*80.f,72,float(bookmarkContent(w,h).h));}
        else if(id.rfind("quick.",0)==0 && id!="quick.close" && id!="quick.orientation"){quickFocus_=index("quick.");reveal(quickScroll_,(quickFocus_/quickColumns(w))*88.f,76,float(quickContent(w,h).h));}
        dirty_=true;
    }
    bool moveFocus(int dx,int dy) {
        if(screen_==ScreenState::Reader && !quickMenu_ && catalogPanel_==CatalogPanel::None) {
            // D-pad Left/Right turns pages until the toolbar is deliberately focused.
            if(!(toolbarVisible_ || appSettings_.get().toolbar)) {readerToolbarFocusActive_=false;return false;}
            if(!readerToolbarFocusActive_) {
                if(dy>=0) return false;
                readerToolbarFocusActive_=true;
            } else if(dy>0) {
                int w,h;getWindowSize(w,h);
                for(const auto &item:navigationFocus_.items())if(item.id==navigationFocus_.active() && item.rect.y>=h-80){readerToolbarFocusActive_=false;dirty_=true;return true;}
            }
        }
        if(screen_==ScreenState::AaModal && settingsEditing_ && catalogPanel_==CatalogPanel::None)return false;
        rebuildFocus();navigationFocus_.move(dx,dy);focusReveal();return true;
    }
    bool activateFocus() {
        if(screen_==ScreenState::Reader && !quickMenu_ && catalogPanel_==CatalogPanel::None && !readerToolbarFocusActive_)return false;
        if(screen_==ScreenState::AaModal && settingsEditing_ && catalogPanel_==CatalogPanel::None)return false;
        rebuildFocus();const auto id=navigationFocus_.active();
        if(id=="reader.seek"){startReaderScrub(true);return true;}
        if(catalogPanel_==CatalogPanel::None && screen_==ScreenState::AaModal && id.rfind("setting.",0)==0) {
            aaRow_=std::stoi(id.substr(8));
            if(numericSetting()){editOriginal_=currentStyle();editOriginalZoom_=pdfZoom_;settingsEditing_=true;dirty_=true;return true;}
        }
        if(id=="reader.library"){closeBook();return true;}
        if(id=="reader.toc"){openContents();return true;}
        if(id=="reader.bookmarks"){screen_=ScreenState::BookmarksModal;dirty_=true;return true;}
        if(id=="reader.settings"){openSettings();return true;}
        for(const auto &item:navigationFocus_.items())if(item.id==id){int w,h;getWindowSize(w,h);auto r=item.rect;handleTap(float(r.x+r.w/2)/w,float(r.y+r.h/2)/h);return true;}
        return false;
    }
    void paintFocus() {
        if(screen_==ScreenState::Reader && !quickMenu_ && catalogPanel_==CatalogPanel::None && !readerToolbarFocusActive_)return;
        rebuildFocus();const auto pal=interfacePalette();
        for(const auto &item:navigationFocus_.items())if(item.id==navigationFocus_.active()) {
            const auto r=item.rect;
            const bool contentsCell=screen_==ScreenState::TocModal && item.id.rfind("toc.",0)==0;
            if(contentsCell){int w,h;getWindowSize(w,h);const auto clip=tocContent(w,h);SDL_RenderSetClipRect(renderer_,&clip);}
            ui::focusRing(renderer_,{r.x,r.y,r.w,r.h},pal,screen_==ScreenState::AaModal && settingsEditing_);
            if(contentsCell)SDL_RenderSetClipRect(renderer_,nullptr);
            break;
        }
    }
