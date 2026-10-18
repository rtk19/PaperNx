# v0.4.0-rc1 release report

Baseline final verification: `28f2e0035f876ff1b86af0651e9124d19ad92c5c`. Branch: `feat/v04-professional-reader-ux`. Previous report: [V03_RELEASE_REPORT.md](V03_RELEASE_REPORT.md).

All automated engineering gates (Portable, Native, Sanitizers ASan/UBSan/LSan, Release, Stress, AArch64 cross-compilation, and Release Packaging) have **PASSED** with clean execution and verified hashes.

## Release Artifacts

Distribution directory: `dist/v0.4.0-rc1/`

| Artifact | Size (bytes) | SHA256 |
|---|---|---|
| `PaperNX.nro` | 47,930,323 | `dc67312298a15bc43349815a8cdeb9f08bae91bab06dbd8dcefa5b798c240496` |
| `PaperNX-switch-v0.4.0-rc1.zip` | 33,399,932 | `81395a41ff743623b183fc8256356f4ba9eb6548f8a17e871a1e9ccb6d01540a` |
| `manifest.json` | 7,512 | Verified against build receipt and source identity |

- **Source Implementation Fingerprint:** `a07ed178289750c8ae7642c3e816f72c361089833c5be7f7d914e8a70b1d3546`
- **Build Receipt:** `verification/v04-switch-build-receipt.json`
- **AArch64 Machine Type:** 183 (ELF64 executable)
- **NRO Format:** NRO0 with valid .text, .rodata, and .data segment boundaries
- **NACP Metadata:** Name: `PaperNX`, Author: `ysfbz`, Version: `0.4.0-rc1`
- **Embedded Icon:** 256×256 baseline JPEG (`b47198387614a9e66a570d7faaefff0fa496033d4481060316a086059b506bb5`), identical bytes verified
- **RomFS Assets:** Noto Sans Regular, Noto Sans SemiBold, Noto Serif Regular, OFL.txt
- **License Notices:** 59 bundled third-party license documents

Package includes upgrade-safe empty `books/` and `data/` directories that will never overwrite user content.

## Hardware Status

- **Physical Switch Hardware Verification:** **UNVERIFIED**.
- **Release Readiness:** **Release Candidate verified through automated suites; awaiting physical device test acceptance.**
- Complete 35-point hardware verification checklist documented in [DEVICE_TEST_PLAN.md](DEVICE_TEST_PLAN.md).
