# PaperNX v0.17.2 — Nintendo Switch build and installation

This guide follows the checked-in [Dockerfile](../Dockerfile), [Compose service](../compose.yaml), `scripts/verify-switch.sh`, `Makefile` and `scripts/package_release.py`. The fork release was built in Linux containers; Windows Docker Desktop commands use the same recipes.

## Requirements

Windows 11 with Docker Desktop (WSL 2 backend, Linux containers) or Linux with Docker Engine/Compose; Git; available disk space and network access for initial devkitPro packages and pinned MuPDF 1.23.11 source. Container volumes map the repository root to `/work`.

## Build from Windows Terminal / PowerShell or Linux shell

```bash
git clone https://github.com/rtk19/PaperNx.git
cd PaperNx
docker compose build switch
docker compose run --rm switch
```

The `switch` service's default command runs `bash scripts/verify-switch.sh`. Explicitly equivalent after building the image:

```bash
docker compose run --rm switch bash scripts/verify-switch.sh
```

The script validates/downloads MuPDF 1.23.11 with pinned source SHA256 `478f2a167feae2a291c8b8bc5205f2ce2f09f09b574a6eb0525bfad95a3cfe66`, verifies/reuses the AArch64 prefix, captures source/build receipts, runs `make all -B -j4`, inspects ELF/NRO/NACP/branding/RomFS, collects SDK/MuPDF license notices, packages `dist/v0.17.2/` and verifies ZIP integrity.

The Dockerfile pins its base-image digest but executes `dkp-pacman -Syu`; installed SDK package versions may change. A published release digest does not imply arbitrary future builds will be byte-identical.

## NACP and assets

[Makefile](../Makefile) defines `APP_TITLE = PaperNX`, `APP_AUTHOR = Refael Bar`, `APP_VERSION = 0.17.2`, `ICON = assets/branding/PaperNX-icon-switch.jpg` and `ROMFS = romfs`. The RomFS bundles Noto Sans Regular/SemiBold, Noto Serif, the matching Noto Hebrew fallback fonts and their OFL/provenance notices. FriBidi 1.0.12 is built from the checksum-verified complete archive in `third_party/fribidi` by `scripts/build-fribidi.sh`; its prefix is separate from SDK portlibs and its source ships in the tagged source ZIP. FriBidi handles UI bidirectional runs; SDL_ttf/HarfBuzz shape logical runs. The pinned MuPDF build applies `scripts/patch-mupdf-rtl.py`; its SHA256 is part of the cross-build cache fingerprint, so old unpatched prefixes are rebuilt automatically.

## Distribution files

`dist/v0.17.2/` contains `PaperNX.nro`, `PaperNX-switch-v0.17.2.zip`, `PaperNX-licenses-v0.17.2.zip`, three `.sha256` files and `manifest.json` (local receipt). The microSD ZIP contains:

```text
switch/
  PaperNX/
    PaperNX.nro
    books/                  (empty)
    fonts/
      NotoSans-Regular.ttf
      NotoSans-SemiBold.ttf
      NotoSerif-Regular.ttf
      OFL.txt
      NotoSansHebrew-Regular.ttf
      NotoSansHebrew-SemiBold.ttf
      NotoSerifHebrew-Regular.ttf
      NotoHebrew-OFL.txt
      HEBREW-SOURCE.md
```

Extract the Switch ZIP to the microSD root (`sdmc:/`), put EPUB/PDF/CBZ/CBR books in `sdmc:/switch/PaperNX/books/`, and open PaperNX from Homebrew Menu. On upgrade, back up `books/` and `data/` first. The distribution does not contain existing user databases. License/SDK notices ship separately in `PaperNX-licenses-v0.17.2.zip`.

## Optional desktop regression runs

```bash
docker compose build desktop
docker compose run --rm desktop bash scripts/verify-desktop.sh
docker compose run --rm desktop bash scripts/verify-pinned-desktop.sh
docker compose run --rm desktop bash scripts/verify-v04.sh native
docker compose run --rm desktop bash scripts/verify-v04.sh sanitizers
```

The system MuPDF used by `verify-desktop.sh` does not contain the fork RTL fixes; use the pinned script for production-equivalent EPUB alignment. It runs the Hebrew pixel/alignment tests in addition to the existing suites.

`verify-pinned-desktop.sh` prepares the `prefix-mupdf-desktop` needed by the pinned-native v04 scripts. The historical `verify-v04.sh` name remains in use for v0.17.2. Desktop tests are not a substitute for [35 real-device checks](DEVICE_TEST_PLAN.md). See [release artifact hashes](RELEASE_REPORT.md).

## Self-update build dependencies

Switch links devkitPro `switch-curl` and initializes libnx socket/NIFM services before the updater starts. Desktop builds require `libcurl4-openssl-dev` (included in Dockerfile). SHA256 uses the pinned MuPDF crypto primitives. The unmodified Mozilla CA bundle and its source/license notices ship in RomFS and the license archive; artifact checks verify the embedded bundle byte-for-byte.

The updater unit/integration tests are part of native/sanitizer CTest. Run the integration suite directly with `python3 tests/self_update_integration.py build-pinned/papernx_self_update_tests build-pinned/papernx_self_update_app_tests`; set `SDL_VIDEODRIVER=offscreen LIBGL_ALWAYS_SOFTWARE=1` for its OpenGL route. It needs loopback sockets and `openssl`, with no external network dependency.
