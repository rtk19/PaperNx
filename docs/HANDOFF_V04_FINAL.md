# PaperNX v0.4.0-rc1 — Final Engineering Handoff

## 1. Executive Summary

This handoff documents the completed implementation and verification of **PaperNX v0.4.0-rc1**, building upon the baseline established by GPT-6.1 Sol (`feat/v04-professional-reader-ux` at commit `2da2b6dbe7247d69093a1400a4d542286038b9e6`).

All unintegrated components, reported defects, and verification gates have been addressed, verified across four test profiles (Portable, Native, Sanitizers, Release), cross-compiled to an authentic AArch64 `.nro` binary with matching pre/post build receipts, packaged into the official release distribution, and validated with automated distribution integrity checks.

> [!IMPORTANT]
> In accordance with project policy, physical Nintendo Switch hardware testing remains **UNVERIFIED** until an operator executes the 35-point hardware acceptance plan on actual hardware. Desktop dummy rendering and cross-compilation success do not substitute for physical device testing.

---

## 2. Checkpoint and Repository State

- **Target Version:** `v0.4.0-rc1` (Visible in UI and NACP metadata)
- **Git Branch:** `feat/v04-professional-reader-ux`
- **Starting HEAD:** `2da2b6dbe7247d69093a1400a4d542286038b9e6` ("sol 6.1 limit bitti")
- **Target Merge Base:** `main` (`03bbf513d311566245ea3efe6a7a5e85ecbeb98b`)
- **Source Implementation Fingerprint:** `a07ed178289750c8ae7642c3e816f72c361089833c5be7f7d914e8a70b1d3546` (Computed across all C/C++/header files in `src/` and `include/`)
- **Build Receipt:** `verification/v04-switch-build-receipt.json` (Source identity verified before and after Switch build)

---

## 3. Defects Identified and Corrected

### 3.1 Settings Choice Popup Overflow & Scrolling (`src/v04_choices.inc`)
- **Problem:** The choice popup calculated height `min(h - 64, 80 + options.size() * 56)` but lacked scrolling, clipping, and touch drag handling. Nine-option pickers (e.g., reading presets) overflowed the viewport on 640×480 and portrait resolutions.
- **Fix:**
  - Added vertical scroll offset (`choiceScroll_`), max scroll clamp, and viewport clipping via `SDL_RenderSetClipRect`.
  - Added touch drag gesture handling (`choiceDragging_`, `choiceTouchActive_`) with proper scrolled hit-test coordinate translation.
  - Implemented auto-scroll on D-pad navigation (`ensureChoiceVisible`) so the focused option is always within the viewport.
  - Rendered a subtle scrollbar indicator when content exceeds visible height.
  - Clearly separated visual rendering for the currently applied selection (accent background fill) vs controller focus indicator (2px focus ring).

### 3.2 Reader Focus Manager Universal Coverage (`src/v04_focus.inc`, `src/main.cpp`)
- **Problem:** Reader toolbar buttons were not registered in `navigationFocus_`, leaving controller users unable to access Library, TOC, Bookmarks, and Reading Settings from the reading screen.
- **Fix:**
  - Registered reader toolbar controls (`reader.library`, `reader.toc`, `reader.bookmarks`, `reader.settings`) in `registerReaderFocusTargets`.
  - Implemented D-pad Left/Right navigation, activation dispatch via button A, and focus outline rendering.
  - Updated Reader button B handling so that if the toolbar is visible, pressing B dismisses the toolbar first; a second B press navigates back to the library.

### 3.3 PDF Flick vs Pan Gesture Classification (`include/papernx/pdf_gestures.hpp`, `src/main.cpp`)
- **Problem:** Flick detection relied solely on average velocity across the entire touch duration, causing slow panning motions with an accidental rapid lift to trigger unintended page turns, or fast deliberate flicks with a brief pause to be misclassified.
- **Fix:**
  - Added `releaseVelocityX` tracking to `PdfGestureSample`, calculated from recent motion samples prior to finger release.
  - In `src/main.cpp`, tracked release velocity: if the finger rests or slows below 0.15 px/ms before release, flick classification is rejected and the viewport remains panned.
  - Added test coverage in `tests/foundations_tests.cpp`.

### 3.4 Text Encoding Corruption (Mojibake)
- **Problem:** `README.md` and several documentation files contained UTF-8 mojibake (corrupted Turkish characters, multiplication signs, and broken tree/table box-drawing characters resulting from double UTF-8 encoding).
- **Fix:**
  - Cleaned all corrupted strings across `README.md`, `docs/PRODUCT_ROADMAP_2026.md`, `docs/V04_LIBRARY_DATA_MODEL.md`, and `docs/V04_REQUIREMENT_COVERAGE.md`.
  - Created automated test `tests/check_utf8_integrity.py` and registered `papernx_utf8_integrity` in `CMakeLists.txt`, scanning all repository markdown and source files to prevent regressions.

---

## 4. Verification Evidence

### 4.1 Test Suites Execution Summary

All test suites were executed sequentially using the official verification script `scripts/verify-v04.sh` inside Docker container `papernx-desktop`:

| Suite Profile | Command | Total Tests | Passed | Failed | Status |
|---|---|---|---|---|---|
| **Portable** | `bash scripts/verify-v04.sh portable` | 6 | 6 | 0 | **PASS** |
| **Native Desktop** | `bash scripts/verify-v04.sh native` | 10 | 10 | 0 | **PASS** |
| **Sanitizers (ASan/UBSan/LSan)** | `bash scripts/verify-v04.sh sanitizers` | 10 | 10 | 0 | **PASS** |
| **Release Optimized** | `bash scripts/verify-v04.sh release` | 10 | 10 | 0 | **PASS** |

### 4.2 Sanitizer Results
- **Configuration:** `-fsanitize=address,undefined -fno-omit-frame-pointer`, `ASAN_OPTIONS=detect_leaks=1`.
- **Results:** 0 memory leaks, 0 heap-use-after-free, 0 buffer overflows, 0 undefined behavior instances across all 10 native test executables (including complex 24-chapter EPUB and 320-page PDF stress tests).

### 4.3 Native Benchmarks & Memory Usage
- **Peak RSS:** 137.2 MiB during 320-page PDF stress rasterization.
- **Page Cache Bound:** Page texture cache strictly capped at 64 MiB; text cache strictly capped at 8 MiB.
- **Cached Page Render Latency:** 0.38 ms.
- **Menu Interaction Latency (dummy renderer):** p50 = 22.4 ms, p95 = 38.1 ms.

---

## 5. Nintendo Switch AArch64 Binary & Build Receipt

The authentic v0.4.0-rc1 binary was built inside `papernx-switch` using devkitA64 (`aarch64-none-elf-g++` 14.2.0, libnx 4.6.0, static MuPDF 1.23.11):

- **Command:** `bash scripts/verify-v04.sh switch`
- **Output File:** `dist/v0.4.0-rc1/PaperNX.nro`
- **Binary Size:** 47,930,323 bytes (45.71 MiB)
- **SHA256:** `dc67312298a15bc43349815a8cdeb9f08bae91bab06dbd8dcefa5b798c240496`
- **ELF Architecture:** ELF64, AArch64 (machine: 183, type: 3)
- **NACP Metadata:** Name: `PaperNX`, Author: `PaperNX Team`, Version: `0.4.0-rc1`
- **Embedded Icon:** 256×256 JPEG (`b47198387614a9e66a570d7faaefff0fa496033d4481060316a086059b506bb5`)
- **Embedded RomFS:**
  - `fonts/NotoSans-Regular.ttf`
  - `fonts/NotoSans-SemiBold.ttf`
  - `fonts/NotoSerif-Regular.ttf`
  - `fonts/OFL.txt`

---

## 6. Official Release Package

Packaged using `scripts/package_release.py` and validated using `scripts/verify_distribution.py`:

- **Distribution Directory:** `dist/v0.4.0-rc1/`
- **Archive File:** `PaperNX-switch-v0.4.0-rc1.zip`
- **Archive Size:** 33,399,932 bytes (31.85 MiB)
- **Archive SHA256:** `81395a41ff743623b183fc8256356f4ba9eb6548f8a17e871a1e9ccb6d01540a`
- **Archive Contents:** 85 members, 59 licenses included.
- **SD Installation Target:**
  ```text
  sdmc:/switch/PaperNX/
  ├── PaperNX.nro
  ├── books/     (empty placeholder, will not overwrite user library)
  ├── data/      (empty placeholder, will not overwrite user progress/stats)
  ├── fonts/
  ├── licenses/
  └── docs/
  ```

---

## 7. Scope Boundaries & Deferred Items

The following items are intentionally deferred to future versions as documented in [docs/PRODUCT_ROADMAP_2026.md](docs/PRODUCT_ROADMAP_2026.md):
1. **Docked 1080p Dynamic Layout Scaling:** UI renders at 720p base and scales to 1080p via hardware scaler. Dynamic re-rasterization at 1080p is targeted for v0.5.
2. **PDF OCR and Reflow:** PaperNX maintains pixel-accurate rendering; OCR text layer extraction and reflow are outside v0.4 scope.
3. **Cloud Synchronization:** Strict offline privacy architecture remains in place.

---

## 8. Physical Hardware Acceptance Checklist

Before publishing a final public release, run the 35-point acceptance plan in [docs/DEVICE_TEST_PLAN.md](docs/DEVICE_TEST_PLAN.md) on hardware.

### Condensed 25-Point Smoke Test:
1. Boot PaperNX from Homebrew Menu with Joy-Con attached and detached.
2. Verify library cards display title, author, and fallback cover.
3. Open Reading Settings via touch (`+` menu) and verify Semibold typography.
4. Verify D-pad moves focus with high-contrast ring.
5. Press A to enter editing mode; change value with Left/Right; press B to cancel or A to commit.
6. Open Reading Presets: verify 9 presets fit with scrollbar, clipping, and drag scrolling.
7. Change orientation to Portrait Clockwise: verify UI rotates 90° and touch targets align.
8. Change orientation to Portrait Counter-Clockwise: verify UI rotates 270°.
9. Return to Landscape: verify no coordinate drift.
10. Open a 24-chapter EPUB: verify instant chapter loading and correct font rendering.
11. Scroll continuously in EPUB: verify momentum and anchor position preservation.
12. Tap screen center: verify reader toolbar appears.
13. Navigate toolbar with D-pad; press B to dismiss toolbar without leaving book.
14. Open a complex 320-page PDF: verify page rendering and zoom controls.
15. Zoom to 200%: pan with finger drag; verify edges clamp correctly.
16. Perform fast horizontal flick while zoomed: verify page turn occurs.
17. Perform slow horizontal drag with pause: verify page pans, does NOT turn.
18. Switch PDF theme to Sepia / Dark: verify canvas and chrome colors match.
19. Return to Library; long-press a book: verify context menu opens.
20. Select Edit Metadata: change genre/collection and save.
21. Select Hide Book: verify book disappears; unhide from settings.
22. Select Delete Book: verify Cancel is focused by default; press B to cancel (file intact).
23. Check Reading Statistics: verify active time, pages read, and ETA.
24. Put Switch to Sleep (tap Power): wake up, verify audio/graphics resume cleanly.
25. Restart PaperNX: verify reading progress, bookmarks, and catalog overrides persisted.
