# v0.4 performance results

Final source benchmark is **UNVERIFIED** pending a complete rebuild/run. Historical v0.3 data is preserved in [V03_PERFORMANCE_RESULTS.md](V03_PERFORMANCE_RESULTS.md), not relabelled as new results. User comparison reference: menu p95 ~31 ms, settings p95 ~45 ms, cold render p95 ~20 ms, cached render p95 ~1.25 ms. Actual historical runs differ; compare like fixtures/builds only.

Current engineering changes: finger-down pressed feedback, cached text measurements/layout, reusable font preview, coherent cached page drawing and explicit release gesture classification. These changes require measured p50/p95/worst before claiming improvement. SDL dummy CPU timings do not measure display scanout, finger sensor latency, Switch VSync or SD stalls.

Use bounded monotonic diagnostic events for SDL arrival/down/motion/up, gesture/hit-test/action, render/present, text work and MuPDF where instrumented. Report missing stages explicitly. Physical tap-to-visible feedback and tap-to-completed-action remain **UNVERIFIED**, including settings, dark/sepia transitions and portrait. No multi-hundred-millisecond hardware stall is declared fixed by desktop tests.
