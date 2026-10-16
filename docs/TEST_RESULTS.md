# Test evidence — 2026-10-17

Environment: CMake, GNU C++17. CTest portable target compiled and **1/1 executable PASS**.

Executable assertions cover: case-insensitive PDF/EPUB extension detection, unsupported file rejection, recursive discovery, UTF-8 file path handling, missing root, search, 1,000-book library scan and scan limit, differing document IDs, page position estimate after relayout, checksum read/write and fallback to backup when the primary file is corrupted.

NOT RUN / UNVERIFIED: exact pushed-source CI run, SDL2 UI, MuPDF EPUB/PDF, font/theme device inputs, devkitPro AArch64 build, actual NRO0 format, physical Switch startup, suspend, real document rendering and memory/performance metrics.
