#define main papernx_application_entry
#include "../src/main.cpp"
#undef main
#include <cassert>
#include <iostream>

namespace {
struct AppTest {
#include "library_fixture_helpers.inc"
    static void wait(PaperNxApp &app) {
        const auto end=SDL_GetTicks()+10000;
        while(SDL_GetTicks()<end){app.pollUpdates();auto s=app.updater_->status();if(s.state!=UpdateState::Checking&&s.state!=UpdateState::Downloading){app.pollUpdates();return;}SDL_Delay(2);}assert(false);
    }
    static void pad(PaperNxApp &app,int button){SDL_Event e{};e.type=SDL_CONTROLLERBUTTONDOWN;e.cbutton.button=button;bool running=true;app.handleEvent(e,running);assert(running);}
    static void axis(PaperNxApp &app,int which,int value){SDL_Event e{};e.type=SDL_CONTROLLERAXISMOTION;e.caxis.axis=which;e.caxis.value=Sint16(value);bool running=true;app.handleEvent(e,running);assert(running);}
    static void tap(PaperNxApp &app,SDL_Rect r){
        int pw,ph;SDL_GetWindowSize(app.window_,&pw,&ph);DisplayTransform t{pw,ph,app.appSettings_.get().displayOrientation};auto p=t.toPhysical(r.x+r.w*.5f,r.y+r.h*.5f);
        for(auto type:{SDL_FINGERDOWN,SDL_FINGERUP}){SDL_Event e{};e.type=type;e.tfinger.fingerId=123;e.tfinger.x=p.first/pw;e.tfinger.y=p.second/ph;bool running=true;app.handleEvent(e,running);assert(running);}
    }
    static void snapshot(PaperNxApp &app,const std::string &name){
        app.render();auto *old=SDL_GetRenderTarget(app.renderer_);if(app.orientationTarget_)SDL_SetRenderTarget(app.renderer_,app.orientationTarget_);
        int w,h;SDL_GetRendererOutputSize(app.renderer_,&w,&h);auto *surface=SDL_CreateRGBSurfaceWithFormat(0,w,h,32,SDL_PIXELFORMAT_ARGB8888);assert(surface);
        assert(SDL_RenderReadPixels(app.renderer_,nullptr,surface->format->format,surface->pixels,surface->pitch)==0);fs::create_directories("verification/updates");assert(SDL_SaveBMP(surface,("verification/updates/"+name+".bmp").c_str())==0);SDL_FreeSurface(surface);SDL_SetRenderTarget(app.renderer_,old);
    }
    static void manual(PaperNxApp &app){
        app.quickMenu_=true;int w,h;app.getWindowSize(w,h);auto actions=app.quickActions();assert(actions.back()=="Check for update");int index=int(actions.size())-1;
        app.quickScroll_=app.maxUiScroll();snapshot(app,"plus-menu");tap(app,app.quickActionRect(index,w,h));assert(app.updateDialog_&&!app.quickMenu_);wait(app);
    }
    static void run(const fs::path &root,const std::string &base){
        auto u=std::make_shared<SelfUpdater>();const auto path=root/"Renamed PaperNX.nro";std::ofstream(path)<<"original application";
        u->configure(path,"0.12.0-reader.4",{},UpdateSource{base+"unavailable-api",base+"download/"});
        PaperNxApp app(root,u);assert(initReady(app));ui::language=1;u->check();wait(app);assert(u->status().state==UpdateState::Error&&!app.updateDialog_);
        manual(app);assert(app.updateDialog_&&u->status().state==UpdateState::Error);snapshot(app,"manual-error");pad(app,SDL_CONTROLLER_BUTTON_B);assert(!app.updateDialog_);
        u->configure(path,"0.12.0-reader.4",{},UpdateSource{base+"api",base+"download/"});u->check();wait(app);assert(app.updateDialog_&&u->status().state==UpdateState::Available&&app.updateFocus_==2);
        for(int orientation:{0,1,2}){
            auto settings=app.appSettings_.get();settings.displayOrientation=orientation;app.appSettings_.put(settings);app.render();int w,h;app.getWindowSize(w,h);
            for(int stick:{0,2}){
                app.updateFocus_=2;const int horizontal=orientation==0?stick:stick+1;
                axis(app,horizontal,orientation==2?-32767:32767); // logical right
                axis(app,horizontal,0);app.pollUpdates();
                axis(app,horizontal,orientation==2?32767:-32767);assert(app.updateFocus_==1);axis(app,horizontal,0);app.pollUpdates();
            }
            snapshot(app,"available-"+std::to_string(orientation));tap(app,app.updateButton(1,3,w,h));assert(app.updateNotes_);
            app.updateNotesScroll_=500;snapshot(app,"notes-"+std::to_string(orientation));assert(app.updateNotesScroll_>0);
            tap(app,app.updateButton(0,1,w,h));assert(!app.updateNotes_);pad(app,SDL_CONTROLLER_BUTTON_B);assert(!app.updateDialog_);manual(app);assert(app.updateDialog_);
        }
        app.activateUpdate(2);assert(u->status().state==UpdateState::Downloading);pad(app,SDL_CONTROLLER_BUTTON_B);assert(app.updateDialog_);wait(app);
        assert(u->status().state==UpdateState::Ready);snapshot(app,"ready");app.activateUpdate(0);assert(app.exitRequested_&&app.updateRestart_);
        std::ifstream old(path);std::string original;std::getline(old,original);assert(original=="original application");
        std::cout<<"PASS: silent failed startup, explicit error, plus action, automatic prompt, release notes, rotated touch, modal input isolation, download progress and restart request\n";
    }
};
}
int main(int argc,char **argv){
    assert(argc==3);assert(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_EVENTS|SDL_INIT_GAMECONTROLLER)==0&&TTF_Init()==0);
    fs::path root=argv[1];fs::create_directories(root/"books");fs::create_directory_symlink(fs::absolute("fonts"),root/"fonts");AppTest::run(root,argv[2]);TTF_Quit();SDL_Quit();
}
