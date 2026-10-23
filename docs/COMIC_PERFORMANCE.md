# Comic curl and fixed counter — 0.17.5

Comics retain one image and paper-only contact in every orientation. They use the same cylinder/lighting renderer as books, now with explicit front, paper reverse and incoming image snapshots. Without a reverse texture, the renderer reused the saturated front image on the folded underside; its mirrored copy could resemble overlapping artwork. The reverse now has a paper surface with subdued ink, while the resting front/incoming image pixels, pan, aspect ratio and outside-to-paper touch intersection remain unchanged. Snapshot resolution and asynchronous base/detail preparation stay bounded.

Comic digits are now independent fixed chrome. They are omitted from resting image/sheet captures and there are no separate folio surfaces or folio curl calls. The outgoing page number stays in the bottom strip through drag, settling and button-driven animation. A completed turn updates it once; a cancelled touch keeps it. Queued turns expose each completed number in order. Open tools retain current-of-total and percentage formatting. Hidden-tool counter placement is consistent between fitted and zoomed comic pages. Book/PDF folios still belong to their sheets.

Pixel regressions read every footer pixel at five held fold positions in fitted and zoomed comics, compare them with the resting footer, inspect the actual paper reverse, evict the base cache mid-animation and check completion, backwards turns, cancellation and queued input. Existing margin-contact, RTL/LTR book curls, rotated comic captions, pan/pinch, archive, CBR latency, library and updater suites remain. Validation includes 30 native and 30 ASan/UBSan/leak tests, 8 portable tests, packaging, and offscreen OpenGL on generated 37-page ~43 MB solid RAR4/RAR5 archives. These development-machine checks do not certify physical Switch behaviour; the actual reported CBR and a Switch were unavailable.

---

# Single-image comic rollback — 0.17.4

Comics now always display one complete archive image in landscape and portrait, independently of the persisted book/PDF spread preference. This is a rendering/navigation rollback: adjacent-page pairing, neighbouring-header wide-page detection, facing-page zoom/pan geometry, the artificial spine and the paired/fullscreen under-sheet compositor are removed from the comic path. Page turns advance exactly one image. A naturally wide or ComicInfo DoublePage image stays whole and is never paired or split.

The original single-image fit rectangle, pan bounds and focal zoom path are restored. Golden regression rectangles for the 0.12 viewport (1280×720, 40px side margins, original 72px footer) cover a portrait image at **424,72,432,576** and a wide image at **64,72,1152,576**. Both remain identical with the landscape preference On and Off. The expanded-reading-area preference still independently changes the usable viewport; it is not reset.

Comic animation now owns only the current/next image snapshots and their moving folios. The full image is the physical paper, even while zoomed. Input still starts anywhere: the touch segment must intersect the image before folding begins; motion entirely outside it does not turn a page. Tests check live/held/committed contact, empty margins, no artificial vertical spine inside the image, small/large zoom, folios surviving cache eviction and successive cold turns.

The visible image and two directional neighbours use at most roughly 24 MiB of base textures before older cached pages. Only the one visible image receives high-resolution detail. The existing 64 MiB LRU, protected displayed image, asynchronous preparation, priority for explicitly requested pages, bounded completed buffers, retained CBR document/dictionary and pending-input cancellation remain. The old synchronous decoder/cache regressions are not restored.

Books and PDFs keep their independent default-on landscape spread setting and animations. The setting is labelled **Two pages in landscape (books/PDF)**. No book data, settings, progress, library or unrelated UI change is rolled back.

Verification uses the existing 30 native and 30 ASan/UBSan/leak tests, 8 portable tests, packaging checks and offscreen OpenGL runs. Original high-resolution CBZ and 37-page ~43 MB stored/compressed solid RAR4/RAR5 fixtures exercise repeated page buttons, immediate successive swipes, pan/pinch, async single-page detail and an archive retained through a 3.2-second idle pause. Results are development-machine evidence: the actual reported CBR and physical Switch remain unavailable, so device latency cannot be certified from these tests.

---

# Consecutive comic spread turns — 0.17.3

The reported 0.17.2 behaviour was: after one spread turn, additional page-turn attempts were ignored for several seconds while zoom remained responsive. The real 37-page, approximately 43 MB CBR and a physical Switch were unavailable; the checks below use original generated archives of the same scale.

## Remaining paths and correction

- A warmed first turn was covered in 0.17.2, but an immediate second turn could still inspect unknown page dimensions and rasterize uncached pages on the UI thread. Even validating queued turns could inspect future archive entries. A cache-only geometry query now gates pair detection and snapshot preparation. A cold turn requests base images from the worker, retains the displayed pair, and keeps accepted navigation until its images and geometry arrive. Queued turns do not inspect unseen headers. Base requests take priority over queued speculative/detail jobs.
- A new finger starting during the preceding release animation did not own the next live curl. A new DOWN now settles that prior turn before acquiring the next sheet. Cold drags retain their contact/current location; if released before loading completes, a committed gesture becomes a pending turn. Repeated gestures and shoulders preserve their accumulated intent. Zoom, settings, backgrounding, jumps and document changes cancel pending navigation.
- The worker closed its document after **three seconds of idle time**. Retaining a libarchive cursor inside that document therefore did not retain it across normal reading pauses. The worker now sleeps without closing its bounded document, including its solid dictionary, until configuration changes or shutdown. An isolated probe using the exact 0.17.2 worker failed a forward read after a 3.2-second pause when the archive pathname was temporarily unavailable; the fixed worker passes the same probe.
- Preparation looks ahead up to four physical pages in the reading direction, covering two subsequent normal pairs. Six base pages remain below 48 MiB; the LRU still has its existing 64 MiB ceiling and protects displayed pages. Detailed zoom surfaces retain their separate two-slot budget.
- The existing Switch single-contact finger-ID normalization now covers release as well as motion. Focus loss clears held paper and touch ownership. This closes another path where a held curl could reject page buttons while pinch still cancels it; its occurrence on the user's device is not established.

## Verification

The performance suite now prepares only the **first** neighbouring pair, then sends an actual swipe, another swipe during settling, another cold swipe and a shoulder press without processing background results between those events. It asserts that the next unknown header remains unknown on the UI thread, pending input is retained, zoom responds/cancels immediately, and successive cold turns complete without additional UI rasterizations. A second phase sends three queued shoulders and drives the normal processing/animation/preparation path. Focus loss during a held curl releases ownership. A 3.2-second worker-idle test temporarily removes the archive pathname before the next forward request, proving that the stream survived the pause.

Checks run against high-resolution CBZ, generated 37-page ~43.5 MB stored RAR5 and additional ~43 MB compressed solid RAR4/RAR5. Snapshot tests wait explicitly for asynchronous preparation; the performance regression deliberately does not warm the second pair before sending input. Native, ASan/UBSan/leak, portable, packaging and offscreen OpenGL checks accompany the release. Desktop measurements do not establish Switch latency, SD performance or a universal no-stall guarantee. First opening, distant uncached seeks and malformed archives still have their own costs.

In the additional desktop OpenGL runs, three consecutive cold shoulder turns (including their normal animations) completed in **1,616 ms for solid RAR4** and **1,581 ms for solid RAR5**, with **zero UI-thread rasterizations**. The longest measured UI iteration was **18 ms / 22 ms** respectively. These are development-machine measurements, not Switch timings. The baseline and final probe logs, full test logs and exact probe source are included in release evidence.

The paper-only curl, full-width drag, smart wide scans, close-facing pages and bounded spine, RTL/LTR controls, folios, focal pinch, return-to-top/Y, default-on layout preferences, 0.16 menus, reading records and library remain.

---

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
