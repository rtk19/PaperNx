# PaperNX product roadmap — 2026

Planning snapshot: 17 October 2026. Status describes source implementation, not Nintendo Switch acceptance. `IMPLEMENTED` requires a working implementation; physical-device validation is tracked separately. Features under active development remain `IN PROGRESS` until the release verification report provides evidence. This is an original offline reader, inspired by broad reading ergonomics rather than copied proprietary interfaces.

## 1. Product vision

Make a Nintendo Switch usable as a comfortable EPUB/PDF reader with the controllers detached. Opening a book, changing preferences, browsing, searching, bookmarking, returning to the library and exiting must be possible with touch alone. Protect users' books and existing reading positions before adding breadth.

## 2. Current verified capabilities

The baseline has native SDL2 controls, MuPDF EPUB/PDF rendering, touch page turns, PDF zoom/pan and filters, EPUB typography, themes, TOC, bookmarks, favorites and persisted reading positions. Continuous EPUB scrolling exists with bounded segment rendering and logical anchors; it still needs explicit physical-device acceptance because the user reported missing discoverability and scrolling behavior in the previous candidate. Desktop simulation is not evidence that a particular Switch installation works.

Versioned core, SDL and MuPDF tests exist. The new thumbnail repository has independent Docker C++17 regression coverage for restart, invalidation, corruption, bounds and recovery. Final aggregate results belong in [TEST_RESULTS.md](TEST_RESULTS.md), not inferred from implementation status. Previous candidates and their test results must not be presented as results for this candidate.

## 3. Core UX principles

Reading content takes priority over chrome. Use calm branded colors, readable text and stable card geometry. Every required action has a touch route; controller shortcuts are optional. Settings open immediately and give visible feedback. Global reading preferences apply to already-read books unless the user explicitly customizes a book. An intentional fallback cover appears before any asynchronous artwork arrives.

## 4. Touch-first interaction model

A discoverable top-right plus action sheet exposes screen-specific actions. Modal hit testing takes precedence over reader gestures. Pagination uses horizontal gestures and page zones; continuous mode uses finger-tracked vertical scrolling. PDF drag pans while zoomed. Buttons have practical minimum 56-pixel hit areas at 1280×720. See [TOUCH_CONTROLS_V03.md](TOUCH_CONTROLS_V03.md) for the implementation mapping and [DESIGN_SYSTEM.md](DESIGN_SYSTEM.md) for native tokens.

## 5. Feature inventory

Complexity: S = small, M = moderate, L = substantial, XL = research or major subsystem. Dependencies identify platform constraints, not mandatory network services. RAM/performance entries describe the principal risk. Status is intentionally conservative during development.

| Feature | Priority / user value | Complexity | Dependencies | RAM / performance concern | Milestone | Status |
|---|---|---|---|---|---|---|
| EPUB/PDF opening, TOC, bookmarks, favorites, saved position | P0 / essential reading | L | SDL2, MuPDF, microSD | Page caches bounded; parsing large books | Existing / v0.3 regression | IMPLEMENTED |
| Touch-only routes and contextual plus menu | P0 / controller-free use | L | SDL2 touch router, libnx keyboard | Hit testing and overlay latency | v0.3 | IN PROGRESS |
| Global EPUB preferences and explicit book override | P0 / predictable appearance | M | Versioned storage and migration | Never paginate unopened books | v0.3 | IN PROGRESS |
| Library settings and generous Aa panel | P0 / readable controls | L | Native SDL UI and sample preview | Debounce relayout; avoid blocking UI | v0.3 | IN PROGRESS |
| App appearance and persistent library choices | P0 / coherent UI | M | Global settings | Small fixed records | v0.3 | IN PROGRESS |
| Professional library, card hierarchy, Turkish empty states | P0 / usable book discovery | L | Existing branding and SDL text | Bound texture count, clip text | v0.3 | IN PROGRESS |
| Immediate intentional fallback covers | P1 / usable library while loading | S | Native vector primitives | No decoding required | v0.3 | IN PROGRESS |
| Persistent bounded thumbnail repository | P1 / faster repeat visits | M | std::filesystem, worker queue | 150×210 RGB cap, 32 MiB disk default | v0.3 | IMPLEMENTED |
| Visible-first thumbnail worker integration | P1 / fast first screen | M | Cover repository, existing worker | Renderer-thread texture upload only | v0.3 | IN PROGRESS |
| Low-latency settings, page reuse, request cancellation | P1 / responsive reading | L | MuPDF worker ownership | Bounded work queues, stale generations | v0.3 | IN PROGRESS |
| EPUB publisher-style policy, fonts, spacing and alignment | P1 / readable text | M | MuPDF reflow, licensed Noto fonts | Full reflow can be expensive | Existing / v0.3 polish | IN PROGRESS |
| Genuine continuous EPUB scroll, sensitivity and inertia | P1 / natural touch reading | L | Logical anchors, tiled content | Bound tiles and nearby preloading | v0.3 | IMPLEMENTED |
| Continuous scroll hardware/discoverability acceptance | P1 / confirmed user outcome | M | Physical Switch and actual books | Measure frame gaps and chapter transitions | v0.3 | BLOCKED |
| PDF fit-width, touch pan, zoom and color filters | P1 / readable PDF | M | MuPDF raster rendering | High zoom surfaces need strict bounds | Existing / v0.3 polish | IN PROGRESS |
| Safe upgrades, legacy progress and global settings recovery | P0 / no data loss | L | FAT-safe replace and backups | Bounded parsing and validation | v0.3 | IN PROGRESS |
| Physical Switch acceptance, sleep/resume and controller-free exit | P0 / release confidence | M | Physical hardware, user session | RSS, SD latency, display readability | v0.3 | BLOCKED |
| Page Flip / temporary navigation back | P1 / explore without losing place | M | Logical bookmarks, reader actions | Fixed-size in-memory history | v0.3–v0.4 | IN PROGRESS |
| Chapter-relative progress | P1 / contextual position | M | EPUB chapter mapping | Recompute after reflow | v0.4 | PLANNED |
| Active reading sessions and reading speed | P2 / meaningful progress | M | Monotonic timers, suspend events | Ignore idle and suspend intervals | v0.4 | PLANNED |
| Time to finish chapter/book | P2 / plan reading time | M | Sufficient measured session history | No fabricated estimate for new users | v0.4 | PLANNED |
| Statistics, daily goals and reading calendar | P2 / motivation | M | Local sessions | Aggregate small records, avoid write storms | v0.4 | PLANNED |
| Completion history and recently-read insights | P2 / find past reading | M | Reliable completion/session events | Compact records with retention policy | v0.4 | PLANNED |
| Text selection and highlights | P1 / close reading | L | MuPDF text quads, Unicode segmentation | Retain text only near current viewport | v0.5 | RESEARCH |
| Notes, quotations and bookmark archive | P2 / retain knowledge | L | Selection anchors, libnx text input | Durable anchored records | v0.5 | PLANNED |
| Export notes/highlights | P2 / user ownership | M | Stable annotation schema | Streaming exports, UTF-8 | v0.5 | PLANNED |
| Offline dictionary and word lookup history | P2 / understand vocabulary | L | Licensed dictionary formats | On-disk index, bounded lookup cache | v0.5 | RESEARCH |
| Vocabulary list and optional local dictionary translation | P2 / learning | L | Dictionary licenses and language coverage | Never load full dictionaries into RAM | v0.5 | RESEARCH |
| Collections, shelves and tags | P1 / organize large libraries | M | Local metadata schema | Lazy indexing and paginated lists | v0.6 | PLANNED |
| Series and author grouping | P2 / browsing | M | EPUB/PDF metadata quality | Indexed normalization, Turkish collation | v0.6 | PLANNED |
| Rich sorting and custom order | P2 / personal organization | M | Persisted app preferences | Stable bounded indexes | v0.6 | PLANNED |
| Duplicate detection | P2 / avoid clutter | M | Optional streaming content hashes | Background scan, SD IO budget | v0.6 | PLANNED |
| Metadata correction and manual cover management | P2 / clean library | M | Separate overlay metadata | Never rewrite user EPUB/PDF by default | v0.6 | PLANNED |
| Batch import and advanced folder scanning | P2 / large collections | L | SD picker and scan scheduler | Cancellation, no unbounded directory scans | v0.6 | PLANNED |
| Data export/import and practical Calibre metadata compatibility | P2 / portability | L | Versioned interchange schema | Validate paths, sizes and encoding | v0.6 | RESEARCH |
| PDF manual/automatic margin crop | P1 / larger readable text | L | MuPDF page bounds/content analysis | Bound analysis work per page | v0.7 | RESEARCH |
| PDF column reading and text extraction | P2 / articles and papers | L | MuPDF structured text | Reading order varies by document | v0.7 | RESEARCH |
| PDF reflow | P2 / flexible PDF reading | XL | Text extraction and layout heuristics | Complex layouts cannot be promised faithful | v0.7 research | RESEARCH |
| OCR | P3 / scanned documents | XL | Licensed OCR engine and language data | High CPU/RAM; practical Switch budget unknown | v0.7 research | RESEARCH |
| CBZ | P2 / comics | M | ZIP/image decoder limits | Zip-bomb defense, bounded page cache | v0.7 | PLANNED |
| CBR | P3 / archive compatibility | L | Permissible RAR implementation/license | Archive extraction limits | v0.7 research | RESEARCH |
| Manga direction and double-page mode | P2 / comics ergonomics | M | Comic page abstraction | Two images can double working surfaces | v0.7 | PLANNED |
| Panel navigation | P3 / small-screen comics | XL | Panel detection or manual metadata | CPU analysis, imperfect detection | Future | RESEARCH |
| Two-page reading and landscape layout | P2 / flexible reading | L | Reflow viewport abstraction | Preserve anchors after aspect change | v0.7 | PLANNED |
| Offline Word Wise-like help | P3 / vocabulary assistance | XL | Licensed lexicon and text segmentation | Sparse annotations, language coverage | Experimental | RESEARCH |
| Character/concept index and spoiler-safe recap | P3 / recall reading | XL | Text analysis with read-boundary policy | Model/data quality; cannot promise cloud-level AI | Experimental | RESEARCH |
| Ask-This-Book search over already-read text | P3 / find remembered passages | XL | Local index and trustworthy read boundary | Streaming index; retrieval before generative models | Experimental | RESEARCH |
| Local TTS | P2 / accessibility | XL | Licensed synthesis engine/voices, audio | Switch CPU, voice storage, thermals | Experimental | RESEARCH |
| Optional OPDS and wireless transfer | P3 / optional convenience | L | Optional networking and explicit consent | Request limits, transport security | Future | RESEARCH |
| Optional Calibre integration | P3 / desktop workflow | L | Documented compatible protocols | Keep standalone SD workflow first-class | Future | RESEARCH |
| Optional cross-device backup/sync | P3 / portability | XL | Conflict resolution, user-owned destination | Encryption, interruption recovery | Future | RESEARCH |
| Auto-scroll | P3 / hands-free reading | M | Continuous reading scheduler | Pause at dialogs, idle frame work | Future | PLANNED |
| Gesture remapping | P2 / personal control | M | Touch router and validation | Avoid conflicting mandatory routes | Future | PLANNED |
| External font import | P2 / language and style | M | Font validation and licensing notice | Bound font cache and malformed font exposure | Future | RESEARCH |
| Accessibility presets | P1 / comfortable reading | M | Theme, typography, contrast and targets | Verify on LCD and OLED | Future | PLANNED |

`BLOCKED` here means hardware evidence is unavailable in the coding environment; it does not mean the feature is removed from v0.3 scope.

## 6. Versioned milestones

**v0.3 — Professional Touch Reader:** complete the P0/P1 interface, settings, covers, input, performance and data-safety work above, produce a real AArch64 NRO and preserve upgrade data. Desktop tests and a package are necessary but insufficient for stable release. The [requirement matrix](V03_REQUIREMENT_COVERAGE.md) is the acceptance ledger.

**v0.4 — Reading Intelligence:** measured local sessions, navigation history, chapter progress and optional goals. Require reliable suspend/idle exclusion before time estimates.

**v0.5 — Notes and Learning:** selection feasibility first, then stable annotation anchors and exports, then optional licensed dictionaries.

**v0.6 — Library Management:** metadata overlays, grouping and safe local data interchange before network integration.

**v0.7 — Advanced PDF and Comics:** bounded image-archive support, crop and layout tools. OCR, CBR and reflow remain research until dependency, license and hardware tests support promotion.

These are milestones, not promised delivery dates. Future features are not part of the v0.3 implementation claim.

## 7. Dependencies and technical feasibility

Keep C++17, SDL2/SDL2_ttf, libnx and MuPDF. No web shell, account system or mandatory service. MuPDF document access requires explicit worker ownership; stale relayout results must never overwrite newer preferences. Logical reading anchors are preferred over raster page offsets. Text selection needs Unicode word/grapheme behavior and layout-independent anchors before annotation UI can be reliable.

Nintendo Switch homebrew has constrained CPU and memory relative to desktop or cloud services. Local generative AI and TTS need measured model/voice size, startup time, peak RSS, sustained latency and power behavior. Prefer local lexical lookup and indexed text search before heavy inference. Do not imply proprietary competitor features are universally available or can be reproduced without engineering and licensing work.

## 8. Performance and memory budgets

Targets are acceptance goals unless linked measurements establish otherwise:

| Resource | Budget / policy |
|---|---|
| Touch frame work | Aim for 16.7 ms frames at 60 Hz; measure median/p95/p99 and worst gaps |
| Menu/settings open | Present chrome without waiting for EPUB reflow or cover IO |
| Reading work queues | Latest-request wins; bound work and discard obsolete generations |
| Persistent thumbnail disk | 32 MiB and 512 entries by default; reserve before writes |
| Single cached thumbnail | At most 150×210 RGB, 94,500 payload bytes |
| Cache IO | Worker only; one startup inventory, memory access order, no per-frame scan or mtime writes |
| Thumbnail textures | Renderer-thread upload only; bound resident visible/nearby set |
| Continuous content | Bounded viewport tiles and nearby preloading; never rasterize an entire book |
| Saves | Explicit dirty/debounce policy; maintain recoverable prior data |
| Hardware memory | Record measured steady/peak RSS for large/image-heavy books before setting release limit |

Desktop timings must be labelled with build, fixture and host. A desktop p95 does not establish Switch responsiveness.

## 9. Security, privacy and licensing

Offline-first by default. No telemetry, accounts or background uploads. EPUB/PDF files are never modified by appearance changes or thumbnail generation. Cache records are isolated from reading progress; failed/corrupt cache is a miss. Preserve old saves before migration and validate replacement writes. Packages contain empty user directories and application assets, never a user's progress or books.

Keep PaperNX branding intact. Bundle font notices, MuPDF notices and required devkitPro/libnx/SDL dependency notices. Verify distribution obligations under the actual linked licenses before publishing a stable binary; technical packaging success does not replace license review. Use original vector fallback covers and icons, not unlicensed commercial artwork. Dictionary, OCR, RAR, TTS voice and model licenses are separate feasibility gates. Optional networking must be opt-in and cannot make offline reading depend on a server.

## 10. User acceptance criteria

Without controllers: start the app; open plus; open global settings without any book; change theme and typography; open two already-read EPUBs and see global values; customize one book and reset inheritance; search using the Switch keyboard; browse SD; scroll the library; open a book; navigate pages/chapters, TOC and bookmarks; pan a fit-width PDF; change continuous mode; return to library; exit; restart and restore exact logical position as supported. Repeat sleep/resume and inspect readability on target hardware. Compare warm and cold cover visits and confirm persistence.

No new physical result is marked passed without recordings/logs from a new device session. See [DEVICE_TEST_PLAN.md](DEVICE_TEST_PLAN.md).

## 11. Known risks

SD latency and FAT power-loss semantics differ from desktop filesystems. Flush/close plus backup recovery reduces risk but is not a power-loss durability guarantee. MuPDF layout cannot promise exact pixel-identical logical restoration across every malformed EPUB. Long Turkish titles, large font scales and smaller windows need clipping/scrolling coverage. Heavy images and extreme PDF zoom need allocation limits. Controller-disconnected SDL behavior and Switch keyboard integration require physical testing. Previously supplied video observations remain open until the new build is accepted on hardware.

## 12. Deferred features and research items

Statistics, learning, rich collections, advanced PDF/comics and optional network/AI functionality remain future work except where the matrix explicitly records a working implementation. Do not place actionable dead buttons in the UI for these features. Promote research only after a small prototype, dependency/license review, memory/performance measurement and user acceptance criteria are available.
