# PaperNX Autonomous Completion Plan & Verification Matrix

## 1. Architectural Audits & Critical Fixes

| Area | Identified Risk / Issue | Engineering Fix |
|:---|:---|:---|
| **C++ Exceptions** | `Makefile` uses `-fno-exceptions`, but `src/core.cpp` had `try/catch` and `std::stoi/stod/stoll` which throw `std::invalid_argument`. | Replace `try/catch` with zero-allocation, no-throw parsing (`std::strtol`/`std::strtod` or `std::from_chars`) and `std::error_code`. Fully conform to `-fno-exceptions`. |
| **SDL2 Lifetime & Memory** | `~PaperNxApp` destroys `renderer_` while `pageCache_` still holds textures. `Typography` aliases `readerFont_ = uiFont_`, leading to double-free in `TTF_CloseFont`. | 1. Implement strict shutdown lifecycle: clear all caches and textures *before* destroying SDL renderer.<br>2. Track font ownership with boolean or distinct pointer checks to prevent double-free. |
| **PDF Viewport Caching** | `PageCache::makeKey` omits `panX`, `panY`, `viewW`, `viewH`, `margin`, `lineHeight`. Panning causes stale tiles. | Create composite typed cache key incorporating viewport origin, zoom factor, margins, line-height, and document ID. |
| **Switch Platform Lifecycle** | `romfsInit()`/`romfsExit()` missing. Log file not initialized. | Add platform init/exit guard (`romfsInit()`, `romfsExit()`, logger to `sdmc:/switch/PaperNX/logs/papernx.log`). |
| **MuPDF Bridge & TOC** | `node->page.page` assumes direct int outline page; bookmark lifetime needs session vs persistence separation. | Safely resolve outline destinations and preserve session bookmark vs persistent document locator. |
| **Persistence Fault Tolerance** | Rotation deleted `.bak` before new file was guaranteed. | Safe rotation: write to `.tmp`, verify checksum, only rotate `.bak` after verified, atomic rename. |
| **Touch & Controller Lifecycle** | Hotplugging controllers not handled; taps behind toolbar registers page turns. | Suppress page turns when toolbar is visible. Add `SDL_CONTROLLERDEVICEADDED` / `REMOVED` event handlers. |
| **Real Document Fixtures** | Synthetically valid EPUB (ZIP container, OPF, XHTML) & PDF (PDF-1.4 header, xref, streams) test generation. | Create test fixtures and automated document validation suites. |

---

## 2. Execution Sequence
1. Fix C++ `-fno-exceptions` and parsing in `src/core.cpp`.
2. Fix SDL2 texture & font lifetime in `src/typography.cpp` and `src/main.cpp`.
3. Fix PDF cache key and stale rendering in `include/papernx/page_cache.hpp` & `src/page_cache.cpp`.
4. Fix MuPDF bridge, outline resolution, and bookmark handling in `src/mupdf_bridge.c`.
5. Harden persistence against power loss in `src/core.cpp`.
6. Add RomFS mount, controller hotplugging, and touch suppression in `src/main.cpp`.
7. Strengthen `scripts/build-mupdf-switch.sh` (SHA verification, remove `|| true`).
8. Generate real EPUB/PDF fixtures and write comprehensive native tests.
9. Verify all builds locally (x86_64 and AArch64).
10. Trigger and monitor GitHub Actions Switch CI.
11. Update documentation and deliver full release report.
