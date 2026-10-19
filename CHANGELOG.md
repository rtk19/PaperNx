# Changelog

All notable changes to **PaperNX** are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

---

## [0.9.0] - 2026-10-19

### Added
- **Production Open-Source Release:** First full production release of PaperNX for the Nintendo Switch.
- **English Default Localization:** Clean installations now launch in English (`language = 1`) by default.
- **Comprehensive UI String Translations:** Added over 60 missing bilingual translation pairs in `localization.hpp` covering all settings tabs, orientation toggles, reading stats, and dialogs.
- **Separate Compliance Package:** Automated packaging of `PaperNX-licenses-v0.9.0.zip` containing GNU AGPL-3.0 and all third-party SDK/library notices with SHA256 checksums.
- **Minimal SD Distribution:** Clean `PaperNX-switch-v0.9.0.zip` containing only necessary executable and font files; excludes test directories, logs, documentation, and user databases to prevent overwriting existing user data.
- **Automated Distribution Verifier:** `scripts/verify_distribution.py` verifying NACP metadata, NRO signatures, ZIP structure, and clean SD bundle layout.

### Changed
- **Author & Copyright:** Standardized application author to `Yusuf Buzrul` across NACP metadata, source code definitions, Makefiles, and build receipts.
- **Settings Reset Logic:** Resetting settings to defaults in Settings UI (Tab 5) now respects the default language setting (`appSettings_.get().language`) instead of hardcoding language 0.
- **Documentation Architecture:** Merged multilingual documentation into a unified `README.md` featuring an English default layout with a collapsible Turkish section.
- **Community Standards:** Added `CONTRIBUTING.md`, `SECURITY.md`, and comprehensive open-source documentation.

### Fixed
- **Reading Journal Test Boundary Condition:** Fixed day-of-week rollover bug in `tests/reading_journal_tests.cpp` where hardcoded test dates caused calendar week mismatch depending on the day the test suite was executed.
- **Font Pointer Cleanup:** Fixed destructor in `src/typography.cpp` to ensure `sectionFont_` and `buttonFont_` pointers are cleanly nulled during teardown.

---

## [0.4.0-rc3] - 2026-10-18 (Development Pre-release)

### Added
- Continuous vertical scrolling mode for EPUB documents.
- Nine natural paper appearance presets with unified UI coloring.
- Interactive live typography preview in settings modal.
- Bounded cover cache with automatic corruption recovery.
- Offline reading statistics and reading journal.

---

## [0.3.0] - 2026-10-15 (Foundation Release)

### Added
- Native Nintendo Switch EPUB/PDF reader core with SDL2 and MuPDF 1.23.11.
- Touchscreen-first user interface with Joy-Con controller support.
- File browser with SD card navigation.
- PDF rendering with zoom, pan, and document filters.
- Dual-language interface support (English / Turkish).
