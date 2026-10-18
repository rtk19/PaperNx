# PaperNX v0.4 requirement coverage

Release target `v0.4.0-rc1`; baseline final v0.3 verification `28f2e0035f876ff1b86af0651e9124d19ad92c5c`, foundation `e788e87`. Status **PASS** requires linked evidence for this source; **UNVERIFIED** means pending or unavailable validation; **DEFERRED** names an unimplemented scope. No new physical check has passed.

| Requirement | Implementation / evidence route | Status |
|---|---|---|
| Native shared components, SemiBold, separate settings labels/steppers | `ui.hpp`, `typography.cpp`, OFL fonts; native component tests | UNVERIFIED final gate |
| Reading/application/custom chrome policy, PDF filters, modal palette | `readerChromePalette`, application palette selection; theme assertions/screenshots | UNVERIFIED final gate |
| Visible focused/selected/pressed/editing states | Focus rings, `FocusManager`, setting navigation/edit path | UNVERIFIED complete per-screen acceptance |
| D-pad default page turns, zoomed PDF turns | Reader handler + app workflow | UNVERIFIED final gate |
| Fast flick vs slow pan, pinch/modal isolation | `pdf_gestures.hpp`, foundations/app workflow | UNVERIFIED final gate |
| Orientation and inverse touch mapping | `orientation.hpp`, rotated render target, persisted app preference | UNVERIFIED final gate and hardware |
| Nine paper presets, swatches, texture/contrast | `theme.hpp`, appearance controls | UNVERIFIED final gate |
| Press feedback/cache/performance instrumentation | `ui.hpp`, typography cache, bounded diagnostics | UNVERIFIED final benchmark |
| Context, safe hide/restore and guarded permanent delete | Catalog + library UI + disposable-file regression | UNVERIFIED final gate |
| Metadata, authors, genres, shelves | Independent catalog sidecar and UI | UNVERIFIED final gate |
| Progress/bookmark compatibility | Existing transport/migration + catalog independent storage | UNVERIFIED final regression |
| Fallback covers | Deterministic native vector variants, stable aspect | UNVERIFIED screenshots |
| Page Flip/history | Existing jump-back improved/reanchor | UNVERIFIED final workflow |
| Offline active time, sessions, daily goal, conservative book ETA | `ReadingStatsStore` + reader/report UI | UNVERIFIED final gate |
| Chapter ETA / full annual charts | No safe implemented chapter estimator/full report claimed | DEFERRED |
| Content-addressed moved/renamed reconciliation, independently normalized authors/subjects | Existing path+size ID; bounded OPF/PDF metadata cache and string overlays; Calibre/EPUB3 series index is implemented | DEFERRED richer metadata model |
| Arbitrary per-action gesture remapping | D-pad defaults plus configurable L/R compatibility shortcuts; no full mapping editor claimed | DEFERRED |
| Full universal per-control focus and every screen resolution | Shared manager exists; inspect integration/screen matrix before claim | UNVERIFIED |
| CBZ/CBR, OCR, PDF reflow, lookup, annotations, AI summaries | No advertised dead buttons; gated roadmap | DEFERRED |
| Portable/native/MuPDF/sanitizer/Release/stress | Final commands/results in TEST_RESULTS | UNVERIFIED final gates |
| Real screenshots 1280Ã—720, 1920Ã—1080, 640Ã—480, both portraits | Actual SDL capture artifacts only | UNVERIFIED final capture |
| Genuine Switch ELF/NRO/NACP/icon/RomFS + ZIP CRC/hash/source | Build receipt + artifact/distribution scripts | UNVERIFIED final build |
| New physical acceptance | DEVICE_TEST_PLAN 35 checks | UNVERIFIED |

A partially implemented requirement is never promoted solely because a pure helper test passes. The release report must disclose remaining UX limitations and distinguish automated correctness from measured real-device usability.
