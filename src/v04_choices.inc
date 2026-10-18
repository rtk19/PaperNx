    int choiceKind_{-1},choiceIndex_{0};
    std::vector<std::string> choiceOptions() const {
        switch(choiceKind_) {
        case 0:return {"Serif","Sans",ui::language?"Publisher":"Yay\u0131nc\u0131"};
        case 1:return ui::language?std::vector<std::string>{"Landscape","Portrait clockwise","Portrait counterclockwise"}:std::vector<std::string>{"Yatay","Dikey: saat y\u00f6n\u00fc","Dikey: saat y\u00f6n\u00fcn\u00fcn tersi"};
        case 2:return ui::language?std::vector<std::string>{"Follow reading theme","Follow application theme","Custom UI theme"}:std::vector<std::string>{"Okuma temas\u0131n\u0131 izle","Uygulama temas\u0131n\u0131 izle","\u00d6zel aray\u00fcz temas\u0131"};
        case 3:case 4:{std::vector<std::string> out;for(const auto &p:g_palettes)out.push_back(p.name);return out;}
        case 5:return ui::language?std::vector<std::string>{"Original","Dark","Sepia","Invert"}:std::vector<std::string>{"Orijinal","Koyu","Sepya","Ters renkler"};
        case 6:return ui::language?std::vector<std::string>{"Off","Soft paper","Natural paper"}:std::vector<std::string>{"Kapal\u0131","Yumu\u015fak ka\u011f\u0131t","Do\u011fal ka\u011f\u0131t"};
        default:return {};
        }
    }
    void openChoice(int kind,int selected) {choiceKind_=kind;choiceIndex_=selected;dirty_=true;}
    SDL_Rect choiceSheet(int w,int h) const {int width=std::min(560,w-32),height=std::min(h-64,80+int(choiceOptions().size())*56);return {(w-width)/2,(h-height)/2,width,height};}
    void commitChoice() {
        auto a=appSettings_.get();
        if(choiceKind_==0){currentFontFamily_=choiceIndex_;queueStyleChange();}
        if(choiceKind_==1){a.displayOrientation=choiceIndex_;appSettings_.put(a);pageCache_.clear();if(doc_ && px_document_is_epub(doc_)){++layoutGeneration_;layoutPending_=true;layoutDue_=SDL_GetTicks()+100;}else if(doc_)setPdfView(extraStyle_.pdfViewMode);}
        if(choiceKind_==2)a.chromePolicy=choiceIndex_;
        if(choiceKind_==3)a.uiTheme=choiceIndex_;
        if(choiceKind_==4)a.customUiTheme=choiceIndex_;
        if(choiceKind_==5){extraStyle_.pdfColorMode=choiceIndex_;pageCache_.clear();if(doc_)saveCurrentPosition();else storeDefaults();}
        if(choiceKind_==6){a.paperTexture=choiceIndex_;pageCache_.clear();}
        if(choiceKind_!=0 && choiceKind_!=5){appSettings_.put(a);persistAppSettings();}
        choiceKind_=-1;gestureTracker_.reset();activeFingers_.clear();dirty_=true;
    }
    bool choiceEvent(const SDL_Event &e) {
        if(choiceKind_<0)return false;
        if(e.type==SDL_KEYDOWN || e.type==SDL_CONTROLLERBUTTONDOWN){bool pad=e.type==SDL_CONTROLLERBUTTONDOWN;int k=pad?e.cbutton.button:e.key.keysym.sym;
            auto m=[&](int key,int button){return k==(pad?button:key);};
            if(m(SDLK_ESCAPE,SDL_CONTROLLER_BUTTON_B)){choiceKind_=-1;dirty_=true;}
            else if(m(SDLK_RETURN,SDL_CONTROLLER_BUTTON_A))commitChoice();
            else {int d=m(SDLK_DOWN,SDL_CONTROLLER_BUTTON_DPAD_DOWN)||m(SDLK_RIGHT,SDL_CONTROLLER_BUTTON_DPAD_RIGHT)?1:m(SDLK_UP,SDL_CONTROLLER_BUTTON_DPAD_UP)||m(SDLK_LEFT,SDL_CONTROLLER_BUTTON_DPAD_LEFT)?-1:0;choiceIndex_=std::clamp(choiceIndex_+d,0,int(choiceOptions().size())-1);dirty_=true;}
        } else if(e.type==SDL_FINGERUP){int w,h;getWindowSize(w,h);auto box=choiceSheet(w,h);int x=int(e.tfinger.x*w),y=int(e.tfinger.y*h);
            if(!ui::contains(box,x,y)){choiceKind_=-1;dirty_=true;return true;}
            int index=(y-box.y-64)/56;if(y>=box.y+64 && index>=0 && index<int(choiceOptions().size())){choiceIndex_=index;commitChoice();}
            lastFingerTick_=SDL_GetTicks();sawFinger_=true;
        } else if(e.type==SDL_MOUSEBUTTONUP && e.button.button==SDL_BUTTON_LEFT && !(sawFinger_ && SDL_GetTicks()-lastFingerTick_<750)) {
            int pw,ph;SDL_GetWindowSize(window_,&pw,&ph);DisplayTransform transform{pw,ph,appSettings_.get().displayOrientation};auto point=transform.toLogical(float(e.button.x),float(e.button.y));
            SDL_Event finger=e;finger.type=SDL_FINGERUP;finger.tfinger.x=point.first/transform.logicalWidth();finger.tfinger.y=point.second/transform.logicalHeight();choiceEvent(finger);
        }
        return true;
    }
    void renderChoices(int w,int h,const ThemePalette &pal) {
        if(choiceKind_<0)return;
        SDL_SetRenderDrawBlendMode(renderer_,SDL_BLENDMODE_BLEND);SDL_SetRenderDrawColor(renderer_,0,0,0,150);SDL_Rect full{0,0,w,h};SDL_RenderFillRect(renderer_,&full);
        auto box=choiceSheet(w,h);ui::surface(renderer_,box,pal.card,18);
        typography_->renderText(renderer_,typography_->sectionFont(),ui::language?"Choose an option":"Bir se\u00e7enek se\u00e7in",box.x+24,box.y+16,pal.fg,box.w-48);
        auto options=choiceOptions();for(int i=0;i<int(options.size());++i)ui::button(renderer_,*typography_,{box.x+16,box.y+64+i*56,box.w-32,56},options[i],pal,false,false,false,i==choiceIndex_);
    }
