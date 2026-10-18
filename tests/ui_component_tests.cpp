#include "papernx/ui.hpp"
#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>

namespace {
double luminance(SDL_Color c) {
    auto channel=[](int n) { double v=n/255.;return v<=.04045?v/12.92:std::pow((v+.055)/1.055,2.4); };
    return .2126*channel(c.r)+.7152*channel(c.g)+.0722*channel(c.b);
}
double contrast(SDL_Color a,SDL_Color b) {
    double x=luminance(a),y=luminance(b);return (std::max(x,y)+.05)/(std::min(x,y)+.05);
}
SDL_Color rgb(int v) { return {Uint8(v>>16),Uint8(v>>8),Uint8(v),255}; }
}
int main() {
    using namespace papernx;
    assert(SDL_Init(SDL_INIT_VIDEO)==0);assert(TTF_Init()==0);
    auto *surface=SDL_CreateRGBSurfaceWithFormat(0,360,140,32,SDL_PIXELFORMAT_ARGB8888); assert(surface);
    auto *renderer=SDL_CreateSoftwareRenderer(surface);assert(renderer);
    {
        Typography type(".");assert(type.init());
        auto draw=[&](const ThemePalette &pal,bool selected,bool pressed,bool focused,bool editing=false,bool disabled=false) {
            SDL_SetRenderDrawColor(renderer,pal.bg.r,pal.bg.g,pal.bg.b,255);SDL_RenderClear(renderer);
            ui::button(renderer,type,{20,20,260,64},"Okuma ayarları",pal,selected,pressed,false,focused,editing,disabled);
            SDL_RenderPresent(renderer);
            std::vector<Uint32> pixels(360*140);assert(SDL_RenderReadPixels(renderer,nullptr,SDL_PIXELFORMAT_ARGB8888,pixels.data(),360*4)==0);return pixels;
        };
        for(const auto &pal:g_palettes) {
            assert(contrast(pal.bg,pal.fg)>=4.5);
            assert(contrast(pal.accent,pal.bg)>=4.5);
            auto normal=draw(pal,false,false,false),selected=draw(pal,true,false,false);
            auto focused=draw(pal,false,false,true),pressed=draw(pal,false,true,false);
            assert(normal!=focused && normal!=pressed && normal!=selected);
            assert(selected!=draw(pal,true,false,true));
            assert(selected!=draw(pal,true,true,false));
            assert(focused!=draw(pal,false,false,true,true));
            assert(normal!=draw(pal,false,false,false,false,true));
        }
        for(const auto &preset:g_paperPresets) {
            assert(contrast(rgb(preset.background),rgb(preset.text))>=4.5);
            ReaderPreferences style;style.theme=preset.theme;style.backgroundColor=preset.background;style.textColor=preset.text;
            auto chrome=readerChromePalette(style,false,0,0);
            assert(contrast(chrome.header,chrome.fg)>=4.5);
            assert(contrast(chrome.accent,chrome.bg)>=4.5);
        }
        for(int width:{64,120,320,560}) {
            ui::StepperLayout l({10,20,width,64});
            for(auto r:{l.minus,l.value,l.plus})assert(r.x>=10 && r.x+r.w<=10+width && r.w>=0);
            assert(l.hit(l.minus.x+l.minus.w/2,40)==-1);
            assert(l.hit(l.plus.x+l.plus.w/2,40)==1);
        }
        // Ellipsis is clipped to the actual control width, including on cache hits.
        for(int repeat=0;repeat<2;++repeat) {
            SDL_SetRenderDrawColor(renderer,255,255,255,255);SDL_RenderClear(renderer);
            type.renderText(renderer,type.uiFont(),"Türkçe küçük alan",20,20,{0,0,0,255},4);
            SDL_RenderPresent(renderer);
            std::vector<Uint32> pixels(360*140);SDL_RenderReadPixels(renderer,nullptr,SDL_PIXELFORMAT_ARGB8888,pixels.data(),360*4);
            for(int y=0;y<140;++y)for(int x=24;x<360;++x)assert(pixels[y*360+x]==0xffffffffu);
        }
    }
    SDL_DestroyRenderer(renderer);SDL_FreeSurface(surface);TTF_Quit();SDL_Quit();
    std::cout<<"UI states, nine preset contrast, narrow steppers and UTF-8 clipping PASS\n";
}
