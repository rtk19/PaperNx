## PaperNX 0.17.1

### חזרה לתפריטים ותיקוני דפדוף

- התפריטים, הספרייה, טעינת הקבצים, ההגדרות, היומן ומסכי הטעינה חזרו לעיצוב של 0.16.0.
- נשמרו כלי הקריאה, אנימציות הדפדוף, מספרי העמודים, התמיכה בעברית, כיוון L/R, זום וההגדרות לשטח הקריאה ולזוג עמודים.
- בקומיקס, הקיפול משתמש בגבולות הדפים עצמם. גרירה בשוליים מזוהה, והדף מתחיל להתקפל כשהאצבע מגיעה אליו. הפס המרכזי מסתיים בקצה האיורים גם בזום.
- תוקן ניתוב המגע בקומיקס, כולל דפדוף בזוג עמודים מוגדל, והכנת זוג העמודים הבא משתמשת כעת במטמון המתאים להפחתת ההמתנה לכפתורים.
- בספרים אפשר להתחיל אנימציה שעוקבת אחר האצבע גם בשוליים העליונים והתחתונים, כשהכפתורים אינם שם.
- הספרייה, ההעדפות והתקדמות הקריאה נשמרות. אין איפוס נתונים.

### הורדה ועדכון

להתקנה ראשונה: `PaperNX-switch-v0.17.1.zip`. לעדכון ידני: החליפו את `sdmc:/switch/PaperNX/PaperNX.nro` בקובץ `PaperNX.nro`. אפשר לעדכן גם מתוך האפליקציה דרך **+ → Check for update**. יש לגבות את `books/` ו־`data/` לפני החלפה ידנית.

הגרסה נבדקה בבדיקות SDL/MuPDF, OpenGL ובדיקות זיכרון; תוצאות מלאות וראיות נכללות ב־Assets. לא בוצעה בדיקה על Switch פיזי, ולכן זמני תגובה על המכשיר עדיין דורשים אימות.

### English

Restores the 0.16.0 external application/menu presentation while retaining reading controls and prior reader features. Comic curls now use paper bounds at fit and zoom; margin gestures wait for paper contact, spines stop at artwork edges, next-group images warm in the reader cache, and button-started animation presents promptly. Book drags from top/bottom margins stay interactive. Existing data is retained. Real Switch acceptance remains unverified.

Developer: Refael Bar. Source, licenses, SHA-256 checksums and verification evidence accompany the NRO and installation ZIP.


Exact source: https://github.com/rtk19/PaperNx/tree/b67553f7c4bc628eaa553952e212b18d75cd2248
