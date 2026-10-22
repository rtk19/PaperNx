# PaperNX v0.12.0-reader.4 — Refael Bar / rtk19 fork

Corrects the book footer in the existing empty bottom strip:

- Toolbar hidden: only the current page, for example `12`.
- Toolbar visible: `12 of 398 • 3%` in that same position and small muted type.

The percentage is rounded from current / total. Continuous EPUB counts reading screens across the book and reaches 100% on the final partial screen. The chapter caption at the top, document content viewport and comics retain their behavior.

## Update / install

If reader.3 is installed, use + → Check for update, Update, then Restart. Otherwise replace sdmc:/switch/PaperNX/PaperNX.nro with the attached NRO, or extract PaperNX-switch-v0.12.0-reader.4.zip at the SD root. Restart and confirm 0.12.0-reader.4. Preserve books/ and data/.

## עברית

כשהתפריט מוסתר מוצג רק מספר העמוד. כשהתפריט מוצג, אותו מקום מציג את העמוד הנוכחי, סך העמודים ואחוז ההתקדמות, למשל: 12 of 398 • 3%. הטקסט נשאר בשוליים הריקים למטה ואינו מכסה או מזיז את הספר.

אם גרסה reader.3 כבר מותקנת, אפשר לעדכן מתוך האפליקציה דרך הפלוס ואז Check for update. אחרת החלף את PaperNX.nro והפעל מחדש. הספרים ונתוני הקריאה נשמרים.

## Verification and source

Passed: 23 native CTests, 23 ASan/UBSan CTests with leak detection, 7 portable tests, packaging tests, offscreen OpenGL book captions, AArch64/NRO/NACP, embedded fonts/CA store, exact source receipts and installation ZIP integrity. Physical Switch acceptance remains unverified.

Source: https://github.com/rtk19/PaperNx/tree/9afbc42653b0b59d7521c1f28c809b51e4a3fa75
Original: https://github.com/ysfbz/PaperNx — Yusuf Buzrul; attribution and licenses remain.

NRO, installation ZIP, exact source, licenses, checksums, manifest and original fixture verification evidence accompany this release.
