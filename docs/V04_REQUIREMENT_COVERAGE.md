# PaperNX v0.4 requirement coverage

Release target `v0.4.0-rc2`; source verified against Docker desktop/switch toolchains. Status **PASS** requires linked evidence for this source; **UNVERIFIED** means pending physical hardware validation; **DEFERRED** names an unimplemented scope. No new physical hardware check has passed.

| Requirement | Implementation / evidence route | Status |
|---|---|---|
| Native shared components, SemiBold, separate settings labels/steppers | `ui.hpp`, `typography.cpp`, OFL fonts; `papernx_ui_component_tests`, `v04-settings-focus-landscape.png` | PASS |
| Unified global appearance and legacy theme-policy migration | `PAPERNX_DEFAULTS` is the canonical source, `AppSettingsStore::put` neutralizes legacy chrome policies, one nine-preset picker in Application settings; desktop regression cases for global/per-book themes and PDF document filters | IMPLEMENTED — physical Switch pending |
| Visible focused/selected/pressed/editing states | Focus rings, `FocusManager`, settingsEditing_ with A/B workflow; `v04UiWorkflow` | PASS |
| D-pad default page turns, zoomed PDF turns | Reader handler + app workflow; `v04UiWorkflow` | PASS |
| Fast flick vs slow pan, pinch/modal isolation | `pdf_gestures.hpp` sampled release velocity; `foundations_tests`, `v04UiWorkflow` | PASS |
| Orientation and inverse touch mapping | `orientation.hpp`, rotated render target, CW/CCW 90/270°, persisted app preference; `v04UiWorkflow` | PASS (physical device UNVERIFIED) |
| Nine paper presets, swatches, texture/contrast | `theme.hpp`, `v04_choices.inc` scroll/clip, swatch picker, live preview; `v04UiWorkflow` | PASS |
| Press feedback/cache/performance instrumentation | `ui.hpp`, typography cache, bounded diagnostics; `v04-ui-latency.json` | PASS |
| Context, safe hide/restore and guarded permanent delete | Catalog + library UI + disposable-file regression; `v04CatalogWorkflow` | PASS |
| Metadata, authors, genres, shelves | Independent catalog sidecar and UI; `v04CatalogWorkflow` | PASS |
| Progress/bookmark compatibility | Existing transport/migration + catalog independent storage; `app_tests` | PASS |
| Fallback covers | Deterministic native vector variants, stable aspect; `papernx_cover_tests` | PASS |
| Page Flip/history | Existing jump-back improved/reanchor; `v04CatalogWorkflow` | PASS |
| Offline active time, sessions, daily goal, conservative book ETA | `ReadingStatsStore` + reader/report UI; `v04CatalogWorkflow` | PASS |
| Chapter ETA / full annual charts | No safe implemented chapter estimator/full report claimed | DEFERRED |
| Content-addressed moved/renamed reconciliation, independently normalized authors/subjects | Existing path+size ID; bounded OPF/PDF metadata cache and string overlays; Calibre/EPUB3 series index is implemented | DEFERRED |
| Arbitrary per-action gesture remapping | D-pad defaults plus configurable L/R compatibility shortcuts; no full mapping editor claimed | DEFERRED |
| Full universal per-control focus and every screen resolution | Universal `FocusManager` covering reader toolbar, settings, modals, choices; 640×480 and 1920×1080 verified | PASS |
| CBZ/CBR, OCR, PDF reflow, lookup, annotations, AI summaries | No advertised dead buttons; gated roadmap | DEFERRED |
| Portable/native/MuPDF/sanitizer/Release/stress | Portable (6/6), Native (10/10), Sanitizers ASan/UBSan/LSan (10/10), Release (10/10); `TEST_RESULTS.md` | PASS |
| Real screenshots 1280×720, 1920×1080, 640×480, both portraits | Actual SDL capture artifacts under `verification/screenshots/` | PASS |
| Genuine Switch ELF/NRO/NACP/icon/RomFS + ZIP CRC/hash/source | Build receipt `v04-switch-build-receipt.json`, verified NRO & distribution package | PASS |
| New physical acceptance | DEVICE_TEST_PLAN 35 checks | UNVERIFIED |

A partially implemented requirement is never promoted solely because a pure helper test passes. The release report must disclose remaining UX limitations and distinguish automated correctness from measured real-device usability.
