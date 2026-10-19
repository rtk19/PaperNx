# PaperNX v0.9.0 — Reported validation results

The release-preparation handoff reports:

| Gate | Reported result | Checked-in script |
|---|---|---|
| Portable C++17 | 7/7 PASS | `scripts/verify-v04.sh portable` |
| Pinned native SDL/MuPDF | 11/11 PASS | `scripts/verify-v04.sh native` |
| Optimized native Release | 11/11 PASS | `scripts/verify-v04.sh release` |
| ASan/UBSan with leak detection | 11/11 PASS | `scripts/verify-v04.sh sanitizers` |
| Large document stress | PASS reported | `v03_stress_fixtures` and `papernx_app_tests` (legacy test target names) |
| AArch64 NRO/NACP/RomFS | PASS reported | `scripts/verify-switch.sh` |
| ZIP and license archive verification | PASS reported | `scripts/verify_distribution.py` |
| Physical Nintendo Switch acceptance | **UNVERIFIED** | [35-item checklist](DEVICE_TEST_PLAN.md) |

The public v0.9.0 tag points to `135df6be80a622d12ce59c173b31068da1844e17`. These are **reported prior test outcomes**; documentation maintenance did not rerun Docker, compile an NRO or establish a new GitHub Actions pass for the tagged source. The `verification/` directory's historical logs and screenshots were intentionally removed from Git tracking. Static UTF-8/NFC checks do not independently demonstrate correct on-device glyph rendering.

See [Release Report](RELEASE_REPORT.md), [build and reproduction instructions](BUILD_SWITCH.md) and [known limitations](KNOWN_ISSUES.md).
