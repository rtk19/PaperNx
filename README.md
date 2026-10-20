<p align="center">
  <img src="assets/branding/PaperNX-icon-512.png" alt="PaperNX logo" width="144">
</p>

<h1 align="center">PaperNX — rtk19 fork</h1>

<p align="center">Offline EPUB, PDF, CBZ and CBR reading on Nintendo Switch, with interactive paper curls and automatic Hebrew support.</p>

<p align="center">
  <a href="https://github.com/rtk19/PaperNx/releases/tag/v0.12.0-reader.1">Download this fork</a> ·
  <a href="docs/COMICS.md">Comic reader guide</a> ·
  <a href="https://github.com/ysfbz/PaperNx">Original project</a> ·
  <a href="LICENSE">AGPL-3.0</a>
</p>

## About this fork

**[rtk19/PaperNx](https://github.com/rtk19/PaperNx)** is an independently maintained fork of **[ysfbz/PaperNx](https://github.com/ysfbz/PaperNx)**, originally developed by **Yusuf Buzrul**. Fork maintenance and the additions described here belong to **Refael Bar / [@rtk19](https://github.com/rtk19)**. This repository has its own releases and changes; download this fork's builds and report fork-specific problems here.

The fork adds touch-controlled, perspective page curls and CBZ/CBR comic reading while retaining the original EPUB/PDF reader, offline library, reading journal and settings. The original project's authorship and license are preserved.

**Current release: [v0.12.0-reader.1](https://github.com/rtk19/PaperNx/releases/tag/v0.12.0-reader.1), a prerelease.** Adds a drag-to-preview progress bar and a thumbnail browser for chapters, all pages and bookmarked positions. Hebrew chapter labels and continuous chapter links are supported. The new navigation suite, reader regressions, sanitizer checks and Switch packaging are covered in [test results](docs/TEST_RESULTS.md). Physical Nintendo Switch compatibility, frame rate and touch latency have **not yet been verified**.

## Downloads — use Release Assets

Open the [release page](https://github.com/rtk19/PaperNx/releases/tag/v0.12.0-reader.1) and expand **Assets**. Choose the installation ZIP for a first install, or the NRO for an upgrade.

| Asset | Purpose |
|---|---|
| [PaperNX-switch-v0.12.0-reader.1.zip](https://github.com/rtk19/PaperNx/releases/download/v0.12.0-reader.1/PaperNX-switch-v0.12.0-reader.1.zip) | SD installation layout: application, bundled fonts and an empty books directory. |
| [PaperNX.nro](https://github.com/rtk19/PaperNx/releases/download/v0.12.0-reader.1/PaperNX.nro) | Switch executable, with fonts also embedded; convenient for upgrades. |
| [PaperNX-licenses-v0.12.0-reader.1.zip](https://github.com/rtk19/PaperNx/releases/download/v0.12.0-reader.1/PaperNX-licenses-v0.12.0-reader.1.zip) | Project and third-party license notices. |
| [PaperNX-source-v0.12.0-reader.1.zip](https://github.com/rtk19/PaperNx/releases/download/v0.12.0-reader.1/PaperNX-source-v0.12.0-reader.1.zip) | Corresponding source at the exact tag used to build the NRO, including the vendored RAR decoder. |
| [SHA256SUMS](https://github.com/rtk19/PaperNx/releases/download/v0.12.0-reader.1/SHA256SUMS) and individual `.sha256` files | Download integrity checks. |
| [manifest.json](https://github.com/rtk19/PaperNx/releases/download/v0.12.0-reader.1/manifest.json) | Binary hashes, build receipt and source revision. |
| [verification-evidence.zip](https://github.com/rtk19/PaperNx/releases/download/v0.12.0-reader.1/verification-evidence.zip) | Test/build logs and screenshots using original test artwork. |
| [README.md](https://github.com/rtk19/PaperNx/releases/download/v0.12.0-reader.1/README.md) | Short installation and source reference to keep with the downloads. |

GitHub's automatic **Source code** archives also contain source. To run the reader on Switch, use the NRO or installation ZIP.

The `manifest.json` asset records the exact source revision used to build the NRO. Later documentation or release-publishing updates on `main` do not change that binary. The `builds/<version>` branches retain archived build copies; Release Assets are the download location for users.

## Installation and upgrades

A Nintendo Switch capable of launching NRO homebrew is required.

1. Download the installation ZIP from **this fork's Release Assets**.
2. Extract it to the microSD root. The executable must end up at `sdmc:/switch/PaperNX/PaperNX.nro`.
3. Put `.epub`, `.pdf`, `.cbz` or `.cbr` files in `sdmc:/switch/PaperNX/books/`. Subfolders and uppercase extensions work. The built-in file browser can open books elsewhere on the SD card.
4. Launch PaperNX from the Homebrew Menu.

```text
sdmc:/switch/PaperNX/
├── PaperNX.nro
├── books/                  # Your books and comics
├── fonts/                  # Bundled fonts and OFL notice
└── data/                   # Created by the app: progress, settings, covers, journal
```

**Before upgrading or switching builds, back up `books/` and `data/`.** Replace the existing `PaperNX.nro`, or extract the new installation ZIP. Keep your existing books and data; the release package includes no user databases or comic files. Individual SD write or power failures can still cause data loss.

**This fork uses the same installation and data location as upstream PaperNX.** Installing it there replaces the existing executable and continues using the existing library records. Keep book paths/names stable: book identity includes its path and file size, so moving or replacing a book can affect its saved progress association.

For a quick NRO integrity check, download `PaperNX.nro.sha256` next to the NRO and run:

```bash
sha256sum --check PaperNX.nro.sha256
```

On PowerShell, use `Get-FileHash .\PaperNX.nro -Algorithm SHA256` and compare with that file.
## Reading and fork features

- **Preview and page browser:** drag the bottom progress bar to preview a destination; release to jump or B/Esc to cancel. The grid button opens Chapters, All pages and Bookmarks with lazy thumbnails. Comic pages show full artwork at any zoom; Hebrew chapter labels wrap and align correctly, including links inside a continuous chapter. LS click/G opens controller preview; arrows or either stick choose, A confirms, B cancels. See [page navigation](docs/PAGE_NAVIGATION.md).
- **Hebrew and mixed text:** bundled Noto Hebrew sans/semibold/serif fallback fonts, Unicode bidirectional layout for titles/authors and other UI text, niqqud-aware shaping and truncation. EPUB paragraphs infer their direction independently; justified final lines and indentation follow that direction. Explicit HTML `dir` remains authoritative. EPUB spine direction, or Hebrew `he`/`iw` metadata when it is absent, sets page-turn direction. See [Hebrew support](docs/HEBREW.md).
- **Series shelves and natural numbering:** All books combines a series into one card. Open it for its issues in numeric order (9 before 10), then use Back/B to return. Background metadata indexing covers the whole library; ComicInfo Series/Number takes priority over conservative filename inference. Search and reading categories continue to open individual books.
- **Comfortable comic zoom:** curls remain available while enlarged; a new page keeps the zoom and starts at the top. Both LS and RS pan enlarged artwork with the same proportional response and navigate the same focused items elsewhere. Arrows, D-pad and both sticks follow the display rotation automatically in either portrait direction, including while held. Hold ZR to zoom in or ZL to zoom out smoothly; release to stop. D-pad/arrows pan continuously without an initial jump. Curl geometry follows the full paper sheet, clipped to the reading area. Horizontal edge drags and L/R turn pages.
- **Interactive paper curl:** drag horizontally, hold the sheet, or reverse the drag to return it. Initial grip height and vertical movement affect the fold; release distance and speed determine completion or cancellation. Paginated EPUB, fit-to-page PDF and comics at any zoom use the animation. Enlarged PDFs retain their existing panning behavior.
- **Comic cover and zoom fixes (comics.2):** larger bounded cover decoding, automatic comic cover-cache refresh, a visible **Fit page** reset, cached GPU panning/pinching, momentum and native viewport detail after motion stops.
- **CBZ/CBR comics:** ZIP, RAR4 and RAR5, including solid archives; natural numeric page order, nested folders, Unicode names, ComicInfo metadata, front covers and contents bookmarks. Images are decoded without extracting the archive onto the SD card.
- **Comic and manga controls:** fit-page, fit-width, actual size, zoom/pan, original artwork colors or an optional reader palette, and left-to-right/right-to-left navigation. ComicInfo can set the initial manga direction. Continuous scrolling fits pages to the viewport width and supports momentum/sensitivity settings.
- **Resume and navigation:** bookmarks and progress include the position inside a continuously scrolled comic page, including across reopen, margin changes and rotation. Contents, page navigation and navigation history remain available.
- **Existing reader/library:** EPUB typography and continuous scrolling, PDF zoom/pan, coordinated paper themes, search, favorites, collections, metadata editing, cover cache, reading status, journal and offline statistics.

Tap the center to show reader tools, or use **+** for the quick menu. Reading settings expose **Artwork**, **Borders**, **Appearance**, **Navigation** and **Page view** as appropriate for the open format. A comic's text is part of its image; use zoom to enlarge lettering and panels. Its original colors are preserved by default, independently of the surrounding theme. While zoomed, **Fit page** stays visible even when the toolbar is hidden; it resets zoom/pan and restores page curls. The same action is available in the + menu and on the right-stick click.

Touch and controllers are supported. D-pad navigation follows the comic's spatial reading direction; optional L/R shortcuts remain logical previous/next. See [comic controls and supported formats](docs/COMICS.md) and [general touch controls](docs/TOUCH_CONTROLS.md).

The app interface is **English or Turkish**; choose the language under **Settings → Application → Language**. Hebrew UI localization is not included. Book contents and metadata keep their original language.

## Important limits and troubleshooting

- This is a prerelease; passing desktop/build checks does not replace testing on an actual Switch.
- Password-protected and split/multipart comics are unsupported. Use a complete, unencrypted CBZ/CBR file.
- Large or very high-resolution scans can exceed the document/image memory budget. Solid RAR seeking can be slower because preceding data may need to be decompressed. [The comic guide](docs/COMICS.md) lists exact size/count limits and supported image types.
- Font size, paragraph spacing and text reflow apply to EPUB. Comic lettering stays embedded in the artwork; zoom, borders, colors and reading-mode controls apply instead.
- Original comic colors are the default. The optional reader palette changes the whole image, including illustrations, and is reversible.
- A damaged individual image reports a page error; other pages may still be readable. A damaged archive/index can prevent the book from opening.
- Keep a backup before upgrading, deleting books or switching to older builds. Compatibility of future or older releases with newer saved data is not guaranteed.

## Building this fork

Docker/Compose and Git are required. The Docker recipes install the desktop or devkitPro Switch toolchain and archive/image dependencies.

```bash
git clone https://github.com/rtk19/PaperNx.git
cd PaperNx

# Build the Switch NRO and SD distribution.
docker compose build switch
docker compose run --rm switch bash scripts/verify-switch.sh

# Build and run desktop regression tests.
docker compose build desktop
docker compose run --rm desktop bash scripts/verify-desktop.sh
# Production MuPDF with automatic paragraph direction and RTL layout fixes.
docker compose run --rm desktop bash scripts/verify-pinned-desktop.sh
```

Switch outputs are written to `dist/v0.12.0-reader.1/`. The Switch build uses pinned MuPDF 1.23.11 source and records the source revision, implementation digest and binary hashes. SDK package updates mean future builds are not guaranteed byte-identical. See [the build guide](docs/BUILD_SWITCH.md) for pinned desktop/sanitizer workflows and [licenses](docs/LICENSES.md) for dependency notices and source obligations.

The manually dispatched **Publish verified release assets** workflow attaches existing, verified `builds/<tag>` artifacts to the corresponding release. It checks the original hashes and source tag, includes a source archive, and publishes an updated checksum list. It does not rebuild or retag the NRO.

## Reporting a fork problem

Use this repository's enabled GitHub support features, or contact [@rtk19](https://github.com/rtk19). Include the fork version, book/archive format, reading mode, orientation, reproduction steps and relevant `logs/papernx.log` output. Remove private book titles, paths and personal reading details before sharing logs. A small original/public-domain sample is preferable to sharing a copyrighted book. Report a fork-specific problem here before assuming it also affects upstream.

## Credits and license

Original PaperNX: **Yusuf Buzrul / [@ysfbz](https://github.com/ysfbz)**, developed with AI assistance. Original repository: [ysfbz/PaperNx](https://github.com/ysfbz/PaperNx).

Fork developer and maintainer: **Refael Bar / [@rtk19](https://github.com/rtk19)**. The fork retains the **[AGPL-3.0 license](LICENSE)** and original attribution. Dependencies include MuPDF, SDL2/SDL2_ttf, libnx, FreeType, HarfBuzz, archive/image codecs and Noto OFL fonts. The RAR-only unarr source and local changes are included under LGPLv3 in [third_party/unarr](third_party/unarr/SOURCE.md). Download the license asset and consult the tagged source/build instructions when redistributing.

## הנחיות קצרות בעברית

זהו פורק עצמאי של **Refael Bar (rtk19)**, המבוסס על הפרויקט המקורי של **Yusuf Buzrul**. ההורדות המתאימות לפורק נמצאות ב־[Release שלו](https://github.com/rtk19/PaperNx/releases/tag/v0.12.0-reader.1), תחת **Assets**.

להתקנה ראשונה הורידו את `PaperNX-switch-v0.12.0-reader.1.zip` וחלצו לשורש כרטיס ה־SD. לשדרוג גַבו תחילה את `books/` ו־`data/`, ואז החליפו את `sdmc:/switch/PaperNX/PaperNX.nro`. הפורק והגרסה המקורית משתמשים באותו נתיב ובאותם נתוני קריאה.

תמיכה בעברית פועלת אוטומטית: גופני עברית כלולים גם ב־NRO עצמו, שמות ספרים ומחברים מוצגים בעברית, וכיוון כל פסקה ב־EPUB מזוהה לפי הטקסט. אין צורך לשנות את שפת הממשק או למחוק נתונים. אם הגדרתם בעבר שימוש בעיצוב המו״ל, כבו **Publisher styles** כדי להשתמש בעיצוב הקריאה המתוקן. האפשרות **Alignment: Start (auto direction)** מיישרת עברית לימין ואנגלית לשמאל; **Justified** מיישרת לשני הצדדים והשורה האחרונה נשארת בצד המתאים.

אפשר להוסיף EPUB, PDF, CBZ ו־CBR לתיקיית `books/` או לפתוח דרך דפדפן הקבצים. קומיקס תומך באנימציות במגע, זום, גלילה רציפה ומנגה; צבעי האיורים נשמרים כברירת מחדל. ארכיונים מוצפנים או מפוצלים אינם נתמכים. במצב זום, הכפתור **Fit page** מאפס את ההגדלה והמיקום ומחזיר את אנימציית הדפדוף. עדכון זה מתקן גם כריכות של קומיקס גדול ומשפר את הגרירה בתוך התמונה. זו גרסת ניסיון שטרם אומתה על Switch פיזי. הממשק עצמו באנגלית או בטורקית.

בקומיקס מוגדל, החצים והסטיק השמאלי מזיזים את התמונה; לחיצה ממושכת ממשיכה להזיז.
RS מתנהג כמו LS בניווט ובהזזה. החצים ושני הסטיקים מתאימים את הכיוון אוטומטית גם לאחר סיבוב המסך לשני הכיוונים. החזקת ZR מגדילה והחזקת ZL מקטינה באופן רציף; שחרור עוצר. החצים מתחילים לזוז ללא קפיצה. הקיפול חל על הדף עצמו. לדפדוף עם אנימציה השתמשו ב־L/R, בהקשה
בקצה העמוד או בגרירה אופקית מהקצה. הזום נשמר והעמוד הבא מתחיל מלמעלה.
סדרות מקובצות אוטומטית במסך **All books** לאחר סריקת המטא־דאטה ברקע;
פתיחת כרטיס הסדרה מציגה את החוברות בסדר מספרי. **Back** או B מחזירים לספרייה.

בסרגל התחתון אפשר לגרור כדי לראות תצוגה מקדימה, ולשחרר כדי לעבור למיקום הנבחר. כפתור הריבועים פותח עמודים ממוזערים, פרקים וסימניות. כותרות פרקים בעברית מוצגות בכיוון הנכון. לחיצה על LS פותחת בחירת מיקום עם השלט; החצים ושני הסטיקים בוחרים, A מאשר ו־B מבטל.
