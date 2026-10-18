# v0.4.0-rc2 release report

Baseline final verification: `d6f8e4ead7f448aee7cac70eb110baa270fa796d`. Branch: `fix/v04-switch-touch-theme-performance`. Previous report: [V03_RELEASE_REPORT.md](V03_RELEASE_REPORT.md).

All automated engineering gates (Portable, Native, Sanitizers ASan/UBSan/LSan, Release, Stress, AArch64 cross-compilation, and Release Packaging) have **PASSED** with clean execution and verified hashes.

## Release Artifacts

Distribution directory: `dist/v0.4.0-rc2/`

| Artifact | Size (bytes) | SHA256 |
|---|---|---|
| `PaperNX.nro` | 47,930,323 | `d45e45327b99da7bb9b4b1bcb9387e609650c47cee9f0405f6aed9a0d7004634` |
| `PaperNX-switch-v0.4.0-rc2.zip` | 33,404,256 | Tracked in `.sha256` sidecar and `manifest.json` |
| `manifest.json` | 7,512 | Verified against build receipt and source identity |

- **Source Implementation Fingerprint:** `22075862deffed04f066f5e9335062ba13fd4bd7be984334522f15f5102072ca`
- **Build Receipt:** `verification/v04-switch-build-receipt.json`
- **AArch64 Machine Type:** 183 (ELF64 executable)
- **NRO Format:** NRO0 with valid .text, .rodata, and .data segment boundaries
- **NACP Metadata:** Name: `PaperNX`, Author: `ysfbz`, Version: `0.4.0-rc2`
- **Embedded Icon:** 256×256 baseline JPEG (`b47198387614a9e66a570d7faaefff0fa496033d4481060316a086059b506bb5`), identical bytes verified
- **RomFS Assets:** Noto Sans Regular, Noto Sans SemiBold, Noto Serif Regular, OFL.txt
- **License Notices:** 59 bundled third-party license documents

Package includes upgrade-safe empty `books/` and `data/` directories that will never overwrite user content.

## Hardware Status

- **Physical Switch Hardware Verification:** **UNVERIFIED**.
- **Release Readiness:** **Release Candidate verified through automated suites; awaiting physical device test acceptance.**
- Complete 35-point hardware verification checklist documented in [DEVICE_TEST_PLAN.md](DEVICE_TEST_PLAN.md).
