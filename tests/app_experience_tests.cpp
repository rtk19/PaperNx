#define main papernx_application_entry
#include "../src/main.cpp"
#undef main
#include <cassert>
#include <iostream>
namespace {
struct AppTest {
#include "library_fixture_helpers.inc"
    static void tap(PaperNxApp &app,SDL_Rect rect) {
        int w,h;app.getWindowSize(w,h);bool running=true;
        for(auto type:{SDL_FINGERDOWN,SDL_FINGERUP}){SDL_Event e{};e.type=type;e.tfinger.fingerId=17;e.tfinger.x=float(rect.x+rect.w/2)/w;e.tfinger.y=float(rect.y+rect.h/2)/h;app.handleEvent(e,running);}
        assert(running);
    }
    static void shot(PaperNxApp &app,const std::string &name) {
        app.render();int w,h;app.getWindowSize(w,h);
        auto *s=SDL_CreateRGBSurfaceWithFormat(0,w,h,32,SDL_PIXELFORMAT_ARGB8888);assert(s);
        assert(SDL_RenderReadPixels(app.renderer_,nullptr,s->format->format,s->pixels,s->pitch)==0);
        fs::create_directories("verification/experience");assert(SDL_SaveBMP(s,("verification/experience/"+name+".bmp").c_str())==0);SDL_FreeSurface(s);
    }
    static bool overlap(SDL_Rect a,SDL_Rect b){return SDL_HasIntersection(&a,&b);}
    static void geometry(PaperNxApp &app,int w,int h) {
        ui::SettingsLayout settings(w,h);SDL_Rect plus{w-80,8,64,64};
        assert(!overlap(settings.close,plus));assert(!overlap(settings.content,settings.footer));assert(!overlap(settings.scope,settings.content));
        assert(!overlap(settings.reset,settings.footer));assert(settings.content.h>=64);
        for(int i=0;i<6;++i){auto r=settings.tab(i);assert(r.w>=56 && r.h>=56);assert(!overlap(r,settings.scope) && !overlap(r,settings.content));}
        ui::NavigationLayout contents(w,h);assert(!overlap(contents.close,plus));
        for(int i=0;i<3;++i){assert(!overlap(contents.tab(i),contents.content));assert(contents.tab(i).h>=56);}
        ui::ShellLayout shell(w,h);auto library=app.libraryLayout(w);
        for(int i=0;i<5;++i){auto r=shell.nav(i);assert(r.w>=56 && r.h>=56);assert(!overlap(r,{library.left,library.top,w-library.left-24,app.libraryBottom(w,h)-library.top}));}
        ui::BrowserLayout browser(w,h);assert(browser.content.h>=56 && browser.content.w>=56);assert(!overlap(browser.import,browser.content));
        app.quickMenu_=true;
        for(int i=0;i<int(app.quickActions().size());++i){auto r=app.quickActionRect(i,w,h);assert(r.w>=56 && r.h>=56);}
        app.quickMenu_=false;
    }
    static void firstLaunch(const fs::path &root) {
        PaperNxApp app(root);assert(initReady(app));assert(app.books_.empty());
        for(auto size:{std::pair<int,int>{1280,720},{720,1280}}){SDL_SetWindowSize(app.window_,size.first,size.second);app.libraryHome_=true;app.screen_=ScreenState::Library;
            shot(app,"welcome-"+std::to_string(size.first));
            for(const auto &item:app.navigationFocus_.items())assert(item.id.rfind("category.",0)!=0);
            tap(app,app.welcomeCard(0,size.first,size.second));assert(app.screen_==ScreenState::FileBrowser);app.shellNavigate(0);
            tap(app,app.welcomeCard(1,size.first,size.second));assert(app.screen_==ScreenState::AaModal);app.dismissSettings();
            tap(app,app.welcomeCard(2,size.first,size.second));assert(app.screen_==ScreenState::ReadingJournal);app.shellNavigate(0);
        }
    }
    static void run(const fs::path &root) {
        PaperNxApp app(root);assert(initReady(app));ui::language=1;
        assert(app.books_.size()>=4);
        const auto hebrew=*std::find_if(app.books_.begin(),app.books_.end(),[](const Book &b){return fs::path(b.path).filename()=="hebrew.epub";});
        auto saved=app.progressStore_.get(hebrew.id);saved.lastRead=std::time(nullptr);saved.fraction=.35;app.progressStore_.put(hebrew.id,saved);
        app.render();for(int i=0;i<80;++i){app.processCompletedThumbnails();app.render();SDL_Delay(3);}
        for(auto size:{std::pair<int,int>{1280,720},{720,1280},{640,480},{1920,1080}}) {
            const int w=size.first,h=size.second;SDL_SetWindowSize(app.window_,w,h);app.screen_=ScreenState::Library;app.libraryHome_=true;app.category_=Category::All;app.updateVisibleBooks();
            geometry(app,w,h);const auto suffix=std::to_string(w)+"x"+std::to_string(h);
            shot(app,"home-"+suffix);ui::ShellLayout shell(w,h);
            tap(app,shell.nav(1));assert(!app.libraryHome_ && app.screen_==ScreenState::Library);shot(app,"library-"+suffix);
            tap(app,shell.nav(2));assert(app.catalogPanel_==PaperNxApp::CatalogPanel::Browse);shot(app,"collections-"+suffix);app.catalogPanel_=PaperNxApp::CatalogPanel::None;
            tap(app,shell.nav(4));assert(app.screen_==ScreenState::FileBrowser);shot(app,"import-"+suffix);
            tap(app,shell.nav(0));assert(app.screen_==ScreenState::Library && app.libraryHome_);
            app.openSettings(true,5);shot(app,"settings-"+suffix);
            for(int tab=0;tab<6;++tab){tap(app,ui::SettingsLayout(w,h).tab(tab));assert(app.aaTab_==tab);}
            tap(app,ui::SettingsLayout(w,h).footer);assert(app.screen_==ScreenState::Library);
            tap(app,shell.nav(3));assert(app.screen_==ScreenState::ReadingJournal);shot(app,"journal-"+suffix);
            tap(app,shell.nav(0));assert(app.screen_==ScreenState::Library);
            app.quickMenu_=true;app.quickScroll_=0;shot(app,"actions-"+suffix);app.quickScroll_=app.maxUiScroll();shot(app,"actions-end-"+suffix);app.quickMenu_=false;
        }
        SDL_SetWindowSize(app.window_,1280,720);app.openBook(hebrew);assert(app.doc_ && app.readingDirection()==-1);app.currentPage_=2;
        auto position=app.currentPage_;app.toolbarVisible_=true;shot(app,"hebrew-reader-tools");
        tap(app,app.readerHeaderButton(2,1280));assert(app.screen_==ScreenState::BookmarksModal);shot(app,"saved-places-empty");
        tap(app,app.bookmarkAdd(1280,720));assert(app.progressStore_.get(hebrew.id).bookmarks.size()==1);shot(app,"saved-places");
        tap(app,ui::NavigationLayout(1280,720).close);assert(app.screen_==ScreenState::Reader && app.currentPage_==position);
        tap(app,app.readerHeaderButton(1,1280));assert(app.screen_==ScreenState::TocModal);
        for(int i=0;i<3;++i){tap(app,app.tocTabRect(i,1280));assert(app.tocTab_==i);shot(app,"contents-"+std::to_string(i));}
        app.closeContents();assert(app.currentPage_==position);app.openSettings(false,0);shot(app,"reader-type");tap(app,ui::SettingsLayout(1280,720).tab(2));shot(app,"reader-paper");app.dismissSettings();
        app.quickMenu_=app.helpMenu_=true;assert(app.maxUiScroll()>0);shot(app,"reader-guide");app.quickScroll_=app.maxUiScroll();shot(app,"reader-guide-end");app.quickMenu_=app.helpMenu_=false;app.quickScroll_=0;
        app.loadingScreen_=true;app.loadingTitle_="PaperNX";app.loadingDetail_=ui::label("Preparing your library...");app.loadingProgress_=.7;shot(app,"loading");app.loadingScreen_=false;
        app.closeBook();assert(app.resumeBook() && app.resumeBook()->id==hebrew.id);app.shellNavigate(3);
        for(int i=0;i<4;++i){app.journalUI_.setTab(i);shot(app,"journal-tab-"+std::to_string(i));}
        app.journalUI_.openGoals();shot(app,"journal-goals");app.journalUI_.back();app.journalUI_.openExport();shot(app,"journal-export");app.journalUI_.back();app.shellNavigate(0);
        app.actionBook_=hebrew;app.setCatalogPanel(PaperNxApp::CatalogPanel::Actions);shot(app,"book-actions");app.setCatalogPanel(PaperNxApp::CatalogPanel::Details);shot(app,"book-details");app.setCatalogPanel(PaperNxApp::CatalogPanel::Metadata);shot(app,"book-metadata");app.catalogPanel_=PaperNxApp::CatalogPanel::None;
        app.editColor(1);shot(app,"color-palette");app.closeColor(false);
        for(int kind:{0,1,6,7,8,9,10}){app.openChoice(kind,0);shot(app,"choice-"+std::to_string(kind));app.choiceKind_=-1;}
        app.actionBook_=hebrew;app.setCatalogPanel(PaperNxApp::CatalogPanel::Metadata);app.requestCatalogText(0,"Title");
        for(auto size:{std::pair<int,int>{640,480},{1280,720}}){SDL_SetWindowSize(app.window_,size.first,size.second);const auto box=app.catalogPanelRect(size.first,size.second);
            const SDL_Rect footer{box.x+20,box.y+box.h-76,box.w-40,56};
            for(int i=0;i<int(app.catalogKeyboard().size());++i){auto r=app.catalogKeyRect(i,size.first,size.second);assert(r.w>=56 && r.h>=56 && !overlap(r,footer));assert(ui::contains(box,r.x,r.y) && ui::contains(box,r.x+r.w-1,r.y+r.h-1));}
            const auto before=app.inputText_;tap(app,app.catalogKeyRect(0,size.first,size.second));assert(app.inputText_==before+"a");shot(app,"metadata-keyboard-"+std::to_string(size.first));
        }
        app.closeCatalogPanel();app.catalogPanel_=PaperNxApp::CatalogPanel::None;
        ui::language=0;app.openSettings(true,5);shot(app,"settings-turkish");app.dismissSettings();ui::language=1;
        std::cout<<"PASS: full app-shell navigation, non-overlapping responsive layouts, all settings sections, loading, import, journal, action grid, book tools and unchanged reading position\n";
    }
};
}
int main() {
    assert(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_EVENTS|SDL_INIT_GAMECONTROLLER)==0 && TTF_Init()==0);
    const auto root=fs::temp_directory_path()/"papernx-experience";fs::remove_all(root);fs::create_directories(root/"books");fs::create_directory_symlink(fs::absolute("fonts"),root/"fonts");
    for(auto files:{std::pair<const char*,const char*>{"hebrew_navigation.epub","hebrew.epub"},{"sample_book.epub","stories.epub"},{"sample_doc.pdf","notes.pdf"},{"comics/smart-spreads.cbz","comic.cbz"},{"comics/manga.cbz","manga.cbz"}})fs::copy_file(fs::path("tests/fixtures")/files.first,root/"books"/files.second);
    AppTest::run(root);fs::remove_all(root);
    fs::create_directories(root/"books");fs::create_directory_symlink(fs::absolute("fonts"),root/"fonts");AppTest::firstLaunch(root);fs::remove_all(root);TTF_Quit();SDL_Quit();
}
