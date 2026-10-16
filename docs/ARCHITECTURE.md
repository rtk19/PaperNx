# PaperNX architecture (initial)

- `papernx_core`: C++17 std::filesystem scan, portable in-memory library, per-book progress persistence. No UI runtime dependency.
- `mupdf_bridge.c`: C-only MuPDF exception boundary. MuPDF's `fz_try/fz_catch` uses setjmp/longjmp, so no nontrivial C++ object lives inside that boundary. Document ownership and pixel buffers are explicitly managed.
- `main.cpp`: SDL2 event/render loop, theme palettes, controller/touch actions and state transitions. It draws actual MuPDF pixels when native dependencies are available.
- `CMakeLists.txt`: builds tested portable core by default; explicit `PAPERNX_NATIVE=ON` requires SDL2, SDL2_ttf and MuPDF.

Important gaps: scanning is currently synchronous, EPUB page restore uses a fractional approximation (not a stable textual anchor), cover previews are placeholders, full focus management and bookmarks are not implemented.

Target SD tree: `switch/PaperNX/PaperNX.nro`, `books/`, `fonts/`, `data/`. No online services are used.
