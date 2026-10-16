# PaperNX System Architecture

PaperNX is designed as a modular, lightweight, native C++17 e-reader application built for the Nintendo Switch Tegra X1 architecture.

---

## 1. Modular Subsystems

```
                     ┌───────────────────────────────────────────────┐
                     │                 main.cpp                      │
                     │  (Screen State Machine, Event-Driven Loop)     │
                     └───────┬──────────────┬─────────────┬──────────┘
                             │              │             │
              ┌──────────────┴───────┐      │      ┌──────┴───────────────┐
              │     Typography       │      │      │     Gestures         │
              │  (Noto Sans/Serif,   │      │      │ (Touch Zones 22/56,  │
              │   UTF-8 Truncation)  │      │      │  Swipes, Joy-Con)    │
              └──────────────────────┘      │      └──────────────────────┘
                                            │
                     ┌──────────────────────┴────────────────┐
                     │            Theme System               │
                     │  (6 Palettes, Dynamic EPUB CSS Reflow) │
                     └──────────────────────┬────────────────┘
                                            │
              ┌─────────────────────────────┴─────────────────┐
              │                                               │
    ┌─────────┴───────────────┐                    ┌──────────┴──────────────┐
    │     MuPDF Bridge        │                    │      Page Cache         │
    │  (C ABI boundary,       │                    │ (Memory-capped Texture  │
    │   fz_try/fz_catch,      │                    │  LRU, Prefetching)      │
    │   Bookmarks, TOC, View) │                    └─────────────────────────┘
    └─────────┬───────────────┘
              │
    ┌─────────┴───────────────┐
    │    papernx_core         │
    │ (Domain Models, Scanner,│
    │  PAPERNX-2 Store,       │
    │  Turkish Case Folding)  │
    └─────────────────────────┘
```

---

## 2. Core Architectural Decisions

1. **Strict Exception Boundary (`src/mupdf_bridge.c`)**:
   - MuPDF uses `setjmp`/`longjmp` under `fz_try` and `fz_catch`.
   - To guarantee that C++ object destructors are not bypassed, the MuPDF engine is completely isolated behind an `extern "C"` ABI.
   - All pixel buffers (`PxImage`) and document instances (`PxDocument`) have explicit, leak-proof lifecycle management.

2. **Logical Bookmarks & Reflow Stability**:
   - Rather than relying on simple fractional page approximations, EPUB reflow captures a persistent bookmark identifier via MuPDF's `fz_make_bookmark` and `fz_location_from_page_number`.
   - When the user changes font size or line height, the bookmark is looked up with `fz_lookup_bookmark` to keep the reader anchored at the same reading position.

3. **Atomic Persistence & Backup Rotation (`src/core.cpp`)**:
   - The `PAPERNX-2` format encodes book reading status, bookmarks, and typography preferences.
   - All disk writes are performed atomically: write to `.tmp` with FNV-1a checksum, flush to disk, rotate `.dat` to `.bak`, and rename `.tmp` to `.dat`.
   - If the primary record is corrupted or truncated (e.g. by an abrupt shutdown), the engine recovers from `.bak`.
   - Automatically migrates existing legacy `PAPERNX-1` records.

4. **Event-Driven Redraw Loop (`src/main.cpp`)**:
   - Replaces battery-draining continuous polling with `SDL_WaitEventTimeout(100ms)`.
   - Only triggers hardware render passes when an interaction occurs or when background textures/caches finish updating.

5. **Memory-Capped LRU Cache (`src/page_cache.cpp`)**:
   - Both reading page textures and library thumbnail covers are bounded by configurable VRAM budgets (64 MB for pages, 32 MB for covers) to strictly conform to Nintendo Switch applet and application memory limits.
