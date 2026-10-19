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
