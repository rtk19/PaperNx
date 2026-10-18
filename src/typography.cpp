#include "papernx/typography.hpp"
#include "papernx/core.hpp"
#include <algorithm>
#include <iostream>
#include <sstream>

namespace papernx {

Typography::Typography(const std::filesystem::path &root) : root_(root) {}

Typography::~Typography() {
    clearTextCache();
    if(previewFont_) TTF_CloseFont(previewFont_);
    TTF_Font* fonts[] = {titleFont_, uiFont_, smallFont_, readerFont_,sectionFont_,buttonFont_};
    for (int i = 0; i < 6; ++i) {
        if (!fonts[i]) continue;
        TTF_CloseFont(fonts[i]);
        for (int j = i + 1; j < 6; ++j) {
            if (fonts[j] == fonts[i]) {
                fonts[j] = nullptr;
            }
        }
        fonts[i] = nullptr;
    }
    titleFont_ = uiFont_ = smallFont_ = readerFont_ = nullptr;
}

TTF_Font *Typography::findAndLoad(const std::vector<std::string> &candidates, int ptSize) {
    for (const auto &c : candidates) {
        if (c.empty()) continue;
        TTF_Font *f = TTF_OpenFont(c.c_str(), ptSize);
        if (f) return f;
    }
    return nullptr;
}

bool Typography::init() {
    std::vector<std::string> sansCandidates = {
#ifdef __SWITCH__
        "sdmc:/switch/PaperNX/fonts/NotoSans-Regular.ttf",
        "romfs:/fonts/NotoSans-Regular.ttf",
        "sdmc:/switch/PaperNX/fonts/LiberationSans-Regular.ttf",
#endif
        (root_ / "fonts" / "NotoSans-Regular.ttf").string(),
        (root_ / "fonts" / "DejaVuSans.ttf").string(),
        "/usr/share/fonts/truetype/noto/NotoSans-Regular.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/liberation2/LiberationSans-Regular.ttf",
        "C:\\Windows\\Fonts\\arial.ttf",
        "C:\\Windows\\Fonts\\segoeui.ttf"
    };

    std::vector<std::string> serifCandidates = {
#ifdef __SWITCH__
        "sdmc:/switch/PaperNX/fonts/NotoSerif-Regular.ttf",
        "romfs:/fonts/NotoSerif-Regular.ttf",
        "sdmc:/switch/PaperNX/fonts/Literata-Regular.ttf",
#endif
        (root_ / "fonts" / "NotoSerif-Regular.ttf").string(),
        (root_ / "fonts" / "DejaVuSerif.ttf").string(),
        "/usr/share/fonts/truetype/noto/NotoSerif-Regular.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSerif.ttf",
        "/usr/share/fonts/truetype/liberation2/LiberationSerif-Regular.ttf",
        "C:\\Windows\\Fonts\\georgia.ttf",
        "C:\\Windows\\Fonts\\times.ttf"
    };

    sansPaths_=sansCandidates; serifPaths_=serifCandidates;
    std::vector<std::string> boldCandidates={
#ifdef __SWITCH__
        "romfs:/fonts/NotoSans-SemiBold.ttf",
#endif
        (root_/"fonts"/"NotoSans-SemiBold.ttf").string(),
        "/usr/share/fonts/truetype/noto/NotoSans-Bold.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf"};
    titleFont_ = findAndLoad(boldCandidates, 30);
    if(!titleFont_) titleFont_=findAndLoad(sansCandidates,30);
    sectionFont_=findAndLoad(boldCandidates,24);
    buttonFont_=findAndLoad(boldCandidates,21);
    uiFont_ = findAndLoad(sansCandidates, 22);
    smallFont_ = findAndLoad(sansCandidates, 18);
    readerFont_ = findAndLoad(serifCandidates, 24);

    if (!uiFont_) {
        // Fallback: try title font or small font for general UI
        uiFont_ = titleFont_ ? titleFont_ : smallFont_;
    }
    if (!readerFont_) {
        readerFont_ = uiFont_;
    }

    return (uiFont_ != nullptr);
}

TTF_Font *Typography::previewFont(int family,int size) {
    if(family!=previewFamily_ || size!=previewSize_) {
        clearTextCache();
        metricsCache_.clear();
        if(previewFont_) TTF_CloseFont(previewFont_);
        previewFont_=findAndLoad(family==1?sansPaths_:serifPaths_,std::clamp(size,14,44));
        previewFamily_=family; previewSize_=size;
    }
    return previewFont_?previewFont_:readerFont_;
}

int Typography::renderParagraph(SDL_Renderer *r,TTF_Font *font,const std::string &str,
                                SDL_Rect area,SDL_Color color,int lineHeight,bool left,int indent) {
    if(!font || area.w<24) return 0;
    std::istringstream input(str); std::vector<std::vector<std::string>> lines;
    std::vector<std::string> line; std::string word; int width=indent,space=0,height=0;
    measureText(font," ",&space,&height);
    while(input>>word) {
        int ww=0; measureText(font,word,&ww,&height);
        if(!line.empty() && width+space+ww>area.w) { lines.push_back(std::move(line)); line.clear(); width=0; }
        if(!line.empty()) width+=space;
        line.push_back(word); width+=ww;
    }
    if(!line.empty()) lines.push_back(std::move(line));
    for(std::size_t i=0;i<lines.size();++i) {
        int x=area.x+(i?0:indent), y=area.y+int(i)*lineHeight,total=0;
        if(y>=area.y+area.h) break;
        for(const auto &part:lines[i]) { int ww=0; measureText(font,part,&ww,&height); total+=ww; }
        int spacing=space;
        if(!left && i+1<lines.size() && lines[i].size()>1) spacing=std::max(space,(area.w-total-(i?0:indent))/int(lines[i].size()-1));
        for(const auto &part:lines[i]) {
            renderText(r,font,part,x,y,color);
            int ww=0; measureText(font,part,&ww,&height); x+=ww+spacing;
        }
    }
    return int(lines.size())*lineHeight;
}

void Typography::renderText(SDL_Renderer *r, TTF_Font *font, const std::string &str,
                            int x, int y, SDL_Color color, int maxWidth) {
    if (!r || !font || str.empty()) return;
    auto key=textKey(r,font,str,color,maxWidth,false);
    auto cached=textCache_.find(key);
    if(cached!=textCache_.end()) {
        cached->second.tick=++textTick_;
        int width=maxWidth>0?std::min(maxWidth,cached->second.width):cached->second.width;
        SDL_Rect src{0,0,width,cached->second.height},dst{x,y,width,cached->second.height};
        SDL_RenderCopy(r,cached->second.texture,&src,&dst); return;
    }

    std::string textToRender = str;
    if (maxWidth > 0) {
        int w = 0, h = 0;
        if (TTF_SizeUTF8(font, textToRender.c_str(), &w, &h) == 0 && w > maxWidth) {
            std::string ellipsis = "...";
            int ellW = 0;
            TTF_SizeUTF8(font, ellipsis.c_str(), &ellW, nullptr);
            int budgetW = maxWidth - ellW;

            std::vector<std::size_t> boundaries{0};
            for(std::size_t i=1;i<=textToRender.size();++i)
                if(i==textToRender.size() || (static_cast<unsigned char>(textToRender[i])&0xc0)!=0x80) boundaries.push_back(i);
            std::size_t low=0,high=boundaries.size()-1;
            while(low<high) {
                auto middle=(low+high+1)/2;
                auto candidate=textToRender.substr(0,boundaries[middle]);
                if(TTF_SizeUTF8(font,candidate.c_str(),&w,nullptr)==0 && w<=budgetW) low=middle; else high=middle-1;
            }
            textToRender.resize(boundaries[low]); textToRender+=ellipsis;
        }
    }

    if (textToRender.empty()) return;

    SDL_Surface *surface = TTF_RenderUTF8_Blended(font, textToRender.c_str(), color);
    if (!surface) return;

    SDL_Texture *texture = SDL_CreateTextureFromSurface(r, surface);
    if (texture) {
        int width=maxWidth>0?std::min(maxWidth,surface->w):surface->w;
        SDL_Rect src{0,0,width,surface->h},dst{x,y,width,surface->h};
        SDL_RenderCopy(r, texture, &src, &dst);
        storeText(std::move(key),texture,surface->w,surface->h);
    }
    SDL_FreeSurface(surface);
}

void Typography::clearTextCache() {
    for(auto &entry:textCache_) SDL_DestroyTexture(entry.second.texture);
    textCache_.clear(); textCacheBytes_=0;
}
std::string Typography::textKey(SDL_Renderer *r,TTF_Font *font,const std::string &text,SDL_Color color,int width,bool wrap) const {
    unsigned rgba=(unsigned(color.r)<<24)|(unsigned(color.g)<<16)|(unsigned(color.b)<<8)|color.a;
    return std::to_string(reinterpret_cast<std::uintptr_t>(r))+":"+std::to_string(reinterpret_cast<std::uintptr_t>(font))+":"+
        std::to_string(rgba)+":"+std::to_string(width)+(wrap?":w:":":t:")+text;
}
void Typography::storeText(std::string key,SDL_Texture *texture,int width,int height) {
    std::size_t bytes=std::size_t(width)*height*4;
    constexpr std::size_t budget=8u*1024u*1024u;
    if(bytes>budget) { SDL_DestroyTexture(texture); return; }
    while(!textCache_.empty() && (textCache_.size()>=256 || textCacheBytes_+bytes>budget)) {
        auto oldest=std::min_element(textCache_.begin(),textCache_.end(),[](const auto &a,const auto &b){return a.second.tick<b.second.tick;});
        textCacheBytes_-=oldest->second.bytes; SDL_DestroyTexture(oldest->second.texture); textCache_.erase(oldest);
    }
    textCacheBytes_+=bytes; textCache_.emplace(std::move(key),CachedText{texture,width,height,bytes,++textTick_});
}
void Typography::renderWrapped(SDL_Renderer *r,TTF_Font *font,const std::string &str,SDL_Rect rect,SDL_Color color) {
    if(!r || !font || str.empty() || rect.w<1 || rect.h<1) return;
    auto key=textKey(r,font,str,color,rect.w,true); auto found=textCache_.find(key);
    if(found==textCache_.end()) {
        auto *surface=TTF_RenderUTF8_Blended_Wrapped(font,str.c_str(),color,rect.w); if(!surface) return;
        auto *texture=SDL_CreateTextureFromSurface(r,surface);
        int width=surface->w,height=surface->h; SDL_FreeSurface(surface); if(!texture) return;
        // Very long metadata is clipped before caching; avoid huge intermediate textures persisting.
        if(std::size_t(width)*height*4>8u*1024u*1024u) { SDL_DestroyTexture(texture); return; }
        storeText(key,texture,width,height); found=textCache_.find(key); if(found==textCache_.end()) return;
    }
    auto &entry=found->second; entry.tick=++textTick_;
    SDL_Rect src{0,0,std::min(rect.w,entry.width),std::min(rect.h,entry.height)};
    SDL_Rect dst{rect.x,rect.y,src.w,src.h}; SDL_RenderCopy(r,entry.texture,&src,&dst);
}

void Typography::measureText(TTF_Font *font, const std::string &str, int *outW, int *outH, int maxWidth) {
    if (!font || str.empty()) {
        if (outW) *outW = 0;
        if (outH) *outH = 0;
        return;
    }
    auto key=std::to_string(reinterpret_cast<std::uintptr_t>(font))+":"+str;
    auto found=metricsCache_.find(key);
    if(found==metricsCache_.end()) {
        int width=0,height=0;
        TTF_SizeUTF8(font,str.c_str(),&width,&height);
        if(metricsCache_.size()>=2048) metricsCache_.clear();
        found=metricsCache_.emplace(std::move(key),std::make_pair(width,height)).first;
    }
    if(outW) *outW=found->second.first;
    if(outH) *outH=found->second.second;
    if (maxWidth > 0 && outW && *outW > maxWidth) {
        *outW = maxWidth;
    }
}

void Typography::drawBox(SDL_Renderer *r, SDL_Rect rect, SDL_Color fill, SDL_Color border) {
    if (!r) return;
    SDL_SetRenderDrawColor(r, fill.r, fill.g, fill.b, fill.a);
    SDL_RenderFillRect(r, &rect);

    if (border.a > 0) {
        SDL_SetRenderDrawColor(r, border.r, border.g, border.b, border.a);
        SDL_RenderDrawRect(r, &rect);
    }
}

void Typography::drawProgressBar(SDL_Renderer *r, int x, int y, int w, int h, float progress, SDL_Color bg, SDL_Color fg) {
    if (!r || w <= 0 || h <= 0) return;
    SDL_Rect bgRect{x, y, w, h};
    SDL_SetRenderDrawColor(r, bg.r, bg.g, bg.b, bg.a);
    SDL_RenderFillRect(r, &bgRect);

    progress = std::clamp(progress, 0.0f, 1.0f);
    int fgW = static_cast<int>(w * progress);
    if (fgW > 0) {
        SDL_Rect fgRect{x, y, fgW, h};
        SDL_SetRenderDrawColor(r, fg.r, fg.g, fg.b, fg.a);
        SDL_RenderFillRect(r, &fgRect);
    }
}

} // namespace papernx
