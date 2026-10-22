# PaperNX v0.12.0-reader.3 — Refael Bar / rtk19 fork

Adds automatic startup checks and + → Check for update against rtk19/PaperNx. Published fork prereleases are included. A newer version shows Later, What's new and Update. Notes scroll with touch, D-pad or either stick. Update streams and verifies the NRO; Restart finishes installation after PaperNX releases its fonts and RomFS. If the loader cannot relaunch, reopen PaperNX from Homebrew Menu.

## First updater-enabled upgrade

Back up sdmc:/switch/PaperNX/books/ and data/. Replace sdmc:/switch/PaperNX/PaperNX.nro with the attached NRO, or extract PaperNX-switch-v0.12.0-reader.3.zip at the SD root. Restart and confirm 0.12.0-reader.3. This one manual upgrade installs the updater; future releases can be installed inside the app. Developer: Refael Bar.

## עברית

יש להתקין את הגרסה הזו ידנית פעם אחת: החלף את PaperNX.nro והפעל מחדש. לאחר מכן האפליקציה בודקת את הפורק שלך אוטומטית בכל פתיחה. אפשר לבדוק גם בתחתית תפריט הפלוס, בכפתור Check for update.

כשיש עדכון מופיע חלון עם Later, What's new ו־Update. לחיצה על Update מורידה ומאמתת את הקובץ; Restart מסיים את ההתקנה ומבקש הפעלה מחדש. אם הטוען אינו תומך בכך, פתח שוב דרך Homebrew Menu. הספרים, ההגדרות והתקדמות הקריאה נשמרים. גם גרסאות prerelease מזוהות.

## Verification and source

Passed: 23 native CTests, 23 ASan/UBSan CTests with leak detection, 7 portable tests, packaging tests, 20 HTTP/HTTPS integration scenarios, offscreen OpenGL updater UI, live fork API over verified TLS, AArch64/NRO/NACP, embedded fonts/CA store, source receipt and installation ZIP integrity. Physical Switch networking, SD replacement and hbloader restart remain unverified.

HTTPS certificate, size, SHA256, NRO resources, PaperNX identity and version checks precede staging. Failed/partial downloads never replace the running executable. Installation occurs only after shutdown; replacement failure attempts restoration from the old NRO. No books/settings/progress files are installed by the updater.

Source: https://github.com/rtk19/PaperNx/tree/6cec7a714475d386c6069a1b85a49a951a2e264b
Guide: https://github.com/rtk19/PaperNx/blob/6cec7a714475d386c6069a1b85a49a951a2e264b/docs/AUTO_UPDATE.md
Original: https://github.com/ysfbz/PaperNx — Yusuf Buzrul; upstream attribution/licenses remain.

Exact source, project/third-party licenses, SHA256SUMS, manifest and verification evidence accompany the release.

NRO SHA256: 3960eccbc314d1e05930a74b9e12f0c978822428871630e772b897d4e3bd2ee1
