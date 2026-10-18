# v0.4 known issues and release limits

All new Switch hardware checks, including both portrait rotations, sleep/resume, touch latency, SD errors and controller reconnect, are **UNVERIFIED**. Previous physical recordings are v0.3 defect evidence. Use [DEVICE_TEST_PLAN.md](DEVICE_TEST_PLAN.md); no stable release before new acceptance.

- Final automated/source/package gates are recorded in [TEST_RESULTS.md](TEST_RESULTS.md). Pure helper tests do not prove every focus target or small-window layout.
- PDF flick defaults are a conservative displacement/duration/average-velocity heuristic. They are not physical Switch tuned, release-velocity sampled or fully edge-aware. Ordinary slow pan takes precedence; real thresholds require hardware retest.
- PDF Dark/Sepia/Invert filters map page colors and matching reader chrome (Dark: RGB 28,30,32; Sepia: RGB 236,224,198) without mutating the global library appearance. Note that whole-page raster filtering affects embedded illustrations as well as text. Crop, column-aware navigation, OCR and reflow are **DEFERRED**.
- Catalog identity is existing path plus file size. Moving/renaming/replacing a book can create a new identity; automatic progress/metadata reconciliation is **DEFERRED**. OPF subjects and Calibre/EPUB3 series indices are extracted on opening; independently normalized multiple author/genre entities and pre-open full-library extraction remain limited.
- Confirmed deletion is limited to scanned regular EPUB/PDF files beneath the allowed root. There is no undo/trash, arbitrary file manager or race-immune descriptor-based deletion claim. Device tests must use disposable copies. Shelf deletion never deletes files.
- Stats are conservative measured activity, not eye tracking. Idle cutoff can count a short pause and exclude a long reading interval without input. ETA needs evidence; chapter ETA and full annual/calendar visualization are **DEFERRED** unless explicitly implemented in the final report.
- Progress anchors retain the prior fallback for malformed EPUB markup; geometric restoration may differ slightly after relayout. Keep backups and original books.
- FAT/SD interruption durability differs from desktop. Flushed replacement/backup recovery reduces risk but cannot prove power-loss guarantees.
- External fonts, full gesture remapping, dictionary datasets, annotations, in-book search, CBZ/CBR, OCR, AI recaps and optional wireless services are **DEFERRED** under the [roadmap gates](PRODUCT_ROADMAP_2026.md). No supported-format claim is made for comic archives.

Historical limitations are preserved in [V03_KNOWN_ISSUES.md](V03_KNOWN_ISSUES.md); they are not automatically closed by this update.
