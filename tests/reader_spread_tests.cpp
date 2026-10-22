#define main papernx_application_entry
#include "../src/main.cpp"
#undef main
#include <cassert>
#include <iostream>

namespace {
struct AppTest {
    struct Shot {int w,h;std::vector<Uint32> pixels;};
    static Shot read(PaperNxApp &app,SDL_Texture *texture) {
        auto *target=SDL_GetRenderTarget(app.renderer_);
        assert(SDL_SetRenderTarget(app.renderer_,texture)==0);
        Shot s{};SDL_GetRendererOutputSize(app.renderer_,&s.w,&s.h);s.pixels.resize(s.w*s.h);
        assert(SDL_RenderReadPixels(app.renderer_,nullptr,SDL_PIXELFORMAT_ARGB8888,s.pixels.data(),s.w*4)==0);
        SDL_SetRenderTarget(app.renderer_,target);return s;
    }
    static void save(const Shot &s,const std::string &name) {
        fs::create_directories("verification/spreads");
        auto *surface=SDL_CreateRGBSurfaceWithFormatFrom(const_cast<Uint32*>(s.pixels.data()),s.w,s.h,32,s.w*4,SDL_PIXELFORMAT_ARGB8888);assert(surface);
        assert(SDL_SaveBMP(surface,("verification/spreads/"+name+".bmp").c_str())==0);SDL_FreeSurface(surface);
    }
    static bool same(const Shot &a,const Shot &b,SDL_Rect area,int bx=0,int by=0) {
        for(int y=0;y<area.h;++y)for(int x=0;x<area.w;++x)
            if(a.pixels[(area.y+y)*a.w+area.x+x]!=b.pixels[(by+y)*b.w+bx+x])return false;
        return true;
    }
    static void finish(PaperNxApp &app) {
        app.pageTurnAnimation_.started=SDL_GetTicks()-2000;app.tickPageTurn();
    }
    static Shot sheet(PaperNxApp &app,int page) {
        auto l=app.readerLayout();auto t=app.captureReaderSurface(l.leafWidth(),l.height,[&]{app.drawReaderLeaf(page,{0,0,l.leafWidth(),l.height});});assert(t);
        return read(app,t.get());
    }
    static Shot spread(PaperNxApp &app,int page) {
        auto l=app.readerLayout();auto t=app.captureReaderSurface(l.width,l.height,[&]{app.drawReaderSheets(page);});assert(t);
        return read(app,t.get());
    }
    static void run(const fs::path &root) {
        PaperNxApp app(root);assert(app.init());ui::language=1;
        auto book=[&](const char *name){auto i=std::find_if(app.books_.begin(),app.books_.end(),[&](const Book &b){return fs::path(b.path).filename()==name;});assert(i!=app.books_.end());return *i;};
        for(const char *name:{"english.epub","hebrew.epub","comic.cbz","comic.cbr","manga.cbz","single.cbz","sample.pdf","spread.pdf"}) {
            app.openBook(book(name));app.extraStyle_.readingMode=0;app.documentMode_=0;
            for(int theme:{0,5}) {
                app.currentTheme_=theme;app.extraStyle_.backgroundColor=theme==5?0:0xffffff;
                app.extraStyle_.textColor=theme==5?0xeae7df:0x242424;
                if(px_document_is_epub(app.doc_))app.reflowEpub(24);else app.pageCache_.clear();
                app.currentPage_=0;app.toolbarVisible_=false;
                const auto l=app.readerLayout();assert(l.spread && l.width==1280 && l.height==720);
                const int firstSide=app.readingDirection()<0?1:0;
                const auto first=sheet(app,0),rest=spread(app,0);
                // Both content and the folio occupy the correct physical leaf.
                assert(same(rest,first,l.leaf(firstSide)));
                const auto paper=app.palette().bg;const Uint32 bg=(Uint32(paper.r)<<16)|(Uint32(paper.g)<<8)|paper.b;
                int ink=0;for(int y=l.height-40;y<l.height-4;++y)for(int x=first.w/2-20;x<first.w/2+20;++x)ink+=(first.pixels[y*first.w+x]&0xffffff)!=bg;
                assert(ink>10);
                const auto prefix=std::string(name)+"-"+std::to_string(theme);
                save(rest,prefix+"-landscape");
                app.turnPage(-1);assert(app.currentPage_==0 && !app.pageTurnAnimation_.active);
                if(app.pageCount_>2) {
                    const auto before=rest;app.turnPage(1);assert(app.currentPage_==2 && app.pageTurnAnimation_.spread);
                    auto &turn=app.pageTurnAnimation_;const int side=turn.direction>0?1:0;
                    auto front=read(app,turn.previous.get()),back=read(app,turn.back.get());
                    assert(same(before,front,l.leaf(side)));
                    const auto after=spread(app,2);assert(same(after,back,l.leaf(1-side)));
                    // Clearing the LRU cannot destroy paper or its number mid-turn.
                    app.pageCache_.clear();
                    for(float p:{0.f,.15f,.35f,.55f,.8f,1.f}) {
                        auto target=app.captureReaderSurface(l.width,l.height,[&]{
                            PageCurlRenderer::renderSpread(app.renderer_,turn.previous.get(),turn.back.get(),turn.under.get(),turn.viewport,{0,0,l.width,l.height},p,turn.direction,.5f,0);
                        });assert(target);const auto frame=read(app,target.get());
                        if(p==0)assert(frame.pixels==before.pixels);
                        if(p==1 && frame.pixels!=after.pixels){save(frame,prefix+"-unexpected");save(after,prefix+"-expected");
                            int count=0;for(int y=0;y<frame.h;++y)for(int x=0;x<frame.w;++x)if(frame.pixels[y*frame.w+x]!=after.pixels[y*frame.w+x]){if(count++<5)std::cerr<<prefix<<" mismatch "<<x<<","<<y<<" "<<std::hex<<frame.pixels[y*frame.w+x]<<" "<<after.pixels[y*frame.w+x]<<std::dec<<"\n";}
                            std::cerr<<"Differing pixels: "<<count<<" pages="<<app.pageCount_<<" direction="<<turn.direction<<"\n";assert(false);}
                        save(frame,prefix+"-forward-"+std::to_string(int(p*100)));
                    }
                    finish(app);app.turnPage(-1);assert(app.currentPage_==0 && app.pageTurnAnimation_.spread);finish(app);
                    // Queued controller turns advance spreads without skipping leaves.
                    if(app.pageCount_>4){app.turnPage(1);app.turnPage(1);finish(app);assert(app.currentPage_==4);finish(app);}
                    app.currentPage_=app.pageCount_-1;app.turnPage(1);assert(!app.pageTurnAnimation_.active);
                } else {app.turnPage(1);assert(app.currentPage_==0 && !app.pageTurnAnimation_.active);}
                if(app.pageCount_%2) {
                    const auto last=spread(app,app.pageCount_-1);const int blankSide=1-firstSide;
                    const auto rect=l.leaf(blankSide);
                    for(int y=0;y<rect.h;++y)for(int x=rect.x;x<rect.x+rect.w;++x)assert((last.pixels[y*last.w+x]&0xffffff)==bg);
                }
                // Portrait is one leaf; folio pixels are in its owned snapshot.
                SDL_SetWindowSize(app.window_,720,1280);
                if(px_document_is_epub(app.doc_))app.reflowEpub(24);else app.pageCache_.clear();
                app.currentPage_=0;assert(!app.readerLayout().spread);
                const auto portrait=spread(app,0);save(portrait,prefix+"-portrait");
                if(app.pageCount_>1) {
                    app.turnPage(1);assert(app.currentPage_==1 && !app.pageTurnAnimation_.spread);
                    auto captured=read(app,app.pageTurnAnimation_.previous.get());assert(captured.pixels==portrait.pixels);
                    finish(app);
                }
                SDL_SetWindowSize(app.window_,1280,720);
                if(px_document_is_epub(app.doc_))app.reflowEpub(24);else app.pageCache_.clear();
            }
            app.closeBook();
        }
        // A live drag commits a spread, while returning the sheet cancels it.
        app.openBook(book("english.epub"));app.currentPage_=0;app.toolbarVisible_=false;
        app.beginPageTurnTouch(.9f,.5f,9);assert(app.movePageTurnTouch(.4f,.55f,9));
        assert(app.pageTurnAnimation_.targetPage==2 && app.currentPage_==0);
        app.releasePageTurnTouch(.4f,.55f,9);finish(app);assert(app.currentPage_==2);
        app.beginPageTurnTouch(.1f,.5f,9);assert(app.movePageTurnTouch(.6f,.5f,9));
        app.movePageTurnTouch(.11f,.5f,9);app.pageTurnTouch_.velocity=0;
        app.releasePageTurnTouch(.11f,.5f,9);assert(!app.pageTurnAnimation_.complete);finish(app);assert(app.currentPage_==2);
        // Rotation invalidates a held mesh without committing a different leaf.
        app.beginPageTurnTouch(.9f,.5f,9);assert(app.movePageTurnTouch(.4f,.5f,9));
        SDL_SetWindowSize(app.window_,720,1280);app.tickPageTurn();assert(!app.pageTurnAnimation_.active && app.currentPage_==2);
        app.closeBook();
        std::cout<<"PASS: LTR/RTL spreads; EPUB/PDF/CBZ/CBR; owned folios; all four curl faces; exact endpoints; blank final leaf; queued/interactive turns; portrait and rotation cancellation\n";
    }
};
}
int main() {
    assert(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_EVENTS|SDL_INIT_GAMECONTROLLER)==0 && TTF_Init()==0);
    const auto root=fs::temp_directory_path()/"papernx-reader-spreads";fs::remove_all(root);fs::create_directories(root/"books");
    for(auto pair:std::vector<std::pair<std::string,std::string>>{{"sample_book.epub","english.epub"},{"hebrew_navigation.epub","hebrew.epub"},{"comics/navigation.cbz","comic.cbz"},{"comics/sample-rar5.cbr","comic.cbr"},{"comics/manga.cbz","manga.cbz"},{"comics/single-page.cbz","single.cbz"},{"sample_doc.pdf","sample.pdf"},{"spread_doc.pdf","spread.pdf"}})
        fs::copy_file(fs::path("tests/fixtures")/pair.first,root/"books"/pair.second);
    fs::create_directory_symlink(fs::absolute("fonts"),root/"fonts");AppTest::run(root);fs::remove_all(root);TTF_Quit();SDL_Quit();
}
