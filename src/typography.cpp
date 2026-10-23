#include "papernx/typography.hpp"
#include "papernx/core.hpp"
#include "papernx/bidi.hpp"
#include <fribidi/fribidi.h>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <chrono>

namespace papernx {
namespace {
struct MetricScope {
    TypographyMetrics::Timing &metric;
    bool active{true};
    std::chrono::steady_clock::time_point start{std::chrono::steady_clock::now()};
    explicit MetricScope(TypographyMetrics::Timing &m):metric(m) {}
    void finish() {
        if(!active) return;
        active=false;
        auto ns=std::uint64_t(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now()-start).count());
        ++metric.count;metric.totalNs+=ns;metric.worstNs=std::max(metric.worstNs,ns);
    }
    ~MetricScope() { finish(); }
};
}
int Typography::timedMeasure(TTF_Font *font,const char *text,int *width,int *height) {
    MetricScope timer(metrics_.textMeasure);
    if(containsRtl(text)) {
        int w=0,h=TTF_FontHeight(font);
        for(const auto &run:bidiLine(text).runs) {
            auto *face=fontForText(font,run.text);
            TTF_SetFontDirection(face,run.rtl?TTF_DIRECTION_RTL:TTF_DIRECTION_LTR);
            TTF_SetFontScriptName(face,run.hebrew?"Hebr":(run.rtl?"Arab":"Zzzz"));
            int rw=0,rh=0; int ok=TTF_SizeUTF8(face,run.text.c_str(),&rw,&rh);
            TTF_SetFontDirection(face,TTF_DIRECTION_LTR); TTF_SetFontScriptName(face,"Zzzz");
            if(ok<0) return ok;
            w+=rw; h=std::max(h,rh);
        }
        if(width) *width=w;
        if(height) *height=h;
        return 0;
    }
    return TTF_SizeUTF8(font,text,width,height);
}

Typography::Typography(const std::filesystem::path &root) : root_(root) {}

Typography::~Typography() {
    clearTextCache();
    if(displayFont_)TTF_CloseFont(displayFont_);
    for(auto &entry:fallbackFonts_) if(entry.second) TTF_CloseFont(entry.second);
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
    titleFont_ = uiFont_ = smallFont_ = readerFont_ = sectionFont_ = buttonFont_ = nullptr;
}

TTF_Font *Typography::findAndLoad(const std::vector<std::string> &candidates, int ptSize) {
    for (const auto &c : candidates) {
        if (c.empty()) continue;
        TTF_Font *f = TTF_OpenFont(c.c_str(), ptSize);
        if (f) {
            int family=c.find("Serif")!=std::string::npos?2:(c.find("Bold")!=std::string::npos?1:0);
            fontInfo_[f]={ptSize,family};
            return f;
        }
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
    displayFont_ = findAndLoad(boldCandidates, 42);
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
        fontInfo_.erase(previewFont_);
        previewFont_=findAndLoad(family==1?sansPaths_:serifPaths_,std::clamp(size,14,44));
        previewFamily_=family; previewSize_=size;
    }
    return previewFont_?previewFont_:readerFont_;
}

TTF_Font *Typography::fontForText(TTF_Font *font,const std::string &text) {
    if(!font || text.empty()) return font;
    std::vector<FriBidiChar> chars(text.size()+1);
    int n=fribidi_charset_to_unicode(FRIBIDI_CHAR_SET_UTF8,text.data(),int(text.size()),chars.data());
    bool missing=false,hebrew=false;
    for(int i=0;i<n;++i) {
        auto c=chars[i];
        if((c>=0x590 && c<=0x5ff) || (c>=0xfb1d && c<=0xfb4f)) {
            hebrew=true; if(!TTF_GlyphIsProvided32(font,c)) missing=true;
        }
    }
    if(!hebrew || !missing) return font;
    auto info=fontInfo_.find(font);
    FontInfo role=info==fontInfo_.end()?FontInfo{TTF_FontHeight(font),0}:info->second;
    auto key=std::make_pair(role.family,role.size);
    auto found=fallbackFonts_.find(key);
    if(found!=fallbackFonts_.end()) return found->second?found->second:font;
    const char *file=role.family==2?"NotoSerifHebrew-Regular.ttf":
        (role.family==1?"NotoSansHebrew-SemiBold.ttf":"NotoSansHebrew-Regular.ttf");
    std::vector<std::string> paths={
#ifdef __SWITCH__
        std::string("romfs:/fonts/")+file,
        std::string("sdmc:/switch/PaperNX/fonts/")+file,
#endif
        (root_/"fonts"/file).string()};
    auto *fallback=findAndLoad(paths,role.size);
    fallbackFonts_[key]=fallback;
    return fallback?fallback:font;
}

SDL_Surface *Typography::rasterLine(TTF_Font *font,const std::string &text,SDL_Color color,int direction) {
    if(!containsRtl(text) && direction!=1) return TTF_RenderUTF8_Blended(font,text.c_str(),color);
    auto line=bidiLine(text,direction);
    struct Piece { SDL_Surface *surface; int ascent; };
    std::vector<Piece> pieces;
    int width=0,ascent=TTF_FontAscent(font),descent=TTF_FontHeight(font)-ascent;
    for(const auto &run:line.runs) {
        auto *face=fontForText(font,run.text);
        TTF_SetFontDirection(face,run.rtl?TTF_DIRECTION_RTL:TTF_DIRECTION_LTR);
        TTF_SetFontScriptName(face,run.hebrew?"Hebr":(run.rtl?"Arab":"Zzzz"));
        auto *piece=TTF_RenderUTF8_Blended(face,run.text.c_str(),color);
        TTF_SetFontDirection(face,TTF_DIRECTION_LTR); TTF_SetFontScriptName(face,"Zzzz");
        if(!piece) continue;
        int a=TTF_FontAscent(face);
        ascent=std::max(ascent,a); descent=std::max(descent,piece->h-a);
        width+=piece->w; pieces.push_back({piece,a});
    }
    auto *surface=width>0?SDL_CreateRGBSurfaceWithFormat(0,width,ascent+descent,32,SDL_PIXELFORMAT_ARGB8888):nullptr;
    int x=0;
    for(auto &piece:pieces) {
        if(surface) {
            SDL_Rect dst{x,ascent-piece.ascent,piece.surface->w,piece.surface->h};
            SDL_SetSurfaceBlendMode(piece.surface,SDL_BLENDMODE_NONE);
            SDL_BlitSurface(piece.surface,nullptr,surface,&dst); x+=piece.surface->w;
        }
        SDL_FreeSurface(piece.surface);
    }
    return surface;
}

SDL_Surface *Typography::rasterWrapped(TTF_Font *font,const std::string &text,SDL_Color color,int width) {
    if(!containsRtl(text)) return TTF_RenderUTF8_Blended_Wrapped(font,text.c_str(),color,width);
    std::vector<std::pair<std::string,bool>> lines;
    std::istringstream paragraphs(text); std::string paragraph;
    while(std::getline(paragraphs,paragraph) && lines.size()<256) {
        bool rtl=baseRtl(paragraph); std::istringstream words(paragraph);
        std::string line,word;
        while(words>>word && lines.size()<256) {
            std::string candidate=line.empty()?word:line+" "+word; int w=0;
            timedMeasure(font,candidate.c_str(),&w,nullptr);
            if(!line.empty() && w>width) { lines.push_back({line,rtl}); line=word; }
            else line=std::move(candidate);
            // A title/filename may be one long word. Break only between complete
            // clusters so RTL clipping never removes the beginning of the word.
            timedMeasure(font,line.c_str(),&w,nullptr);
            while(w>width && lines.size()<256) {
                auto boundaries=textClusterBoundaries(line);
                if(boundaries.size()<3) break;
                std::size_t low=1,high=boundaries.size()-1;
                while(low<high) {
                    auto middle=(low+high+1)/2; int part=0;
                    timedMeasure(font,line.substr(0,boundaries[middle]).c_str(),&part,nullptr);
                    if(part<=width) low=middle; else high=middle-1;
                }
                lines.push_back({line.substr(0,boundaries[low]),rtl}); line.erase(0,boundaries[low]);
                timedMeasure(font,line.c_str(),&w,nullptr);
            }
        }
        lines.push_back({line,rtl});
    }
    int step=TTF_FontLineSkip(font),height=0;
    std::vector<SDL_Surface *> rendered;
    for(auto &line:lines) {
        auto *s=line.first.empty()?nullptr:rasterLine(font,line.first,color,line.second?1:0);
        if(s) step=std::max(step,s->h);
        rendered.push_back(s);
    }
    height=int(lines.size())*step;
    auto *surface=height>0?SDL_CreateRGBSurfaceWithFormat(0,width,height,32,SDL_PIXELFORMAT_ARGB8888):nullptr;
    for(std::size_t i=0;i<rendered.size();++i) if(rendered[i]) {
        if(surface) {
            SDL_Rect dst{lines[i].second?std::max(0,width-rendered[i]->w):0,int(i)*step,rendered[i]->w,rendered[i]->h};
            SDL_SetSurfaceBlendMode(rendered[i],SDL_BLENDMODE_NONE);
            SDL_BlitSurface(rendered[i],nullptr,surface,&dst);
        }
        SDL_FreeSurface(rendered[i]);
    }
    return surface;
}

int Typography::renderParagraph(SDL_Renderer *r,TTF_Font *font,const std::string &str,
                                SDL_Rect area,SDL_Color color,int lineHeight,bool left,int indent) {
    if(!font || area.w<24) return 0;
    MetricScope layoutTimer(metrics_.paragraphLayout);
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
    layoutTimer.finish();
    for(std::size_t i=0;i<lines.size();++i) {
        int x=area.x+(i?0:indent), y=area.y+int(i)*lineHeight,total=0;
        if(y>=area.y+area.h) break;
        for(const auto &part:lines[i]) { int ww=0; measureText(font,part,&ww,&height); total+=ww; }
        int spacing=space;
        if(!left && i+1<lines.size() && lines[i].size()>1) spacing=std::max(space,(area.w-total-(i?0:indent))/int(lines[i].size()-1));
        if(containsRtl(str)) {
            // Keep the paragraph's base direction across wrapped mixed lines.
            // The engine handles reader justification; this path is the UI preview.
            std::string logical;
            for(const auto &part:lines[i]) { if(!logical.empty()) logical+=' ';logical+=part; }
            bool rtl=baseRtl(str);
            if(rtl) logical=u8"\u200f"+logical;
            int measured=0; measureText(font,logical,&measured,&height);
            renderText(r,font,logical,rtl?area.x+area.w-measured-(i?0:indent):x,y,color);
            continue;
        }
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
        ++metrics_.textCacheHits;
        cached->second.tick=++textTick_;
        int width=maxWidth>0?std::min(maxWidth,cached->second.width):cached->second.width;
        int offset=maxWidth>0 && baseRtl(str)?maxWidth-width:0;
        SDL_Rect src{0,0,width,cached->second.height},dst{x+offset,y,width,cached->second.height};
        SDL_RenderCopy(r,cached->second.texture,&src,&dst); return;
    }

    std::string textToRender = str;
    if (maxWidth > 0) {
        int w = 0, h = 0;
        if (timedMeasure(font, textToRender.c_str(), &w, &h) == 0 && w > maxWidth) {
            std::string ellipsis = "...";
            int ellW = 0;
            timedMeasure(font, ellipsis.c_str(), &ellW, nullptr);
            int budgetW = maxWidth - ellW;

            auto boundaries=textClusterBoundaries(textToRender);
            std::size_t low=0,high=boundaries.size()-1;
            while(low<high) {
                auto middle=(low+high+1)/2;
                auto candidate=textToRender.substr(0,boundaries[middle]);
                if(timedMeasure(font,candidate.c_str(),&w,nullptr)==0 && w<=budgetW) low=middle; else high=middle-1;
            }
            textToRender.resize(boundaries[low]); textToRender+=ellipsis;
        }
    }

    if (textToRender.empty()) return;

    SDL_Surface *surface=nullptr;
    { MetricScope timer(metrics_.textRaster); surface=rasterLine(font,textToRender,color,baseRtl(str)?1:0); }
    if (!surface) return;

    SDL_Texture *texture=nullptr;
    { MetricScope timer(metrics_.textureCreate); texture=SDL_CreateTextureFromSurface(r,surface); }
    if (texture) {
        int width=maxWidth>0?std::min(maxWidth,surface->w):surface->w;
        int offset=maxWidth>0 && baseRtl(str)?maxWidth-width:0;
        SDL_Rect src{0,0,width,surface->h},dst{x+offset,y,width,surface->h};
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
        SDL_Surface *surface=nullptr;
        { MetricScope timer(metrics_.textRaster);surface=rasterWrapped(font,str,color,rect.w); }
        if(!surface) return;
        SDL_Texture *texture=nullptr;
        { MetricScope timer(metrics_.textureCreate);texture=SDL_CreateTextureFromSurface(r,surface); }
        int width=surface->w,height=surface->h; SDL_FreeSurface(surface); if(!texture) return;
        // Very long metadata is clipped before caching; avoid huge intermediate textures persisting.
        if(std::size_t(width)*height*4>8u*1024u*1024u) { SDL_DestroyTexture(texture); return; }
        storeText(key,texture,width,height); found=textCache_.find(key); if(found==textCache_.end()) return;
    } else ++metrics_.textCacheHits;
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
        timedMeasure(font,str.c_str(),&width,&height);
        if(metricsCache_.size()>=2048) metricsCache_.clear();
        found=metricsCache_.emplace(std::move(key),std::make_pair(width,height)).first;
    } else ++metrics_.measureCacheHits;
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
