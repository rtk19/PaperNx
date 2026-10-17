# PaperNX — Native Nintendo Switch EPUB/PDF Reader

Offline, browser-free Nintendo Switch EPUB and PDF reader built in native C++17, SDL2, and the **MuPDF C API**.
Zero Chromium, zero WebView, zero HTTP servers, zero background daemons. Extremely lightweight, battery-friendly, and optimized for handheld reading on the Nintendo Switch Tegra X1.

---

## Key Features

- **Kindle-Grade Reading Surface**:
  - Distraction-free full-screen reading with warm, eye-friendly color palettes.
  - 6 Built-in Themes: *Paper*, *Ivory*, *Sepia*, *Slate*, *Dark*, and *OLED Black*.
  - Configurable typography: Font family, font size, line height, page margins, and text alignment.
  - Bundled high-legibility Google Noto Sans and Noto Serif typography with full Turkish UTF-8 character support (`İ`, `ı`, `ğ`, `ş`, `ü`, `ö`, `ç`).
  - True EPUB layout reflow and font size adjustments on the fly.
  - PDF fit-page rendering with smooth zoom (up to 3.0x) and 2D panning.
- **Kindle-Style Library & Navigation**:
  - Book shelf categories: *All Books*, *Continue Reading*, *Favorites*, *Unread*, *Completed*.
  - Fast Turkish-aware search with case-folding.
  - Cover thumbnails and multi-level LRU page/texture caching with memory budgeting.
  - In-app Table of Contents (TOC) and Bookmarks modal.
  - Read-only native SD card file picker.
- **Dual Touch & Joy-Con Controls**:
  - Standard Kindle touch zones: Tap left (previous page), tap right (next page), tap center (toggle toolbar).
  - Swipe left and right gesture tracking.
  - Full Joy-Con and Pro Controller support: D-Pad, face buttons, shoulder triggers (L/R/ZL/ZR), and analog sticks with hardware deadzone filtering.
  - Hotplugging: dynamically recovers when controllers disconnect or reconnect.
- **Fail-Safe Persistence (`PAPERNX-2`)**:
  - Multi-stage atomic file rotation (`.tmp` -> `.bak.old` -> `.bak` -> `.dat`) to protect progress against sudden power loss or SD removal.
  - Stores reading page, progress percentage, layout settings, and multiple bookmarks per document.
  - Checksum verification on save and load with automatic recovery from backup files.
  - Seamless migration from legacy `PAPERNX-1` saves.
- **Engineered for Nintendo Switch**:
  - Standalone `PaperNX.nro` homebrew binary with embedded RomFS assets.
  - Clean startup (`romfsInit()`) and safe teardown ensuring textures and caches are purged before SDL renderer destruction.
  - Log output to `sdmc:/switch/PaperNX/logs/papernx.log` with size bounds and zero privacy leakage.

---

## Directory Structure on Nintendo Switch SD Card

When extracted to the root of your SD card (`sdmc:/`), PaperNX installs to:

```text
sdmc:/
└── switch/
    └── PaperNX/
        ├── PaperNX.nro                  # Native Homebrew application
        ├── books/                       # Put your .epub and .pdf books here
        │   └── Sample_Book.epub
        ├── fonts/                       # Bundled and custom TTF/OTF fonts
        │   ├── NotoSans-Regular.ttf
        │   ├── NotoSerif-Regular.ttf
        │   └── OFL.txt
        ├── data/                        # Progress & reading states (PAPERNX-2)
        │   └── progress.txt
        ├── logs/                        # Diagnostics log
        │   └── papernx.log
        └── README.md
```

---

## Building from Source

### 1. Portable Core Tests (Host OS)

Verify format detection, Turkish case-folding, persistence rotation, and gesture navigation:

```bash
cmake -S . -B build-core -DBUILD_TESTING=ON
cmake --build build-core --parallel
ctest --test-dir build-core --output-on-failure
python tests/test_package.py
```

### 2. Native Desktop Reader & MuPDF EPUB/PDF Integration Tests

Prerequisites: `libsdl2-dev`, `libsdl2-ttf-dev`, `libmupdf-dev`.

```bash
python3 tests/generate_fixtures.py
cmake -S . -B build-desktop -DPAPERNX_NATIVE=ON
cmake --build build-desktop --parallel
./build-desktop/mupdf_render_tests
./build-desktop/PaperNX ./test-data
```

### 3. Nintendo Switch AArch64 Cross-Compilation

Prerequisites: devkitPro (`devkitA64`, `libnx`, `switch-sdl2`, `switch-sdl2_ttf`).

```bash
# 1. Build and install MuPDF AArch64 static libraries
chmod +x scripts/build-mupdf-switch.sh
./scripts/build-mupdf-switch.sh prefix-mupdf-switch

# 2. Build PaperNX.nro
export MUPDF_PREFIX=$(pwd)/prefix-mupdf-switch
make all -j$(nproc)

# 3. Assemble and verify release ZIP package
python3 scripts/package_release.py PaperNX.nro --destination dist --version "v0.1.0"
```

---

## Automated CI Pipeline

A guarded GitHub Actions workflow (`.github/workflows/manual-switch-build.yml`) provides:
1. **Preflight**: Compiles and runs all portable unit tests and packaging validators.
2. **Native Desktop Verification**: Compiles with SDL2/MuPDF, generates real EPUB/PDF fixtures, and executes `mupdf_render_tests`.
3. **Switch Cross-Compile**: Builds `PaperNX.nro` in `devkitpro/devkita64` with pinned dependencies, validates NRO0 segments, and bundles the SD distribution ZIP.

---

## Controls Summary

| Action | Touch | Joy-Con / Controller |
|:---|:---:|:---:|
| Next Page | Tap Right Zone / Swipe Left | **D-Pad Right**, **R**, **ZR**, **A** |
| Previous Page | Tap Left Zone / Swipe Right | **D-Pad Left**, **L**, **ZL**, **Y** |
| Toggle Menu / Toolbars | Tap Center Zone | **X**, **Plus (+)** |
| Close Modal / Return to Library | Tap Outside / Back Button | **B**, **Minus (-)** |
| PDF Zoom In / Out | Pinch / UI Scrubber | **Right Stick Up / Down** |
| PDF Pan | Touch Drag | **Right Stick / D-Pad** |
| Font & Theme (Aa) | Tap "Aa" in Toolbar | **Plus (+)** -> Navigate |

---

## Licenses & Compliance

- **PaperNX**: Released under the **GNU Affero General Public License (AGPL-3.0-or-later)**.
- **MuPDF**: Licensed by Artifex Software under **AGPL-3.0**.
- **Noto Fonts**: Licensed under the **SIL Open Font License (OFL-1.1)**.
- For third-party notices and corresponding source instructions, see [docs/LICENSES.md](docs/LICENSES.md).
