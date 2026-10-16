# PaperNX Engineering Status

| Component | Implemented in Source | Native Built | Desktop Tested | Switch Device |
|:---|:---:|:---:|:---:|:---:|
| Portable scan, Turkish search & identity | Implemented | Built | Tested | Unverified on hardware |
| Persistence & backup recovery (PAPERNX-2) | Implemented | Built | Tested | Unverified on hardware |
| SDL2 Library UI, touch & controller | Implemented | Built | Tested | Unverified on hardware |
| MuPDF EPUB & PDF document/page rendering | Implemented | Built | Tested | Unverified on hardware |
| Font-size relayout & 6-theme palette | Implemented | Built | Tested | Unverified on hardware |
| True EPUB bookmarks & location anchors | Implemented | Built | Tested | Unverified on hardware |
| Cover extraction, thumbnail & page LRU cache | Implemented | Built | Tested | Unverified on hardware |
| Read-only native SD card file picker | Implemented | Built | Tested | Unverified on hardware |
| Table of Contents (TOC) & bookmarks modal | Implemented | Built | Tested | Unverified on hardware |
| Touch zones (%22/%56/%22) & swipe gestures | Implemented | Built | Tested | Unverified on hardware |
| Joy-Con lifecycle & analog deadzones | Implemented | Built | Tested | Unverified on hardware |
| PDF zoom & viewport pan rasterization | Implemented | Built | Tested | Unverified on hardware |
| Bundled fonts (Noto Sans, Noto Serif, OFL) | Implemented | Built | Tested | Unverified on hardware |
| NRO0 validation & verified SD ZIP packaging | Implemented | Built | Tested | Unverified on hardware |
| Guarded GitHub Actions Switch CI pipeline | Implemented | Built | Tested | Unverified on hardware |

---

## Verification Summary
- **Portable & Core Suites**: Passed (Turkish case-folding, UTF-8 byte boundary truncation, PAPERNX-2 serialization, backwards-compatible migration from PAPERNX-1, corrupted state recovery from `.bak`, 1,000-book scalability, touch gesture tracker, and controller navigator).
- **Packaging Suite**: Passed (Synthetic NRO0 format, rejection of non-NRO0 binaries, segment boundary overflow prevention, segment overlap prevention, SD directory extraction, and SHA256 integrity).
- **Physical Switch Hardware**: Unverified (no local Nintendo Switch console connected in developer environment; builds must be copied to SD card for physical hardware verification).
