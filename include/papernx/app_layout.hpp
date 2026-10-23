#pragma once
#include <SDL.h>
#include <algorithm>
namespace papernx::ui {
// One geometry model for painting, touch, controller focus and tests.
struct ShellLayout {
    int w,h,left,bottom;bool rail;
    ShellLayout(int width,int height):w(width),h(height),left(width>=1000?224:24),bottom(width>=1000?32:96),rail(width>=1000){}
    SDL_Rect nav(int i) const {return rail?SDL_Rect{16,148+i*70,184,60}:SDL_Rect{8+i*((w-16)/5),h-88,(w-16)/5-4,72};}
    SDL_Rect settings() const {return rail?SDL_Rect{16,h-152,184,60}:SDL_Rect{w-152,16,56,56};}
    SDL_Rect headerAction(int i) const {return {w-224+i*64,16,56,56};}
    SDL_Rect body() const {return {left,104,w-left-24,h-104-bottom};}
};
struct BrowserLayout {
    ShellLayout shell;SDL_Rect content,import,up,back,path,info;int rowHeight{80};
    BrowserLayout(int w,int h):shell(w,h) {
        const int right=shell.rail?288:0;
        content={shell.left,192,w-shell.left-24-right,h-216-shell.bottom-(shell.rail?0:72)};
        path={shell.left,112,w-shell.left-24,56};
        up={shell.left,112,56,56};back={shell.left,16,56,56};
        info={w-288,192,264,std::max(260,h-224-shell.bottom)};
        import=shell.rail?SDL_Rect{info.x+16,info.y+info.h-80,info.w-32,56}:SDL_Rect{shell.left,h-shell.bottom-76,w-shell.left-24,56};
    }
    SDL_Rect row(int i,float scroll) const {return {content.x,content.y+i*rowHeight-int(scroll),content.w,rowHeight-8};}
};
struct NavigationLayout {
    SDL_Rect sheet,close,content;bool rail;int width;
    NavigationLayout(int w,int h):rail(w>=1000),width(w) {
        const int sw=std::min(1120,w-32);sheet={(w-sw)/2,32,sw,h-64};
        close={sheet.x+sheet.w-152,sheet.y+16,56,56};
        const int x=sheet.x+(rail?232:24),top=sheet.y+(rail?112:184);
        content={x,top,sheet.x+sheet.w-x-24,sheet.y+sheet.h-top-24};
    }
    SDL_Rect tab(int i) const {return rail?SDL_Rect{sheet.x+16,sheet.y+112+i*72,192,64}:SDL_Rect{sheet.x+24+i*((sheet.w-48)/3),sheet.y+108,(sheet.w-48)/3-8,56};}
};
struct LibraryLayout {
    int columns,cardWidth,cardHeight{294},top,stepX,stepY{318},left,right;
    bool home;
    LibraryLayout(int w,bool landing=true):left(w>=1000?224:24),right(24),home(landing) {
        top=landing?(w>=1000?388:436):(w>=1000?204:252);
        columns=std::max(1,(w-left-right)/204);stepX=(w-left-right)/columns;cardWidth=stepX-20;
    }
    SDL_Rect category(int i,int w) const {
        const int cols=w>=1000?6:3,cw=(w-left-right)/cols;
        return {left+(i%cols)*cw,top-(w>=1000?84:132)+(i/cols)*60,cw-8,56};
    }
    SDL_Rect hero(int w) const {return {left,100,w-left-right,184};}
    SDL_Rect card(int i,float scroll) const {return {left+(i%columns)*stepX,top+(i/columns)*stepY-int(scroll),cardWidth,cardHeight};}
};
} // namespace papernx::ui
