# PaperNX 0.13.0 — Refael Bar / rtk19 fork

All reader formats now share the bottom page caption: EPUB, PDF, CBZ and CBR.

- Toolbar hidden: only the current page, for example `12`.
- Toolbar visible: `12 of 398 • 3%`, in the same empty bottom strip and muted type.

The seek bar appears above the caption. Comic artwork, zoom/pan, manga order and book content geometry remain. Continuous EPUB uses reading-screen totals. Continuous comics count image pages. Single-page titles show `1 of 1 • 100%` with tools visible.

## Versions

New versions have no reader/comics suffix. This feature release is 0.13.0. A later fix becomes 0.13.1; a later feature release becomes 0.14.0. Git tags use the conventional v prefix. Historical release tags are preserved, and existing automatic updaters recognize the new version.

## Update / install

Use + → Check for update → Update → Restart if your current version includes the updater. Otherwise replace sdmc:/switch/PaperNX/PaperNX.nro with the attached file, or extract PaperNX-switch-v0.13.0.zip at the SD root. Keep books/ and data/. The new application and Homebrew Menu version is 0.13.0.

## עברית

התצוגה התחתונה חלה כעת גם על קומיקס וגם על ספרים: כשהתפריט מוסתר מופיע רק מספר העמוד; כשהוא מוצג מופיעים העמוד הנוכחי, סך העמודים ואחוז ההתקדמות, באותו מקום בשוליים הריקים.

מספור הגרסאות עבר למספרים רגילים ללא סיומות: 0.13.0 לתוספת הזו, 0.13.1 לתיקון קטן בהמשך ו־0.14.0 לתוספת תכונות בהמשך. אפשר לעדכן דרך הפלוס ואז Check for update. אין צורך למחוק ספרים או התקדמות.

## Verification and source

Passed: 23 native CTests, 23 ASan/UBSan CTests with leak detection, 7 portable tests, packaging tests, offscreen OpenGL book/comic captions, stable-version update migration tests, AArch64/NRO/NACP, embedded fonts/CA store, exact source receipt and installation ZIP integrity. Physical Switch acceptance remains unverified.

Source: https://github.com/rtk19/PaperNx/tree/4c0c762f2d2ab223c72c5bc1d026ee289442ae34
Original: https://github.com/ysfbz/PaperNx — Yusuf Buzrul; attribution and licensing remain.

NRO, installation ZIP, exact source, licenses, checksums, manifest and verification evidence accompany the release.
