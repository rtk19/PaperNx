<p align="center"><img src="assets/branding/PaperNX-icon-512.png" alt="PaperNX" width="160"></p>

| **English** | [Türkçe](README.tr.md) |
|:---:|:---:|

# PaperNX

A **native, fully offline EPUB/PDF reader for Nintendo Switch**, with a Turkish user interface. Built with C++17, SDL2 and MuPDF. Books and reading records stay on your microSD card; no account, server or browser engine is required.

**v0.2.0-rc1 — candidate for device acceptance testing.** An earlier version was tested on a physical Switch, where save and touch issues were reported. Docker and package results for this branch are in the [test report](docs/TEST_RESULTS.md); physical Switch acceptance of this new build is **UNVERIFIED**. This is not a stable release.

- EPUB: Serif/Sans/Publisher fonts, font size, line and paragraph spacing, margins, indentation and alignment; Paper, Ivory, Sepia, Slate, Dark and OLED Black presets.
- Choose text and background colors independently using swatches or six-digit HEX values. Low-contrast combinations trigger a warning. Save a book's preferences as defaults for new books.
- PDF: fit page, fit width, a 72 dpi reference 100% view, zoom and bounded touch panning. Original, Dark, Sepia and Invert are **whole-page filters; they also affect photographs**.
- EPUB3 navigation, illustrations and covers; bookmarks, favorites, reading progress, per-book preferences, search and an SD file browser.
- Six library categories, grid/list views, background cover loading, preserved cover proportions and multiline titles.
- Existing PAPERNX-1/2/3 records remain readable; new saves use PAPERNX-4. Back up `data/` before upgrading and use that backup if reverting to an older version.

Desktop test screenshots captured from the actual application using SDL's dummy video driver (these are not Switch photographs):

![EPUB appearance settings](verification/screenshots/epub-settings.png)
![PDF settings](verification/screenshots/pdf-settings.png)
![Library](verification/screenshots/library.png)

## SD installation / upgrade

1. Use a Switch capable of running homebrew. Merge the `switch/` folder from `dist/PaperNX-switch-v0.2.0-rc1.zip` into the root of your microSD card.
2. Add books under `switch/PaperNX/books/`, or open another folder using **SD Gözat** (Browse SD).
3. Launch PaperNX from Homebrew Menu and tap a book cover.

```text
switch/PaperNX/
  PaperNX.nro
  books/
  data/
  fonts/  (NotoSans-Regular.ttf, NotoSerif-Regular.ttf, OFL.txt)
  licenses/
```

Keep existing folders when upgrading. Preserve `books/`, `data/`, `logs/` and your personal fonts. The ZIP contains no books, reading records or logs; it updates only the required application, fonts and license files. SHA256 files are provided in `dist/`. Available memory is limited for large PDFs and in applet mode.

## Touch and controller controls

The application currently uses Turkish interface labels; their English meanings are included below where useful.

| Action | Touch | Joy-Con / Pro Controller |
|---|---|---|
| Next/previous page | Tap the right/left 30% edge or swipe horizontally | L/R; right/left in the normal view |
| Toolbar | Tap the center | A |
| Settings | Tap the center → **Aa** at the top right | + |
| Setting field / tab / value | Tap a button or tab | Y / left-right / up-down |
| HEX color | Temalar (Themes) → Yazı HEX / Zemin HEX (Text / Background HEX) | Themes, Y to select a color field, up/down |
| Add/open bookmark | Toolbar → Yer İmi (Bookmark) → bottom button / list entry | Y to add/remove; Minus to list |
| Table of contents | Toolbar → İçindekiler (Contents) → chapter | X, directions, A |
| PDF zoom / pan | −/+ in Aa; drag the zoomed page | ZL/ZR; left stick |
| Return to library | Toolbar → Kitaplık (Library) | B |
| Search / SD browser | Ara / SD Gözat at the top (Search / Browse SD) | X / Y |
| Favorite | + / F at the bottom right of a book card | Left stick click |
| Scroll a list | Swipe up/down | Directions / stick |

Choose 20%, 30% or 40% touch edges in Aa → Kontroller (Controls). Pinch zoom depends on SDL multitouch events and is **UNVERIFIED** on physical Switch hardware; −/+ buttons provide an alternative. On desktop, press Enter to apply a HEX value or Esc to cancel.

## Local Docker verification

```sh
docker version
docker info
docker compose version
docker compose build desktop switch
docker compose run --rm desktop bash scripts/verify-final.sh
docker compose run --rm switch bash scripts/verify-switch.sh
```

The recorded verification used existing local `papernx-desktop` and `papernx-switch` images; the [verification report](docs/TEST_RESULTS.md) lists the exact commands executed. Checks cover the MuPDF 1.23.11 source SHA256, native tests, ASan/UBSan, genuine AArch64 ELF/NRO output, ASET/NACP/RomFS/fonts and embedded icon bytes. Build tools may require network access; the application operates offline.

If saving fails, `switch/PaperNX/logs/papernx.log` records the operation, path, errno, native Result and recovery-copy status. Identical errors are not repeatedly reported; retries use a 6–60 second backoff. Temporarily enable touch logging in Aa → Kontroller, or create `data/touch-debug.flag`. Disable it after testing.

[Device test plan](docs/DEVICE_TEST_PLAN.md) · [Technical changes](docs/READER_V2.md) · [Switch build guide](docs/BUILD_SWITCH.md)

## Branding and social preview

The four user-provided assets are preserved under `assets/branding/`. The NRO uses `PaperNX-icon-switch.jpg`; this README displays `PaperNX-icon-512.png`. The GitHub social graphic is ready, but the repository setting has not been changed (**UNVERIFIED**): open repository **Settings → General → Social preview → Edit → Upload an image**, select `assets/branding/PaperNX-github-social.png` and save.

## License

PaperNX and MuPDF are licensed under **AGPL-3.0**; Noto fonts use the **SIL Open Font License**. See [LICENSE](LICENSE) and the [license guide](docs/LICENSES.md) for source-distribution obligations and dependency notices.

EPUB continuous scrolling is available in the v0.3 development candidate. See [controls, implementation and validation](docs/EPUB_CONTINUOUS_SCROLL.md).
