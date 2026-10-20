# PaperNX 0.9.0-curl.2 — interactive paper curl

Source commit: [a4e0d046562679171bf4eb7507c2825a6f9987f9](https://github.com/rtk19/PaperNx/tree/a4e0d046562679171bf4eb7507c2825a6f9987f9).

Drag a page to control its curl. Grip height and vertical movement change the crease. Pause or reverse the drag; release to complete or cancel. The sheet uses perspective, a translucent paper back, directional lighting and soft shadows. Buttons and taps still work.

See interactive-page-curl.mp4 or interactive-page-curl.gif. The preview comes from native SDL/OpenGL; the cyan marker indicates simulated finger input. It is not a recording from Switch hardware.

Install: back up your data, then replace only `sdmc:/switch/PaperNX/PaperNX.nro`. Keep existing books and data directories. Paginated EPUB and fit-to-page PDF support the curl.

All 7 portable CTest checks, 14 native CTest checks, Python packaging regressions, a separate OpenGL interaction run, devkitA64/libnx build, AArch64/NRO/assets validation and distribution checks passed. Verification evidence records the exact source and binary hashes.

Switch hardware compatibility and frame rate are unverified. The previous build is available on builds/v0.9.0-curl.1. AGPL-3.0 and bundled dependency licenses apply.
