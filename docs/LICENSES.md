# PaperNX v0.9.0 — License and corresponding source

PaperNX is [AGPL-3.0](../LICENSE) software developed by Yusuf Buzrul with AI assistance.

The [GitHub v0.9.0 Release](https://github.com/ysfbz/PaperNx/releases/tag/v0.9.0) separates user-installable files and license notices:

- `PaperNX-switch-v0.9.0.zip`: microSD NRO, font copies/OFL and an empty books directory. It **does not** install `switch/PaperNX/licenses/`.
- `PaperNX-licenses-v0.9.0.zip`: the project `LICENSE` plus collected third-party notices under `third-party-licenses/`.
- [Tagged source code](https://github.com/ysfbz/PaperNx/tree/v0.9.0) and build scripts: the corresponding PaperNX implementation.

Major dependencies: [MuPDF 1.23.11](https://github.com/ArtifexSoftware/mupdf/tree/1.23.11) (AGPL/COPYING and third-party material), libnx, SDL2/SDL2_ttf, FreeType, HarfBuzz, JPEG/PNG/zlib-related libraries and [Noto OFL fonts](../fonts/OFL.txt). The exact SDK dependency version depends on the build environment; the Dockerfile pins the base image digest but installs updates from devkitPro repositories.

`scripts/build-mupdf-switch.sh` verifies the MuPDF source tarball checksum; `scripts/collect_licenses.py` collects notices and `SOURCE.txt`; `scripts/verify-switch.sh` adds SDK notice files; `scripts/package_release.py` produces the **separate** license ZIP.

AGPL/third-party redistribution may require complete corresponding source and appropriate build/install information. A notices archive alone does not replace those obligations. See [build guide](BUILD_SWITCH.md) and [current Release Report](RELEASE_REPORT.md).


## Comic reader dependencies in this fork

Release `v0.10.0-comics.1` adds libarchive (BSD-style notices), libwebp (BSD
notices) and the RAR-only unarr subset (LGPLv3, with public-domain PPMd files).
Vendored unarr source, its exact upstream commit, modifications, LGPLv3/GPLv3
texts and authors are in [third_party/unarr](../third_party/unarr/SOURCE.md).
CMake and the Switch Makefile compile those sources, so the tagged release
contains the application source and build instructions for relinking with a
modified decoder. The separate license archive includes unarr and refreshed
Switch portlib notices for the archive/image codecs and compression libraries.
See [comic reader guide](COMICS.md) and [the fork release](https://github.com/rtk19/PaperNx/releases/tag/v0.10.0-comics.1).

## Hebrew support in this fork

`v0.11.0-reader.1` adds FriBidi 1.0.12 (LGPL, complete unmodified upstream source archive and license included in [third_party/fribidi](../third_party/fribidi/SOURCE.md)) and unmodified Noto Sans Hebrew Regular/SemiBold and Noto Serif Hebrew Regular (SIL OFL 1.1). Font copyright, pinned upstream source and SHA256 hashes are in [fonts/HEBREW-SOURCE.md](../fonts/HEBREW-SOURCE.md); their license is [NotoHebrew-OFL.txt](../fonts/NotoHebrew-OFL.txt). The installation ZIP, NRO RomFS and separate license ZIP carry the font notices. The checked-in [MuPDF patch script](../scripts/patch-mupdf-rtl.py) contains the complete RTL changes applied to the checksum-pinned upstream source.
