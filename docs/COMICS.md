# CBZ and CBR comics — v0.10.0-comics.2

Copy `.cbz` or `.cbr` files into `sdmc:/switch/PaperNX/books/`, including
subfolders, or open them directly through the SD browser. Extensions are case
insensitive. Library search, favorites, shelves, metadata editing, reading status,
history, journal, progress, bookmarks, cover caching and guarded deletion work as
they do for existing books. Existing EPUB/PDF records and files remain compatible.

## Reading

- Paginated mode fits the complete artwork on screen. Drag a page horizontally
  to curl it, hold it, or reverse the drag. Grip height and vertical movement
  affect the fold. Release distance and speed decide completion or cancellation.
  Taps, D-pad and optional L/R shortcuts also turn pages.
- **Reading settings → Page view** offers fit-page, fit-width, actual size, zoom,
  zoom preservation, original/reader colors and left-to-right/right-to-left
  reading. Pinch or use the toolbar to zoom; drag enlarged artwork to pan.
  A floating **Fit page** button remains visible while zoomed, including with
  the toolbar hidden. It resets zoom and pan, selects fit-page, and restores
  curls. The + menu and right-stick click expose the same reset.
  As with PDF, a zoomed viewport pans instead of curling. Return to fit-page
  for a complete sheet curl. Logical next/previous buttons stay next/previous;
  spatial touch edges, swipes and D-pad follow the manga direction.
- **Reading mode → Continuous scrolling** places pages edge to edge at viewport
  width, with momentum and sensitivity controls. Drag vertically or use vertical
  navigation; horizontal navigation moves between pages. Zoom/fit settings apply
  to paginated mode. Continuous mode uses width fitting.
- **Artwork** preserves original colors by default, independently of the reading
  theme. Reader palette is an optional reversible whole-image filter; it also
  changes illustrations. Transparent pixels are composited on white page paper.
  **Appearance** changes surrounding chrome/background. **Borders** adjusts the
  reading margins. Comic artwork has fixed lettering and layout; use zoom rather
  than EPUB font, paragraph or justification controls.
- Bookmarks and resume positions include the vertical position inside a page.
  That position survives reopening, margin changes and display rotation.
  Contents uses ComicInfo bookmarks when present, otherwise page filenames.

Reading mode, direction, image filter and view choices persist per book. A
`ComicInfo.xml` Manga value of `YesAndRightToLeft` sets the initial manga direction;
the reader can override it. Two existing reserved fixed-layout preference bits
store comic colors/direction, so no data-file format migration is required.

## Archive and metadata handling

ZIP/CBZ and RAR5/CBR use libarchive. RAR4 uses the vendored LGPLv3 unarr decoder,
including solid archives. Random forward/backward navigation is tested for both
solid formats. Solid archives may need to decompress earlier entries to reach a
page, so non-solid archives offer faster seeking on large comics.

Nested folders and Unicode names work. Numeric filename sorting puts page 2
before page 10, without integer overflow on unusually long numbers. Hidden files,
Mac resource forks, symlinks, absolute paths and parent traversal are excluded.
Nothing is extracted to the SD filesystem. Extension and archive signature are
independent, so ZIP/RAR containers with swapped CBZ/CBR suffixes still open.

Supported images: JPEG, PNG, WebP (including alpha), GIF, BMP, TIFF and JPEG2000
through MuPDF/libwebp. Each image file is one page; animated images are not a
comic animation format. EXIF orientation is applied to JPEG artwork.

Optional ComicInfo fields: Title, Writer (Penciller fallback), Genre, Series,
Number, Manga, FrontCover and page bookmarks. Malformed optional XML is ignored.
Archives without ComicInfo use the filename and first image as fallbacks.
Only supported images count toward ComicInfo page indexes, in natural order.

Password-protected, split/multipart and damaged archives produce an error;
password entry and volume assembly are not implemented. A damaged individual
image displays a page error, while the reader can navigate to other pages.
Limits: 50,000 archive entries, 20,000 image pages, 48 MiB encoded per image,
1 MiB metadata XML and 100 million pixels (64 million for WebP). Oversized pages
may hit the existing bounded document memory budget before those ceilings.
MuPDF and RAR4 allocations use the document allocator; libarchive/libwebp retain
their own codec allocations. Geometry is cached separately from decoded artwork,
and continuous mode renders bounded strips rather than full-height textures.

## Build and validation

Desktop dependencies now include `libarchive-dev` and `libwebp-dev`; Switch
dependencies include `switch-libarchive` and `switch-libwebp`. Docker recipes,
CMake and Makefile include them and the RAR-only unarr sources. Corresponding
vendor source, local modifications and licenses are in `third_party/unarr/`.
The implementation receipt includes that directory. SDK notices are refreshed
even when MuPDF is reused from a build cache.

Run `bash scripts/verify-desktop.sh` in the desktop container and
`bash scripts/verify-switch.sh` in the Switch container. Comic tests include real
compressed RAR4/RAR5 solid and non-solid fixtures, random access, numeric and
Unicode filenames, ComicInfo covers, JPEG/WebP/alpha, tiles, bookmarks, malformed
input, concurrent document contexts and the integrated reader workflows.
All fixture artwork is original; see `tests/fixtures/comics/README.md`.

Automated desktop rendering and AArch64/NRO packaging checks do not establish
physical Nintendo Switch touch latency, frame rate or hardware compatibility.
Those require a device run.


## Cover and pan fixes in comics.2

Comic library covers now receive the same bounded 96 MiB document budget as the
reader, replacing the 16 MiB thumbnail budget that could read metadata but fail
to decode a normal high-resolution PNG scan. Comic cover cache keys are versioned,
so older cached black artwork is regenerated automatically; EPUB/PDF caches and
reading records are preserved. If a ComicInfo cover is damaged, thumbnails try a
bounded number of other pages. No brightness heuristic discards legitimate dark
artwork.

Zoomed comic motion reuses a GPU page texture (up to 3072 pixels per side and
6 Mi pixels, constrained by renderer limits), rather than rasterizing/uploading
a new viewport per touch event. Drag positions accumulate from the starting
point, pinch preserves the image point under the fingers, and panning has optional
momentum using the existing inertia preference. A pan inside a zoomed comic does
not accidentally turn a page. At extreme zoom, a single native-resolution viewport
is refined after movement stops, retaining reading detail without per-motion
rasterization. The document and texture budgets remain bounded.

Regression fixtures include original 3000×4500 PNG scans in CBZ, RAR5 and solid
RAR4. Tests reproduce the old low-budget failure, verify the library worker and
cache migration, exercise the visible reset followed by a page curl, check
fractional pan movement/pinch focus and momentum, and verify that 60 moving frames
reuse the same page rasterization. Desktop timing is not a Switch frame-rate claim.
