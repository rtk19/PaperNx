# PaperNX v0.12.0-reader.1 — Refael Bar / rtk19 fork

Adds a preview scrubber and thumbnail navigation for EPUB, PDF, CBZ and CBR. Original PaperNX: Yusuf Buzrul / ysfbz; upstream credits and licenses are retained.

## Install / upgrade

Back up `sdmc:/switch/PaperNX/books/` and `data/`. Replace `sdmc:/switch/PaperNX/PaperNX.nro` with the attached NRO, or extract `PaperNX-switch-v0.12.0-reader.1.zip` at the SD root. Keep books and data. Restart and confirm `0.12.0-reader.1`. Fonts, including Hebrew, are also embedded in the NRO. Homebrew Menu developer: **Refael Bar**.

## New navigation

- Tap the page center to show the toolbar. Drag the bottom progress bar: a thumbnail and destination number appear without changing the open/saved position. Release to jump once; B/Esc, multi-touch, rotation or focus loss cancels. Comic zoom stays and new pages start at the top.
- The footer grid button or **+ → Page thumbnails** opens Chapters, All pages and Bookmarks. Comics default to the page grid. Scroll/tap cards, or use arrows/either stick and A. B/Close returns; L/R switch tabs. Current and bookmarked pages are marked.
- **Contents** uses thumbnail chapter rows with readable Hebrew/niqqud, wrapped mixed-language titles and nesting. Continuous EPUB fragment links jump to the heading inside its chapter. The page grid remains available when chapter metadata is absent.
- **LS click / G** starts controller preview; left/right chooses one page, up/down ten, A/Enter confirms and B/Esc cancels. Both sticks and the D-pad follow display rotation. RTL books reverse the slider/page order. Existing ZR/ZL zoom, comic curls, bookmarks, history, series shelves and saved data remain.

## עברית

גבה את books ואת data. החלף את PaperNX.nro בתיקייה switch/PaperNX והפעל מחדש. בדוק שמופיעה הגרסה 0.12.0-reader.1; אין צורך למחוק ספרים או נתוני קריאה.

הקש באמצע העמוד להצגת הכלים, ואז גרור בסרגל האחוזים: תופיע תצוגה מקדימה, ורק בשחרור יתבצע מעבר. B מבטל. כפתור הריבועים פותח תמונות ממוזערות של כל העמודים, פרקים וסימניות. כותרות פרקים בעברית מוצגות נכון, כולל קישורים לכותרת פנימית בגלילה רציפה. לחיצה על LS מאפשרת בחירה בשלט; החצים או שני הסטיקים בוחרים, A מאשר ו־B מבטל. L/R מחליפים לשוניות בתוך הדפדפן.

## Source and checks

Source: https://github.com/rtk19/PaperNx/tree/a45a52ffa6a8ddc5a267416c71fbf942cde11f79
Guide: https://github.com/rtk19/PaperNx/blob/a45a52ffa6a8ddc5a267416c71fbf942cde11f79/docs/PAGE_NAVIGATION.md
Original: https://github.com/ysfbz/PaperNx

Passed: 20 native tests, 7 portable tests, 6 focused ASan/UBSan tests with leak detection, offscreen OpenGL navigation/comic readers, AArch64/NRO/NACP, embedded fonts, source receipts and actual installation ZIP checks. Physical Switch appearance, FPS and touch latency remain unverified. Evidence screenshots use original test artwork.

License and corresponding source archives accompany the release, including the complete FriBidi source archive, unarr, font notices and MuPDF patch/build scripts. See SHA256SUMS and manifest.json.

NRO SHA256: 41d2d0addbf276d42add3ca81119b4a018fd970dfc2d65c6f0098d59fc9e07bb
