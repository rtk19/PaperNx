# PaperNX v0.11.0-reader.1 — rtk19 fork

Hebrew fonts and automatic EPUB paragraph direction; series shelves with numeric issue sorting; zoomed comic page curls with top-of-page resets; held D-pad/LS panning and global RS scrolling.

## Install / update

1. Back up `sdmc:/switch/PaperNX/books/` and `data/`.
2. Extract `PaperNX-switch-v0.11.0-reader.1.zip` at the SD root, allowing existing application/font files to be replaced. Or replace only `sdmc:/switch/PaperNX/PaperNX.nro` with the new NRO: Hebrew fonts are also embedded.
3. Keep books and data. Start PaperNX and confirm `v0.11.0-reader.1` in the footer or + → Help. Duplicate old NRO copies can launch an older build.

No library reset, database deletion or manual font installation is required. This fork and upstream use the same SD paths.

## Controls

- Zoomed comics: hold D-pad/arrows or LS to pan; drag the image interior. Tilt RS to pan/scroll; clicking RS still resets zoom.
- Turn pages using L/R, edge taps or a horizontal drag starting in the outer 12% of the screen. Curl animation works at zoom. The next page retains zoom and starts at the top (right edge for manga).
- All books groups series into a single card. A/tap opens its issues in number order; Back/B returns. Search and reading categories show matching individual books. Initial background indexing may take time for large/solid CBR libraries.
- Hebrew titles/authors use bundled fonts automatically. EPUB start alignment follows each paragraph; justified endings and indentation follow its direction. Publisher left/right alignment is preserved. The interface remains English/Turkish.

## הוראות קצרות בעברית

גבה את books ואת data. חלץ את חבילת ההתקנה לשורש ה־SD ואשר החלפת קבצים, או החלף את PaperNX.nro בתיקייה switch/PaperNX. אל תמחק נתוני קריאה. פתח מחדש ובדוק שמופיעה הגרסה v0.11.0-reader.1.

בזום: החזק חצים או LS כדי להזיז את התמונה; הטיית RS גוללת גם ברשימות ובתפריטים. L/R, הקשה בקצה או גרירה אופקית מהקצה מדפדפים עם אנימציה. הזום נשמר והעמוד הבא נפתח מלמעלה. כרטיס סדרה נפתח לחוברות בסדר מספרי; Back או B מחזירים לספרייה. הסריקה הראשונה מתבצעת ברקע.

## Source / verification

Source: https://github.com/rtk19/PaperNx/tree/7c0691eb55f7f89821300745560db2cfb626f507
Guides: https://github.com/rtk19/PaperNx/blob/7c0691eb55f7f89821300745560db2cfb626f507/docs/COMICS.md and docs/HEBREW.md
Original project: https://github.com/ysfbz/PaperNx — Yusuf Buzrul

AGPL-3.0 and dependency licenses apply. The corresponding source ZIP includes the complete FriBidi 1.0.12 source archive, vendored unarr, font notices and the MuPDF RTL patch/build scripts. See the license ZIP and SHA256SUMS.

7 portable tests, 19 native tests, 5 focused ASan/UBSan tests, offscreen OpenGL, AArch64/NRO/RomFS and ZIP checks passed. Test screenshots use original fixtures. Physical Nintendo Switch appearance, touch latency and FPS remain UNVERIFIED.

NRO SHA256: 9deffbe7f5f0fec607d0c0cedad3580288f363b6bc28401a4427814e57778445
