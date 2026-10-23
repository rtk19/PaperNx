# PaperNX 0.15.0

## עברית

- נוספו שתי הגדרות נפרדות תחת Application: **Expanded reading area** ו־**Two pages in landscape**. שתיהן פעילות כברירת מחדל, ואפשר לכבות כל אחת כדי לחזור למצב המקורי.
- בקומיקס העמודים צמודים לפס אמצעי. סריקות רחבות וקבצים שמסומנים ב־ComicInfo כדף כפול מוצגים בשלמותם, והדפדוף שומר על סדר העמודים.
- זום בקומיקס במצב רוחב שומר את שני העמודים. ההגדלה משתמשת בנקודת מחוות האצבעות ובתמונות שכבר נטענו, כדי להסיר את הקפיצה בכניסה לזום.
- בזמן זום נוספו חץ למעלה וקיצור **Y** לחזרה לראש העמוד, בלי לשנות את ההגדלה או המיקום האופקי. Fit page נשאר כפתור נפרד.
- בספרים ובקומיקס, גרירת דף כפול משתמשת כעת במרחק הגרירה על פני המסך המלא, במקום בחצי מהמרחק.

עדכון: **+ → Check for update → Update → Restart**. להתקנה ידנית החליפו את `sdmc:/switch/PaperNX/PaperNX.nro`, או חלצו את קובץ ההתקנה לשורש ה־SD. אין צורך למחוק ספרים או נתוני קריאה.

## Changes and verification

Independent default-on expanded-viewport and landscape-spread settings restore the earlier modes when disabled. Comic pages align at the centre gutter, wide/ComicInfo double-page scans stay whole, and cached local boundaries preserve group order without scanning every preceding solid-RAR entry. Landscape comic zoom keeps both pages, reuses fitted textures at the first pinch and preserves the image point beneath the gesture centroid. Up-arrow/Y returns to the top while retaining horizontal pan and zoom; Y outside comic zoom remains a bookmark shortcut. Spread curls now map to full-screen finger travel.

Passed: 24 native CTests, 24 ASan/UBSan CTests with leak detection, 7 portable tests, packaging tests, offscreen OpenGL reader checks and Switch AArch64/NRO/NACP/source-receipt/archive verification. New tests exercise saved switches and setting-row taps, wide/marked scans, forward/back groups, first-pinch texture reuse, focal stability, touch/Y actions and full-screen drag progress. Physical Switch FPS, touch feel and on-device installation remain unverified.

Assets include NRO, installation ZIP, exact source, license notices, checksums, manifest and original-fixture test evidence. Developer: **Refael Bar**. Original project by Yusuf Buzrul / ysfbz; licensing and attribution retained.


Exact source: https://github.com/rtk19/PaperNx/tree/17ae24c0a82130bf65e1000efa2d32ba94312074
