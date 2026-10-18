# PaperNX — 18 Oct 2026 physical Switch video audit

**Evidence:** 11 uploaded 1280×720 screen recordings, total ~130 seconds. Reviewed with 8 evenly sampled frames per clip (additional intermediate states may not be visible in sampled stills). These are *device-recorded visuals*, but they are **not proof of current feature branch behavior**, because recorded NRO provenance cannot be cryptographically tied to the new code. User screenshots and videos remain the primary acceptance baseline.

**Patch branch:** `fix/v04-unified-appearance-reading-status-ux` based on current PR #6, not main. This is a **source implementation / review**, with **fresh compilation and device retesting still outstanding**.

## Observed UX defects

| Video timestamped filename suffix | Time sampled | Observed behavior and problem | Classification |
|---|---:|---|---|
| `2026101822211300-...` | 00–30s | Library grid, long-press context menu, collections, empty group list. Context rows and book metadata are tightly packed; unsupported selection glyphs are a risk. Collection rows need consistent selected state. | Visual/accessibility |
| `2026101822214000-...` | 00–27s | PDF and separate PDF filter controls, zoom and toolbar. PDF page changes to dark while a distinct PDF filter tab remains; incompatible with requested single appearance source. | P0 behavior |
| `2026101822214700-...` | 00–06s | TOC and new journal overview; journal is much denser than supplied spacious HTML mockup. Header hierarchy and badges compete for space; several icon symbols fail. | P1 UI |
| `2026101822215000-...` | 00–03s | Calendar entered from overview. Calendar cells are tiny at playback scale, right-hand day details show multiple 0-minute entries. | P1 UX |
| `2026101822215900-...` | 00–08s | Calendar, book list and book detail. Long title/author/status lines visually collide; book detail progress and session area need alignment and better relative sizing. | P1 UI |
| `2026101822221000-...` | 00–11s | Book details, history, and delete-confirmation dialog. 0-minute rows can mask valid short sessions; modal copy truncated rather than wrapped. | P1 data+UI |
| `2026101822221400-...` | 00–04s | Goal dialog. Four small adjacent controls and tight headline/body hierarchy; action targets and focus need retesting. | P1 touch |
| `2026101822221800-...` | 00–04s | Goal dialog disabled. Correct selected control can be seen, but small text and modal density remain. | P2 UI |
| `2026101822223600-...` | 00–18s | Goal settings, history and overview. Totals change after history actions; insufficient evidence to claim calculation bug. 0-minute rows visually misleading. | P1 accuracy validation |
| `2026101822224800-...` | 00–12s | Landscape journal and portrait rotation selector. Rotated recording is not an upright screenshot; upright-oriented inspection reveals densely packed charts/metrics and clipped secondary labels. | P1 responsive |
| `2026101822225600-...` | 00–08s | Portrait calendar, book list, history. Narrow columns and missing full-width day detail, dense rows, visual clipping and small navigation. | P1 responsive |

## Confirmed source issues (independent of still-frame interpretation)

1. `main.cpp` chose PDF's `aaTab_=4` on reader settings open while EPUB used the shared Appearance tab. `v04_choices.inc` retained a dedicated filter selector and `mupdf_bridge.c` transformed the full RGB raster independently of selected theme.
2. `catalog_ui.inc` initially stored ReadingStatus without changing `ProgressStore`. Setting unread on a book at 30% left 30% on other progress surfaces; completed similarly lacked a 100% navigation update.
3. `reading_journal_ui.cpp` used emoji and ornamental Unicode pictograms (`📅`, `📖`, `📋`, `⊞`, `◷`, `◎`). Bundled Noto Sans does not guarantee color-emoji coverage; recorded squares are consistent with missing glyphs.
4. Journal touch regions did not align with rendered controls: overview period segments rendered 90px but hit test was 96px; overview detail links had a different vertical origin; calendar month navigation and day grids used different y offsets; book-list hit testing used 88px rows while rendering 92px rows.
5. Journal overview used tightly packed titles and metrics, omitted the goal card entirely under a narrow width, and left portrait day-details inaccessible because the calendar was only split into left/right columns.
6. Journal UI rounds sub-minute sessions to '0 dk', which conceals valid reading evidence.

## Source changes implemented on this branch

- Remove separate PDF filter choice from settings and quick-choice model. Preserve serialized `pdfColorMode` as inert compatibility data only.
- Reuse `ReaderPreferences.theme/backgroundColor/textColor` for both reader canvases and UI chrome. PDF neutral raster paper maps to identical chosen RGB background; neutral ink maps to chosen foreground; colorful illustrations remain chromatic. No change to PDF page geometry, zoom or pan. **Visual outcomes must be reviewed on real documents**.
- Apply manual catalog status to persisted page/fraction: unread 0, completed 100, reading retains real intermediate progress; starting to reread a completed book resets to start rather than inventing 99%. Bookmarks, journal sessions and book-scoped appearance preserved.
- Journal overview, book list and book detail display authoritative catalog/progress percentages rather than stale session progress.
- Draw main journal navigation marks using SDL primitives instead of unsupported emojis, adjust sidebar brand hierarchy and headline spacing.
- Render sub-minute durations in seconds; align calendar, period selector and detail-button hitboxes with actual render geometry.
- Portrait overview stacks goal, 2×2 metrics, currently-reading and recent activity. Portrait calendar places day detail *below* the calendar.
- New regression assertions for PDF/EPUB palette parity, neutral raster mapping, manual status semantics, and deterministic SDL snapshot capture at 1280×720, 1920×1080, 640×360 and both portrait rotations.

## Known unverified gaps and developer TODO

- **Compiler/renderer UNVERIFIED** in this chat. GitHub writes are NOT proof of successful CMake/SDL2/MuPDF/Switch build. Run portable, native, ASan/UBSan, Release and Switch NRO builds locally. Update legacy tests only to reflect changed business behavior, never delete unrelated regression coverage.
- Source bitmap tint cannot distinguish every grayscale illustration from black-and-white text. Audit scanned PDFs, colored images, gradients, transparency, dark pages, sepia, invert-like and pure-paper themes. Consider MuPDF display-list/object-aware recoloring only if proven necessary and safe.
- Ensure every PDF screenshot's *interior page and exterior canvas* use the exact chosen backdrop; compare pixel RGB at multiple coordinates. Check black text contrast and picture fidelity.
- One source of truth for manual status must also work for completed-book count, reading streak/ETA, archive and recatalog migrations. Distinguish manual completion from physical last-page navigation.
- Need rigorous vertical touch-only UI walkthrough, modal text wrapping, scrolling/focus on 640×360, long Unicode metadata, scrollable full history and at least 56px actionable hit regions.
- Pure static source changes do not validate whether 0-minute or sub-minute session data is valid. Check persisted checksum/recovery, local UTC/night boundary and no fake session state.
- Native screenshot harness now captures overview/calendar/books/history and scaled/rotated views. Review the actual BMPs (convert or rotate for human inspection), annotate clipping/overlap findings and **iterate after each build**.
- User EPUB/PDF files, legacy progress/settings, old distributions, and main branch must remain untouched. No auto merge or GitHub Actions without approval.

## Mandatory QA matrix for Gemini 3.8 Flash High

1. Cross theme: Paper/Soft Ivory/Warm Cream/Natural Paper/Vintage Sepia/Linen/Cool Paper/Charcoal Night/OLED.
2. Cross format: EPUB paginated, EPUB continuous, vector PDF and scanned/image PDF.
3. Cross orientation: 1280×720, 1920×1080, 720×1280 CW and CCW, 640×360; normalize screenshot rotation for human inspection.
4. Cross input: touch only (no Joy-Con), D-pad+A/B, long press, gesture with menu open.
5. Cross status: from 30% to Unread=0, Completed=100, Reading from 30%=30, Completed->Reading fresh read, close/reopen/resume, hide/archive; Journal books must match.
6. Cross persistence: SD write failure, power interruption, .bak recovery, non-destructive migration, export, no orphan session generation.
7. For each screenshot annotate all text overflow, overlap, contrast, missing glyph, incomplete focus, tiny hitbox and inaccessible action; fix and recapture until acceptance.

**Hardware status: unverified for this new branch.** Previous videos describe earlier physical behavior and cannot be used to mark this patch PASS.
