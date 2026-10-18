# v0.4 performance results

Historical v0.3 data is preserved in [V03_PERFORMANCE_RESULTS.md](V03_PERFORMANCE_RESULTS.md).

Measured v0.4 desktop pipeline timings (recorded in `verification/v04-ui-latency.json` across 32 iterations under the Docker SDL software renderer; CPU event-to-present return):

| Metric | p50 | p95 | Worst | Notes |
|---|---|---|---|---|
| `tap_to_pressed_present` | 8.35 ms | 9.54 ms | 12.65 ms | Immediate visual feedback on `SDL_FINGERDOWN` |
| `tap_to_action_present` | 16.82 ms | 18.89 ms | 23.02 ms | Action dispatch and presentation on `SDL_FINGERUP` |
| `focus_to_present` | 8.33 ms | 10.12 ms | 12.34 ms | Directional navigation focus ring redraw |

Current engineering improvements:
- Finger-down pressed feedback on primary controls.
- Cached text measurements and stable font metrics in `Typography`.
- Fast single-pass scroll and clipping in choice popup.
- Hardware-tuned PDF flick classifier with sampled release velocity.
- Direct rotated target presentation without double copying.

Note: Desktop SDL software renderer timings measure CPU event-to-present return and do not include physical Switch touchscreen scanning, physical display scanout, or micro-SD latency. Physical hardware latency and feel remain **UNVERIFIED** until on-device acceptance testing.
