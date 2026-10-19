# PaperNX Touch and Controller Controls

Controls overview for touch gestures and Nintendo Switch controllers.

| Action | Touch | Controller |
|---|---|---|
| Quick actions | Top-right + | Plus |
| Open book | Tap library card/file | A on focused item |
| Book actions | Hold card; drag cancels | X on focused book |
| Paginated EPUB / PDF page | Left/right zones or intentional horizontal flick | D-pad Left/Right, including zoomed PDF |
| Continuous EPUB | Vertical drag | Up/Down scroll; Left/Right chapter/page-equivalent navigation |
| PDF pan | Slow drag while zoomed | Left analog; Up/Down vertical pan |
| PDF zoom | Toolbar minus/plus or pinch | ZL/ZR |
| Reader tools | Center tap | A |
| Settings | + → Reading settings | Reader X; focused action A |
| Numeric setting | Independent minus/plus | A edit; Left/Right adjust; A confirm; B cancel/restore |
| Palette/swatches | Tap preset or swatch, preview/confirm | Move focus, A select/confirm, B cancel |
| TOC/bookmarks | + → named action; select row | Focus/A; B back |
| Return to library | + → Return to library | B after frontmost modal closes |
| Portrait | Settings → Application → Display Orientation | Focus setting and activate |
| Exit | + → Exit | Root back/exit route |

Keep a distinction between selecting a setting and editing its value. Focus navigation alone must not mutate it. A frontmost modal owns input and cannot turn the document underneath. A PDF quick flick changes page; a slow drag pans. Pinch and UI interactions cannot become page flicks. Zoom preservation is configurable.

Hide/Remove from Library keeps the source file and persists hidden state. Restore through management. Permanent deletion has separate confirmation and validation; cancelling preserves the file. Test confirmed deletion only with a disposable copy. Metadata edits and collection changes stay local and do not rewrite the book.

Landscape is default. Clockwise/counterclockwise portrait rotates the output in software and inversely maps touch. No accelerometer detection is claimed. Both portrait modes still require physical acceptance for touch alignment and comfort.
