#pragma once
#include <SDL.h>
#include <cstdio>
#include <string>

namespace papernx {

struct ThemePalette {
    const char *name;
    SDL_Color bg;
    SDL_Color fg;
    SDL_Color muted;
    SDL_Color accent;
    SDL_Color card;
    SDL_Color cardBorder;
    SDL_Color header;
};

// Curated e-reader color palettes tailored for Switch LCD & OLED
inline const ThemePalette g_palettes[] = {
    // 0: Paper (Clean, warm off-white)
    {"Paper", {248, 246, 240, 255}, {34, 34, 34, 255}, {125, 120, 110, 255}, {139, 90, 43, 255},
     {255, 255, 255, 255}, {225, 220, 210, 255}, {240, 237, 230, 255}},

    // 1: Ivory (Soft cream, gentle on eyes)
    {"Ivory", {254, 252, 243, 255}, {45, 40, 35, 255}, {135, 125, 115, 255}, {145, 95, 45, 255},
     {255, 255, 250, 255}, {235, 228, 215, 255}, {247, 243, 232, 255}},

    // 2: Sepia (Classic vintage parchment)
    {"Sepia", {236, 224, 198, 255}, {65, 50, 36, 255}, {130, 105, 82, 255}, {140, 85, 45, 255},
     {245, 235, 212, 255}, {215, 200, 175, 255}, {228, 214, 186, 255}},

    // 3: Slate (Modern cool gray)
    {"Slate", {218, 224, 228, 255}, {38, 48, 56, 255}, {108, 120, 130, 255}, {48, 92, 130, 255},
     {228, 234, 238, 255}, {198, 206, 212, 255}, {208, 215, 220, 255}},

    // 4: Dark (Warm graphite night mode)
    {"Dark", {28, 30, 32, 255}, {230, 228, 222, 255}, {150, 148, 142, 255}, {195, 160, 110, 255},
     {38, 40, 44, 255}, {55, 58, 62, 255}, {22, 24, 26, 255}},

    // 5: OLED Black (True black #000000 for maximum battery & contrast)
    {"OLED Black", {0, 0, 0, 255}, {235, 235, 235, 255}, {140, 140, 140, 255}, {210, 180, 130, 255},
     {16, 16, 16, 255}, {36, 36, 36, 255}, {0, 0, 0, 255}}
};

inline const int g_paletteCount = sizeof(g_palettes) / sizeof(g_palettes[0]);

inline std::string generateEpubCss(int themeIdx, int fontFamily, int fontSize, int lineHeightPercent, int marginPx, int textAlign) {
    if (themeIdx < 0 || themeIdx >= g_paletteCount) themeIdx = 0;
    const auto &p = g_palettes[themeIdx];

    const char *fontFamilyCss = "serif";
    if (fontFamily == 1) fontFamilyCss = "sans-serif";
    else if (fontFamily == 2) fontFamilyCss = "inherit"; // publisher default

    const char *alignCss = (textAlign == 1) ? "left" : "justify";
    float lineHeight = (lineHeightPercent > 0) ? (float)lineHeightPercent / 100.0f : 1.4f;

    char buffer[1024];
    std::snprintf(buffer, sizeof(buffer),
        "html, body { "
        "background-color: #%02x%02x%02x !important; "
        "color: #%02x%02x%02x !important; "
        "font-family: %s !important; "
        "font-size: %dpx !important; "
        "line-height: %.2f !important; "
        "text-align: %s !important; "
        "margin: %dpx !important; "
        "padding: 0 !important; "
        "} "
        "p { margin-top: 0; margin-bottom: 0.8em; text-indent: 1.2em; } "
        "h1, h2, h3, h4, h5, h6 { color: #%02x%02x%02x !important; text-align: center; } "
        "a { color: #%02x%02x%02x !important; text-decoration: none; } "
        "img { max-width: 100%%; height: auto; display: block; margin: 0 auto; }",
        p.bg.r, p.bg.g, p.bg.b,
        p.fg.r, p.fg.g, p.fg.b,
        fontFamilyCss,
        fontSize,
        lineHeight,
        alignCss,
        marginPx,
        p.fg.r, p.fg.g, p.fg.b,
        p.accent.r, p.accent.g, p.accent.b
    );
    return std::string(buffer);
}

} // namespace papernx
