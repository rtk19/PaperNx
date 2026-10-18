# PaperNX — Native Reading Journal handoff (work in progress)

Branch: `feat/reading-journal-native-v1`
Base: `fix/v04-book-scoped-appearance-and-pdf-chrome` at `4b9582ac03b36daa17b82d177d6c36a30fd2cb90`.
This is not merged into main or PR #5. **No local Docker/real Switch tests were run by the GitHub connector author.** The branch must not be released before compilation, sanitizer regression and physical Switch acceptance.

## Implemented source changes (unverified)

- `reading_journal.hpp/.cpp`: separate microSD `data/reading_journal/sessions.v1` snapshot, version marker `PAPERNX-JOURNAL-1`, per-line FNV-1a 64-bit checksum, `.tmp/.bak` backup, recovery in read-only mode after malformed data, monotonic active duration, idle 2/5/10 minutes, per-book and per-calendar aggregates, goal preference.
- The old `progress.txt`, `settings.txt`, and `reading-stats.txt` are not rewritten or migrated. Session counters start from this feature's first use, and are not retroactively fabricated.
- `reading_journal_ui.hpp/.cpp`: native SDL2 four-tab views (overview, calendar, books, history), a simple goal editor, per-book session detail, and a reader preview. Uses the existing UI palette and typography; no browser engine.
- `main.cpp` and `catalog_ui.inc`: library one-tap entry, quick-action links, per-book history, reader mini-panel, reader-context back handling, sleep/background/menu exclusion from the new timer, goal and session persistence. Old ReadingStatsStore retained for compatibility.
- `tests/reading_journal_tests.cpp`: synthetic time, idle and midnight boundaries, safe restart, schema mismatch. `CMakeLists.txt` includes portable test and native screen compilation; `Makefile` picks up new .cpp files from src automatically.

## Gaps / acceptance blockers

1. **Build not verified**. Run desktop portable, sanitized, native SDL/MuPDF and Switch toolchain compilation. Correct all linker/compile/runtime findings before calling this complete. Do not trigger GitHub Actions unless explicitly approved; use local Antigravity + Docker.
2. New journal currently keys books using the existing path/size-derived `book.id`; rename/move does NOT guarantee identity preservation. Implement a bounded content fingerprint and cautious alias map before claiming stable ID behavior. Never silently join similarly named books.
3. Sessions are held in memory and snapshotted (default about every 60 seconds and at close/suspend); refine interruption/backup replay and blocking IO. Power loss can lose the latest unflushed seconds. Validate on real FAT32/exFAT cards.
4. The session file format is a snapshot with per-record checksum, **not** the fully append-only event log proposed in the plan. Version any wire changes and provide safe migration.
5. CSV/JSON export/import, data correction/deletion confirmation, all archived book statuses, completed-book/re-read distinctions, reliable pace/ETA, clock/DST hardening, adjustable “still reading” affordance, full goal ring, detailed session-per-day drilldown and accessibility audits are not complete. Never present synthetic metrics.
6. Audit responsive layout at 1280×720, 1920×1080, 640×360 and portrait rotations. Adjust long Turkish labels, scrolling, touch hit targets and every gamepad focus path.
7. The current C++ widgets are an implementation starting point, not proof of equivalence to the accompanying interactive HTML design.

## Local verification order

```powershell
cd C:\Users\Yusuf\AndroidStudioProjects\PaperNx
git fetch origin
git switch feat/reading-journal-native-v1
git status --short
docker compose config
docker compose build desktop
docker compose run --rm desktop
```
Check `compose.yaml` for exact service names before executing (above desktop commands are indicative, not verified). Use repo verification scripts `scripts/verify-sanitizers.sh`, `scripts/verify-v04.sh`, `scripts/verify-switch.sh` after inspecting their arguments. Do not use old v0.4.0-rc2 build artifacts. Avoid Docker prune.

Physical Switch: book open/close, EPUB/PDF, dark/sepia/global theme restore, 10min suspend, midnight, touch/portrait, archived book, unplugged Joy-Cons, corrupt last checkpoint, SD error, power loss, frame timings. Status remains **UNVERIFIED** until measured on device.
