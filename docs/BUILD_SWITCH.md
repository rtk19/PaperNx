# Nintendo Switch build status — BLOCKED

No devkitPro, devkitA64, libnx, Switch SDL2 portlibs or ARM64 MuPDF library is present in the current execution environment. Therefore there is no compiled Switch artifact.

Cross-compile reference: https://github.com/switchbrew/switch-examples/tree/master/graphics/sdl2 and https://devkitpro.org/ . Packages such as `switch-dev`, `switch-sdl2`, `switch-sdl2_ttf` are documented for devkitPro; the existence of a prebuilt `switch-mupdf` package **must not be assumed**.

A preliminary libnx `Makefile` is provided and deliberately refuses to run without `MUPDF_AARCH64_ROOT`. It was adapted from switchbrew's SDL2 sample. Its linker settings are unverified and require verified MuPDF/transitive ARM64 libraries (`MUPDF_EXTRA_LIBS`) and actual compilation. To complete this, pin MuPDF and dependencies, prepare an audited ARM64 static build, and generate/verify an actual NRO0 with real hardware. `scripts/package_release.py` performs only header/segment sanity checks; it does not compile or prove the binary runs. Do not rename desktop or ELF outputs as `.nro`.
