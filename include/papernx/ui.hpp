#pragma once
#include "papernx/typography.hpp"
#include "papernx/theme.hpp"
#include "papernx/localization.hpp"
#include <algorithm>
namespace papernx::ui {
// Native touch design tokens, in output pixels. Logical sizes scale with layout.
inline constexpr int touch = 56, gap = 16, radius = 12, row = 64;
inline int pointerX=0,pointerY=0;
inline bool pointerDown=false;
inline int language=0;
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
inline void button(SDL_Renderer *r, Typography &type, SDL_Rect rect, const std::string &text,
                   const ThemePalette &pal, bool selected=false, bool pressed=false,bool translate=true) {
    auto label=translate?ui::label(text):text;
    pressed=pressed || (pointerDown && contains(rect,pointerX,pointerY));
    surface(r,rect,selected?pal.accent:pressed?pal.cardBorder:pal.header);
    int tw=0,th=0; type.measureText(type.uiFont(),label,&tw,&th,rect.w-24);
    type.renderText(r,type.uiFont(),label,rect.x+std::max(12,(rect.w-tw)/2),rect.y+(rect.h-th)/2,
                    selected?pal.bg:pal.fg,rect.w-24);
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
    SDL_Rect sheet, close, scope, content, preview, footer;
    int sidebar;
    SettingsLayout(int w,int h) {
        int inset=w<800?8:24;
        sheet={inset,16,w-2*inset,h-32}; sidebar=w<800?142:190;
        close={sheet.x+sheet.w-208,sheet.y+8,112,56};
        int x=sheet.x+sidebar+24;
        scope={x,sheet.y+72,sheet.w-sidebar-40,56};
        int previewW=w>=1100?280:0;
        content={x,sheet.y+148,sheet.w-sidebar-40-previewW-(previewW?24:0),sheet.h-230};
        preview={x+content.w+24,content.y,previewW,content.h};
        footer={x,sheet.y+sheet.h-72,scope.w,56};
    }
    SDL_Rect tab(int i) const { return {sheet.x+12,sheet.y+80+i*56,sidebar-12,56}; }
    SDL_Rect control(int i) const { return {content.x,content.y+i*64,content.w,56}; }
};
struct LibraryLayout {
    int columns,cardWidth,cardHeight=330,top=174,stepX,stepY=350,left=32;
    LibraryLayout(int w) { top=w<900?238:174; columns=std::max(1,(w-48)/224); stepX=(w-64)/columns; cardWidth=stepX-20; }
};
} // namespace papernx::ui
