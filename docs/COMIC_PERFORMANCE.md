# Comic interaction performance — 0.17.2

## Reproduced regression

Since the close-facing-page layout, fitted comic pages used the same up-to-six-megapixel textures as zoom. A texture conservatively consumes four bytes per pixel, so two visible pages plus four speculative neighbours could require 144 MiB in the 64 MiB page cache. Neighbour preparation evicted visible pages. The next toolbar frame or gesture then reopened and rasterized those pages on the UI thread. Preparation itself also decoded images synchronously on that thread.

The generated high-resolution CBZ reproduced **12 UI-thread rasterizations in four preparation/toolbar cycles**, taking **498 ms** on the desktop software renderer. These are reproduction measurements, not Switch timings.

RAR added another cost: ZIP/RAR5 page reads reopened the archive for every image. For a solid RAR5 this can replay preceding compressed entries. Speculative reverse reads also disrupted the already-persistent RAR4 decoder's forward state.

## Correction

- Base images use a two-megapixel/2048-side ceiling. Four active/forward images fit below 32 MiB; six base pages fit below 48 MiB. The existing overall LRU ceiling remains 64 MiB.
- Speculative texture uploads protect both currently displayed page keys. An upload that cannot fit alongside protected images is rejected without displacing them.
- A worker owns a separate, 96 MiB-budgeted MuPDF document. Archive reads, image decoding and reading-colour conversion for neighbours and zoom detail happen there. Its queue holds at most six jobs and its completed RGB queue at most two images. SDL texture creation/upload remains on the UI thread, at most one result per iteration.
- Base jobs precede detail jobs. Detail uploads wait for rest; results arriving during a gesture are discarded and may be retried later, keeping queue capacity available. Only two visible high-detail textures are retained, outside the base-page LRU; their ceiling remains six megapixels/3072 per side. Panning and pinching reuse their surfaces. On Switch, the worker requests a lower priority than normal UI work.
- Neighbours queue during input and animations, rather than requiring an event-free frame. Preparation follows the last navigation direction; already visited pages remain in the cache instead of being decoded again speculatively in reverse.
- A retained libarchive stream preserves physical forward ZIP/RAR5 traversal and the solid dictionary. Backward/random reads reopen when required. Errors reset the cursor, and closing a document releases it. RAR4 retains its existing persistent unarr stream.
- Completed jobs carry their book/style identity and generation; closing/changing the book discards stale work. Oriented dimensions transfer into the active document's metadata cache on its owner thread, avoiding extra archive reads for warmed page geometry.
- The small animated folio uses a much smaller mesh and skips unused lighting calculations. The full paper mesh remains. The event loop presents a newly started button turn promptly, while draining later input without presenting once per queued event.

Menus, library data, progress, reading settings, landscape spreads, smart wide pages, RTL/LTR controls, pinch-centroid zoom, return-to-top/Y, folios, paper-only folds and edge gestures retain their behaviour. No data migration or reset is introduced.

## Verification and limits

The performance suite covers original high-resolution CBZ art and a generated **37-page, approximately 43.5 MB stored RAR5/CBR**. Its standalone Python generator needs no proprietary archiver. Extra release-evidence runs use original **compressed solid RAR4 and RAR5** archives of 37 pages and approximately 43 MB. Existing fixtures also cover compressed/solid RAR4/RAR5, random access, damaged images and natural filename order.

The fixed scenario recorded **zero UI-thread rasterizations** in toolbar, warm-turn, pan and pinch sequences. Four toolbar/preparation operations measured approximately **6–10 ms** in the 37-page CBR runs (desktop software/OpenGL). Tests assert decode counts, cache budgets, both detail surfaces, cancellation and shutdown rather than flaky wall-clock thresholds. A warm turn and pan still work with the fixture's backing path temporarily unavailable. Separate document tests remove that path after the first read to verify retained forward streams, then restore it to verify backward/error recovery.

Native and ASan/UBSan/leak checks cover all reader/library functions. Offscreen OpenGL checks include both compressed solid RAR variants, paper bounds, spread turns and reading chrome. Release evidence includes the reproduction and corrected-run logs.

These synthetic archives match the reported book's page count and approximate size; the user's actual book and a physical Switch were not available. First opening, an uncached distant seek, corrupt data and underlying SD access can still require work. Desktop results do not establish device frame rate or latency. Device acceptance should cover rapid L/R turns in both directions, gestures, zoom, menus, sleep/resume and the reported CBR.
