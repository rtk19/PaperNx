# PaperNX application experience (0.17.0, historical)

**0.17.1 rolls back the external application and menu redesign below to 0.16.0. The compact reading toolbar remains. See [the correction report](READER_INPUT_FIXES.md) for current behaviour.**

This release changes information architecture, interaction paths and responsive geometry. The companion NX-torrent-player supplies the shared accent family and the separation of browsing from content consumption. Book-library and reading applications inform the resume-reading entry point, cover-first library, saved-place workspace and quiet reading controls. The reader remains native SDL/MuPDF; no browser or heavyweight UI runtime was introduced.

## Screen and feature map

| Screen | New structure | Existing functionality retained |
| --- | --- | --- |
| Opening | Branded reading-space composition, stage progress, dedicated book-opening frame | Existing startup recovery and book opening; no artificial delay |
| Empty library | Three actionable guidance cards and first-folder entry | Browse individual files, import folders, reading preferences and journal |
| Home | Resume-reading hero with title, author, cover and progress; cover-first shelf | Series grouping, progress, favorites and long-press management |
| Library | Dedicated compact browse view; filters, search and add-book actions | All six categories, grid/list, sorts, search, metadata/series labels |
| Navigation | Home, Library, Collections, Journal, Files; side rail on wide screens, bottom bar on compact/portrait screens | Touch, mouse, D-pad, LS/RS, orientation-aware controls and + menu |
| Import | File cards, parent breadcrumb, import summary and persistent Add this folder action | Individual EPUB/PDF/CBZ/CBR opening, nested folders, launch/manual scans, original file locations |
| Settings | Category rail or six top category chips, individual setting rows, explicit Done/Reset and context-sensitive preview | Every text, paper, layout, navigation, comic/PDF, application and per-book/default option |
| Application settings | Interface and library options first; scan/layout/device/default-style options remain scrollable | Both default-on reader-layout switches, themes/accents, scan roots, launch scan, orientation, language and toolbar policies |
| Reading | Compact floating tool clusters in the existing empty top strip; consistent seek/previous/next/page controls below | Unchanged physical reading canvas, paper colours, captions, chapter progress, typography, page numbers and their animations |
| Contents | Book title, category navigation and a distinct preview/list workspace | Chapters, nested/RTL titles, page thumbnails, saved places, preview/cancel/confirm and scrub history |
| Saved places | Context panel and bookmark cards, separate Save this place and Remove actions | Bookmark identity, anchors, jump and delete |
| Quick actions | Scrollable tile grid, descriptive labels and orientation shortcut | Every former quick action, including update, help, history, Page Flip and statistics |
| Book management | Cover/title/progress header and two-column action grid | Open, favorite, all reading statuses, collections, metadata, details, hide, restore and confirmed deletion |
| Catalog | Shared elevated workspace, grouped actions and larger text-entry targets | Authors, genres, collections, collection editing, metadata, hidden books and statistics |
| Journal | Same application shell, horizontal section navigation and full-width dashboard | Overview, calendar/day detail, books/detail, history, goals, export, confirmed session deletion and reading-state resolution |
| Choices | Explicit back control, radio/check selection rows and scrolling | Every font/orientation/preset/interface/accent/folder option and controller selection |
| Paper colours | Larger swatches, a reading preview and explicit cancel/apply | All 16 colours, keyboard/controller selection, contrast hint and cancelled-change rollback |
| Software update | Status composition and separate release-note content area | Startup/manual discovery, notes, streamed download, integrity/identity checks, stage/restart and recovery |

## Reading behavior is an independent boundary

The application shell does not supply reader page dimensions. `ReaderLayout`, document pagination, comic pairing, mesh construction and zoom transforms retain their existing calculations. Top and bottom tools occupy the existing empty strips, and opening tools does not shift or shrink the book. The folio remains part of each animated physical sheet. Expanded reading area and Two pages in landscape remain independently switchable and On by default.

Hebrew/manga ordering, per-book landscape L/R controls, single-page/continuous modes, wide/ComicInfo scans, first-pinch texture reuse, centroid zoom, zoom-preserving return-to-top/Y and the corrected full-screen drag mapping remain covered by the earlier regression suites. Opening settings and contents preserves the reader's position and anchors.

## Layout and implementation

`app_layout.hpp` provides shared shell, import, library and navigation geometry. Painting, touch, focus and integration tests use those models. Settings use a shared setting-identity-to-display-order map, so reorganizing Application settings cannot silently change which option a row edits. Compact settings retain a fully visible touch row at 640×480, and controls remain reachable by dragging. Plus, back, tabs, scope and footer actions have separate hit regions.

The Home resume candidate is cached per library revision, avoiding a full catalog scan on every frame. Cover textures, text textures, page/thumbnail caches and asynchronous folder/metadata indexing retain their budgets. Only visible library rows draw. All screen artwork uses native geometry, bundled fonts and actual book covers. The new display face uses an already bundled font; no new font licenses or remote assets are required.

Existing progress, catalog, settings, folder inventories, covers and journal stores keep their formats and paths. This release introduces no data migration or collection reset. Physical Switch performance and touch feel still require real-device acceptance.
