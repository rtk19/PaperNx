# PaperNX Test Evidence & Verification Report

---

## 1. Portable C++17 Core & Gesture Test Suite (`papernx_core_tests`)
**Result**: **PASS** (Compiled with Clang 18 / GCC 13+ with `-Wall -Wextra`, verified with static binary).

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
7. **Corrupted State Recovery**:
   - Immediate detection of checksum mismatch upon file corruption.
   - Automatic fallback to `.bak` backup without crashing or losing user state.
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

## 3. Physical Hardware Status
- **Physical Nintendo Switch Device**: **DEVICE UNVERIFIED** (no physical console connected to developer workstation).
- All cross-compilation scripts, toolchains, CI workflows, and binaries conform strictly to official devkitPro and libnx specifications.
