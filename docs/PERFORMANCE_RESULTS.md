# v0.4 performance results

Historical v0.3 data is preserved in [V03_PERFORMANCE_RESULTS.md](V03_PERFORMANCE_RESULTS.md).

Measured v0.4 desktop pipeline timings (recorded in `verification/v04-ui-latency.json` across 32 iterations under the Docker SDL software renderer; CPU event-to-present return):

| Metric | p50 | p95 | Worst | Notes |
|---|---|---|---|---|
| `tap_to_pressed_present` | 8.22 ms | 8.38 ms | 8.62 ms | Immediate visual feedback on `SDL_FINGERDOWN` |
| `tap_to_action_present` | 16.45 ms | 16.82 ms | 17.02 ms | Action dispatch and presentation on `SDL_FINGERUP` |
| `focus_to_present` | 8.22 ms | 8.40 ms | 8.40 ms | Directional navigation focus ring redraw |

Measured document and layout pipeline timings:
- **Large EPUB open:** 30.25 ms (24 chapters)
- **Illustrated EPUB open:** 3.82 ms (8 chapters, 2048×1536 illustrations)
- **PDF open to first page:** 2.39 ms (320-page document)
- **Cold page render (p95):** 4.05 ms
- **Warm / cached page render (p95):** 1.28 ms
- **Settings tab render (p95):** 27.78 ms
- **Menu open render (p95):** 20.35 ms
- **Asynchronous layout & atomic swap:** 233.24 ms (coalesced 3 rapid requests into 1 accepted layout)
- **Peak page texture cache:** 66.59 MB (within 64 MiB logical budget)
- **Process RSS high-water mark:** 148.66 MB

Current engineering improvements:
- Finger-down pressed feedback on primary controls.
- Cached text measurements and stable font metrics in `Typography`.
- Fast single-pass scroll and clipping in choice popup.
- Hardware-tuned PDF flick classifier with sampled release velocity.
- Direct rotated target presentation without double copying.
- Book-scoped appearance: atomic canvas and reader chrome synchronization during asynchronous reflow; zero mixed-color frames.
- Background metadata extraction for cached covers without UI thread blocking (bounded to 2 updates per frame).

Note: Desktop SDL software renderer timings measure CPU event-to-present return and do not include physical Switch touchscreen scanning, physical display scanout, or micro-SD latency. Physical hardware latency and feel remain **UNVERIFIED** until on-device acceptance testing.
