## PaperNX 0.17.0 — A new reading space

עיצוב מחדש של מבנה האפליקציה וחוויית השימוש:

- **בית וספרייה נפרדים**: כרטיס המשך קריאה עם כריכה, מחבר והתקדמות, מדפי ספרים, סינון וחיפוש.
- **ניווט קבוע**: תפריט צד ברוחב וניווט תחתון בפורטרייט, עם גישה לבית, ספרייה, אוספים, יומן וקבצים.
- **מסלול הוספת ספרים חדש**: רשימת קבצים עם כרטיסים, תיקיית אב, הסבר והוספת תיקייה שלמה. לספרייה ריקה יש הדרכה עם פעולות ישירות.
- **הגדרות שתוכננו מחדש**: פריסה מותאמת לכיוון המסך, קטגוריות, סדר ברור להגדרות האפליקציה, תצוגה מקדימה וכפתורי סיום ואיפוס נפרדים.
- **כלי קריאה חדשים**: קבוצות כפתורים צפות באזורי השוליים הריקים, תוכן עניינים עם ניווט משלו, כרטיסי סימניות ותפריט פעולות באריחים.
- **ניהול ספרים ויומן**: כרטיס ספר עם כריכה והתקדמות, פעולות מסודרות, יומן בתוך מבנה הניווט החדש, בחירות וצבעי קריאה עם תצוגה מקדימה.
- **מסכי פתיחה** לאפליקציה ולפתיחת ספר, ללא השהיה מלאכותית.

כל מה שנבנה לקריאה נשמר: אנימציות הדפדוף, שני עמודים ברוחב, מספרים כחלק מהעמוד והאנימציה, ניצול המסך והאפשרות לחזור לפריסה המקורית, כיווני עברית/אנגלית ו־L/R, צימוד חכם בקומיקס, זום לפי מרכז המחווה, חזרה לראש העמוד/Y, העדפות אישיות, תיקיות, אוספים, יומן ועדכונים אוטומטיים. אין איפוס של ספרים או נתוני קריאה.

### Install / update

Use **+ → Check for update → Update → Restart**, or accept the startup update prompt. For a first install, extract `PaperNX-switch-v0.17.0.zip` to the SD root. For a manual upgrade, replace `sdmc:/switch/PaperNX/PaperNX.nro` and keep your `books/` and `data/`.

### Verification

27 native tests; the same 27 under ASan/UBSan with leak detection; 8 portable tests; packaging checks; offscreen OpenGL app-experience, spread and book-caption tests. Existing pixel-level reading/curl/zoom/RTL regressions remain, with a new real-touch screen/navigation suite. Switch artifacts validate AArch64, NRO/NACP version/developer, fonts/CA, licenses, source receipts and installation ZIP. Logs and actual fixture screenshots are included in `verification-evidence.zip`.

Physical Switch touch latency, FPS and installation acceptance still require device testing. Developer: **Refael Bar**. Original attribution and licenses remain.


Exact source: https://github.com/rtk19/PaperNx/tree/2d0ba25834f95dfd0b9abc6e94e127c9ee8ebbe2
