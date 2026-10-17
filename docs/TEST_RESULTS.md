# PaperNX Test Evidence & Verification Report

---

## 1. Portable C++17 Core & Gesture Test Suite (`papernx_core_tests`)
**Result**: **PASS** (Verified with `-std=c++17 -fno-exceptions -fno-rtti`).

### Test Scenarios Covered
1. **Document Format Detection**:
   - Case-insensitive EPUB / PDF detection (`book.EPUB`, `doc.pdf`, `document.PDF`).
   - Rejection of unsupported extensions (`notes.txt`, `image.png`).
2. **Turkish Unicode & Case-Folding**:
   - Dotted uppercase `İ` -> lowercase `i`.
   - Dotless uppercase `I` -> lowercase `ı`.
   - Characters `Ğ`, `Ş`, `Ü`, `Ö`, `Ç` correctly folded.
   - Exact case-insensitive matching in Turkish book queries.
3. **UTF-8 Safe Truncation**:
   - Multi-byte continuation byte protection (never cuts inside a UTF-8 codepoint).
   - Codepoint counting accuracy.
4. **Library Scanning & Identity**:
   - Recursive directory discovery with depth limits.
   - Symlink protection and skip permission denied handling.
   - Stable FNV-1a document fingerprint hashing.
5. **Page Position Restoration**:
   - Fractional restoration clamp bounds.
   - Edge case handling (0 pages, 1 page, relayout).
6. **PAPERNX-2 Persistence & Bookmarks**:
   - Serialization and deserialization of page, pageCount, fraction, font size, theme, line height, margins, text alignment, last read timestamp, and multiple bookmarks.
   - Safe parsing via `parseSafeInt`, `parseSafeLL`, and `parseSafeDouble` (compatible with `-fno-exceptions`).
7. **Multi-Stage Atomic Backup Rotation**:
   - Immediate detection of checksum mismatch upon file corruption.
   - Multi-stage backup rotation (`.tmp` -> `.bak.old` -> `.bak` -> `.dat`).
   - Automatic fallback to `.bak`, `.bak.old`, or `.tmp` without crashing or losing user state.
8. **Schema Migration from PAPERNX-1**:
   - Automatic reading and parsing of legacy `PAPERNX-1` records.
   - Upgrading and saving state into `PAPERNX-2` format.
9. **Book Categorization & Sorting**:
   - Filters for `ContinueReading`, `Recent`, `Favorites`, `Unread`, `Completed`.
   - Sorting by Title, Author, Filename, Modified, and LastRead.
10. **1,000-Book Stress Test**:
    - High volume scan, query, and search execution time benchmarking.
11. **Touch Gestures & Controller Deadzones**:
    - Tap left (0-22%), tap right (78-100%), tap center (22-78%).
    - Swipe left / right velocity and distance threshold classification.
    - Controller analog stick deadzone filtering (<12000 filtered, >12000 dispatched).

---

## 2. Release Packaging & SD Layout Verification Suite (`test_package.py`)
**Result**: **PASS** (Python 3.9+).

### Test Scenarios Covered
1. **NRO0 Magic & Header Validation**: Validates entrypoint branch, `NRO0` magic, declared size, and version.
2. **Binary Header Enforcement**: Strictly rejects desktop ELFs (`\x7fELF`) or non-NRO binaries.
3. **Segment Boundary Enforcement**: Rejects segment overflow beyond declared image size.
4. **Segment Overlap Prevention**: Rejects corrupted binaries with overlapping segment ranges.
5. **SD Card Layout Extraction**:
   - Verifies created archive extracts into `switch/PaperNX/PaperNX.nro`.
   - Verifies directories `switch/PaperNX/books/`, `switch/PaperNX/data/`, `switch/PaperNX/fonts/`.
   - Verifies bundled fonts `NotoSans-Regular.ttf`, `NotoSerif-Regular.ttf`, and `OFL.txt`.
6. **Integrity & Manifest**: Verifies SHA256 checksum and JSON manifest generation.

---

## 3. Native MuPDF EPUB & PDF Integration Test Suite (`mupdf_render_tests`)
**Result**: **PASS** (Automated C test runner).

### Test Scenarios Covered
1. **Error Resiliency on Corrupted Inputs**:
   - Corrupted EPUB container rejected gracefully without crash (`bad_epub == NULL`).
   - Corrupted PDF rejected gracefully without crash (`bad_pdf == NULL`).
   - Error messages populated and returned cleanly.
2. **EPUB Document Opening & Metadata**:
   - Opens valid EPUB3 fixture (`sample_book.epub`).
   - `px_document_is_epub` returns `1`.
   - Extracts metadata title ("İstanbul Masalları ve Şiirler") and author ("Ahmet Hamdi Tanpınar").
3. **EPUB Layout Reflow & Rendering**:
   - `px_document_layout_epub` computes page count based on viewport dimensions.
   - Page 0 renders to valid `PxImage` buffer (pixels != NULL, width > 0, height > 0).
   - Changing font size from 24pt to 36pt reflows the document, resulting in a distinct pixel hash (`hash_epub0 != hash_reflow`).
4. **EPUB Table of Contents & Bookmarks**:
   - `px_document_get_toc` extracts sections ("Birinci Bölüm: Boğaziçi ve Sabah Güneşi", "İkinci Bölüm: Sonbahar Ağaçları ve Yağmur").
   - Resolves internal URI destinations (`#chapter1`, `#chapter2`) via `fz_resolve_link`.
   - Creates and looks up layout bookmarks via `px_document_make_bookmark` and `px_document_lookup_bookmark`.
5. **PDF Rendering & Multi-Page Verification**:
   - Opens valid 2-page PDF fixture (`sample_doc.pdf`).
   - `px_document_is_epub` returns `0`.
   - Renders Page 0 and Page 1; asserts pixel hashes differ (`hash_pdf0 != hash_pdf1`).
6. **PDF Zoom & Viewport Pan Cutout**:
   - Renders Page 0 at 1.5x zoom with pan offsets (`panX = 100`, `panY = 50`).
   - Asserts zoomed/panned cutout produces a distinct pixel hash from full page.
7. **TOC Outlines in PDF**:
   - Extracts PDF bookmarks ("Sayfa 1 - Giris" -> page 0, "Sayfa 2 - Gelisme" -> page 1).

---

## 4. Hardware Verification Status
- **Switch Hardware Test**: **HARDWARE UNVERIFIED — ALPHA**.
- Automated checks pass completely in simulated/cross environments.
- Real hardware execution on Nintendo Switch requires physical SD card deployment.
