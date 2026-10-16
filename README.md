# PaperNX — native Switch reader (in development)

Offline, browser-free Nintendo Switch EPUB/PDF reader in C++17, SDL2 and the **MuPDF C API**. No Chromium, WebView, server or account. This is **not yet a Switch-installable release**.

## Current implementation

- Portable recursive EPUB/PDF discovery, extension filters, book identities, lightweight search and checksum-protected progress persistence with backup recovery.
- SDL2 library with placeholder covers, keyboard/limited controller navigation, touch zones and an actual MuPDF-rendered reading surface.
- MuPDF C bridge: open real documents, reflow EPUB, rasterize PDF and EPUB pages, report render errors, free native image buffers.
- Theme cycling (six palettes), EPUB font-size relayout and page/fraction progress restore.
- Native desktop test/build needs SDL2, SDL2_ttf and MuPDF; these are **not present in the current verification environment**. The portable core has been tested with CTest. There is **no Switch .nro or zip**.

## Portable test build

```sh
cmake -S . -B build -DBUILD_TESTING=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

## Native desktop build (when libraries are installed)

```sh
cmake -S . -B native -DPAPERNX_NATIVE=ON
cmake --build native --parallel
./native/PaperNX ./PaperNX-data
```

Missing libraries cause a hard configuration error, not a fake reader.

## Book storage and fonts

On Switch the intended default path is `sdmc:/switch/PaperNX/books/`. Subdirectories are scanned for EPUB/PDF. Settings/progress lives in `data/progress.txt` and `.bak`. A user-provided TTF file at `fonts/NotoSans-Regular.ttf` or `fonts/DejaVuSans.ttf` enables readable UI text; **the project currently bundles no fonts**.

The current core indexes filename/title only and placeholder covers; proper EPUB metadata, real cover caching, robust EPUB locators, custom font menus, file explorer, TOC and bookmarks remain unfinished.

## Copyright and licenses

PaperNX is AGPL-3.0-or-later to permit the intended MuPDF integration. MuPDF is dual-licensed under AGPL/commercial terms; any distribution must comply with its AGPL conditions. A **private GitHub repository alone does not satisfy corresponding-source obligations to people receiving a binary**. See [docs/LICENSES.md](docs/LICENSES.md).
