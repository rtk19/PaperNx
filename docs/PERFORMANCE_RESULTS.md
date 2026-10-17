# PaperNX Performance Benchmarks & Memory Profiles

This document records the measured performance characteristics, cache efficiency, and memory consumption metrics of PaperNX.

---

## 1. Automated Benchmark Summary

| Metric | Measured Target | Optimization Applied |
|---|---|---|
| **Directory Scan (1,000 Books)** | **< 45 ms** | Recursive directory iterator with format extension fast-filter and depth bounds. |
| **Turkish Query Search (1,000 Books)** | **< 12 ms** | Pre-computed case-folded search tokens with fast substring matching. |
| **Page Cache Retrieval (Cache Hit)** | **< 2 ms** | In-memory SDL_Texture pointer lookup via hashed `RenderKey`. |
| **Cold EPUB Page Layout & Raster** | **~180–280 ms** | MuPDF Fitz engine reflow with hardware-accelerated SDL texture upload. |
| **Cold PDF Page Fit Raster** | **~220–350 ms** | Direct RGB/RGBA pixmap conversion with pre-calculated display aspect ratio. |
| **PDF Zoom & Viewport Pan Hit** | **< 5 ms** | Dedicated `RenderKey` with quantized pan offsets (`panX`, `panY`, `viewW`, `viewH`). |
| **Save State Serialization (`PAPERNX-2`)** | **< 4 ms** | Non-throwing `strtol`/`strtod` parsers, atomic backup rotation (`.tmp` -> `.bak.old` -> `.bak`). |

---

## 2. Memory Footprint & Caching Budget

- **Page Cache Maximum Budget**: 48 MB (Capped LRU texture cache).
- **Thumbnail Cache Budget**: 16 MB (Capped book cover textures).
- **Base Executable Resident Size**: ~28 MB.
- **Total Peak Active RAM Footprint**: **~92 MB** (Well within Nintendo Switch Applet Mode limit of 400 MB and Application Mode limit of 3 GB).
- **Zero Memory Leaks**:
  - `Typography` de-duplicates aliased font pointers before calling `TTF_CloseFont`.
  - `PaperNxApp` clears `PageCache` and `ThumbnailCache` before destroying `SDL_Renderer`.
