# PaperNX native design system — v0.3

Implementation sources: `include/papernx/ui.hpp`, `include/papernx/theme.hpp`, `src/typography.cpp` and application screen composition in `src/main.cpp`. This is SDL drawing and SDL_ttf text; no HTML, WebView or external UI runtime. Existing open-book branding is preserved.

## Geometry and touch tokens

| Token | Actual implementation | Purpose |
|---|---|---|
| `ui::touch` | 56 output pixels | Practical button hit-height at 1280×720 |
| `ui::gap` | 16 pixels | Shared spacing increment |
| `ui::radius` | 12 pixels | Rounded native surfaces |
| `ui::row` | 64 pixels | Control row cadence, 56-pixel control plus spacing |
| Settings sheet | 24-pixel horizontal inset, 8 below 800 output width; top/bottom 16 | Use handheld space efficiently |
| Settings sidebar | 190 pixels, 142 below 800 output width | Six readable sections |
| Settings preview | 280 pixels when width ≥1100, omitted below | Sample remains separate from controls |
| Settings footer | 56-pixel controls at sheet bottom | Close/reset/save routes remain discoverable |
| Library grid | Adaptive columns using `(width−48)/224`, minimum one | Preserve card geometry |
| Library card | 330-pixel height, 350-pixel vertical cadence | Stable cover/title/author/progress region |
| Library content origin | 174 pixels from top, 32 left | Distinguish header, filters and content |
| Quick action rows | 56-pixel buttons on 60-pixel cadence | Comfortable contextual menu |

These are output-pixel tokens, not a blanket claim of density-independent scaling. Smaller desktop windows need clipping/scrolling tests. Use the shared `SettingsLayout` for hit testing and drawing so visible controls and touch targets agree.

## Typography

The implementation loads Noto Sans for UI and Noto Serif for reading/preview, with platform fallback fonts. UI hierarchy is 32 px title, 22 px primary UI and 18 px secondary UI; base reader preview is 24 px. Selected reader font size uses a separately cached preview font bounded to 14–44 px. Turkish Unicode text must be truncated only on UTF-8 boundaries. Long titles need a two-line title region and restrained secondary author text. Font fallback does not authorize copying unlicensed font files.

## Palette

`ThemePalette` provides `bg`, `fg`, `muted`, `accent`, `card`, `cardBorder` and `header`. The six existing curated palettes remain available:

| Palette | Background | Foreground | Accent |
|---|---|---|---|
| Paper | `#F8F6F0` | `#222222` | `#8B5A2B` |
| Ivory | `#FEFCF3` | `#2D2823` | `#915F2D` |
| Sepia | `#ECE0C6` | `#413224` | `#8C552D` |
| Slate | `#DAE0E4` | `#263038` | `#305C82` |
| Dark | `#1C1E20` | `#E6E4DE` | `#C3A06E` |
| OLED Black | `#000000` | `#EBEBEB` | `#D2B482` |

Application appearance and reading appearance have separate preference scopes. EPUB custom text/background colors are reader preferences, not UI palette mutations. Contrast calculations use linearized relative luminance in `readingContrast`; a preview warning cannot replace device readability checks. Avoid claims of measured battery savings for OLED Black without measurements.

## Components and states

`ui::surface` draws restrained rounded native surfaces. `ui::button` handles label placement and selected/pressed palette treatment: selected uses accent, pressed uses card border, ordinary uses header. Library covers keep an intentional format-specific vector fallback while artwork is not ready; successful artwork is aspect-fitted without changing card dimensions. The cover state model distinguishes not-requested, loading, loaded, no embedded cover, failed and cached. Only renderer-thread code uploads SDL textures.

Touch routing must give the frontmost modal or quick action sheet exclusive access before reader page zones. A captured drag cannot become a page turn on release. Long press, dragging and pinch need distinct gesture state. No required action is represented only by a controller glyph. Do not advertise future statistics, sync, annotations or search features unless an actual action exists.

## Validation

Capture actual application screenshots at 1280×720, 1920×1080 and a smaller supported desktop size. Check Turkish titles, no cover, loading/failed cover, empty categories, all six themes, sheet scrolling and dialog stacking. Physical LCD/OLED contrast, finger target comfort and docked scaling remain UNVERIFIED until the new hardware session.
