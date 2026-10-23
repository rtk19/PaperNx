# Fork update: 0.17.4

Single-image comic reading is restored in all orientations, independently of existing book/PDF spread preferences. The paired comic compositor, header-pair detection, centre spine and spread zoom geometry are removed. Single-image pan/zoom, paper-bounded animation and outside-to-paper gesture contact remain, along with asynchronous loading, retained CBR streams, controls, folios, expanded-area settings, menus and data. [Verification](COMIC_PERFORMANCE.md) includes historical single-image fit rectangles, repeated cold input and original 37-page CBR fixtures. Physical Switch timings remain unverified.

# Fork update: 0.17.3

Consecutive comic turns preserve input while cold pair images/geometry prepare asynchronously. A new touch grips after the prior release settles; zoom remains immediately available and cancels pending turns. The CBR worker no longer discards its stream/dictionary after three idle seconds, and prepares two following normal pairs. New real SDL event regressions cover the cold second spread, repeated gestures/buttons, idle archive retention and focus-loss recovery. Existing reading behaviour, data and 0.16 menus remain. See [performance](COMIC_PERFORMANCE.md). Switch timings remain unverified.

# Fork update: 0.17.2

Fixes comic texture-cache churn and synchronous neighbour work on the UI thread. Base pages stay within a separate resolution budget; directional preparation and two-page zoom sharpening run on a worker. Visible pages are protected, forward ZIP/RAR5 reads retain their stream, and input does not repeatedly present per queued event. Original 37-page CBR performance fixtures and streaming tests cover the regression. Reading features, data and 0.16 menus remain. See [performance](COMIC_PERFORMANCE.md). Switch timings remain unverified.

# Fork update: 0.17.1

Restores external application menus and screens to 0.16.0 while keeping the compact reading controls and reader features. Comic animations use actual paper bounds, margins establish contact before folding, zoomed pairs can turn, spines stop at artwork edges and both next-group images warm in the correct cache. Book drags from top/bottom margins remain interactive. Existing library, preferences and reading records are retained. See [interaction corrections](READER_INPUT_FIXES.md). Physical Switch acceptance remains unverified.

# Fork update: 0.17.0

Rebuilds the application structure around Home, Library, Collections, Journal and Files, with side navigation in wide layouts and bottom navigation in portrait/compact layouts. Includes first-library guidance, resume hero, new import workspace, responsive settings with reorganized application options, tiled quick actions, a book-action workspace, new contents/bookmarks navigation, selection dialogs, paper-colour preview and staged loading screens. Journal screens share the shell. Reading-page layout, animations, folios, Hebrew direction, zoom, folders, catalog and journal features remain. See [experience and preserved-feature map](APP_EXPERIENCE.md).

# Fork update: 0.16.0

Modern menus, cards, settings, dialogs and journal use dark/light/follow-paper appearance with seven companion-player accents. Reading paper keeps its own style. Whole-folder import supports up to 32 additional scan roots, launch/manual asynchronous scans and an atomic disposable inventory. Hash-based merging and metadata-only EPUB opening reduce repeated work. In landscape, shoulder buttons follow the opened EPUB's reading direction: English R forward/L back, Hebrew L forward/R back. Portrait retains its existing shortcuts. See [menus and library](MODERN_LIBRARY.md).

# Fork update: 0.15.0

Reading-area expansion and landscape spreads are independent default-on settings with original-mode rollback. Comics gain a tight centre gutter, whole wide/ComicInfo double pages, cached local pairing, stable two-page zoom, pinch-centroid anchoring and up-arrow/Y return-to-top. Spread drag follows full-screen travel. See [reading layout](READER_SPREADS.md).

# Fork update: 0.14.0

Landscape paginated books and comics now use two physical leaves with language-aware ordering and centre-seam curls. Folios belong to the animated sheets in landscape and portrait. Duplicate EPUB margins are removed; clear header/footer space remains. Open tools show the visible page range and progress. Continuous scrolling and enlarged artwork retain their selected modes. See [layout and behavior](READER_SPREADS.md) and [verification](TEST_RESULTS.md).

# Fork update: 0.13.0

The page footer is now shared by books and comics in every reading mode: a numeral with tools hidden; `current of total • x%` with tools shown. Both occupy the same empty bottom strip. Releases now use ordinary numeric versions without reader/comics suffixes: 0.13.0 for this addition, 0.13.1 for a later fix, 0.14.0 for a later feature. Automatic upgrades from the old versions remain supported. See [reader controls](PAGE_NAVIGATION.md) and [verification](TEST_RESULTS.md).

# Fork update: v0.12.0-reader.4

Book footer visibility now follows the requested interaction: current-page numeral with tools hidden; `current of total • x%` with tools visible. Both occupy the same existing bottom strip. Includes complete continuous-reading totals and final partial-screen completion. Automatic fork updates remain available. See [book controls](PAGE_NAVIGATION.md) and [verification](TEST_RESULTS.md).

# Fork update: v0.12.0-reader.3

PaperNX now checks this fork on launch and through **+ → Check for update**. Version prompts, notes, verified streamed NRO downloads, deferred replacement and restart follow the NX-torrent-player interaction. Fork prereleases and numeric reader suffixes are supported. Existing reader/comic features remain. See [automatic updates](AUTO_UPDATE.md) and [verification](TEST_RESULTS.md). Physical Switch acceptance remains unverified.

# Fork update: v0.12.0-reader.2

Book-only captions occupy the existing empty top/bottom strips: chapter pages remaining while the toolbar is visible, current-page numeral always. Hebrew bold/italic and author emphasis survive reader typography settings, with styled fallback fonts isolated from regular text. Comics keep their existing behavior. See [controls](PAGE_NAVIGATION.md), [Hebrew rendering](HEBREW.md) and [verification](TEST_RESULTS.md). Physical Switch acceptance remains unverified.

# Fork update: v0.12.0-reader.1

Adds transactional progress-bar scrubbing with asynchronous page previews, a virtualized page/chapter/bookmark browser, Hebrew and nested chapter labels, exact continuous EPUB fragment links, RTL page order and controller navigation. Previews do not change the saved position until confirmation; comic zoom and top-of-page resets remain. Developer: Refael Bar. See [PAGE_NAVIGATION.md](PAGE_NAVIGATION.md) and [TEST_RESULTS.md](TEST_RESULTS.md).

# Fork update: v0.11.0-reader.4

The fork developer displayed by Homebrew Menu is now **Refael Bar**. Build metadata and README credits identify the fork developer; original PaperNX attribution and licenses remain. Reading behavior is inherited from reader.3.

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
