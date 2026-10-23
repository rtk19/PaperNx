#pragma once
#include "papernx/typography.hpp"
#include "papernx/theme.hpp"
#include "papernx/localization.hpp"
#include "papernx/app_layout.hpp"
#include <algorithm>
namespace papernx::ui {
// Native touch design tokens, in output pixels. Logical sizes scale with layout.
inline constexpr int touch = 56, gap = 16, radius = 12, row = 64;
struct Tokens {
    static constexpr int title=30,section=24,label=22,metadata=18,description=17;
    static constexpr int spaceSmall=8,space=16,spaceLarge=24,border=1,focusBorder=3;
    static constexpr int headerHeight=80,footerHeight=64,dialogMaxWidth=920,coverWidth=154,coverHeight=218;
};
inline int pointerX=0,pointerY=0;
inline bool pointerDown=false;
inline int language=1;
inline std::string label(const std::string &text) { return localizeUiText(text,language); }
inline void pointer(int x,int y,bool down) { pointerX=x; pointerY=y; pointerDown=down; }
inline bool contains(SDL_Rect r, int x, int y) { return x>=r.x && x<r.x+r.w && y>=r.y && y<r.y+r.h; }
inline void surface(SDL_Renderer *r, SDL_Rect rect, SDL_Color color, int corner=radius) {
    if(rect.w<=0 || rect.h<=0) return;
    corner=std::min({corner,rect.w/2,rect.h/2});
    SDL_SetRenderDrawColor(r,color.r,color.g,color.b,color.a);
    SDL_Rect center{rect.x,rect.y+corner,rect.w,rect.h-corner*2}; SDL_RenderFillRect(r,&center);
    for(int y=0;y<corner;++y) {
        float d=corner-y-.5f; int inset=corner-int(std::sqrt(float(corner*corner)-d*d));
        SDL_RenderDrawLine(r,rect.x+inset,rect.y+y,rect.x+rect.w-1-inset,rect.y+y);
        SDL_RenderDrawLine(r,rect.x+inset,rect.y+rect.h-1-y,rect.x+rect.w-1-inset,rect.y+rect.h-1-y);
    }
}
inline void focusRing(SDL_Renderer *r,SDL_Rect rect,const ThemePalette &pal,bool editing=false) {
    auto color=pal.accent;
    SDL_SetRenderDrawColor(r,color.r,color.g,color.b,255);
    for(int i=0;i<Tokens::focusBorder;++i) {
        const int radius=std::min({14,rect.w/2,rect.h/2});SDL_Point points[37];int n=0;
        const int xs[]={rect.x+rect.w-radius-i,rect.x+radius+i,rect.x+radius+i,rect.x+rect.w-radius-i};
        const int ys[]={rect.y+rect.h-radius-i,rect.y+rect.h-radius-i,rect.y+radius+i,rect.y+radius+i};
        for(int corner=0;corner<4;++corner)for(int step=0;step<9;++step) {
            const float angle=(corner*90+step*90.f/8)*3.14159265f/180;
            points[n++]={xs[corner]+int(std::round((radius-i)*std::cos(angle))),ys[corner]+int(std::round((radius-i)*std::sin(angle)))};
        }
        points[n++]=points[0];SDL_RenderDrawLines(r,points,n);
    }
    if(editing) { SDL_Rect underline{rect.x+8,rect.y+rect.h-7,rect.w-16,3}; SDL_RenderFillRect(r,&underline); }
}
inline void elevated(SDL_Renderer *renderer,SDL_Rect rect,const ThemePalette &pal,int corner=18) {
    SDL_BlendMode mode;SDL_GetRenderDrawBlendMode(renderer,&mode);SDL_SetRenderDrawBlendMode(renderer,SDL_BLENDMODE_BLEND);
    surface(renderer,{rect.x+2,rect.y+5,rect.w,rect.h},{0,0,0,36},corner);
    SDL_SetRenderDrawBlendMode(renderer,mode);
    surface(renderer,rect,pal.cardBorder,corner);
    surface(renderer,{rect.x+1,rect.y+1,rect.w-2,rect.h-2},pal.card,corner-1);
}
inline SDL_Color destructiveColor(const ThemePalette &pal) {
    return int(pal.bg.r)+pal.bg.g+pal.bg.b<384?SDL_Color{255,160,150,255}:SDL_Color{155,36,32,255};
}
inline void button(SDL_Renderer *r, Typography &type, SDL_Rect rect, const std::string &text,
                   const ThemePalette &pal, bool selected=false, bool pressed=false,bool translate=true,
                   bool focused=false,bool editing=false,bool disabled=false,bool destructive=false) {
    auto label=translate?ui::label(text):text;
    pressed=!disabled && (pressed || (pointerDown && contains(rect,pointerX,pointerY)));
    auto fill=selected?(pressed?blendColor(pal.accent,pal.bg,12):pal.accent):pressed?blendColor(pal.header,pal.accent,20):focused?pal.card:pal.header;
    surface(r,rect,selected?fill:pal.cardBorder,16);
    surface(r,{rect.x+1,rect.y+1,rect.w-2,rect.h-2},fill,15);
    if(focused || editing) focusRing(r,rect,pal,editing);
    int tw=0,th=0; type.measureText(type.buttonFont(),label,&tw,&th,rect.w-24);
    const SDL_Color white{255,255,255,255},black{16,18,25,255};
    const auto selectedInk=readingContrast(fill,white)>=readingContrast(fill,black)?white:black;
    type.renderText(r,type.buttonFont(),label,rect.x+std::max(12,(rect.w-tw)/2),rect.y+(rect.h-th)/2,
                    disabled?pal.muted:selected?selectedInk:destructive?destructiveColor(pal):pal.fg,rect.w-24);
}
// Professional e-reader list action: left-aligned label, separate focus/selection,
// restrained divider, and consistent typography across modal and quick-action lists.
inline void menuRow(SDL_Renderer *r,Typography &type,SDL_Rect rect,const std::string &rawLabel,
                    const ThemePalette &pal,bool selected=false,bool focused=false,bool destructive=false) {
    if(rect.w<=0||rect.h<=0)return;
    const std::string label = ui::label(rawLabel);
    const auto fill=selected?blendColor(pal.card,pal.accent,20):(pointerDown&&contains(rect,pointerX,pointerY)?pal.header:pal.card);
    surface(r,rect,fill,14);
    if(selected)surface(r,{rect.x+5,rect.y+14,3,rect.h-28},pal.accent,1);
    int textHeight=0;
    type.measureText(type.buttonFont(),label,nullptr,&textHeight,rect.w-76);
    const auto color=destructive?destructiveColor(pal):pal.fg;
    type.renderText(r,type.buttonFont(),label,rect.x+20,rect.y+std::max(0,(rect.h-textHeight)/2),color,rect.w-76);
    SDL_SetRenderDrawColor(r,pal.muted.r,pal.muted.g,pal.muted.b,255);
    const int x=rect.x+rect.w-28,y=rect.y+rect.h/2;
    SDL_RenderDrawLine(r,x-3,y-5,x+2,y);SDL_RenderDrawLine(r,x+2,y,x-3,y+5);
    if(focused)focusRing(r,rect,pal);
}
inline void preferenceRow(SDL_Renderer *r,Typography &type,SDL_Rect rect,const std::string &text,
                          const ThemePalette &pal,bool selected=false) {
    const auto label=ui::label(text);surface(r,rect,selected?blendColor(pal.header,pal.accent,16):pal.header,14);
    auto split=label.rfind(": ");std::string title=split==std::string::npos?label:label.substr(0,split);
    std::string value=split==std::string::npos?"":label.substr(split+2);
    const bool on=value=="On" || value=="Açık",off=value=="Off" || value=="Kapalı";
    int th=0;type.measureText(type.uiFont(),title,nullptr,&th);
    int valueWidth=0;type.measureText(type.smallFont(),value,&valueWidth,nullptr);
    valueWidth=std::min(std::max(on||off?72:0,valueWidth+20),rect.w/2);
    type.renderText(r,type.uiFont(),title,rect.x+18,rect.y+(rect.h-th)/2,pal.fg,std::max(1,rect.w-42-valueWidth));
    if(on || off) {
        SDL_Rect track{rect.x+rect.w-70,rect.y+(rect.h-28)/2,52,28};surface(r,track,on?pal.accent:pal.cardBorder,14);
        surface(r,{track.x+(on?26:4),track.y+4,20,20},{255,255,255,255},10);
    } else if(!value.empty()) {
        type.measureText(type.smallFont(),value,nullptr,&th);
        type.renderText(r,type.smallFont(),value,rect.x+rect.w-valueWidth,rect.y+(rect.h-th)/2,pal.muted,valueWidth-12);
    } else {
        const int x=rect.x+rect.w-28,y=rect.y+rect.h/2;
        SDL_SetRenderDrawColor(r,pal.muted.r,pal.muted.g,pal.muted.b,255);
        if(selected){SDL_Rect dot{x-5,y-5,10,10};surface(r,dot,pal.accent,5);}
        else {SDL_RenderDrawLine(r,x-3,y-5,x+2,y);SDL_RenderDrawLine(r,x+2,y,x-3,y+5);}
    }
}
struct StepperLayout {
    SDL_Rect label,minus,value,plus;
    explicit StepperLayout(SDL_Rect r) {
        int size=std::min({touch,std::max(0,r.h),std::max(0,r.w)/3});
        int valueWidth=std::min(80,std::max(0,r.w-2*size));
        plus={r.x+r.w-size,r.y,size,r.h}; value={plus.x-valueWidth,r.y,valueWidth,r.h};
        minus={value.x-size,r.y,size,r.h}; label={r.x+12,r.y,std::max(0,minus.x-r.x-24),r.h};
    }
    int hit(int x,int y) const { return contains(minus,x,y)?-1:contains(plus,x,y)?1:0; }
};
inline void settingsRow(SDL_Renderer *r,Typography &type,SDL_Rect rect,const std::string &label,
                        const std::string &value,const ThemePalette &pal,bool focused=false,bool editing=false) {
    surface(r,rect,focused?pal.card:pal.header,14);
    StepperLayout layout(rect); int height=0; type.measureText(type.uiFont(),ui::label(label),nullptr,&height);
    if(layout.label.w>0) type.renderText(r,type.uiFont(),ui::label(label),layout.label.x,rect.y+(rect.h-height)/2,pal.fg,layout.label.w);
    button(r,type,layout.minus,"-",pal,false,false,false);
    button(r,type,layout.plus,"+",pal,false,false,false);
    int width=0; type.measureText(type.buttonFont(),value,&width,&height);
    type.renderText(r,type.buttonFont(),value,layout.value.x+std::max(0,(layout.value.w-width)/2),rect.y+(rect.h-height)/2,pal.fg,layout.value.w);
    if(focused || editing) focusRing(r,rect,pal,editing);
}
inline void favorite(SDL_Renderer *r,SDL_Rect rect,const ThemePalette &pal,bool selected) {
    surface(r,rect,selected?pal.accent:(pointerDown&&contains(rect,pointerX,pointerY)?pal.cardBorder:pal.header));
    auto color=selected?pal.bg:pal.accent; SDL_SetRenderDrawColor(r,color.r,color.g,color.b,255);
    SDL_Point points[11];
    for(int i=0;i<10;++i) {
        float angle=float(i)*3.14159265f/5-3.14159265f/2, radius=i%2?8.f:18.f;
        points[i]={rect.x+rect.w/2+int(std::round(std::cos(angle)*radius)),rect.y+rect.h/2+int(std::round(std::sin(angle)*radius))};
    }
    points[10]=points[0]; SDL_RenderDrawLines(r,points,11);
}
struct SettingsLayout {
    SDL_Rect sheet,close,scope,content,preview,footer,reset,heading;
    int sidebar;bool compact;
    SettingsLayout(int w,int h):sidebar(w>=1000?224:0),compact(w<1000) {
        sheet={16,16,w-32,h-32};close={w-168,24,56,56};heading={48,32,w-240,64};
        const int top=compact?312:164,x=compact?40:272;
        const int previewW=w>=1180?248:0;
        scope={x,compact?236:100,w-x-48,56};
        content={x,top,w-x-48-(previewW?previewW+24:0),h-top-96};
        preview={content.x+content.w+24,top,previewW,content.h};
        footer={w-264,h-88,216,56};reset={x,h-88,std::min(216,w-x-304),56};
    }
    SDL_Rect tab(int i) const {return compact?SDL_Rect{40+(i%3)*((sheet.w-48)/3),104+(i/3)*60,(sheet.w-48)/3-8,56}:SDL_Rect{32,112+i*68,208,60};}
    SDL_Rect control(int i) const {return {content.x,content.y+i*72,content.w,64};}
};
} // namespace papernx::ui
