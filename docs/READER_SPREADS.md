# Page layout in 0.14.0

Landscape paginated reading displays two consecutive pages, each with its own
centred folio in the bottom paper margin. EPUB follows its detected reading
direction; Hebrew opens the first page on the right. Comics follow their
existing left-to-right/manga preference. PDF uses left-to-right ordering.
Turning advances a spread (two pages); a missing final facing page stays blank.
Opening a bookmark, chapter or page-grid position still focuses that exact
page and shows the spread containing it.

The leaf curls across the centre seam. Its front, actual reverse page and the
stationary pages underneath have independent textures. Text and page numbers
therefore turn together, including the previous/next page numbers revealed by
the animation. Touch can hold, complete or cancel the curl; buttons and queued
turns use the same spread model. Rotation, backgrounding and reflow invalidate
an unfinished animation without committing a touch preview.

Portrait uses one full-height sheet with its folio inside the animation too.
EPUB no longer combines app margins with MuPDF's extra default @page margins.
Selected outer margins still apply independently to each leaf. The header
retains its 72px clear strip; the paper footer is 44px. In a 1280×720 spread,
default 40px margins give each text column 560×604px. In 720×1280 portrait,
the text area is 640×1164px. Publisher content, font size and chapter layout can
still leave short or intentionally blank pages.

With tools hidden, each page shows only its number. Opening the tools replaces
the folios with the existing navigation controls and progress summary: e.g.
`12 of 398 • 3%` in portrait, or `11–12 of 398 • 3%` in a spread. The percentage
uses the last visible page. Chapter progress remains at the top for books.
These controls occupy the clear chrome areas rather than the text area.

Continuous vertical scrolling remains the explicitly selected single-column
mode, with its existing screen-page counter. Zoomed PDF/comic viewing remains
one page for detailed pan/zoom; returning to Fit page restores landscape
spreads. Original comic/PDF artwork keeps its aspect ratio.

EPUB pagination changes when orientation, font or margins change. The existing
text anchors/bookmarks restore the reading location; page numbers can change.
Stored library, progress and bookmarks require no reset or migration.

Verification: the spread integration suite compares actual pixels in the
front/reverse textures and resting pages, including their folios. Animation
endpoints must be identical to their resting spreads, even after LRU eviction.
Coverage includes both directions, light/night themes, EPUB/PDF/CBZ/CBR, mixed
PDF page dimensions, odd/single-page endings, queued and interactive turns,
portrait and cancellation on rotation. An asymmetric reverse texture catches
mirrored ink in both geometry and fallback renderers. Hardware Switch FPS,
touch feel and visual acceptance still require device testing.
