# PaperNX Engineering Status

| Component | Implemented in Source | Native Built | Desktop Tested | Switch Device |
|:---|:---:|:---:|:---:|:---:|
| Portable scan, Turkish search & identity | Implemented | Built | Tested | Unverified on hardware |
| Persistence & backup recovery (PAPERNX-2) | Implemented | Built | Tested | Unverified on hardware |
| SDL2 Library UI, touch & controller | Implemented | Built | Tested | Unverified on hardware |
| MuPDF EPUB & PDF document/page rendering | Implemented | Built | Tested | Unverified on hardware |
| Font-size relayout & 6-theme palette | Implemented | Built | Tested | Unverified on hardware |
| True EPUB bookmarks & location anchors | Implemented | Built | Tested | Unverified on hardware |
| Async thumbnail background worker & LRU cache | Implemented | Built | Tested | Unverified on hardware |
| Read-only native SD card file picker | Implemented | Built | Tested | Unverified on hardware |
| Interactive Table of Contents (TOC) & bookmarks modal | Implemented | Built | Tested | Unverified on hardware |
| Modal backdrop dimming & background preservation | Implemented | Built | Tested | Unverified on hardware |
| Touch zones (%22/%56/%22) & swipe gestures | Implemented | Built | Tested | Unverified on hardware |
| Joy-Con lifecycle & analog deadzones | Implemented | Built | Tested | Unverified on hardware |
| PDF zoom & viewport pan rasterization | Implemented | Built | Tested | Unverified on hardware |
| Bundled fonts (Noto Sans, Noto Serif, OFL) | Implemented | Built | Tested | Unverified on hardware |
| Complete Turkish localized UI (Library, Aa, TOC) | Implemented | Built | Tested | Unverified on hardware |
| NRO0 validation & verified SD ZIP packaging | Implemented | Built | Tested | Unverified on hardware |
| Guarded GitHub Actions Switch CI pipeline | Implemented | Built | Tested | Unverified on hardware |

---

## Proven Verification Gates

- **Gate A — Portable Core & Gestures**:
  - `papernx_core_tests`: **PASS** (13 comprehensive scenarios, zero exceptions, zero leaks).
  - Turkish UTF-8 case folding, safe truncation, 1,000-book stress test.
  - Multi-stage atomic file rotation (`.tmp` -> `.bak.old` -> `.bak` -> `.dat`).
  - Gesture tracker and controller deadzones.
  - Malformed row resilience & power-loss edge cases.
- **Gate B — Packaging & NRO0 Validation**:
  - `test_package.py`: **5/5 PASS** (NRO0 magic, declared size, segment non-overlap, non-overflow, SD card extraction hierarchy).
- **Gate C — Native MuPDF EPUB & PDF Integration**:
  - `tests/generate_fixtures.py`: Standards-compliant EPUB3 container with chapters, navigation, cover, and Turkish UTF-8 text; 2-page PDF fixture with outlines.
  - `tests/mupdf_render_tests.c`: Automated verification of real EPUB opening, layout reflow, text rasterization, bookmark creation/lookup, PDF 2-page distinct rendering, 1.5x zoom and pan cutout rendering, TOC extraction, and corruption rejection.
- **Gate D — Switch Toolchain Compatibility**:
  - Verified static compilation with `-fno-exceptions -fno-rtti` on AArch64 Clang 18.
  - `scripts/build-mupdf-switch.sh`: Pinned SHA256 (`478f2a167feae2a291c8b8bc5205f2ce2f09f09b574a6eb0525bfad95a3cfe66`), strict AArch64 readelf assertions, and cache fingerprinting.
- **Gate E — Physical Console Verification**:
  - Status: **HARDWARE UNVERIFIED — ALPHA**.
  - No physical Nintendo Switch console is attached to this automated build environment; real console testing must be verified by installing the resulting ZIP on an SD card.
