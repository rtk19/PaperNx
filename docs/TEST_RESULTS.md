# v0.4.0-rc1 test results

Final candidate source verification completed with native and cross-compiled Switch toolchains. Historical report: [V03_TEST_RESULTS.md](V03_TEST_RESULTS.md).

| Gate | Status | Evidence |
|---|---|---|
| Portable C++17 Release | PASS | `scripts/verify-v04.sh portable` (6/6 tests passed) |
| Pinned native SDL/MuPDF 1.23.11 | PASS | `scripts/verify-v04.sh native` (10/10 tests passed) |
| ASan/UBSan/LSan | PASS | `scripts/verify-v04.sh sanitizers` (10/10 tests passed, detect_leaks=1) |
| Native Release | PASS | `scripts/verify-v04.sh release` (10/10 tests passed) |
| Large-document stress | PASS | `v03_stress_fixtures` + `papernx_app_tests` workflow |
| Actual SDL screenshot matrix | PASS | Real captures under `verification/screenshots/` (1280×720, 1920×1080, 640×480, CW/CCW portrait) |
| AArch64 ELF + NRO/NACP/icon/RomFS | PASS | `scripts/verify-v04.sh switch` (`PaperNX.nro`, `verification/v04-switch-build-receipt.json`) |
| ZIP CRC/SHA256/source consistency | PASS | `package_release.py` + `verify_distribution.py` (dist/v0.4.0-rc1) |
| New physical Switch | UNVERIFIED | 35-item device checklist in [DEVICE_TEST_PLAN.md](DEVICE_TEST_PLAN.md) awaiting hardware test |

Verification executed in Docker images `papernx-desktop` and `papernx-switch` with cached MuPDF 1.23.11 prefixes. Automated suites cover UI interaction, settings navigation without value mutation, A edit/confirm and B rollback, choice popup scrolling and clipping, swatches, inverse orientation mapping, theme chrome matching, zoomed flick/pan with sampled release velocity, modal isolation, long-press cancellation, catalog metadata/collections/hide, confirmed disposable deletion, and UTF-8 documentation integrity. Physical device operations remain UNVERIFIED until real hardware testing.
