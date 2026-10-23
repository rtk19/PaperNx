#define main papernx_application_entry
#include "../src/main.cpp"
#undef main
#include <cassert>
#include <iostream>
namespace {
struct AppTest {
#include "library_fixture_helpers.inc"
    static void touch(PaperNxApp &app,Uint32 type,float x,float y) {
        SDL_Event event{};event.type=type;event.tfinger.fingerId=63;event.tfinger.x=x;event.tfinger.y=y;
        bool running=true;app.handleEvent(event,running);assert(running);
    }
    static void finish(PaperNxApp &app) {
        app.pageTurnAnimation_.started=SDL_GetTicks()-2000;app.tickPageTurn();awaitComicTurn(app);
    }
    static std::vector<Uint32> pixels(PaperNxApp &app) {
        app.render();int w,h;app.getWindowSize(w,h);std::vector<Uint32> out(w*h);
        assert(SDL_RenderReadPixels(app.renderer_,nullptr,SDL_PIXELFORMAT_ARGB8888,out.data(),w*4)==0);return out;
    }
    static void screenshot(PaperNxApp &app,const std::string &name) {
        auto data=pixels(app);int w,h;app.getWindowSize(w,h);
        auto *image=SDL_CreateRGBSurfaceWithFormatFrom(data.data(),w,h,32,w*4,SDL_PIXELFORMAT_ARGB8888);assert(image);
        fs::create_directories("verification/reader-input");assert(SDL_SaveBMP(image,("verification/reader-input/"+name+".bmp").c_str())==0);SDL_FreeSurface(image);
    }
    static void run(const fs::path &root) {
        PaperNxApp app(root);assert(initReady(app));ui::language=1;
        auto book=[&](const char *name){auto it=std::find_if(app.books_.begin(),app.books_.end(),[&](const Book &b){return fs::path(b.path).filename()==name;});assert(it!=app.books_.end());return *it;};
        // The whole persistent plus control owns its corners as well as centre.
        app.openBook(book("english.epub"));app.toolbarVisible_=false;
        touch(app,SDL_FINGERDOWN,1260.f/1280,68.f/720);assert(!app.pageTurnTouch_.tracking);
        touch(app,SDL_FINGERUP,1260.f/1280,68.f/720);assert(app.quickMenu_);app.quickMenu_=false;app.closeBook();
        // The 0.16 external menu geometry and scan/settings routes are restored.
        ui::LibraryLayout library(1280);assert(library.top==174 && library.left==32);
        assert(app.quickSheet(1280,720).x==804);
        app.openSettings(true,5);assert(app.aaRow_==0 && app.settingsRows()==15);app.dismissSettings();
        for(const char *name:{"english.epub","hebrew.epub"})for(bool landscape:{true,false}) {
            SDL_SetWindowSize(app.window_,landscape?1280:720,landscape?720:1280);
            app.openBook(book(name));app.reflowEpub(24);const int dir=app.readingDirection();
            for(float y:{.015f,.06f,.5f,.94f,.985f}) {
                app.cancelPageTurn();app.currentPage_=2;app.toolbarVisible_=false;
                const float start=dir>0?.86f:.14f,end=dir>0?.36f:.64f;
                touch(app,SDL_FINGERDOWN,start,y);touch(app,SDL_FINGERMOTION,end,y);
                assert(app.pageTurnAnimation_.active && app.pageTurnAnimation_.dragging);
                assert(app.currentPage_==2 && app.pageTurnAnimation_.interactive);
                const float held=app.pageTurnAnimation_.progress;app.tickPageTurn();assert(app.pageTurnAnimation_.progress==held);
                touch(app,SDL_FINGERUP,end,y);assert(app.pageTurnAnimation_.complete);finish(app);
                assert(app.currentPage_==(landscape?4:3));
            }
            // A visible header remains a button, rather than starting a curl.
            app.toolbarVisible_=true;const auto header=app.readerHeaderButton(1,landscape?1280:720);
            const float x=(header.x+28.f)/(landscape?1280:720),y=36.f/(landscape?720:1280);
            touch(app,SDL_FINGERDOWN,x,y);assert(!app.pageTurnTouch_.tracking);touch(app,SDL_FINGERUP,x,y);
            assert(app.screen_==ScreenState::TocModal);app.closeContents();app.closeBook();
        }
        SDL_SetWindowSize(app.window_,1280,720);app.openBook(book("smart.cbz"));
        app.toolbarVisible_=false;app.setPdfView(0);app.currentPage_=0;app.render();
        // Warm neighbouring images; turning must reuse the next single image.
        app.prefetchPage_=0;app.prefetchStep_=0;
        for(int i=0;i<4;++i){app.prefetchDue_=0;app.idlePrefetch();}
        const auto warmDeadline=SDL_GetTicks()+10000;
        while(app.comicPages_.pending() && SDL_GetTicks()<warmDeadline){app.processComicPages();SDL_Delay(2);}
        app.processComicPages();assert(!app.comicPages_.pending());
        const auto decodes=app.comicTextureBuilds_;preparedTurn(app,1);
        assert(app.pageTurnAnimation_.active && app.pageTurnAnimation_.comicSheet);
        assert(app.comicTextureBuilds_==decodes);finish(app);
        // A touch in the empty outer margin is owned immediately, without
        // folding any background, and grips only after crossing actual paper.
        app.cancelPageTurn();app.currentPage_=0;app.setPdfView(0);app.toolbarVisible_=false;
        const auto bounds=app.comicTurnBounds(0);assert(bounds.x>0 && bounds.x+bounds.w<1280);
        const float start=(bounds.x+bounds.w+20.f)/1280,near=(bounds.x+bounds.w+5.f)/1280;
        touch(app,SDL_FINGERDOWN,start,.5f);touch(app,SDL_FINGERMOTION,near,.5f);
        assert(app.pageTurnTouch_.tracking && !app.pageTurnAnimation_.active);
        const float end=(bounds.x+bounds.w*.2f)/1280;
        touch(app,SDL_FINGERMOTION,end,.5f);
        assert(app.pageTurnAnimation_.active && app.pageTurnAnimation_.dragging && app.pageTurnAnimation_.comicSheet);
        assert(app.pageTurnAnimation_.viewport.y>=72 && app.pageTurnAnimation_.viewport.h<720);
        touch(app,SDL_FINGERUP,end,.5f);finish(app);assert(app.currentPage_==1);
        // A gesture wholly above the artwork consumes no page turn on release.
        app.currentPage_=0;touch(app,SDL_FINGERDOWN,.85f,.01f);touch(app,SDL_FINGERMOTION,.35f,.01f);
        assert(!app.pageTurnAnimation_.active);touch(app,SDL_FINGERUP,.35f,.01f);assert(app.currentPage_==0);
        // Zoomed single image supports controller curls; a
        // horizontal outer-edge drag turns, while an interior drag still pans.
        app.zoomPdf(2,.5f,.5f,true);app.pdfPanY_=0;app.pdfPanX_=0;
        preparedTurn(app,1);assert(app.pageTurnAnimation_.active && app.pageTurnAnimation_.comicSheet);finish(app);
        app.currentPage_=0;app.pdfPanX_=0;touch(app,SDL_FINGERDOWN,.94f,.5f);touch(app,SDL_FINGERMOTION,.3f,.5f);
        assert(app.pageTurnAnimation_.dragging);touch(app,SDL_FINGERUP,.3f,.5f);finish(app);
        app.closeBook();app.openBook(book("paper-bounds.cbz"));app.toolbarVisible_=false;
        for(float zoom:{1.f,1.01f}) {
            app.setPdfView(0);app.currentPage_=0;if(zoom>1)app.zoomPdf(zoom,.5f,.5f,true);
            app.pdfPanX_=app.pdfPanY_=0;
            const auto paper=app.comicTurnBounds(0);assert(paper.y>72 && paper.y+paper.h<676);
            auto frame=pixels(app);const auto bg=app.palette().bg;const auto background=(Uint32(bg.r)<<16)|(Uint32(bg.g)<<8)|bg.b;
            // No artificial spine inside the single image or in its margins.
            const int seam=640; // Single images have no artificial spine, even with spreads enabled.
            assert((frame[(paper.y+paper.h/4)*1280+seam]&0xffffff)==0x2878b9);
            for(int y=73;y<paper.y-2;++y)for(int x=seam-1;x<=seam;++x)assert((frame[y*1280+x]&0xffffff)==background);
            for(int y=paper.y+paper.h+2;y<676;++y)for(int x=seam-1;x<=seam;++x)assert((frame[y*1280+x]&0xffffff)==background);
            screenshot(app,zoom==1?"fit-bounded-spine":"zoom-bounded-spine");
            const auto resting=frame;
            preparedTurn(app,1);auto &turn=app.pageTurnAnimation_;assert(!turn.spread && turn.comicSheet && !turn.under && turn.back);
            assert(app.readerFooterCaption(1280,720)=="1"); // Buttons must not show the incoming number early.
            assert(turn.viewport.y==paper.y && turn.viewport.h==paper.h && turn.viewport.w<1280);
            // Explicit paper reverse, rather than another saturated mirrored
            // image. Read actual rendered pixels and preserve the target state.
            auto pixel=[&](SDL_Texture *texture) {
                int w,h;SDL_QueryTexture(texture,nullptr,nullptr,&w,&h);auto *old=SDL_GetRenderTarget(app.renderer_);
                assert(SDL_SetRenderTarget(app.renderer_,texture)==0);SDL_Rect r{w/4,h/4,1,1};Uint32 rgb=0;
                assert(SDL_RenderReadPixels(app.renderer_,&r,SDL_PIXELFORMAT_ARGB8888,&rgb,4)==0);
                SDL_SetRenderTarget(app.renderer_,old);return rgb;
            };
            const auto front=pixel(turn.previous.get()),reverse=pixel(turn.back.get());
            auto distance=[&](Uint32 rgb){return std::abs(int((rgb>>16)&255)-bg.r)+std::abs(int((rgb>>8)&255)-bg.g)+std::abs(int(rgb&255)-bg.b);};
            assert(front!=reverse && distance(reverse)<distance(front));
            // The physical mesh survives cache eviction. Every footer pixel
            // stays identical through the fold, with just one stationary digit.
            app.pageCache_.clear();turn.dragging=true;
            for(float progress:{0.f,.25f,.5f,.75f,1.f}) {
                turn.progress=progress;frame=pixels(app);
                for(int y=0;y<6;++y)for(int x=0;x<1280;++x)assert((frame[y*1280+x]&0xffffff)==background);
                for(int y=676;y<720;++y)for(int x=0;x<1280;++x)assert(frame[y*1280+x]==resting[y*1280+x]);
                assert(app.readerFooterCaption(1280,720)=="1");
                screenshot(app,std::string(zoom==1?"paper-":"zoom-paper-")+std::to_string(int(progress*100)));
            }
            turn.dragging=false;finish(app);assert(app.readerFooterCaption(1280,720)=="2");
            assert(pixels(app)!=resting);
        }
        // Interactive cancellation keeps the old number. A committed touch
        // updates it only at completion, just like a controller-driven turn.
        app.currentPage_=0;app.setPdfView(0);app.toolbarVisible_=false;
        touch(app,SDL_FINGERDOWN,.92f,.5f);touch(app,SDL_FINGERMOTION,.4f,.5f);awaitComicTurn(app);
        assert(app.pageTurnAnimation_.dragging && app.readerFooterCaption(1280,720)=="1");
        touch(app,SDL_FINGERMOTION,.919f,.5f);app.pageTurnTouch_.velocity=0;
        touch(app,SDL_FINGERUP,.919f,.5f);assert(!app.pageTurnAnimation_.complete);finish(app);
        assert(app.currentPage_==0 && app.readerFooterCaption(1280,720)=="1");
        touch(app,SDL_FINGERDOWN,.92f,.5f);touch(app,SDL_FINGERMOTION,.4f,.5f);touch(app,SDL_FINGERUP,.4f,.5f);
        assert(app.pageTurnAnimation_.complete && app.readerFooterCaption(1280,720)=="1");finish(app);
        assert(app.currentPage_==1 && app.readerFooterCaption(1280,720)=="2");
        preparedTurn(app,-1);assert(app.readerFooterCaption(1280,720)=="2");finish(app);
        assert(app.readerFooterCaption(1280,720)=="1");
        preparedTurn(app,1);app.turnPage(1);assert(app.readerFooterCaption(1280,720)=="1");finish(app);
        assert(app.pageTurnAnimation_.active && app.readerFooterCaption(1280,720)=="2");finish(app);
        assert(app.readerFooterCaption(1280,720)=="3");
        app.closeBook();std::cout<<"PASS: book edge drags; single comic contact and book-style image-only curl, paper reverse, fixed counter until completion/cancellation and zoomed paging\n";
    }
};
}
int main(){assert(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_EVENTS|SDL_INIT_GAMECONTROLLER)==0 && TTF_Init()==0);
 auto root=fs::temp_directory_path()/"papernx-reader-input";fs::remove_all(root);fs::create_directories(root/"books");
 for(auto pair:std::vector<std::pair<std::string,std::string>>{{"sample_book.epub","english.epub"},{"hebrew_navigation.epub","hebrew.epub"},{"comics/smart-spreads.cbz","smart.cbz"},{"comics/paper-bounds.cbz","paper-bounds.cbz"}})fs::copy_file(fs::path("tests/fixtures")/pair.first,root/"books"/pair.second);
 fs::create_directory_symlink(fs::absolute("fonts"),root/"fonts");AppTest::run(root);fs::remove_all(root);TTF_Quit();SDL_Quit();}
