#pragma once
#include <SDL.h>
#include <SDL_ttf.h>
#include <filesystem>
#include <string>
#include <vector>
#include <map>
#include <cstdint>

namespace papernx {

class Typography {
public:
    explicit Typography(const std::filesystem::path &root);
    ~Typography();

    bool init();

    TTF_Font *titleFont() const { return titleFont_; }
    TTF_Font *uiFont() const { return uiFont_; }
    TTF_Font *smallFont() const { return smallFont_; }
    TTF_Font *sectionFont() const { return sectionFont_?sectionFont_:uiFont_; }
    TTF_Font *buttonFont() const { return buttonFont_?buttonFont_:uiFont_; }
    TTF_Font *readerFont() const { return readerFont_; }
    TTF_Font *previewFont(int family, int size);

    void renderText(SDL_Renderer *r, TTF_Font *font, const std::string &str,
                    int x, int y, SDL_Color color, int maxWidth = 0);
    void renderWrapped(SDL_Renderer *r,TTF_Font *font,const std::string &str,SDL_Rect rect,SDL_Color color);
    std::size_t textCacheBytes() const { return textCacheBytes_; }

    void measureText(TTF_Font *font, const std::string &str, int *outW, int *outH, int maxWidth = 0);
    int renderParagraph(SDL_Renderer *r, TTF_Font *font, const std::string &str,
                        SDL_Rect area, SDL_Color color, int lineHeight, bool left, int indent);

    void drawBox(SDL_Renderer *r, SDL_Rect rect, SDL_Color fill, SDL_Color border);
    void drawProgressBar(SDL_Renderer *r, int x, int y, int w, int h, float progress, SDL_Color bg, SDL_Color fg);

private:
    std::filesystem::path root_;
    TTF_Font *titleFont_{nullptr};
    TTF_Font *uiFont_{nullptr};
    TTF_Font *smallFont_{nullptr};
    TTF_Font *readerFont_{nullptr};
    TTF_Font *sectionFont_{nullptr}, *buttonFont_{nullptr};
    std::map<std::string,std::pair<int,int>> metricsCache_;
    TTF_Font *previewFont_{nullptr};
    int previewFamily_{-1}, previewSize_{-1};
    std::vector<std::string> sansPaths_, serifPaths_;
    struct CachedText { SDL_Texture *texture; int width,height; std::size_t bytes; std::uint64_t tick; };
    std::map<std::string,CachedText> textCache_;
    std::size_t textCacheBytes_{0};
    std::uint64_t textTick_{0};
    void clearTextCache();
    std::string textKey(SDL_Renderer *r,TTF_Font *font,const std::string &text,SDL_Color color,int width,bool wrap) const;
    void storeText(std::string key,SDL_Texture *texture,int width,int height);

    TTF_Font *findAndLoad(const std::vector<std::string> &candidates, int ptSize);
};

} // namespace papernx
