# PaperNX — Native Reading Journal Handoff & Verification Report

- Branch: `feat/reading-journal-native-v1`
- Base: `fix/v04-book-scoped-appearance-and-pdf-chrome` at `4b9582ac03b36daa17b82d177d6c36a30fd2cb90`
- Target PR: [#6](https://github.com/ysfbz/PaperNx/pull/6)
- Test Version Identifier: `0.4.0-journal` (Release archive: `dist/v0.4.0-journal/PaperNX-switch-v0.4.0-journal.zip`)

---

## 1. Implemented Source Changes

### A. Reading Journal Engine (`include/papernx/reading_journal.hpp`, `src/reading_journal.cpp`)
- **Monotonic Duration Tracking:** Uses `std::chrono::steady_clock` to compute elapsed seconds. System clock jumps or daylight saving changes cannot produce negative reading times.
- **Active State Filtering:** Active duration is measured only while reading EPUB or PDF documents. Menus, modals, book loading, application background, idle time, and sleep modes are excluded from counted reading time.
- **Configurable Idle Timeout:** Supports 2, 5 (default), and 10-minute idle thresholds. A "keep reading" affordance (`stillReading()`) lets long-page readers extend the current active window without synthetic page turn requirements.
- **Midnight Splitting:** Sessions that span across midnight (e.g. 23:58–00:08) are automatically split and attributed to their respective calendar days.
- **Bounded Content Fingerprinting & Alias Map:**
  - `computeBookFingerprint()` samples file size and up to 8KB of header/tail content, providing robust identity detection across file moves/renames without blocking on multi-gigabyte PDFs.
  - `registerBookAlias()` maps alternative file paths to a canonical book ID and persists mappings to `data/reading_journal/aliases.v1` with per-record FNV-1a checksums.
  - Never automatically merges books with ambiguous or colliding identities.
- **Data Integrity & Storage Safety:**
  - Snapshots stored in `data/reading_journal/sessions.v1` with file header `PAPERNX-JOURNAL-1` and per-line 64-bit FNV-1a checksums.
  - Atomic writes with `.tmp` staging and `.bak` fallback recovery on corrupted or truncated files.
  - Legacy files (`progress.txt`, `settings.txt`, `reading-stats.txt`) are preserved intact with zero retroactive fabrication.
- **Session Deletion & Dynamic Aggregates:**
  - `deleteSession()` removes records with dynamic recalculation of all calendar, book, and period aggregates.
- **Exporting:**
  - Full offline export to JSON (`exportJson()`) and CSV (`exportCsv()`) under `data/reading_journal/export/`.

### B. Native SDL2 User Interface (`include/papernx/reading_journal_ui.hpp`, `src/reading_journal_ui.cpp`)
Matches the design language of `PaperNx_Okuma_Gunlugu_Prototip(1).html` natively using SDL2 rendering and PaperNX typography:
- **Tab 0 — Genel Bakış (Overview):**
  - Segmented period selector: Hafta (7 days), Ay (4-5 weeks), Yıl (12 months).
  - Active time display (hours & minutes).
  - Honest trend indicators (no fabricated percentages when prior period data is absent).
  - Vertical bar charts with current day/period highlighted in accent color.
  - Daily goal ring with progress dots.
  - 4 summary tiles: Okunan gün, Ortalama oturum, Bitirilen kitap, Okuma serisi.
  - Currently reading card with progress bar and "Detay →" shortcut.
  - Recent activity feed with "Tümünü gör ↗" navigation.
- **Tab 1 — Takvim (Calendar):**
  - Monday-first monthly calendar grid with ‹/› navigation.
  - 5-tier heatmap intensity (0 / 1–29 / 30–59 / 60–89 / 90+ minutes).
  - Day selection focus ring.
  - Daily detail panel displaying chosen day's total active time and session breakdown.
- **Tab 2 — Kitaplarım (My Books):**
  - Two-column responsive card grid with styled covers, authors, progress meters, active time, and session count.
  - Per-book detail drilldown displaying book-specific timeline and session history.
  - Archived status for deleted or moved files.
- **Tab 3 — Geçmiş (History / Journal):**
  - Chronological session feed grouped by date with symbols (◷ / ✓), book titles, start/end times, and progress deltas.
  - Destructive session deletion with confirmation modal.
- **Modals & Overlays:**
  - Goals modal (+/- 5 min adjustments, presets, toggle).
  - Data Export modal (JSON and CSV export triggers).
  - Session delete confirmation dialog.
  - Reader '+' menu mini-statistics panel with non-destructive return to reader.
- **Controls & Responsiveness:**
  - Controller mapping: L/R (tab cycle), D-pad (navigation), A (select), B (back), + (goals), X (export), Y (keep reading).
  - Full touch navigation with 56px minimum hit targets.
  - Responsive layouts: Desktop/Handheld (1280×720, 1920×1080), compact/retro (640×360), and portrait orientations.
  - Theme isolation: Reading Journal strictly adheres to global application theme and restores reader-specific themes upon return.

### C. Main Integration (`src/main.cpp`, `src/catalog_ui.inc`)
- Catalog entry point with quick access to Reading Journal.
- Reader '+' menu action to inspect book statistics without losing page, zoom, pan, or font settings.
- Global theme enforcement for Reading Journal screen state.

---

## 2. Docker Verification Results

All tests executed locally in Docker on Windows 11 without triggering GitHub Actions.

### A. Desktop Portable Tests (`verify-desktop.sh` / `build-core`)
- Total Tests: 7 / 7 **PASSED** (100%)
  1. `papernx_catalog_tests`: PASSED
  2. `papernx_foundations_tests`: PASSED
  3. `papernx_localization_tests`: PASSED
  4. `papernx_cover_tests`: PASSED
  5. `papernx_core_tests`: PASSED
  6. `papernx_reading_journal_tests`: PASSED
  7. `papernx_utf8_integrity`: PASSED

### B. Desktop Native Tests (`verify-desktop.sh` / `build-desktop`)
- Total Tests: 11 / 11 **PASSED** (100%)
  - Includes `mupdf_render_tests`, `v03_stress_fixtures`, and `papernx_app_tests`.

### C. Sanitizers (`verify-sanitizers.sh` / AddressSanitizer + UndefinedBehaviorSanitizer)
- Total Tests: 11 / 11 **PASSED** (100%)
- Memory leaks: 0
- Undefined behavior: 0

### D. Desktop Release Build (`build-core-release`)
- Total Tests: 7 / 7 **PASSED** (100%)

### E. Nintendo Switch Toolchain Compilation (`verify-switch.sh`)
- Architecture: ARM64 ELF (Machine 183)
- Assets: NACP (Title: PaperNX, Version: `0.4.0-journal`), RomFS (Embedded Noto fonts), 256×256 JPEG icon.
- Result: **PASS**
- NRO Checksum (SHA256): `4e8891c4d227369741162d22ac3e01fa99a71eb4be14d95993309422854a7d02`
- Distribution ZIP: `dist/v0.4.0-journal/PaperNX-switch-v0.4.0-journal.zip`
- Distribution ZIP SHA256: `932797dac5b9167400a2d8c9f8d43a7868218922f697ffbfbc6df0b9c959dbde`
- Distribution Verification: 59 license files, 86 zip members, package integrity verified.

---

## 3. Physical Nintendo Switch Hardware Verification Matrix

> [!WARNING]
> In accordance with safety rules, hardware status is explicitly **UNVERIFIED** until executed on physical retail/test hardware. Docker and toolchain passes verify binary validity but do not replace device acceptance.

| Test Case | Scenario / Steps | Expected Behavior | Hardware Status |
| :--- | :--- | :--- | :---: |
| **TC-01** | Open Journal from Library | Single-tap "Günlüğüm" row or button opens Tab 0 smoothly | UNVERIFIED |
| **TC-02** | Tab Navigation (Controller) | L/R shoulder buttons cycle tabs; D-Pad navigates tiles | UNVERIFIED |
| **TC-03** | Touch-Only Navigation | Joy-Cons disconnected; all tabs, buttons, calendar cells respond | UNVERIFIED |
| **TC-04** | Reading Time Accuracy | Read EPUB/PDF for 5 minutes; verify exactly ~5 min added | UNVERIFIED |
| **TC-05** | Idle Timeout | Leave device untouched for >5 minutes; verify counter pauses | UNVERIFIED |
| **TC-06** | Still Reading Affordance | Press Y or tap "Okumaya devam ediyorum" before timeout; counter extends | UNVERIFIED |
| **TC-07** | Sleep / Wake Suspension | Suspend console via Power button for 10 minutes; verify 0 min counted | UNVERIFIED |
| **TC-08** | Midnight Crossing | Read across 23:58 to 00:08; verify session splits across both days | UNVERIFIED |
| **TC-09** | Reader '+' Menu Integration | Press '+' in reader -> "Bu kitabın istatistikleri"; press B to return | UNVERIFIED |
| **TC-10** | Reader State Preservation | Returning from stats preserves exact page, zoom, pan, and theme | UNVERIFIED |
| **TC-11** | Theme Isolation | Reading Journal stays in global app theme; reader theme unchanged | UNVERIFIED |
| **TC-12** | Calendar Heatmap | Read on multiple days; verify 5 intensity levels match minutes | UNVERIFIED |
| **TC-13** | Book Renaming / Moving | Rename EPUB/PDF file; verify reading history preserved via alias/hash | UNVERIFIED |
| **TC-14** | Session Deletion | Delete session in Tab 3; confirm deletion; verify stats update | UNVERIFIED |
| **TC-15** | Data Export | Trigger JSON & CSV export; verify files in `sdmc:/data/reading_journal/export/` | UNVERIFIED |
| **TC-16** | Corrupted Checkpoint Recovery | Force corrupt `sessions.v1`; verify `.bak` recovery with 0 crash | UNVERIFIED |
| **TC-17** | Portrait / Rotation | Rotate Switch 90°/270°; verify UI adapts cleanly without clipping | UNVERIFIED |
