# Nintendo Switch build status — BLOCKED

No devkitPro, devkitA64, libnx, Switch SDL2 portlibs or ARM64 MuPDF library is present in the current execution environment. Therefore there is no compiled Switch artifact.

Cross-compile reference: https://github.com/switchbrew/switch-examples/tree/master/graphics/sdl2 and https://devkitpro.org/ . Packages such as `switch-dev`, `switch-sdl2`, `switch-sdl2_ttf` are documented for devkitPro; the existence of a prebuilt `switch-mupdf` package **must not be assumed**.

To complete this, pin a MuPDF release and its dependencies, prepare an audited ARM64 static build, add appropriate Switch link directives and generate/verify the actual NRO0 format. Do not rename desktop or ELF outputs as `.nro`.
