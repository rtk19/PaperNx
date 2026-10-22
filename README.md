# PaperNX v0.12.0-reader.2 — Refael Bar / rtk19 fork

Books: chapter pages remaining at the top while the toolbar is visible, and only the current page numeral at the bottom at all times. Both captions occupy the existing empty margin strips. Original Hebrew bold/italic, combined styles, underline and other author emphasis now render with reader typography. Comics retain their controls and appearance.

## Install / upgrade

Back up `sdmc:/switch/PaperNX/books/` and `data/`. Replace `sdmc:/switch/PaperNX/PaperNX.nro`, or extract `PaperNX-switch-v0.12.0-reader.2.zip` at the SD root. Restart PaperNX and confirm `0.12.0-reader.2`. Hebrew/Latin fonts are embedded in the NRO. Homebrew Menu developer: Refael Bar.

## Book controls

Tap the middle to show/hide tools. Chapter pages remaining appears only with the toolbar (including the existing always-visible-toolbar option). The last chapter page says “Last page in chapter”. The bottom numeral is always visible, without a total or remaining count. Reflowable page numbers follow current font/layout settings; continuous EPUB uses screen-sized pages across spine items. EPUB without a TOC uses spine boundaries; PDF without an outline omits the unknown chapter caption. Header icons open Library, Contents, Bookmarks and Aa.

## עברית

בספרים בלבד: לחיצה באמצע מציגה את התפריט ואת מספר העמודים שנותרו בפרק למעלה. כשהתפריט נסגר הטקסט העליון נעלם. בתחתית מופיע תמיד רק מספר העמוד הנוכחי. שניהם ממורכזים בשוליים הריקים ואינם מכסים או מזיזים את הספר. בולד, איטליק ושילובים שלהם בעברית נשמרים מהעיצוב המקורי של ה־EPUB. להעדכון החלף את PaperNX.nro והפעל מחדש; אין צורך למחוק ספרים או התקדמות.

## Source and verification

Source: https://github.com/rtk19/PaperNx/tree/2b337ec58a52ee4bc40891f8b24c13043afae6a4
Original: https://github.com/ysfbz/PaperNx — Yusuf Buzrul; upstream attribution and licenses remain.

Passed: 21 native CTests, 21 ASan/UBSan CTests with leak detection, 7 portable tests, packaging rejection/extraction tests, offscreen OpenGL book rendering, AArch64/NRO/NACP, embedded fonts, source receipt and installation ZIP integrity. Physical Switch acceptance remains unverified. Verification evidence includes original Hebrew fixtures and screenshots.

License notices and corresponding tagged source accompany the release. See SHA256SUMS and manifest.json.

NRO SHA256: 9d2d74caaa90e86e88820e29f802ae3ad6a3999293a9225c196988d276701015
