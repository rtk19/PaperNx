# PaperNX v0.9.0 Test Results

Final production verification completed across desktop native and cross-compiled Nintendo Switch toolchains.

| Gate | Status | Evidence |
|---|---|---|
| Portable C++17 Release | PASS | `scripts/verify-v04.sh portable` (7/7 tests passed) |
| Pinned native SDL/MuPDF 1.23.11 | PASS | `scripts/verify-v04.sh native` (11/11 tests passed) |
| ASan/UBSan/LSan | PASS | `scripts/verify-v04.sh sanitizers` (11/11 tests passed, 0 leaks, 0 errors) |
| Native Release | PASS | `scripts/verify-v04.sh release` (11/11 tests passed) |
| Large-document stress | PASS | `v03_stress_fixtures` + `papernx_app_tests` workflow |
| Packaging & Distribution | PASS | `scripts/verify_distribution.py` (`PaperNX-switch-v0.9.0.zip`, `PaperNX-licenses-v0.9.0.zip`) |
| AArch64 ELF + NRO/NACP/icon/RomFS | PASS | `scripts/verify-switch.sh` (`PaperNX.nro`, NACP author: Yusuf Buzrul, version: 0.9.0) |
| Physical Switch Hardware Acceptance | UNVERIFIED | 35-item device checklist in [DEVICE_TEST_PLAN.md](DEVICE_TEST_PLAN.md) |

Verification executed in Docker containers `papernx-desktop` and `papernx-switch` with cached MuPDF 1.23.11 prefixes. Automated test suites cover UI interaction, settings navigation, default English localization with Turkish toggle, choice popup scrolling and clipping, theme chrome matching, zoomed flick/pan, modal isolation, catalog metadata/collections, reading journal and statistics calculation across calendar boundaries, atomic persistence, and UTF-8 documentation integrity.
