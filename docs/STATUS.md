# Fork update: v0.11.0-reader.3

Arrows, D-pad, LS and RS now follow the display orientation in landscape and both portrait directions. The same rotation used for touch is applied to directional controls, including continuous zoomed panning and modal navigation. Holding a direction across an orientation change redirects movement immediately; release stops it. Zoom and logical next/previous shortcuts keep their behavior. See [COMICS.md](COMICS.md) and [TEST_RESULTS.md](TEST_RESULTS.md). Physical Switch acceptance remains pending.

# Fork update: v0.11.0-reader.2

LS and RS now share directional navigation, modal focus, repeat timing and zoomed panning. Holding ZR/ZL changes zoom smoothly around the viewport center; D-pad/arrows begin panning without the former 64-pixel impulse. Enlarged comic curls use the actual complete sheet and a separately positioned incoming page, clipped to the reading area. Zoom and top-of-page resets remain. See [COMICS.md](COMICS.md) for controls and [TEST_RESULTS.md](TEST_RESULTS.md) for validation. Physical Switch acceptance remains pending.

# Fork update: v0.11.0-reader.1

Adds series shelves with natural issue numbering and background metadata indexing, zoom-preserving comic curls with top-of-page resets, continuous held-direction panning, global RS scrolling, and automatic Hebrew EPUB paragraph direction, directional justification and indentation, Hebrew UI fallback fonts and Unicode mixed-language display. EPUB language/spine metadata controls page curls. Existing books, reading records and the English/Turkish UI remain in place. See [HEBREW.md](HEBREW.md). Physical Switch acceptance remains pending.

# Fork update: v0.10.0-comics.2

Fixes high-resolution comic covers, automatically refreshes comic cover caches,
adds an always-visible Fit page zoom reset, and accelerates zoomed panning/pinch
with a bounded GPU page cache and momentum. Native viewport detail is restored
at rest for extreme zoom. Physical Switch performance remains unverified.

# Fork update: v0.10.0-comics.1

CBZ/CBR comics now share the existing reader and library: page curls with touch,
original artwork colors, manga direction, zoom/pan, continuous scrolling,
ComicInfo covers/metadata and persistent within-page bookmarks. See
[COMICS.md](COMICS.md) for supported formats, controls, validation and limits.
Physical Switch testing remains pending. Historical upstream status follows.

# PaperNX v0.9.0 — Project status

**Release:** [v0.9.0](https://github.com/ysfbz/PaperNx/releases/tag/v0.9.0), published 19 October 2026  
**Tagged source:** `135df6be80a622d12ce59c173b31068da1844e17`  
**Platform:** Nintendo Switch (native C++17/C11, SDL2, SDL2_ttf, MuPDF 1.23.11); Linux desktop targets support development/testing.  
**Developer:** Yusuf Buzrul, with AI assistance.

## Shipped implementation

- Offline EPUB/PDF reader, paginated and continuous EPUB reading, PDF zoom/fit/pan/filter.
- Touch-first library and file browser, quick `+` menu, optional controller navigation, display orientation and global/per-book appearance.
- Reading bookmarks/progress/history, library management, metadata and collections.
- Offline reading journal, goals, statistics and export to SD.
- English default UI, switchable Turkish and saved language preference.
- Minimal Switch NRO ZIP and independent license/notice archive.

## Verification state

The release-preparation handoff reports portable 7/7, native 11/11, Release-native 11/11 and sanitizer 11/11 test suites passing, plus verified cross-compiled AArch64 NRO/package metadata. These reports are **not fresh tests executed by documentation editing**. See [TEST_RESULTS.md](TEST_RESULTS.md).

**Physical Nintendo Switch acceptance remains UNVERIFIED for all 35 checks** in [DEVICE_TEST_PLAN.md](DEVICE_TEST_PLAN.md). Desktop simulation cannot establish real touchscreen latency, portrait touch mapping, controller reconnect, suspend/resume or microSD durability. The public Release is labelled a normal release, but the GitHub release flag does not imply hardware certification.

See [KNOWN_ISSUES.md](KNOWN_ISSUES.md), [BUILD_SWITCH.md](BUILD_SWITCH.md), [RELEASE_REPORT.md](RELEASE_REPORT.md) and [PRODUCT_ROADMAP_2026.md](PRODUCT_ROADMAP_2026.md). Earlier v0.3/v0.4 branches and deleted reports are historical and not active release targets.
