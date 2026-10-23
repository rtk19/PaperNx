// The application shell is separate from the physical reading-page layout.
bool libraryHome_{true},loadingScreen_{false};
std::string loadingTitle_,loadingDetail_;
float loadingProgress_{0};
ui::LibraryLayout libraryLayout(int w) const {
    int pw,ph;SDL_GetWindowSize(window_,&pw,&ph);const int h=appSettings_.get().displayOrientation?pw:ph;
    return {w,libraryHome_ && openSeriesKey_.empty() && h>=640 && (w>=1000 || h>=1000)};
}
int libraryBottom(int w,int h) const {return h-ui::ShellLayout(w,h).bottom;}
SDL_Rect libraryListRow(int i,int w,float scroll) const {auto l=libraryLayout(w);return {l.left,l.top+i*112-int(scroll),w-l.left-24,104};}
SDL_Rect librarySearch(int w) const {return {w-224,16,56,56};}
SDL_Rect libraryAdd(int w) const {return {w-160,16,56,56};}
SDL_Rect emptyLibraryAction(int w,int h) const {auto l=libraryLayout(w);return {l.left+24,std::min(l.top+148,libraryBottom(w,h)-64),240,56};}
int activeShellTab() const {return screen_==ScreenState::FileBrowser?4:screen_==ScreenState::ReadingJournal?3:libraryHome_?0:1;}
void shellNavigate(int tab) {
    quickMenu_=false;helpMenu_=false;catalogPanel_=CatalogPanel::None;
    if(tab==4){openBrowser();return;}
    if(tab==3){enterJournal(false);return;}
    if(doc_)closeBook();
    screen_=ScreenState::Library;globalSettings_=false;
    if(tab==2){libraryHome_=false;setCatalogPanel(CatalogPanel::Browse);return;}
    libraryHome_=tab==0;category_=Category::All;searchQuery_.clear();searchActive_=false;openSeriesKey_.clear();catalogFilter_.reset();updateVisibleBooks();
}
bool shellTap(int x,int y,int w,int h) {
    if(screen_!=ScreenState::Library && screen_!=ScreenState::FileBrowser && screen_!=ScreenState::ReadingJournal)return false;
    ui::ShellLayout l(w,h);
    for(int i=0;i<5;++i)if(ui::contains(l.nav(i),x,y)){shellNavigate(i);return true;}
    if(l.rail && ui::contains(l.settings(),x,y)){openSettings(true,5);return true;}
    return false;
}
void drawShell(int w,int h,const ThemePalette &p) {
    ui::ShellLayout l(w,h);
    ui::surface(renderer_,l.rail?SDL_Rect{0,0,208,h}:SDL_Rect{0,h-96,w,96},p.card,0);
    if(l.rail) {
        ui::surface(renderer_,{24,30,40,40},p.accent,12);ui::icon(renderer_,{24,30,40,40},ui::Icon::Books,{255,255,255,255});
        typography_->renderText(renderer_,typography_->sectionFont(),"PaperNX",76,32,p.fg,124);
        ui::eyebrow(renderer_,*typography_,"READING SPACE",24,92,p,176);
        ui::navItem(renderer_,*typography_,l.settings(),ui::Icon::Settings,"Settings",p,false);
        typography_->renderText(renderer_,typography_->smallFont(),ui::label("On your device"),24,h-68,p.muted,170);
        typography_->renderText(renderer_,typography_->smallFont(),APP_VERSION,24,h-40,p.muted,170);
    }
    const ui::Icon icons[]={ui::Icon::Home,ui::Icon::Books,ui::Icon::Collection,ui::Icon::Journal,ui::Icon::Folder};
    const char *names[]={"Home","Library","Collections","Journal","Files"};
    for(int i=0;i<5;++i)ui::navItem(renderer_,*typography_,l.nav(i),icons[i],names[i],p,i==activeShellTab(),!l.rail);
}
unsigned libraryRevision_{1},resumeRevision_{0};
std::optional<Book> resumeSnapshot_;
std::optional<Book> resumeBook() {
    if(resumeRevision_==libraryRevision_)return resumeSnapshot_;
    std::optional<Book> choice;std::int64_t last=-1;
    for(const auto &book:books_) {
        if(catalog_.get(book.id).hidden)continue;
        const auto p=progressStore_.get(book.id);
        if(p.lastRead>last && p.fraction<.99){choice=catalog_.apply(book);last=p.lastRead;}
    }
    if(!choice && !visibleBooks_.empty())choice=visibleBooks_.front();
    resumeSnapshot_=choice;resumeRevision_=libraryRevision_;return choice;
}
void drawLibraryHero(int w,const ThemePalette &p) {
    const auto r=libraryLayout(w).hero(w);auto hero=p;hero.card=blendColor(p.card,p.accent,16);ui::elevated(renderer_,r,hero,24);
    const auto b=resumeBook();
    const int textW=r.w-(w>=1000?240:168);
    ui::eyebrow(renderer_,*typography_,b?"A LITTLE MORE TIME FOR YOURSELF":"YOUR NEXT CHAPTER STARTS HERE",r.x+24,r.y+16,p,textW-24);
    typography_->renderText(renderer_,typography_->titleFont(),b?b->title:ui::label("Make room for a good book."),r.x+24,r.y+46,p.fg,textW-24);
    typography_->renderText(renderer_,typography_->smallFont(),b?(b->author.empty()?std::string(formatName(b->format)):b->author):ui::label("Your books, comics and quiet moments."),r.x+24,r.y+86,p.muted,textW-24);
    ui::button(renderer_,*typography_,{r.x+24,r.y+r.h-68,200,56},b?"Continue reading":"Add books",p,true);
    if(b){drawCover(*b,{r.x+r.w-140,r.y+14,112,156},p);const auto f=progressStore_.get(b->id).fraction;
        ui::surface(renderer_,{r.x+r.w-84,r.y+r.h-48,68,32},p.card,12);
        typography_->renderText(renderer_,typography_->smallFont(),std::to_string(int(f*100))+"%",r.x+r.w-72,r.y+r.h-44,p.fg,48);
    } else {ui::icon(renderer_,{r.x+r.w-140,r.y+30,110,120},ui::Icon::Books,p.accent);}
}
void drawLoading(int w,int h,const ThemePalette &p) {
    const int cx=w/2,cy=h/2;
    for(int i=3;i>=0;--i)ui::surface(renderer_,{cx-86-i*16,cy-186-i*8,172+i*32,172+i*16},blendColor(p.bg,p.accent,3+i*2),32);
    ui::surface(renderer_,{cx-64,cy-168,128,128},p.accent,32);ui::icon(renderer_,{cx-44,cy-148,88,88},ui::Icon::Books,{255,255,255,255});
    auto centered=[&](TTF_Font *font,const std::string &text,int y,SDL_Color c){int tw=0;typography_->measureText(font,text,&tw,nullptr,w-80);typography_->renderText(renderer_,font,text,cx-tw/2,y,c,w-80);};
    centered(typography_->displayFont(),loadingTitle_.empty()?"PaperNX":loadingTitle_,cy+8,p.fg);
    centered(typography_->uiFont(),loadingDetail_.empty()?ui::label("A quieter place for your stories."):loadingDetail_,cy+72,p.muted);
    typography_->drawProgressBar(renderer_,cx-140,cy+132,280,4,std::clamp(loadingProgress_,.04f,1.f),p.cardBorder,p.accent);
    centered(typography_->smallFont(),ui::label("YOUR READING SPACE"),h-64,p.muted);
}
void loadingFrame(const std::string &title,const std::string &detail,float progress) {
    if(!renderer_ || !typography_)return;
    loadingScreen_=true;loadingTitle_=title;loadingDetail_=detail;loadingProgress_=progress;render();loadingScreen_=false;
}
SDL_Rect quickContent(int w,int h) const {auto r=quickSheet(w,h);return {r.x+20,r.y+100,r.w-40,r.h-120};}
int quickColumns(int w) const {return w>=700?2:1;}
SDL_Rect quickActionRect(int i,int w,int h) const {auto c=quickContent(w,h);int cols=quickColumns(w),step=c.w/cols;return {c.x+(i%cols)*step,c.y+(i/cols)*88-int(quickScroll_),step-12,76};}
int quickExtent(int w) const {return ((int(quickActions().size())+quickColumns(w)-1)/quickColumns(w))*88;}
SDL_Rect bookmarkContent(int w,int h) const {auto l=ui::NavigationLayout(w,h);auto r=l.content;if(!l.rail)r.h-=72;return r;}
SDL_Rect bookmarkAdd(int w,int h) const {auto l=ui::NavigationLayout(w,h);return l.rail?SDL_Rect{l.sheet.x+20,l.sheet.y+l.sheet.h-84,188,56}:SDL_Rect{l.sheet.x+24,l.sheet.y+l.sheet.h-80,l.sheet.w-48,56};}
SDL_Rect bookmarkRow(int i,int w,int h) const {auto c=bookmarkContent(w,h);return {c.x,c.y+i*80-int(bookmarkScroll_),c.w,72};}

int settingsSlot(int row) const {
    static const int order[]={13,14,1,2,10,11,12,8,9,3,6,5,0,7,4};
    if(aaTab_!=5)return row;
    for(int i=0;i<15;++i)if(order[i]==row)return i;
    return row;
}
SDL_Rect settingsControl(int row,int w,int h) const {return ui::SettingsLayout(w,h).control(settingsSlot(row));}
SDL_Rect colorSwatch(int i,int w,int h) const {
    auto panel=colorPanel(w,h);const int width=panel.w-(w>=1000?304:48),step=width/4;
    return {panel.x+24+(i%4)*step,panel.y+96+(i/4)*(h<600?64:72),step-12,h<600?56:60};
}
SDL_Rect colorAction(bool apply,int w,int h) const {auto p=colorPanel(w,h);return {apply?p.x+p.w-216:p.x+24,p.y+p.h-80,192,56};}
void renderColorDialog(int w,int h,const ThemePalette &p) {
    auto panel=colorPanel(w,h);ui::elevated(renderer_,panel,p,28);
    ui::eyebrow(renderer_,*typography_,"READING, YOUR WAY",panel.x+24,panel.y+16,p,panel.w-48);
    typography_->renderText(renderer_,typography_->titleFont(),ui::label(colorEditing_?"Arka plan rengi":"Metin rengi"),panel.x+24,panel.y+44,p.fg,panel.w-48);
    for(int i=0;i<16;++i){auto r=colorSwatch(i,w,h);ui::surface(renderer_,r,readingColor(swatchColors_[i],p.bg),18);if(i==swatchFocus_)ui::focusRing(renderer_,r,p);}
    if(w>=1000){auto paper=palette();SDL_Rect preview{panel.x+panel.w-264,panel.y+96,240,std::min(276,panel.h-200)};ui::surface(renderer_,preview,paper.bg,24);
        typography_->renderText(renderer_,typography_->displayFont(),"Aa",preview.x+80,preview.y+48,paper.fg,160);
        typography_->renderWrapped(renderer_,typography_->uiFont(),ui::label("One page, a whole world."),{preview.x+24,preview.y+140,192,112},paper.fg);}
    if(readingContrast(readingColor(extraStyle_.backgroundColor,p.bg),readingColor(extraStyle_.textColor,p.fg))<4.5)typography_->renderText(renderer_,typography_->smallFont(),ui::label("Düşük kontrast"),h<600?panel.x+panel.w-180:panel.x+24,h<600?panel.y+16:panel.y+panel.h-120,p.accent,h<600?156:panel.w-48);
    ui::button(renderer_,*typography_,colorAction(false,w,h),"Vazgeç",p);ui::button(renderer_,*typography_,colorAction(true,w,h),"Kaydet",p,true);
}
SDL_Rect welcomeCard(int i,int w,int h) const {
    (void)h;auto l=libraryLayout(w);const int cols=w>=1000?3:1,width=(w-l.left-24)/cols;
    return {l.left+(i%cols)*width,320+(i/cols)*176,width-16,w>=1000?252:160};
}
void drawWelcome(int w,int h,const ThemePalette &p) {
    (void)h;const char *titles[]={"Add your books","Make yourself at home","Keep the good parts"};
    const char *descriptions[]={"Choose a folder. We will find your books and comics.","Pick your paper, typography and page layout.","Save places, build collections and make reading a habit."};
    const ui::Icon glyphs[]={ui::Icon::Folder,ui::Icon::Type,ui::Icon::Bookmark};
    for(int i=0;i<3;++i){auto r=welcomeCard(i,w,h);ui::elevated(renderer_,r,p,24);
        ui::icon(renderer_,{r.x+16,r.y+16,48,48},glyphs[i],p.accent);
        const int textY=r.y+(w>=1000?80:24),textX=r.x+(w>=1000?20:80);
        typography_->renderWrapped(renderer_,typography_->sectionFont(),ui::label(titles[i]),{textX,textY,r.w-(textX-r.x)-20,64},p.fg);
        typography_->renderWrapped(renderer_,typography_->smallFont(),ui::label(descriptions[i]),{textX,textY+68,r.w-(textX-r.x)-20,96},p.muted);
    }
}
std::vector<std::string> helpLines() const {
    std::vector<std::string> lines{std::string("PaperNX ")+APP_VERSION};
    if(doc_){lines.push_back(journalTitle_);if(!journalAuthor_.empty())lines.push_back(journalAuthor_);}
    const char *guide[]={"+ opens all actions.","Tap the centre to show reading tools.","Swipe to turn pages. Pinch comics to zoom.",
        "L/R follows the book direction in landscape.","LS click / G: preview; A confirms, B cancels.","Progress bar: drag, preview, release.",
        "Choose a folder to add all its books.","Default settings apply to books without an override.","This book keeps its own reading preferences.","Your books and progress stay on your device."};
    for(const auto *text:guide)lines.push_back(ui::label(text));
    return lines;
}
int helpLineHeight(const std::string &line,int width) const {
    int measured=0;typography_->measureText(typography_->uiFont(),line,&measured,nullptr);
    return std::max(48,(measured/std::max(1,width)+2)*30+8);
}
int helpExtent(int w,int h) const {int total=0;const auto content=quickContent(w,h);for(const auto &line:helpLines())total+=helpLineHeight(line,content.w-16);return total;}
