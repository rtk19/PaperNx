# v0.4.0-rc1 test results

Final candidate source verification is pending. No historical v0.3 result is relabelled as v0.4. Historical report: [V03_TEST_RESULTS.md](V03_TEST_RESULTS.md).

| Gate | Status | Evidence |
|---|---|---|
| Portable C++17 Release | UNVERIFIED final gate | `scripts/verify-v04.sh portable` |
| Pinned native SDL/MuPDF 1.23.11 | UNVERIFIED final gate | `scripts/verify-v04.sh native` |
| ASan/UBSan/LSan | UNVERIFIED final gate | `scripts/verify-v04.sh sanitizers` |
| Native Release | UNVERIFIED final gate | `scripts/verify-v04.sh release` |
| Large-document stress | UNVERIFIED final gate | app workflow/stress fixtures |
| Actual SDL screenshot matrix | UNVERIFIED final capture | 1280×720, 1920×1080, 640×480; two portrait orientations |
| AArch64 ELF + NRO/NACP/icon/RomFS | UNVERIFIED final gate | `scripts/verify-v04.sh switch` |
| ZIP CRC/SHA256/source consistency | UNVERIFIED final gate | package/verify_distribution + build receipt |
| New physical Switch | UNVERIFIED | 35-item device checklist |

Reuse existing Docker images `papernx-desktop` and `papernx-switch` and prefixes `prefix-mupdf-desktop` / `prefix-mupdf-switch`. Run from repository root, mounting `${PWD}:/work`; do not prune/reinstall caches. Final logs and timings will be recorded after source freeze.

New tests must cover actual UI integration as well as pure helpers: navigation without mutation, A edit/confirm and B rollback, swatches without keyboard, inverse orientation mapping, theme chrome restoration, zoomed flick/pan, modal isolation, long-press cancellation, metadata/shelves/hide, confirmed disposable deletion, unsafe paths and unchanged progress. A helper-only PASS does not establish complete screen focus or touchscreen workflow.
