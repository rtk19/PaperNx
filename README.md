# PaperNX 0.14.0 — Refael Bar / rtk19 fork

Landscape paginated EPUB/PDF/CBZ/CBR reading now displays two physical pages.
Hebrew and manga use right-to-left order. A leaf curls across the centre onto
its real reverse page; text and folios turn together, including the incoming
page numbers. Portrait also uses a numbered sheet. Duplicate EPUB margins are
removed so text uses more of the screen, with clear header/footer areas.

Opening the toolbar shows current of total and percentage in portrait, or the
visible page range and percentage for a spread (for example 11–12 of 398 • 3%).
Continuous vertical scrolling and zoomed artwork retain their selected modes.
EPUB numbers can change with typography/orientation; existing text anchors,
bookmarks and page-grid targets remain available.

## Install / update

Use + → Check for update → Update → Restart if your installed version has the
updater. For manual upgrade replace sdmc:/switch/PaperNX/PaperNX.nro with the
attached NRO. For first installation extract PaperNX-switch-v0.14.0.zip at the
SD root. Keep books/ and data/. Application and Homebrew Menu version: 0.14.0.

## עברית

במצב רוחב מוצגים שני עמודים, בסדר קריאה מתאים לעברית. המספר של כל עמוד הוא
חלק מהנייר ונע עם הטקסט בדפדוף, גם בדפים הנחשפים וגם במצב לאורך. בוטלו
השוליים הכפולים בספרי EPUB כדי לנצל יותר מהמסך. עדכון: פלוס ואז
Check for update → Update → Restart. אין צורך למחוק ספרים או התקדמות.

## Verification and source

Passed: 24 native CTests, 24 ASan/UBSan CTests with leak detection, 7 portable
tests, packaging tests, offscreen OpenGL spread/chrome tests, AArch64/NRO/NACP,
embedded fonts/CA, exact source receipt and installation ZIP integrity.
Physical Switch acceptance, animation FPS and on-device installation remain
unverified. Test screenshots contain only original generated fixture content.

Source: https://github.com/rtk19/PaperNx/tree/f911380177df0fde638ef88e79d4d83c69932b48
Layout details: https://github.com/rtk19/PaperNx/blob/v0.14.0/docs/READER_SPREADS.md
Original: https://github.com/ysfbz/PaperNx — Yusuf Buzrul. Licensing retained.

Assets include NRO, installation ZIP, exact source, licenses, checksums,
manifest and verification evidence. New versions use numeric semantic versions.
