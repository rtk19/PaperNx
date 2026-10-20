# Page previews, contents and quick navigation

Tap the middle of a page to show the reader tools. The bottom bar contains previous/next controls, the page/chapter count, the percentage slider and a grid button. It works with EPUB, PDF, CBZ and CBR.

## Drag to preview

Touch the slider and drag horizontally. A floating thumbnail and destination number follow the selection; the open page, saved progress and history stay at the original position. Release to jump once. Comic zoom stays at its current level and the new page starts at the top/reading side. The previous position remains available through the existing jump-back/Page Flip tools.

B/Esc cancels a preview. A second finger, focus loss, backgrounding, resizing, rotation or reader relayout cancels the touch transaction. A cancelled gesture cannot jump when its fingers are released. In continuous mode the slider also selects the position within a page/chapter. Comic previews show the complete page; continuous EPUB previews show the selected viewport region.

## Browse small pages and chapters

Tap the footer grid button or choose **+ → Page thumbnails**. The existing **Contents** control opens the same browser, defaulting to Chapters for text/PDF books and All pages for comics.

- **Chapters:** thumbnail rows, wrapped chapter labels, nesting and location numbers. The current chapter is marked. Hebrew/niqqud and mixed-language titles keep their logical text, shape correctly and align to the reading side. Continuous EPUB fragment links jump to the heading's actual location inside a chapter.
- **All pages:** full-page thumbnails with numeric labels and a current-page outline. Bookmark dots identify saved pages. Comic/manga and Hebrew RTL books place the first page on the right.
- **Bookmarks:** thumbnail cards for existing saved positions, retaining their titles and continuous offsets. Use the existing reader bookmark controls to add/remove positions; the original bookmark-management screen remains available.

Drag vertically to browse. Tap a card to open that position. Close/B returns without changing the reading position. Empty chapter/bookmark lists give a clear message; All pages remains available when a book has no chapter metadata. Unreadable images show an unavailable preview while other pages remain navigable.

## Controller and keyboard

LS click (or G) starts a preview directly. Left/right changes one page; up/down moves ten positions. Both sticks use the same navigation and repeat behavior. A/Enter confirms; B/Esc cancels. The slider can also be activated through reader toolbar focus.

In the browser, arrows/D-pad or either stick move focus, A/Enter opens a position, and L/R (Page Up/Page Down) switch tabs. Touch, mouse and directional controls follow the current display orientation. Slider direction and page-card order follow the book's reading direction. ZR/ZL zoom and existing next/previous reading shortcuts remain available outside the browser/preview.

## Performance and saved data

Thumbnails are lazy and use a separate MuPDF context on a worker thread. SDL uploads stay on the UI thread. Visible rows are virtualized; preview textures are capped at 16 MiB, decoder memory at 96 MiB, and queued/completed jobs at 24 each. Rapid dragging or scrolling drops queued work for old destinations. Closing the browser stops pending work; changing books/layout invalidates old results. No whole-book bitmap atlas or additional preview files are written to the SD card.

Chapter outlines use the existing metadata capacity (256 entries); the page grid covers the document's complete page count. These are new navigation controls over the existing library, progress, bookmark and history records; no data reset or migration is required. Physical Switch appearance, touch latency and FPS require device testing.
