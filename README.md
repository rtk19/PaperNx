# PaperNX v0.11.0-reader.4 — Refael Bar / rtk19 fork

The fork developer displayed in Homebrew Menu is now **Refael Bar**. Original PaperNX was developed by **Yusuf Buzrul / ysfbz**; original attribution and licenses are retained.

## Install / update

Back up `sdmc:/switch/PaperNX/books/` and `data/`. Replace `sdmc:/switch/PaperNX/PaperNX.nro` with the attached NRO, or extract `PaperNX-switch-v0.11.0-reader.4.zip` at the SD root. Keep books and data. Restart and confirm `reader.4`; Homebrew Menu should display **Refael Bar** as developer.

**עברית:** שם המפתח המוצג הוא כעת **Refael Bar**. החלף את PaperNX.nro בתיקייה switch/PaperNX והפעל מחדש. אין צורך למחוק ספרים או נתוני קריאה. הקרדיט ליוצר המקורי נשמר.

## Reader controls retained from reader.3

- Arrows/D-pad and LS/RS follow landscape and both portrait rotations, including held zoomed panning.
- Hold ZR/ZL for smooth zoom in/out. L/R and touch edges turn pages with animation; comic zoom stays and new pages start at the top. Clicking RS or Fit page resets zoom.
- Series shelves, natural issue order, Hebrew fonts/direction, EPUB/PDF/CBZ/CBR support and saved reading data continue to work as in reader.3.

## Source and validation

Source: https://github.com/rtk19/PaperNx/tree/0e97d555660484294f8222eb4623ff41a1cf3276
Guides: https://github.com/rtk19/PaperNx/blob/0e97d555660484294f8222eb4623ff41a1cf3276/README.md
Original: https://github.com/ysfbz/PaperNx

This metadata update passed 7 portable tests, package tests, the real AArch64/NRO/NACP author/version checks, embedded font verification and installation ZIP validation. Native/OpenGL/sanitizer reader regressions were last run for reader.3 and were not rerun for this name change. Physical Switch acceptance remains unverified.

AGPL-3.0 and dependency licenses apply. License notices and exact tagged source, including the full FriBidi source archive and MuPDF patch/build scripts, accompany this release. See SHA256SUMS and manifest.json.

NRO SHA256: 9fce43fbffa65266ef62df9d1b0f7a1e12224f11547275da6e22adbd54f37b86
