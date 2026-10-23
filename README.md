## ממשק וספרייה חדשים · Modern menus and library

- תפריטים, ספרייה, הגדרות וחלונות בעיצוב חדש בהשראת NX-torrent-player. מראה כהה/בהיר/לפי ערכת הקריאה ושבעה צבעי הדגשה לבחירה.
- הוספת תיקיית ספרים שלמה, כולל תת־תיקיות, באמצעות Add this folder בדפדפן או Y. ניהול תיקיות, סריקה אוטומטית בפתיחה וסריקה ידנית בהגדרות.
- סריקה ברקע ומטמון ספרייה לפתיחה מהירה; מניעת כפילויות, מיזוג באצווה וקריאת פרטי EPUB ללא עימוד מלא.
- ברוחב: באנגלית R קדימה ו־L אחורה; בעברית L קדימה ו־R אחורה. הכיוון מתעדכן לפי הספר שנפתח. פורטרייט שומר על ההתנהגות הקיימת.
- אפשרויות ניצול המסך, שני עמודים, זום הקומיקס ותיקוני הגרירה שפורסמו בנפרד ב־[0.15.0](https://github.com/rtk19/PaperNx/releases/tag/v0.15.0) כלולים.

Modern shared controls, cards, settings, dialogs and quick actions follow the companion player's colour family. Interface styling is independent of reading paper. Import whole book folders, manage scan roots, scan asynchronously on launch or manually, and reopen an atomic cached inventory. EPUB metadata indexing avoids full pagination. Landscape shoulder controls follow each opened book's reading direction. Existing catalog, journal, reading preferences, updates and comic controls remain available.

### Install / update

Choose **+ → Check for update**, then **Update → Restart**, or allow the startup prompt. For a first installation, extract `PaperNX-switch-v0.16.0.zip` to the SD root. For a manual upgrade, replace `sdmc:/switch/PaperNX/PaperNX.nro`; keep your existing books and data.

### Verification

26 native tests; the same 26 under ASan/UBSan with leak detection; 8 portable tests; distribution checks; offscreen OpenGL modern-menu, spread and caption tests; AArch64/NRO/NACP/fonts/license/source-receipt verification. Logs and original-fixture screenshots are in `verification-evidence.zip`.

Physical Switch performance, touch latency and installation acceptance still require device testing. Developer: **Refael Bar**. Original project attribution and licenses are retained.


Exact source: https://github.com/rtk19/PaperNx/tree/699ef34daa1a3507b5ed3428b0af3bb18fecca60
