# v0.4.0-rc3 release report

Baseline final verification: `1863c55fc0bf06c506b8b23065d71af55f2bc61c`. Branch: `fix/v04-book-scoped-appearance-and-pdf-chrome`. Previous report: [V03_RELEASE_REPORT.md](V03_RELEASE_REPORT.md).

All automated engineering gates (Portable, Native, Sanitizers ASan/UBSan/LSan, Release, Stress, AArch64 cross-compilation, and Release Packaging) have **PASSED** with clean execution and verified hashes.

## Release Artifacts

Distribution directory: `dist/v0.4.0-rc3/`

| Artifact | Size (bytes) | SHA256 |
|---|---|---|
| `PaperNX.nro` | 47,938,515 | `0e77206a3dc18f171b79df7b293bc89dbcf39ab73b9d63c442463f85cc13a0b5` |
| `PaperNX-switch-v0.4.0-rc3.zip` | 33,411,959 | `5d50c807189c3d8b5fc7f098fafff714108f5af5a10d947c8a5acf076fb1279e` |
| `manifest.json` | 7,354 | Verified against build receipt and source identity |

- **Source Implementation Fingerprint:** `1495b0bd9dd428666ec8868310efaa1a53f317b3233cc7c969331c28c44aaf83`
- **Build Receipt:** `verification/v04-switch-build-receipt.json`
- **AArch64 Machine Type:** 183 (ELF64 executable)
- **NRO Format:** NRO0 with valid .text, .rodata, and .data segment boundaries
- **NACP Metadata:** Name: `PaperNX`, Author: `ysfbz`, Version: `0.4.0-rc3`
- **Embedded Icon:** 256×256 baseline JPEG (`b47198387614a9e66a570d7faaefff0fa496033d4481060316a086059b506bb5`), identical bytes verified
- **RomFS Assets:** Noto Sans Regular, Noto Sans SemiBold, Noto Serif Regular, OFL.txt
- **License Notices:** 59 bundled third-party license documents

Package includes upgrade-safe empty `books/` and `data/` directories that will never overwrite user content.

## Hardware Status

- **Physical Switch Hardware Verification:** **UNVERIFIED**.
- **Release Readiness:** **Release Candidate verified through automated suites; awaiting physical device test acceptance.**
- Complete 35-point hardware verification checklist documented in [DEVICE_TEST_PLAN.md](DEVICE_TEST_PLAN.md).
