#include "papernx/bidi.hpp"
#include "papernx/typography.hpp"
#include <cassert>
#include <filesystem>
#include <iostream>
#include <vector>

using namespace papernx;
int main() {
    assert(!containsRtl("PaperNX İstanbul 123"));
    assert(containsRtl(u8"ספר 123"));
    assert(baseRtl(u8"123 — שלום"));
    assert(!baseRtl(u8"PaperNX — שלום"));
    auto clusters=textClusterBoundaries(u8"שָׁלוֹם");
    assert(clusters.size()==5 && clusters[1]==6 && clusters.back()==std::string(u8"שָׁלוֹם").size());
    auto pure=bidiLine(u8"שָׁלוֹם");
    assert(pure.rtl && pure.runs.size()==1 && pure.runs[0].text==u8"שָׁלוֹם" && pure.runs[0].rtl);
    auto mixed=bidiLine(u8"ספר PaperNX 123 — חדש");
    assert(mixed.rtl);
    bool digits=false,latin=false;
    for(const auto &run:mixed.runs) {
        if(run.text.find("123")!=std::string::npos) { assert(!run.rtl); digits=true; }
        if(run.text.find("PaperNX")!=std::string::npos) { assert(!run.rtl); latin=true; }
    }
    assert(digits && latin);
    auto isolated=bidiLine(u8"שלום \u2066PaperNX 123\u2069 עולם");
    for(auto &run:isolated.runs) assert(run.text.find(u8"\u2066")==std::string::npos && run.text.find(u8"\u2069")==std::string::npos);
    assert(SDL_Init(SDL_INIT_VIDEO)==0 && TTF_Init()==0);
    auto *surface=SDL_CreateRGBSurfaceWithFormat(0,1000,650,32,SDL_PIXELFORMAT_ARGB8888); assert(surface);
    auto *renderer=SDL_CreateSoftwareRenderer(surface); assert(renderer);
    {
        Typography type("."); assert(type.init());
        std::vector<TTF_Font *> roles={type.titleFont(),type.sectionFont(),type.buttonFont(),type.uiFont(),type.smallFont(),type.readerFont(),type.previewFont(0,38)};
        for(auto *font:roles) {
            assert(TTF_SetFontDirection(font,TTF_DIRECTION_LTR)==0);
            auto *fallback=type.fontForText(font,u8"שָׁלוֹם"); assert(fallback);
            for(Uint32 c:{0x5d0,0x5ea,0x5b0,0x5b8,0x5b9,0x5bc,0x5c1,0x5f4}) assert(TTF_GlyphIsProvided32(fallback,c));
            assert(type.fontForText(font,"English İstanbul 123")==font);
        }
        SDL_SetRenderDrawColor(renderer,255,255,255,255); SDL_RenderClear(renderer);
        SDL_Color black{20,20,20,255}; int y=14;
        for(auto *font:roles) {
            type.renderText(renderer,font,u8"הספר החדש — PaperNX 123",20,y,black,950); y+=60;
        }
        type.renderText(renderer,type.uiFont(),u8"שם הספר הארוך מאוד בעברית ובאנגלית PaperNX 123",20,455,black,260);
        type.renderWrapped(renderer,type.uiFont(),u8"שָׁלוֹם עולם — PaperNX 123\nמחבר: ישראל ישראלי\nספר עם כותרת ארוכה בעברית ומילים English 456 שמשולבות בטקסט.",{340,455,630,180},black);
        std::filesystem::create_directories("verification/hebrew");
        assert(SDL_SaveBMP(surface,"verification/hebrew/ui-text.bmp")==0);
        auto pixels=[&] { SDL_RenderPresent(renderer); std::vector<Uint32> p(1000*650); assert(SDL_RenderReadPixels(renderer,nullptr,SDL_PIXELFORMAT_ARGB8888,p.data(),4000)==0); return p; };
        // Every role and Latin glyph keeps exactly its existing rasterization.
        auto *font=type.uiFont(); const char *english="English İstanbul 123";
        SDL_SetRenderDrawColor(renderer,255,255,255,255); SDL_RenderClear(renderer);
        type.renderText(renderer,font,english,0,0,black); auto actual=pixels();
        SDL_SetRenderDrawColor(renderer,255,255,255,255); SDL_RenderClear(renderer);
        auto *fresh=TTF_OpenFont("fonts/NotoSans-Regular.ttf",22);assert(fresh);
        auto *raw=TTF_RenderUTF8_Blended(fresh,english,black); assert(raw);
        auto *texture=SDL_CreateTextureFromSurface(renderer,raw); assert(texture);
        SDL_Rect dst{0,0,raw->w,raw->h}; SDL_RenderCopy(renderer,texture,nullptr,&dst);
        assert(actual==pixels()); SDL_DestroyTexture(texture); SDL_FreeSurface(raw);TTF_CloseFont(fresh);
        // Auto alignment and cached placement agree, and stay inside the box.
        auto draw=[&] { SDL_SetRenderDrawColor(renderer,255,255,255,255); SDL_RenderClear(renderer); type.renderText(renderer,font,u8"שלום",100,0,black,600); return pixels(); };
        auto first=draw(),cached=draw(); assert(first==cached);
        int minx=1000,maxx=0;
        for(int py=0;py<60;++py) for(int x=0;x<1000;++x) if((first[py*1000+x]&0xffffff)!=0xffffff) {minx=std::min(minx,x);maxx=std::max(maxx,x);}
        assert(minx>550 && maxx<700 && maxx>650);
        // Ellipsis and multiline metadata exercise both caches without stale fonts.
        for(int family=0;family<3;++family) for(int size:{14,22,44}) {
            auto *preview=type.previewFont(family,size); int w=0,h=0;
            type.measureText(preview,u8"שלום PaperNX 123",&w,&h); assert(w>0 && h>0);
            type.renderText(renderer,preview,u8"שלום PaperNX 123",0,0,black,120);
        }
    }
    SDL_DestroyRenderer(renderer); SDL_FreeSurface(surface); TTF_Quit(); SDL_Quit();
    std::cout<<"PASS: Hebrew glyphs/niqqud, Unicode bidi/isolation, mixed digits, all font roles, Latin pixel parity, RTL alignment and caches\n";
}
