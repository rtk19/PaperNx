#pragma once
#include <SDL.h>
#include <SDL_ttf.h>
#include <filesystem>
#include <string>
#include <vector>

namespace papernx {

class Typography {
public:
    explicit Typography(const std::filesystem::path &root);
    ~Typography();

    bool init();

    TTF_Font *titleFont() const { return titleFont_; }
    TTF_Font *uiFont() const { return uiFont_; }
    TTF_Font *smallFont() const { return smallFont_; }
    TTF_Font *readerFont() const { return readerFont_; }

    void renderText(SDL_Renderer *r, TTF_Font *font, const std::string &str,
                    int x, int y, SDL_Color color, int maxWidth = 0);

    void measureText(TTF_Font *font, const std::string &str, int *outW, int *outH, int maxWidth = 0);

    void drawBox(SDL_Renderer *r, SDL_Rect rect, SDL_Color fill, SDL_Color border);
    void drawProgressBar(SDL_Renderer *r, int x, int y, int w, int h, float progress, SDL_Color bg, SDL_Color fg);

private:
    std::filesystem::path root_;
    TTF_Font *titleFont_{nullptr};
    TTF_Font *uiFont_{nullptr};
    TTF_Font *smallFont_{nullptr};
    TTF_Font *readerFont_{nullptr};

    TTF_Font *findAndLoad(const std::vector<std::string> &candidates, int ptSize);
};

} // namespace papernx
