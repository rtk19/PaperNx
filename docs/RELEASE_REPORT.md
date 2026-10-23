# Fork release: 0.17.1

A forward patch that rolls back external application/menu presentation to 0.16.0 and corrects comic paper bounds, centre spines, touch routing and preparation of neighbouring facing pages. The compact reading toolbar, prior reading layouts, Hebrew support, L/R direction, folios, zoom and animations remain. Book top/bottom drags now stay interactive outside visible controls. No stored-data migration or reset is introduced. See [implementation and verification details](READER_INPUT_FIXES.md).

[Release assets](https://github.com/rtk19/PaperNx/releases/tag/v0.17.1) contain the NRO, SD installation layout, corresponding tagged source, licenses, SHA-256 checksums and verification evidence. Numeric version: 0.17.1; developer: Refael Bar. Physical Switch interaction and cold-SD timings remain unverified.

# Fork release: 0.17.0

A structural UI/UX remake, with a Home/resume experience, dedicated book library, persistent application navigation, a guided first-library flow, a folder-import workspace, responsive settings, tiled actions, book-management cards, new contents and saved-place workspaces, a unified journal, colour previews and opening screens. All existing reader behavior and data remain: physical pages and folios, RTL/LTR controls, smart comic spreads, curl animation, full-width drag, focal pinch zoom, return-to-top, scan roots, catalog operations, journal and verified automatic updates. See [screen and feature map](APP_EXPERIENCE.md).

[Release assets](https://github.com/rtk19/PaperNx/releases/tag/v0.17.0) include the NRO, installation ZIP, exact tagged source, licenses, checksums and test/screenshot evidence. Developer: Refael Bar. Physical Switch acceptance remains unverified.

# Fork release: 0.16.0

The second requested release modernizes all menus through shared rounded controls, cards, elevated dialogs, switch rows, focus outlines and a cached accent background. Dark, light and follow-reading appearances and seven accents are independent of reading paper. New library-folder management adds whole folders, scans on launch or manually in a background worker, and loads an atomic inventory on subsequent launches. Deduplicated roots, hash-based batch merges and metadata-only EPUB indexing avoid repeated traversal and full pagination during indexing. Landscape R/L now follows the language/direction of each opened book, including Hebrew.

[Release assets](https://github.com/rtk19/PaperNx/releases/tag/v0.16.0) include the NRO, installation ZIP, exact tagged source, licenses, checksums and verification evidence. The first requested layout/comic release remains separately available as [0.15.0](https://github.com/rtk19/PaperNx/releases/tag/v0.15.0). Developer: Refael Bar. See [menus and library](MODERN_LIBRARY.md). Physical Switch acceptance remains unverified.

# Fork release: 0.15.0

Adds default-on controls for expanded reading area and landscape spreads, with independent return to the original modes. Comic artwork meets at a centre gutter; wide scans and ComicInfo double pages remain whole. Comic spread zoom keeps both pages, reuses fitted textures at the first pinch and anchors to the gesture centroid. The up-arrow or Y returns to the top without resetting zoom or horizontal position. Spread curls now use full-screen finger travel. See [layout details](READER_SPREADS.md).

[Release assets](https://github.com/rtk19/PaperNx/releases/tag/v0.15.0) include NRO, installation ZIP, exact source, licenses, hashes and test evidence. Developer: Refael Bar. Physical Switch acceptance remains unverified.

# Fork release: 0.14.0

Adds landscape two-page spreads for paginated EPUB, PDF, CBZ and CBR, with RTL ordering for Hebrew/manga. A leaf turns across the centre onto its real reverse page; outgoing and incoming page numerals are part of the paper. Portrait also curls its folio. EPUB uses more screen area by removing duplicate MuPDF margins, and fitted artwork gains a smaller bottom margin. Open-tool summaries show a page range for spreads and reach 100% on the final spread. Explicit continuous scroll and zoom/pan modes remain available.

Available through automatic update or [Release Assets](https://github.com/rtk19/PaperNx/releases/tag/v0.14.0): NRO, installation ZIP, exact source, licenses, checksums and verification evidence. Developer: Refael Bar. See [layout details](READER_SPREADS.md). Physical Switch acceptance remains unverified.

# Fork release: 0.13.0

EPUB, PDF, CBZ and CBR now share the bottom caption: current page with tools hidden; `current of total • x%` with tools visible. The caption always occupies the existing bottom strip, with the seek bar above it. The obsolete comic footer and transient hint no longer cover this caption. Book chapter progress and comic artwork, zoom, pan and ordering retain their behavior.

New application versions use plain semantic numbers. This new comic behavior is a feature release, 0.13.0; a subsequent fix becomes 0.13.1, and the next feature release becomes 0.14.0. Historical suffixed tags remain unchanged. The release is published as a normal GitHub release; old automatic updaters can migrate to it.

Available via automatic update or [Release Assets](https://github.com/rtk19/PaperNx/releases/tag/v0.13.0), including NRO, installation ZIP, exact source, licenses, checksums and evidence. Developer: Refael Bar. Physical Switch acceptance remains unverified.

# Fork release: v0.12.0-reader.4

Corrects the book footer: only the current page is shown while the toolbar is hidden. Opening the toolbar replaces that caption with `current of total • x%` in the same position and type. The percentage is rounded from current / total. Continuous EPUB totals count reading screens, and the final partial screen reaches the final page and 100%. Content layout, chapter captions and comics keep their existing behavior.

Available through the reader.3 automatic updater or [Release Assets](https://github.com/rtk19/PaperNx/releases/tag/v0.12.0-reader.4). NRO, installation ZIP, source, licenses, checksums and verification evidence are included. Developer: Refael Bar. Physical Switch acceptance remains unverified.

# Fork release: v0.12.0-reader.3

Adds automatic startup checks and **+ → Check for update** against rtk19/PaperNx. The updater includes fork prereleases, compares complete versions, shows release notes and streams the NRO with progress. HTTPS certificate, SHA256, size, NRO resources, application identity and version checks precede staging. Only the executable reported by the loader is eligible; books, settings and reading progress are untouched.

The running file is replaced after PaperNX releases its fonts and RomFS. Restart relaunches the updated path when hbloader supports it; otherwise reopen PaperNX from Homebrew Menu. Interrupted transfers cannot become pending updates. A failed swap attempts to restore the old executable. Developer: Refael Bar.

The first updater-enabled NRO must be installed manually. [Release Assets](https://github.com/rtk19/PaperNx/releases/tag/v0.12.0-reader.3) include NRO, installation ZIP, exact source, licenses, checksums, manifest and verification evidence. See [automatic updates](AUTO_UPDATE.md) and [test results](TEST_RESULTS.md). Physical Switch acceptance remains unverified.

# Fork release: v0.12.0-reader.2

Books now show chapter pages remaining in the top empty strip only when the reader toolbar is visible. A centered current-page numeral remains in the bottom strip on every page, without a total or remaining count. The document viewport is unchanged. Continuous EPUB uses screen-sized reading pages rather than spine/chapter numbers. Chapter boundaries use outlines, falling back to EPUB spine items; PDFs without outlines omit the unknown chapter caption.

Hebrew bold/italic and combined styles render through independent fallback font faces. Reader-controlled typography retains semantic, class and inline weight/style, underline, strikethrough and vertical emphasis while preserving chosen colors, spacing and alignment. Comic controls/appearance remain unchanged. Developer: Refael Bar.

The NRO, installation ZIP, exact tagged source, licenses, manifest, checksums and verification evidence are in [Release Assets](https://github.com/rtk19/PaperNx/releases/tag/v0.12.0-reader.2). See [verification](TEST_RESULTS.md) and [book controls](PAGE_NAVIGATION.md). Physical Switch acceptance remains unverified.

# Fork release: v0.12.0-reader.1

The bottom progress bar now previews pages during a drag and commits one position on release. A footer grid button and the + menu open Chapters, All pages and Bookmarks. The browser uses bounded, asynchronous thumbnails and virtualized rows, preserves comic zoom, supports RTL order and renders Hebrew chapter titles and nested links correctly. Continuous EPUB headings retain their resolved within-chapter offsets. Developer: Refael Bar; upstream attribution remains.

The complete installation ZIP, NRO, corresponding tagged source, licenses, checksums, manifest and verification evidence are in [Release Assets](https://github.com/rtk19/PaperNx/releases/tag/v0.12.0-reader.1). No user library or reading database is installed. See [controls](PAGE_NAVIGATION.md), [build instructions](BUILD_SWITCH.md) and [verification](TEST_RESULTS.md). Physical Switch acceptance remains unverified.

# Fork release: v0.11.0-reader.4

Updates the fork developer to **Refael Bar** in Switch NACP metadata, the application constant and current documentation. Original PaperNX credits and licensing are retained. Reading behavior is inherited from reader.3. The NRO, installation ZIP, exact source, license notices, hashes and evidence are available in [Release Assets](https://github.com/rtk19/PaperNx/releases/tag/v0.11.0-reader.4).

# Fork release: v0.11.0-reader.3

The current fork release adds Hebrew fonts and automatic EPUB direction, series
shelves and numeric issue sorting, zoom-preserving comic curls with a reset to
the top of each new page. This update also rotates arrows, D-pad and both sticks with the display in
either portrait direction, including held panning and modal focus. It retains
unified LS/RS navigation, continuous
ZR/ZL zoom, removes the initial directional-pan jump and makes zoomed curls
follow the complete paper geometry instead of a viewport rectangle.
Download the NRO or installation ZIP from
[the fork Release Assets](https://github.com/rtk19/PaperNx/releases/tag/v0.11.0-reader.3).
The manifest and checksums identify the actual binary and exact source revision;
the source ZIP includes the full FriBidi 1.0.12 source archive and the MuPDF RTL
patch/build scripts. Font notices, third-party licenses and verification logs
are separate release assets. User books/progress are excluded from the package.

See [current checks](TEST_RESULTS.md), [installation/build instructions](BUILD_SWITCH.md),
[comic/series controls](COMICS.md) and [Hebrew behavior](HEBREW.md).
Physical Switch acceptance remains unverified. The report below describes the
historical upstream v0.9.0 release and its different artifacts.

# PaperNX v0.9.0 — Release verification report

**Published:** [v0.9.0](https://github.com/ysfbz/PaperNx/releases/tag/v0.9.0), 19 October 2026  
**Tag source commit:** `135df6be80a622d12ce59c173b31068da1844e17`  
**Application / author:** PaperNX / Yusuf Buzrul  
**Distribution:** Nintendo Switch AArch64 homebrew. Linux/desktop targets exist for development, not as separately published platform ports.

## Published release artifacts

| File | Bytes | SHA256 (GitHub Release digest) |
|---|---:|---|
| [`PaperNX.nro`](https://github.com/ysfbz/PaperNx/releases/download/v0.9.0/PaperNX.nro) | 48,090,067 | `9020d67962b5dda01b7da2405a5d023631e47e3a4e3be96662e4a48ad292b9ef` |
| [`PaperNX-switch-v0.9.0.zip`](https://github.com/ysfbz/PaperNx/releases/download/v0.9.0/PaperNX-switch-v0.9.0.zip) | 33,261,595 | `99b8ef910277a67992993112c73d0e5c2bfef651e481dfd97581a036baa18bd0` |
| [`PaperNX-licenses-v0.9.0.zip`](https://github.com/ysfbz/PaperNx/releases/download/v0.9.0/PaperNX-licenses-v0.9.0.zip) | 190,942 | `9c4c0d1f2042479c4723cbbaa3a83d0ce9c0584180a53d94127c575170fff859` |

Three corresponding `.sha256` sidecar assets are also published. **Verification scope:** The values above were checked against the *current published GitHub Release asset API*, rather than copied from older local reports. In this documentation session, actual binary downloads and independent recalculation of SHA256 **were not possible**. To perform independent byte-level verification, download an artifact and check `sha256sum <file>`, or on Windows `Get-FileHash <file> -Algorithm SHA256`, comparing both the table and its sidecar.

Older local report hashes `f05651ba…` (NRO) and `60220685…` (Switch ZIP) belong to a different build; they must **not** be used for the current Release. Local `dist/v0.9.0/manifest.json` is a build receipt and is not an additional public download.

## Build and automated quality gates

The v0.9.0 release-preparation report records:

| Check | Reported outcome | Reproduce with |
|---|---|---|
| Portable C++17 | PASS, 7/7 | `scripts/verify-v04.sh portable` |
| Native SDL/MuPDF | PASS, 11/11 | `scripts/verify-v04.sh native` |
| Native Release | PASS, 11/11 | `scripts/verify-v04.sh release` |
| ASan/UBSan and leak detection | PASS, 11/11 | `scripts/verify-v04.sh sanitizers` |
| Switch AArch64 ELF, NRO, NACP, icon, RomFS | PASS reported | `scripts/verify-switch.sh` |
| Minimal ZIP / separate license ZIP validation | PASS reported | `scripts/verify_distribution.py` |
| Physical Nintendo Switch (35 checks) | **UNVERIFIED** | [Device test plan](DEVICE_TEST_PLAN.md) |

These are outcomes reported by the prior build process, **not newly executed by this documentation change**. A successful GitHub Actions run for the exact final tagged commit was not independently found; historical `verification/` logs are no longer tracked.

The repository [Makefile](../Makefile) specifies NACP title `PaperNX`, author `Yusuf Buzrul`, version `0.9.0` and icon `assets/branding/PaperNX-icon-switch.jpg`. The verifier checks AArch64 ELF/NRO0 and embedded RomFS fonts (NotoSans Regular/SemiBold, NotoSerif and OFL notice). The Switch ZIP ships `switch/PaperNX/PaperNX.nro`, bundled fonts and an empty `books/` placeholder; the license notices are a **separate ZIP**. No user `data/` or `logs/` is installed, reducing overwrite risk but **not guaranteeing zero data loss** during power/SD failures.

**Physical acceptance:** Real Switch operation, touch/portrait, sleep/resume, controller reconnection and media failure remain **UNVERIFIED**. [Build guide](BUILD_SWITCH.md) · [Test report](TEST_RESULTS.md) · [Known limitations](KNOWN_ISSUES.md).

A documentation-only commit on `main` does not alter the published `v0.9.0` tag or binaries.
