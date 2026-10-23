## PaperNX 0.17.2 — Comic performance fix

### תיקון ההשהיות בקומיקס

- תוקנה סיבת הפענוחים החוזרים: הכנת עמודים שכנים פינתה מהמטמון את הדפים המוצגים, וכך פעולות תפריט ומגע פענחו אותם שוב.
- תמונות התצוגה ותמונות הזום מופרדות ובעלות תקציבי זיכרון נפרדים. הדפים הפעילים מוגנים מפינוי של טעינה מקדימה.
- הכנת הדפים הבאים ושיפור חדות הזום עברו לעובד רקע, בלי לחסום את המגע והכפתורים. הזום נשאר על זוג הדפים ומתחדד ברקע.
- קריאה קדימה של ZIP/RAR5 שומרת את רצף הארכיון, וב־Solid נשמר מצב הפענוח. טעינה מקדימה הולכת בכיוון הדפדוף כדי לא להתחיל מחדש ארכיון RAR בגלל קריאה ספקולטיבית לאחור.
- הופחתה העבודה באנימציית מספרי העמודים ונמנע ציור חוזר לכל אירוע קלט ממתין.
- נשמרו התפריטים של 0.16.0 וכל שיפורי הקריאה, הדפים הכפולים, מחוות, אנימציות, מספרי עמודים, עברית וכיוון L/R. אין איפוס נתונים.

### בדיקות

נוספו בדיקות עם CBR מקורי לבדיקה של **37 עמודים וכ־43.5MB**, וכן RAR4 ו־RAR5 דחוסים במצב Solid. בתרחיש ששחזר את התקלה היו קודם 12 פענוחים במסלול התפעול בארבע פעולות תפריט/טעינה מקדימה; אחרי התיקון היו אפס. נבדקו גם דפדוף וזום ללא פענוחים חוזרים, חדות שני הדפים, תקציבי זיכרון, ביטול עבודות ושמירת התכונות הקיימות. דוחות ולוגים נכללים ב־Assets.

התוצאות הן מבדיקות מחשב; ספר הקומיקס המסוים וה־Switch הפיזי לא היו זמינים לבדיקה. פתיחה ראשונה או קפיצה לעמוד שלא נטען עדיין יכולות לדרוש קריאה מה־SD.

### התקנה ועדכון

אפשר לעדכן מתוך האפליקציה דרך **+ → Check for update**.
לעדכון ידני החליפו את `sdmc:/switch/PaperNX/PaperNX.nro` בקובץ `PaperNX.nro`. להתקנה ראשונה השתמשו ב־`PaperNX-switch-v0.17.2.zip`. גַבו את `books/` ו־`data/` לפני החלפה ידנית.

### English

Fixes comic cache churn and UI-thread neighbour decoding. Bounded base images remain protected while directional neighbours and both pages' zoom detail prepare on an independent worker. ZIP/RAR5 forward reads preserve archive/solid state. Folio rendering and queued-event presentation do less work. Existing menus, reading features and data remain. Regression evidence includes original 37-page ~43 MB CBR, compressed solid RAR4/RAR5, native/OpenGL and memory checks. Physical Switch performance remains unverified.

Developer: Refael Bar. NRO, installation ZIP, corresponding source, licenses, SHA-256 checksums and verification evidence are supplied together.


Exact source: https://github.com/rtk19/PaperNx/tree/d2b4bcfd10b1a66043c5b8cd36da08340cffec6d
