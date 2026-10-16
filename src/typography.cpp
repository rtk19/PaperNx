#include "papernx/typography.hpp"
#include "papernx/core.hpp"
#include <algorithm>
#include <iostream>

namespace papernx {

Typography::Typography(const std::filesystem::path &root) : root_(root) {}

Typography::~Typography() {
    if (titleFont_) TTF_CloseFont(titleFont_);
    if (uiFont_) TTF_CloseFont(uiFont_);
    if (smallFont_) TTF_CloseFont(smallFont_);
    if (readerFont_) TTF_CloseFont(readerFont_);
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

    titleFont_ = findAndLoad(sansCandidates, 28);
    uiFont_ = findAndLoad(sansCandidates, 22);
    smallFont_ = findAndLoad(sansCandidates, 17);
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

void Typography::renderText(SDL_Renderer *r, TTF_Font *font, const std::string &str,
                            int x, int y, SDL_Color color, int maxWidth) {
    if (!r || !font || str.empty()) return;

    std::string textToRender = str;
    if (maxWidth > 0) {
        int w = 0, h = 0;
        if (TTF_SizeUTF8(font, textToRender.c_str(), &w, &h) == 0 && w > maxWidth) {
            std::string ellipsis = "...";
            int ellW = 0;
            TTF_SizeUTF8(font, ellipsis.c_str(), &ellW, nullptr);
            int budgetW = maxWidth - ellW;

            while (!textToRender.empty()) {
                // Safely drop characters honoring UTF-8 boundaries
                std::size_t lastByte = textToRender.size() - 1;
                while (lastByte > 0 && (static_cast<unsigned char>(textToRender[lastByte]) & 0xC0) == 0x80) {
                    --lastByte;
                }
                textToRender.resize(lastByte);

                if (TTF_SizeUTF8(font, textToRender.c_str(), &w, nullptr) == 0 && w <= budgetW) {
                    textToRender += ellipsis;
                    break;
                }
            }
        }
    }

    if (textToRender.empty()) return;

    SDL_Surface *surface = TTF_RenderUTF8_Blended(font, textToRender.c_str(), color);
    if (!surface) return;

    SDL_Texture *texture = SDL_CreateTextureFromSurface(r, surface);
    if (texture) {
        SDL_Rect dst{x, y, surface->w, surface->h};
        SDL_RenderCopy(r, texture, nullptr, &dst);
        SDL_DestroyTexture(texture);
    }
    SDL_FreeSurface(surface);
}

void Typography::measureText(TTF_Font *font, const std::string &str, int *outW, int *outH, int maxWidth) {
    if (!font || str.empty()) {
        if (outW) *outW = 0;
        if (outH) *outH = 0;
        return;
    }
    TTF_SizeUTF8(font, str.c_str(), outW, outH);
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
