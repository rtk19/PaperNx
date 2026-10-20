# Fork release: v0.11.0-reader.2

The current fork release adds Hebrew fonts and automatic EPUB direction, series
shelves and numeric issue sorting, zoom-preserving comic curls with a reset to
the top of each new page. This update unifies LS/RS navigation, adds continuous
ZR/ZL zoom, removes the initial directional-pan jump and makes zoomed curls
follow the complete paper geometry instead of a viewport rectangle.
Download the NRO or installation ZIP from
[the fork Release Assets](https://github.com/rtk19/PaperNx/releases/tag/v0.11.0-reader.2).
The manifest and checksums identify the actual binary and exact source revision;
the source ZIP includes the full FriBidi 1.0.12 source archive and the MuPDF RTL
patch/build scripts. Font notices, third-party licenses and verification logs
are separate release assets. User books/progress are excluded from the package.

See [current checks](TEST_RESULTS.md), [installation/build instructions](BUILD_SWITCH.md),
[comic/series controls](COMICS.md) and [Hebrew behavior](HEBREW.md).
Physical Switch acceptance remains unverified. The report below describes the
historical upstream v0.9.0 release and its different artifacts.

# PaperNX v0.9.0 — Release verification report

**Published:** [v0.9.0](https://github.com/ysfbz/PaperNx/releases/tag/v0.9.0), 19 October 2026  
**Tag source commit:** `135df6be80a622d12ce59c173b31068da1844e17`  
**Application / author:** PaperNX / Yusuf Buzrul  
**Distribution:** Nintendo Switch AArch64 homebrew. Linux/desktop targets exist for development, not as separately published platform ports.

## Published release artifacts

| File | Bytes | SHA256 (GitHub Release digest) |
|---|---:|---|
| [`PaperNX.nro`](https://github.com/ysfbz/PaperNx/releases/download/v0.9.0/PaperNX.nro) | 48,090,067 | `9020d67962b5dda01b7da2405a5d023631e47e3a4e3be96662e4a48ad292b9ef` |
| [`PaperNX-switch-v0.9.0.zip`](https://github.com/ysfbz/PaperNx/releases/download/v0.9.0/PaperNX-switch-v0.9.0.zip) | 33,261,595 | `99b8ef910277a67992993112c73d0e5c2bfef651e481dfd97581a036baa18bd0` |
| [`PaperNX-licenses-v0.9.0.zip`](https://github.com/ysfbz/PaperNx/releases/download/v0.9.0/PaperNX-licenses-v0.9.0.zip) | 190,942 | `9c4c0d1f2042479c4723cbbaa3a83d0ce9c0584180a53d94127c575170fff859` |

Three corresponding `.sha256` sidecar assets are also published. **Verification scope:** The values above were checked against the *current published GitHub Release asset API*, rather than copied from older local reports. In this documentation session, actual binary downloads and independent recalculation of SHA256 **were not possible**. To perform independent byte-level verification, download an artifact and check `sha256sum <file>`, or on Windows `Get-FileHash <file> -Algorithm SHA256`, comparing both the table and its sidecar.

Older local report hashes `f05651ba…` (NRO) and `60220685…` (Switch ZIP) belong to a different build; they must **not** be used for the current Release. Local `dist/v0.9.0/manifest.json` is a build receipt and is not an additional public download.

## Build and automated quality gates

The v0.9.0 release-preparation report records:

| Check | Reported outcome | Reproduce with |
|---|---|---|
| Portable C++17 | PASS, 7/7 | `scripts/verify-v04.sh portable` |
| Native SDL/MuPDF | PASS, 11/11 | `scripts/verify-v04.sh native` |
| Native Release | PASS, 11/11 | `scripts/verify-v04.sh release` |
| ASan/UBSan and leak detection | PASS, 11/11 | `scripts/verify-v04.sh sanitizers` |
| Switch AArch64 ELF, NRO, NACP, icon, RomFS | PASS reported | `scripts/verify-switch.sh` |
| Minimal ZIP / separate license ZIP validation | PASS reported | `scripts/verify_distribution.py` |
| Physical Nintendo Switch (35 checks) | **UNVERIFIED** | [Device test plan](DEVICE_TEST_PLAN.md) |

These are outcomes reported by the prior build process, **not newly executed by this documentation change**. A successful GitHub Actions run for the exact final tagged commit was not independently found; historical `verification/` logs are no longer tracked.

The repository [Makefile](../Makefile) specifies NACP title `PaperNX`, author `Yusuf Buzrul`, version `0.9.0` and icon `assets/branding/PaperNX-icon-switch.jpg`. The verifier checks AArch64 ELF/NRO0 and embedded RomFS fonts (NotoSans Regular/SemiBold, NotoSerif and OFL notice). The Switch ZIP ships `switch/PaperNX/PaperNX.nro`, bundled fonts and an empty `books/` placeholder; the license notices are a **separate ZIP**. No user `data/` or `logs/` is installed, reducing overwrite risk but **not guaranteeing zero data loss** during power/SD failures.

**Physical acceptance:** Real Switch operation, touch/portrait, sleep/resume, controller reconnection and media failure remain **UNVERIFIED**. [Build guide](BUILD_SWITCH.md) · [Test report](TEST_RESULTS.md) · [Known limitations](KNOWN_ISSUES.md).

A documentation-only commit on `main` does not alter the published `v0.9.0` tag or binaries.
