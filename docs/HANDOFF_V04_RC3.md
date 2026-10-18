# PaperNX v0.4.0-rc3 — Final Engineering Handoff

## 1. Executive Summary

This engineering handoff documents the final local Docker build, behavioral verification, regression analysis, performance validation, and release packaging for **PaperNX v0.4.0-rc3**.

The changes in this release candidate address book-scoped reader appearance, PDF reader chrome color parity, quick display orientation switching from the `+` menu header, non-blocking asynchronous metadata recovery for cached covers, and atomic palette synchronization during background EPUB reflow.

All automated engineering gates (Portable, Native, Sanitizers ASan/UBSan/LSan, Release, Stress, AArch64 cross-compilation, and Distribution Integrity) have **PASSED** with clean execution and verified cryptographic digests.

> [!IMPORTANT]
> In accordance with project policy, physical Nintendo Switch hardware testing remains **UNVERIFIED** until an operator executes the 35-point hardware acceptance plan on a physical console. Automated dummy rendering and cross-compilation success do not substitute for physical device testing.

---

## 2. Checkpoint and Repository State

- **Target Version:** `v0.4.0-rc3` (configured in `Makefile`, `src/main.cpp`, `scripts/verify-v04.sh`, and `scripts/package_release.py`)
- **Git Branch:** `fix/v04-book-scoped-appearance-and-pdf-chrome`
- **Starting HEAD Commit:** `1863c55fc0bf06c506b8b23065d71af55f2bc61c`
- **Base Branch:** `fix/v04-switch-touch-theme-performance` (previous release `v0.4.0-rc2` at `2e3c051`)
- **Pull Request:** [PR #5](https://github.com/ysfbz/PaperNx/pull/5)
- **Source Implementation Fingerprint:** `1495b0bd9dd428666ec8868310efaa1a53f317b3233cc7c969331c28c44aaf83` (calculated across `src/`, `include/`, `Makefile`, `CMakeLists.txt`, `scripts/build-mupdf-switch.sh`)
- **Build Receipt:** `verification/v04-switch-build-receipt.json`

---

## 3. Behavior and Feature Verification

### 3.1 EPUB Book-Scoped Appearance & Atomic Reflow
- **Per-Book Appearance Isolation:** Setting a theme or color within an open EPUB saves the style exclusively for that book (`extraStyle_.styleOverride = 1`). The global application defaults in `PAPERNX_DEFAULTS` remain untouched.
- **Global Restoration on Exit:** Exiting to the library immediately restores the application's global palette.
- **Restoration on Reopen:** Reopening the EPUB restores its custom book-specific palette.
- **Scope Reset:** Clicking the scope toggle in reader settings reverts the book to inheriting global theme defaults.
- **Atomic Reflow Synchronization:** When changing font size, margins, or theme in an EPUB, `presentedStyle()` returns `renderedStyle_` while the background worker (`layoutWorker_`) formats the new document. The canvas background, chrome, and visible page raster remain on the identical palette throughout reflow, eliminating mixed-color frames.
- **Anchor & History Persistence:** Changing theme or font size preserves chapter index, character offset, page anchor, and bookmark positions.

### 3.2 Synchronized PDF Filter Chrome & Navigation
- **Filter Chrome Matching:** PDF filters (Original, Dark, Sepia, Invert) color both the page raster and surrounding reader chrome.
  - **Dark Filter:** Canvas background matches RGB `(28, 30, 32)` where filtered white paper appears.
  - **Sepia Filter:** Canvas background matches RGB `(236, 224, 198)`.
  - **Invert Filter:** Canvas background matches RGB `(0, 0, 0)`.
- **Global Theme Protection:** PDF filter selection changes only the document-level preference; the global library theme is unmodified.
- **Reopen Restoration:** Reopening a PDF restores its chosen color filter.
- **Zoom & Gesture Preserved:** D-pad page turns, touchscreen swipes, zoomed pan, pinch-to-zoom, and fit-page/fit-width behavior remain fully functional.

### 3.3 Quick Display Orientation Access
- **Fixed Header Control:** A dedicated `↻ Yatay/Dikey` button is placed in the top-right header of the `+` quick menu sheet (`{r.x + r.w - 164, r.y + 4, 148, 36}`). It is immediately visible without scrolling the action list.
- **Touch & Controller Support:** Tapping the button or navigating to it via D-pad focus (`quick.orientation`) and pressing `A` opens the orientation choice popup with Landscape, Portrait clockwise, and Portrait counterclockwise.
- **Coordinate Transformation:** Orientation persists across app restarts, and touch coordinates map inversely through `DisplayTransform`.

### 3.4 Non-Blocking Metadata Recovery
- **Resolution Precedence:** Library card titles prioritize:
  1. User-defined manual title override (`CatalogEntry::title`).
  2. Authentic embedded metadata from document (`CatalogEntry::sourceTitle`).
  3. Filename fallback (`defaultTitle(path)`).
  Author metadata follows the identical precedence model.
- **Asynchronous Cover Worker Recovery:** For migrated cover caches without stored catalog metadata, `px_document_open_limited` runs asynchronously in the thumbnail background worker without blocking the UI thread.
- **Bounded Frame Dispatch:** UI thread consumes at most 2 metadata updates per frame (`std::min<std::size_t>(2, completedMetadata_.size())`), preventing frame drops.

---

## 4. Verification Gate Results

All Docker verification suites were executed using the existing local Docker images (`papernx-desktop` and `papernx-switch`):

| Test Suite Profile | Command | Tests | Result | Notes |
|---|---|---|---|---|
| **Portable C++17** | `bash scripts/verify-v04.sh portable` | 6/6 | **PASS** | Core logic, catalog, UTF-8 integrity |
| **Pinned Native Desktop** | `bash scripts/verify-v04.sh native` | 10/10 | **PASS** | MuPDF bridge, UI workflow, touch regressions |
| **Sanitizers (ASan/UBSan/LSan)** | `bash scripts/verify-v04.sh sanitizers` | 10/10 | **PASS** | 0 leaks, 0 UB, clean exit with `detect_leaks=1` |
| **Release Optimized** | `bash scripts/verify-v04.sh release` | 10/10 | **PASS** | `-O3`, all 10 unit and integration tests pass |
| **Switch AArch64 Cross-Build** | `bash scripts/verify-v04.sh switch` | N/A | **PASS** | Clean ELF64, NACP `0.4.0-rc3`, NRO0 binary |
| **Distribution Integrity** | `python3 scripts/verify_distribution.py` | 85 entries | **PASS** | ZIP CRC/SHA256, 59 licenses, no user data overwrite |

---

## 5. Performance Measurements

Recorded under the Docker SDL software renderer (see `verification/v04-ui-latency.json`, `verification/v03-ui-benchmark.json`, `verification/v03-stress-benchmark.json`):

| Metric | Measured Value | Target / Budget | Status |
|---|---|---|---|
| `tap_to_pressed_present` (p50 / p95) | 8.22 ms / 8.38 ms | < 16.6 ms (60 FPS) | **PASS** |
| `tap_to_action_present` (p50 / p95) | 16.45 ms / 16.82 ms | < 33.3 ms (30 FPS) | **PASS** |
| `focus_to_present` (p50 / p95) | 8.22 ms / 8.40 ms | < 16.6 ms (60 FPS) | **PASS** |
| Large EPUB open (24 chapters) | 30.25 ms | < 100 ms | **PASS** |
| Illustrated EPUB open (8 chapters) | 3.82 ms | < 50 ms | **PASS** |
| PDF open to first page (320 pages) | 2.39 ms | < 50 ms | **PASS** |
| Cold page render (p95) | 4.05 ms | < 16.6 ms | **PASS** |
| Warm / cached page render (p95) | 1.28 ms | < 5 ms | **PASS** |
| Menu open render (p95) | 20.35 ms | < 33.3 ms | **PASS** |
| Settings tab render (p95) | 27.78 ms | < 33.3 ms | **PASS** |
| Asynchronous layout & swap | 233.24 ms | Debounced background | **PASS** |
| Peak page texture cache | 66.59 MB | <= 64 MiB logical (+ headroom) | **PASS** |
| Process RSS high-water mark | 148.66 MB | < 256 MiB | **PASS** |

---

## 6. Release Distribution Artifacts

The release package was generated under `dist/v0.4.0-rc3/` without modifying prior RC1/RC2 distributions:

```text
dist/v0.4.0-rc3/
├── PaperNX.nro
├── PaperNX.nro.sha256
├── PaperNX-switch-v0.4.0-rc3.zip
├── PaperNX-switch-v0.4.0-rc3.zip.sha256
└── manifest.json
```

### Cryptographic Checksums and File Sizes:

| File | Size (bytes) | SHA256 Checksum |
|---|---|---|
| `PaperNX.nro` | 47,938,515 | `0e77206a3dc18f171b79df7b293bc89dbcf39ab73b9d63c442463f85cc13a0b5` |
| `PaperNX-switch-v0.4.0-rc3.zip` | 33,411,959 | `5d50c807189c3d8b5fc7f098fafff714108f5af5a10d947c8a5acf076fb1279e` |
| `manifest.json` | 7,354 | Verified with build receipt and source identity |

### Binary Metadata:
- **Architecture:** AArch64 (ELF64 executable, machine 183)
- **Format:** NRO0 with valid `.text`, `.rodata`, `.data` sections
- **NACP Title:** `PaperNX`
- **NACP Author:** `ysfbz`
- **NACP Version:** `0.4.0-rc3`
- **Embedded Icon:** 256×256 baseline JPEG (`b47198387614a9e66a570d7faaefff0fa496033d4481060316a086059b506bb5`)
- **Embedded RomFS:** `NotoSans-Regular.ttf`, `NotoSans-SemiBold.ttf`, `NotoSerif-Regular.ttf`, `OFL.txt`
- **Bundled Licenses:** 59 licenses (MuPDF, SDL2, HarfBuzz, libnx, switch-portlibs, SIL OFL)

---

## 7. Remaining Physical Nintendo Switch Tests

The following 35 verification items from `docs/DEVICE_TEST_PLAN.md` must be conducted on a real Nintendo Switch before a final release:
1. Cold boot from Homebrew Menu (album / title override mode).
2. Orientation toggle (Landscape, Portrait CW, Portrait CCW) with Joy-Con attached and detached.
3. Touchscreen responsiveness, swipe gestures, and edge swipe handling.
4. Physical volume and sleep/wake button behavior during active reading.
5. MicroSD card hot-removal prevention and write flush integrity.
6. Battery life and thermal profiling during extended reading sessions.
