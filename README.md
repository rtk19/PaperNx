<p align="center">
  <img src="assets/branding/PaperNX-icon-512.png" alt="PaperNX logo" width="144">
</p>

<h1 align="center">PaperNX — rtk19 fork</h1>

<p align="center">Offline EPUB, PDF, CBZ and CBR reading on Nintendo Switch, with interactive paper curls.</p>

<p align="center">
  <a href="https://github.com/rtk19/PaperNx/releases/tag/v0.10.0-comics.1">Download this fork</a> ·
  <a href="docs/COMICS.md">Comic reader guide</a> ·
  <a href="https://github.com/ysfbz/PaperNx">Original project</a> ·
  <a href="LICENSE">AGPL-3.0</a>
</p>

## About this fork

**[rtk19/PaperNx](https://github.com/rtk19/PaperNx)** is an independently maintained fork of **[ysfbz/PaperNx](https://github.com/ysfbz/PaperNx)**, originally developed by **Yusuf Buzrul**. Fork maintenance and the additions described here belong to **[@rtk19](https://github.com/rtk19)**. This repository has its own releases and changes; download this fork's builds and report fork-specific problems here.

The fork adds touch-controlled, perspective page curls and CBZ/CBR comic reading while retaining the original EPUB/PDF reader, offline library, reading journal and settings. The original project's authorship and license are preserved.

**Current release: [v0.10.0-comics.1](https://github.com/rtk19/PaperNx/releases/tag/v0.10.0-comics.1), a prerelease.** Desktop tests, archive sanitizers, OpenGL rendering and the Switch build/package checks passed. Physical Nintendo Switch compatibility, frame rate and touch latency have **not yet been verified**.

## Downloads — use Release Assets

Open the [release page](https://github.com/rtk19/PaperNx/releases/tag/v0.10.0-comics.1) and expand **Assets**. Choose the installation ZIP for a first install, or the NRO for an upgrade.

| Asset | Purpose |
|---|---|
| [PaperNX-switch-v0.10.0-comics.1.zip](https://github.com/rtk19/PaperNx/releases/download/v0.10.0-comics.1/PaperNX-switch-v0.10.0-comics.1.zip) | SD installation layout: application, bundled fonts and an empty books directory. |
| [PaperNX.nro](https://github.com/rtk19/PaperNx/releases/download/v0.10.0-comics.1/PaperNX.nro) | Switch executable, with fonts also embedded; convenient for upgrades. |
| [PaperNX-licenses-v0.10.0-comics.1.zip](https://github.com/rtk19/PaperNx/releases/download/v0.10.0-comics.1/PaperNX-licenses-v0.10.0-comics.1.zip) | Project and third-party license notices. |
| [PaperNX-source-v0.10.0-comics.1.zip](https://github.com/rtk19/PaperNx/releases/download/v0.10.0-comics.1/PaperNX-source-v0.10.0-comics.1.zip) | Corresponding source at the exact tag used to build the NRO, including the vendored RAR decoder. |
| [SHA256SUMS](https://github.com/rtk19/PaperNx/releases/download/v0.10.0-comics.1/SHA256SUMS) and individual `.sha256` files | Download integrity checks. |
| [manifest.json](https://github.com/rtk19/PaperNx/releases/download/v0.10.0-comics.1/manifest.json) | Binary hashes, build receipt and source revision. |
| [verification-evidence.zip](https://github.com/rtk19/PaperNx/releases/download/v0.10.0-comics.1/verification-evidence.zip) | Test/build logs and screenshots using original test artwork. |
| [README.md](https://github.com/rtk19/PaperNx/releases/download/v0.10.0-comics.1/README.md) | Short installation and source reference to keep with the downloads. |

GitHub's automatic **Source code** archives also contain source. To run the reader on Switch, use the NRO or installation ZIP.

The released binary comes from commit [`34473d3`](https://github.com/rtk19/PaperNx/commit/34473d348e941e68a40821e2348a88a5ddfb5b5d). Later documentation or release-publishing updates on `main` do not change that binary. The `builds/<version>` branches retain archived build copies; Release Assets are the download location for users.

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

On PowerShell, use `Get-FileHash .\PaperNX.nro -Algorithm SHA256` and compare with that file. The current NRO SHA-256 is:

```text
2e7c3e7c7d0bad45da4d3bcc3025d54cdbd745003bae7a5b8770a35fb77e2988
```

## Reading and fork features

- **Interactive paper curl:** drag horizontally, hold the sheet, or reverse the drag to return it. Initial grip height and vertical movement affect the fold; release distance and speed determine completion or cancellation. Paginated EPUB and fit-to-page PDF/comics use the animation. Enlarged fixed-layout pages pan instead of curling.
- **CBZ/CBR comics:** ZIP, RAR4 and RAR5, including solid archives; natural numeric page order, nested folders, Unicode names, ComicInfo metadata, front covers and contents bookmarks. Images are decoded without extracting the archive onto the SD card.
- **Comic and manga controls:** fit-page, fit-width, actual size, zoom/pan, original artwork colors or an optional reader palette, and left-to-right/right-to-left navigation. ComicInfo can set the initial manga direction. Continuous scrolling fits pages to the viewport width and supports momentum/sensitivity settings.
- **Resume and navigation:** bookmarks and progress include the position inside a continuously scrolled comic page, including across reopen, margin changes and rotation. Contents, page navigation and navigation history remain available.
- **Existing reader/library:** EPUB typography and continuous scrolling, PDF zoom/pan, coordinated paper themes, search, favorites, collections, metadata editing, cover cache, reading status, journal and offline statistics.

Tap the center to show reader tools, or use **+** for the quick menu. Reading settings expose **Artwork**, **Borders**, **Appearance**, **Navigation** and **Page view** as appropriate for the open format. A comic's text is part of its image; use zoom to enlarge lettering and panels. Its original colors are preserved by default, independently of the surrounding theme.

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
```

Switch outputs are written to `dist/v0.10.0-comics.1/`. The Switch build uses pinned MuPDF 1.23.11 source and records the source revision, implementation digest and binary hashes. SDK package updates mean future builds are not guaranteed byte-identical. See [the build guide](docs/BUILD_SWITCH.md) for pinned desktop/sanitizer workflows and [licenses](docs/LICENSES.md) for dependency notices and source obligations.

The manually dispatched **Publish verified release assets** workflow attaches existing, verified `builds/<tag>` artifacts to the corresponding release. It checks the original hashes and source tag, includes a source archive, and publishes an updated checksum list. It does not rebuild or retag the NRO.

## Reporting a fork problem

Use this repository's enabled GitHub support features, or contact [@rtk19](https://github.com/rtk19). Include the fork version, book/archive format, reading mode, orientation, reproduction steps and relevant `logs/papernx.log` output. Remove private book titles, paths and personal reading details before sharing logs. A small original/public-domain sample is preferable to sharing a copyrighted book. Report a fork-specific problem here before assuming it also affects upstream.

## Credits and license

Original PaperNX: **Yusuf Buzrul / [@ysfbz](https://github.com/ysfbz)**, developed with AI assistance. Original repository: [ysfbz/PaperNx](https://github.com/ysfbz/PaperNx).

Fork maintainer: **[@rtk19](https://github.com/rtk19)**. The fork retains the **[AGPL-3.0 license](LICENSE)** and original attribution. Dependencies include MuPDF, SDL2/SDL2_ttf, libnx, FreeType, HarfBuzz, archive/image codecs and Noto OFL fonts. The RAR-only unarr source and local changes are included under LGPLv3 in [third_party/unarr](third_party/unarr/SOURCE.md). Download the license asset and consult the tagged source/build instructions when redistributing.

## הנחיות קצרות בעברית

זהו פורק עצמאי של **rtk19**, המבוסס על הפרויקט המקורי של **Yusuf Buzrul**. ההורדות המתאימות לפורק נמצאות ב־[Release שלו](https://github.com/rtk19/PaperNx/releases/tag/v0.10.0-comics.1), תחת **Assets**.

להתקנה ראשונה הורידו את `PaperNX-switch-v0.10.0-comics.1.zip` וחלצו לשורש כרטיס ה־SD. לשדרוג גַבו תחילה את `books/` ו־`data/`, ואז החליפו את `sdmc:/switch/PaperNX/PaperNX.nro`. הפורק והגרסה המקורית משתמשים באותו נתיב ובאותם נתוני קריאה.

אפשר להוסיף EPUB, PDF, CBZ ו־CBR לתיקיית `books/` או לפתוח דרך דפדפן הקבצים. קומיקס תומך באנימציות במגע, זום, גלילה רציפה ומנגה; צבעי האיורים נשמרים כברירת מחדל. ארכיונים מוצפנים או מפוצלים אינם נתמכים. זו גרסת ניסיון שטרם אומתה על Switch פיזי. הממשק עצמו באנגלית או בטורקית.
