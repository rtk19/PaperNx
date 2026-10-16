# PaperNX engineering status — 2026-10-17

This is a partial implementation, not a completed Switch application.

| Component | Implemented in source | Native built | Desktop tested | Switch device |
|---|---|---|---|---|
| Portable scan/search/identity | Initial | Yes | Yes: assertions | No |
| Checksum persistence and recovery | Initial | Yes | Yes: assertions | No |
| SDL2 library, touch & controller | Initial | No | No | No |
| MuPDF EPUB document/page rendering | Initial | No | No | No |
| MuPDF PDF document/page rendering | Initial | No | No | No |
| Font-size relayout, theme cycling | Initial | No | No | No |
| True EPUB location anchors | No; page/fraction estimate | No | No | No |
| Cover extraction/caching and metadata | No | No | No | No |
| Read-only native SD file picker | Initial | No | No | No |
| TOC, bookmarks, text search | No | No | No | No |
| Background jobs, PDF zoom/tiling | No | No | No | No |
| NRO/Switch package | No | No | No | No |

Local environment had a C++ compiler/CMake but lacked devkitPro, libnx, Switch SDL2 and MuPDF. One local CTest executable passed (including an added 1,000-book library scenario). It checks file types, nested scan, Unicode path, search, identity, position estimate, persistence and primary-file corruption recovery. The exact GitHub-pushed source has not been independently executed in this environment.

## Next engineering steps

1. Pin MuPDF source SHA and produce a reproducible Nintendo Switch AArch64 static build, including compatible third-party libraries.
2. Confirm SDL2/TTF linkage under devkitPro and implement actual .nro target.
3. Run native desktop EPUB/PDF fixtures, ASan/UBSan and real Switch hardware tests.
4. Add asynchronous metadata/cover indexing, true EPUB logical positions, TOC, bookmarks and native SD file picker.
5. Implement PDF fit/zoom/tiles, cache budget, settings customization and full Turkish text localization.
6. Produce source-compliant release only after required compile, tests and device smoke validation.
