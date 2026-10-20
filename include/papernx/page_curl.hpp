#pragma once

// A GPU-rendered, textured cylindrical page curl for SDL2 2.0.18+.
// No additional Switch libraries, shaders, or assets are needed.
#include <SDL.h>
#include <algorithm>
#include <array>
#include <cmath>

namespace papernx {

class PageCurlRenderer {
public:
    // progress: 0 (previous page is visible) ... 1 (next page is visible)
    // direction: +1 turns the right edge toward the left; -1 mirrors the effect.
    static void render(SDL_Renderer *renderer, SDL_Texture *previous,
                       SDL_Texture *next, const SDL_Rect &page,
                       float progress, int direction) {
        if (!renderer || !next || page.w <= 0 || page.h <= 0) return;
        progress = std::clamp(progress, 0.0f, 1.0f);
        if (!previous || progress >= 1.0f) {
            SDL_RenderCopy(renderer, next, nullptr, &page);
            return;
        }
        if (progress <= 0.0f) {
            SDL_RenderCopy(renderer, previous, nullptr, &page);
            return;
        }

        constexpr float pi = 3.14159265358979323846f;
        const float phase = .5f - .5f * std::cos(pi * progress); // eased 0..1
        const float width = static_cast<float>(page.w);
        // A half-cylinder consumes pi*radius of paper. Move its contact line
        // by half that distance so the turned edge traverses width -> -width.
        // Shrinking the radius at the endpoints makes the final frame continuous.
        if (phase <= 0.f) { SDL_RenderCopy(renderer, previous, nullptr, &page); return; }
        if (phase >= 1.f) { SDL_RenderCopy(renderer, next, nullptr, &page); return; }
        const float radius = width * .14f * std::sin(pi * phase);
        const float fold = width * (1.0f - phase) - .5f * pi * radius;
        const bool forward = direction >= 0;

        struct DrawState {
            SDL_Renderer *renderer;
            SDL_Rect clip{};
            SDL_bool clipped;
            SDL_BlendMode blend{};
            Uint8 r{}, g{}, b{}, a{};
            explicit DrawState(SDL_Renderer *value) : renderer(value), clipped(SDL_RenderIsClipEnabled(value)) {
                SDL_RenderGetClipRect(renderer, &clip);
                SDL_GetRenderDrawBlendMode(renderer, &blend);
                SDL_GetRenderDrawColor(renderer, &r, &g, &b, &a);
            }
            ~DrawState() {
                SDL_RenderSetClipRect(renderer, clipped ? &clip : nullptr);
                SDL_SetRenderDrawBlendMode(renderer, blend);
                SDL_SetRenderDrawColor(renderer, r, g, b, a);
            }
        } state(renderer);
        SDL_Rect clip = page;
        if (state.clipped) SDL_IntersectRect(&page, &state.clip, &clip);
        SDL_RenderSetClipRect(renderer, &clip);
        SDL_RenderCopy(renderer, next, nullptr, &page);

        // Soft drop shadow on the paper exposed by the departing sheet.
        SDL_BlendMode savedMode = SDL_BLENDMODE_NONE;
        SDL_GetRenderDrawBlendMode(renderer, &savedMode);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        const int shadowWidth = std::max(4, int(width * .12f));
        const int seam = page.x + int(forward ? fold : width - fold);
        for (int i = 0; i < 16; ++i) {
            float fraction = static_cast<float>(i) / 15.0f;
            int offset = static_cast<int>(fraction * shadowWidth);
            int x = forward ? seam + offset : seam - offset - shadowWidth / 16;
            int alpha = int(65.0f * std::sin(pi * phase) * (1.0f - fraction));
            SDL_SetRenderDrawColor(renderer, 12, 9, 9, static_cast<Uint8>(alpha));
            SDL_Rect strip{x, page.y, std::max(1, shadowWidth / 16 + 1), page.h};
            SDL_RenderFillRect(renderer, &strip);
        }
        SDL_SetRenderDrawBlendMode(renderer, savedMode);

#if SDL_VERSION_ATLEAST(2, 0, 18) && !defined(PAPERNX_CURL_FORCE_FALLBACK)
        const float height = static_cast<float>(page.h);
        // Keep the untouched sheet as a normal copy. Tessellating this flat
        // region creates one-pixel cracks on SDL's integer software rasterizer.
        const int flatWidth = std::clamp(int(fold), 0, page.w);
        if (flatWidth > 0) {
            int textureW = 0, textureH = 0;
            if (SDL_QueryTexture(previous, nullptr, nullptr, &textureW, &textureH) == 0) {
                const int sourceWidth = std::max(1, int(float(textureW) * flatWidth / width));
                SDL_Rect source{forward ? 0 : textureW - sourceWidth, 0, sourceWidth, textureH};
                SDL_Rect destination{page.x + (forward ? 0 : page.w - flatWidth), page.y, flatWidth, page.h};
                SDL_RenderCopy(renderer, previous, &source, &destination);
            }
        }
        constexpr int columns = 72;
        constexpr int rows = 12;
        constexpr int cols1 = columns + 1;
        constexpr int rows1 = rows + 1;
        std::array<SDL_Vertex, cols1 * rows1> vertices{};
        std::array<int, columns * rows * 6> indices{};

        for (int iy = 0; iy <= rows; ++iy) {
            const float v = static_cast<float>(iy) / rows;
            const float vertical = 1.0f - std::abs(2.0f * v - 1.0f);
            const float skew = radius * .12f * vertical;
            const float localFold = fold + skew;
            for (int ix = 0; ix <= columns; ++ix) {
                const float u = (flatWidth + (width - flatWidth) * float(ix) / columns) / width;
                // The source sheet lies flat up to its rolling contact point.
                const float srcX = u * width;
                const float dx = srcX - localFold;
                float projectedX = srcX;
                float bend = 0;
                if (dx > 0.0f) {
                    const float angle = dx / radius;
                    if (angle <= pi) {
                        projectedX = localFold + radius * std::sin(angle);
                        bend = std::sin(angle);
                    } else {
                        // The folded-over part is behind the cylindrical edge.
                        projectedX = localFold - (dx - pi * radius);
                        bend = 0;
                    }
                }
                // Lifting the center of the sheet makes the edge subtly curved.
                const float liftedY = height * .012f * bend * vertical * std::sin(pi * phase);
                const float screenX = forward ? projectedX : width - projectedX;
                const int idx = iy * cols1 + ix;
                SDL_Vertex &vertex = vertices[idx];
                vertex.position = {page.x + screenX, page.y + v * height - liftedY};
                vertex.tex_coord = {forward ? u : 1.0f - u, v};
                // Shade the rolled paper: a dim underside and a narrow highlight.
                float light = 1.0f;
                if (dx > 0.0f) {
                    const float angle = dx / radius;
                    if (angle <= pi) {
                        light = .68f + .32f * std::max(0.0f, std::cos(angle));
                        light += .11f * std::exp(-std::pow((angle - 1.15f) * 3.0f, 2.0f));
                    } else {
                        light = .74f;
                    }
                }
                const Uint8 shade = static_cast<Uint8>(std::clamp(light, .4f, 1.0f) * 255.0f);
                vertex.color = {shade, shade, shade, 255};
            }
        }
        int cursor = 0;
        for (int iy = 0; iy < rows; ++iy) {
            for (int ix = 0; ix < columns; ++ix) {
                int a = iy * cols1 + ix;
                int b = a + 1;
                int c = a + cols1;
                int d = c + 1;
                indices[cursor++] = a; indices[cursor++] = c; indices[cursor++] = b;
                indices[cursor++] = b; indices[cursor++] = c; indices[cursor++] = d;
            }
        }
        if (!forward) {
            for (int i = 0; i < cursor; i += 3) std::swap(indices[i + 1], indices[i + 2]);
        }
        if (SDL_RenderGeometry(renderer, previous, vertices.data(), int(vertices.size()),
                               indices.data(), int(indices.size())) == 0) return;
#endif
        // Some SDL render drivers do not expose geometry; degrade to a clipped turn.
        const int visible = std::max(1, int(width * (1.f - phase)));
        int sourceW = 0, sourceH = 0;
        if (SDL_QueryTexture(previous, nullptr, nullptr, &sourceW, &sourceH) != 0) return;
        const int sourceVisible = std::max(1, int(sourceW * (1.f - phase)));
        SDL_Rect source{forward ? 0 : sourceW - sourceVisible, 0, sourceVisible, sourceH};
        SDL_Rect destination{page.x + (forward ? 0 : page.w - visible), page.y, visible, page.h};
        SDL_RenderCopy(renderer, previous, &source, &destination);
    }
};

} // namespace papernx
