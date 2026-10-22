# Hebrew reading and mixed-language text

This fork keeps its English/Turkish interface. Hebrew book titles, author names,
filenames, contents entries, bookmarks and journal text use the shared UI text
renderer with Unicode bidirectional layout and bundled Noto Hebrew fallback
fonts. Latin fonts and their appearance remain unchanged. Hebrew niqqud stays
with its letter when shaping, wrapping or shortening a title. Numbers and Latin
words keep their own order inside Hebrew text. Names are stored in logical UTF-8
order; display ordering never rewrites catalog or journal data.

## Upgrade

Back up `sdmc:/switch/PaperNX/books/` and `data/`, then replace
`sdmc:/switch/PaperNX/PaperNX.nro` with the NRO from
[v0.12.0-reader.2](https://github.com/rtk19/PaperNx/releases/tag/v0.12.0-reader.2).
Alternatively extract the installation ZIP at the SD root, allowing files to be
replaced. Keep existing books and data. All six Latin/Hebrew fonts and their
notices are also embedded in the NRO, so an NRO-only upgrade works even with an
older `fonts/` folder. Restart PaperNX and reopen the book after updating.

No language switch, library reset, cover-cache deletion or data migration is
needed. Confirm version **v0.12.0-reader.2** in the app's information screen; duplicate
old NRO files can launch an older build by mistake.

## EPUB layout

MuPDF detects each paragraph's base direction using its Unicode bidi engine.
Hebrew paragraphs align at the right; English paragraphs in the same book retain
their left-to-right layout. Explicit HTML `dir="rtl"`, `dir="ltr"` and `dir="auto"`
remain authoritative. First-line indentation and the final line of a justified
paragraph follow the resolved direction, including after hard line breaks and
in continuous mode.

In **Aa → Layout**, **Alignment: Start (auto direction)** selects right alignment
for Hebrew and left alignment for English. **Alignment: Justified** fills the
line and places the final line on the appropriate side. Existing alignment
preferences keep their meaning; no preference file changes are required.

If **Publisher styles** is enabled, the book's embedded typography remains in
control. Turn it off to apply reader typography when a publisher forces an
unwanted physical alignment. Explicit direction declarations in the HTML remain
respected in either mode.

The EPUB spine's `page-progression-direction="rtl"` or `"ltr"` determines page
turning, touch zones and curl direction. When absent, Hebrew `dc:language` values
`he`, `he-IL`, `iw` and `iw-IL` select right-to-left turning. Without either kind
of metadata, page turning keeps its existing left-to-right default while text
alignment still detects Hebrew independently. Physical next/previous controls
and animation follow the same book direction.

PDF and comic pages retain their fixed artwork and layout. Their library/UI
names gain Hebrew support; PDF page geometry is unchanged, and comic reading
direction continues to use its existing manga setting.

## Development and verification

UI text uses FriBidi to resolve visual runs, then SDL_ttf/HarfBuzz shapes each run
in logical order with the appropriate font and direction. Hebrew fallback faces
are cached by point size and weight/family. Font provenance and hashes are in
[fonts/HEBREW-SOURCE.md](../fonts/HEBREW-SOURCE.md).

[scripts/patch-mupdf-rtl.py](../scripts/patch-mupdf-rtl.py) applies checked
changes to checksum-pinned MuPDF 1.23.11: automatic default paragraph direction,
directional indentation and endings for justified lines, and logical `start`/`end` alignment. Publisher `left`/`right` declarations retain their physical meaning in both LTR and RTL text. The patch
hash participates in the Switch library cache fingerprint and source receipt.
The original source tarball is never modified. Run
`scripts/verify-pinned-desktop.sh` for the same patched renderer on desktop;
distribution/system MuPDF does not contain these changes.

Tests cover actual bundled Hebrew glyph coverage and niqqud, mixed numbers and
Latin words, isolation controls, clipped/wrapped metadata, cache placement,
unchanged English/Turkish rasterization, actual EPUB pixel alignment in both
reading modes, relayout/bookmark restoration and RTL touch curls followed by
returning to English/PDF books. Physical Switch font appearance and touch
performance still require testing on the device.

## Author emphasis

Reader typography preserves semantic bold/italic, combined styles, class/inline font weight/style, underline, strikethrough and vertical emphasis. The author’s font size, color, spacing and physical alignment do not override the reader settings in this mode. MuPDF caches Hebrew fallback faces separately for regular, bold, italic and combined requests; when a fallback has only a regular face, it synthesizes weight/slant. This keeps following plain paragraphs regular. Text receives only the emphasis present in the EPUB; the reader does not infer formatting from plain text or a screenshot. PDF pages already contain their fixed formatting.
