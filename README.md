# PaperNX v0.11.0-reader.3 — rtk19 fork

Hebrew fonts and automatic EPUB paragraph direction; series shelves with numeric issue sorting; zoomed comic page curls with top-of-page resets; rotation-aware arrows/D-pad/LS/RS, matching stick navigation, smooth held ZR/ZL zoom, continuous D-pad panning without an initial jump and full paper geometry for comic curls.

## Install / update

1. Back up `sdmc:/switch/PaperNX/books/` and `data/`.
2. Extract `PaperNX-switch-v0.11.0-reader.3.zip` at the SD root, allowing existing application/font files to be replaced. Or replace only `sdmc:/switch/PaperNX/PaperNX.nro` with the new NRO: Hebrew fonts are also embedded.
3. Keep books and data. Start PaperNX and confirm `v0.11.0-reader.3` in the footer or + → Help. Duplicate old NRO copies can launch an older build.

No library reset, database deletion or manual font installation is required. This fork and upstream use the same SD paths.

## Controls

- Arrows, D-pad, LS and RS follow the current display orientation in landscape and either portrait direction. The mapping updates while held; touch and directional controls use the same inverse rotation. Modal navigation follows the same rule.
- LS and RS have identical directional behavior across the app: proportional panning in enlarged fixed pages, focused navigation and repeat in menus/lists. Existing stick-click shortcuts remain; RS click resets zoom.
- Hold ZR to zoom in or ZL to zoom out smoothly around the viewport center. Release to stop. Opposing triggers cancel.
- In zoomed comics hold D-pad/arrows to pan continuously, without the previous 64-pixel initial jump, or drag the image interior. Curl animation bends the complete paper sheet, not the viewport/margins.
- Turn pages using L/R, edge taps or a horizontal drag starting in the outer 12% of the screen. Curl animation works at zoom. The next page retains zoom and starts at the top (right edge for manga).
- All books groups series into a single card. A/tap opens its issues in number order; Back/B returns. Search and reading categories show matching individual books. Initial background indexing may take time for large/solid CBR libraries.
- Hebrew titles/authors use bundled fonts automatically. EPUB start alignment follows each paragraph; justified endings and indentation follow its direction. Publisher left/right alignment is preserved. The interface remains English/Turkish.

## הוראות קצרות בעברית

גבה את books ואת data. חלץ את חבילת ההתקנה לשורש ה־SD ואשר החלפת קבצים, או החלף את PaperNX.nro בתיקייה switch/PaperNX. אל תמחק נתוני קריאה. פתח מחדש ובדוק שמופיעה הגרסה v0.11.0-reader.3.

החצים ושני הסטיקים מתאימים את הכיוון אוטומטית לסיבוב המסך לשני הכיוונים, גם בזמן החזקה. RS מתנהג כמו LS בניווט ובהזזה. החזק ZR כדי להגדיל או ZL כדי להקטין ברציפות; שחרור עוצר. בזום: החזק חצים להזזה רציפה ללא קפיצה בתחילת הלחיצה. הקיפול חל על הדף עצמו והרקע נשאר במקום. L/R, הקשה בקצה או גרירה אופקית מהקצה מדפדפים עם אנימציה. הזום נשמר והעמוד הבא נפתח מלמעלה. כרטיס סדרה נפתח לחוברות בסדר מספרי; Back או B מחזירים לספרייה. הסריקה הראשונה מתבצעת ברקע.

## Source / verification

Source: https://github.com/rtk19/PaperNx/tree/3adf684d249242aea968b8950add947eea66fce4
Guides: https://github.com/rtk19/PaperNx/blob/3adf684d249242aea968b8950add947eea66fce4/docs/COMICS.md and docs/HEBREW.md
Original project: https://github.com/ysfbz/PaperNx — Yusuf Buzrul

AGPL-3.0 and dependency licenses apply. The corresponding source ZIP includes the complete FriBidi 1.0.12 source archive, vendored unarr, font notices and the MuPDF RTL patch/build scripts. See the license ZIP and SHA256SUMS.

7 portable tests, 19 native tests, 5 focused ASan/UBSan tests, offscreen OpenGL, AArch64/NRO/RomFS and ZIP checks passed. Test screenshots use original fixtures. Physical Nintendo Switch appearance, touch latency and FPS remain UNVERIFIED.

NRO SHA256: 2f390a524468742ab2a3fda79b2ce8be780d5c2c49062603f6666fb3ca0f1899
