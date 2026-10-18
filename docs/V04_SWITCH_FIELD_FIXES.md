# PaperNX v0.4 — Switch field-fix handoff (2026-10-18)

## Evidence and baseline

- Baseline: `feat/v04-professional-reader-ux` @ `fa655ee5bf8146e5d31df932be1471ea4adfe57a`.
- Remediation: `fix/v04-switch-touch-theme-performance`; PR #4 targets the v0.4 feature branch and does not alter `main`.
- Operator evidence: eleven videos captured on a physical Switch (not committed to GitHub); previous v0.4 automated tests do **not** establish acceptance for this later source.
- The current release candidate `dist/v0.4.0-rc1` and associated build receipt predate this branch. Do not rename/repackage the old NRO as the remediated application.

## Observed issues and source-level remediations

| Issue | Existing source concern | Change | Remaining verification |
|---|---|---|---|
| Reading status appears nonfunctional | Actions used invariant labels, no selected state; catalog save failure hidden beneath modal | Selected status label/check and action row; SD write errors visible in modal; status/category assertions | Persist/restart on FAT SD and touch menu |
| Library toolbar remains white | `palette()` returned application `uiTheme` outside a reader regardless of Follow Reading Theme | Library + general settings derive chrome from persistent `PAPERNX_DEFAULTS` under chrome policy Follow Reading; explicit Application/Custom remains honored | All light/dark/sepia settings combinations on device |
| EPUB paper interior remains white | CSS conflict with publisher styles; asynchronous pagination means old raster may remain temporarily | Enforce explicit paper color preference in MuPDF reader CSS even with publisher styles; retain embedded book images | Complex EPUB CSS, image preservation, reflow completion latency |
| Catalog/modal/buttons feel unfinished | Button boundaries too subtle, number values use regular font | Consistent outlines and SemiBold stepper values, selected reading state and modal error notice | Screenshot QA, Turkish labels and long-text clipping |
| Choice list drags fail | Parent modal guard cancelled long finger motion before choice scroller received it | Allow choice-owned drags; add app-level short-viewport event regression | All orientation/touch ID variants |
| D-pad consumed by persistent reader toolbar | Focus routing intercepted Left/Right rather than turning pages | Explicit toolbar focus mode: Up enters, Down/B exits; Left/Right pages otherwise | Controller / touch interoperability |
| Accidental PDF flick | Release movement could contradict total gesture direction | Reject opposite-direction release | Actual gesture tuning on physical touchscreen |
| Slow opening and intermittent jank | Every open synchronously saved catalog metadata and wrote bookmark anchor before first reader frame; thumbnail upload batch unbounded | Save only changed metadata, defer SD save until after first frame, skip expensive initial bookmark recreation, cap cover texture uploads to two per frame; avoid anchor traversals on every appearance step | Benchmark *real* cold/warm open p50/p95 and FPS on Switch; CPU profiler before more changes |

## Known limits of these remote source edits

- These edits were made through GitHub; the local Windows Docker images `papernx-desktop` and `papernx-switch` have **not** been invoked by this agent.
- New handheld hardware captures validate the *old* v0.4 RC UX issues, **not** these source changes.
- Disabling conflicting publisher CSS for explicit paper colors may change publisher-defined formatting. Preserve illustration content; test real fixed-layout/complex EPUBs before treating this as fully solved.
- Book opening still performs MuPDF document open, page count, TOC and rendering on the interactive path. Defer further optimization until per-stage timings prove the bottleneck; do not claim a universal speedup from the metadata/thumbnail changes alone.
- Dictionary, OCR, reflow, full PDF crop, annotations, and CBZ remain intentionally deferred by the v0.4 product scope.

## Required local completion gates (Gemini 3.8 Flash High)

1. Check out this branch in `C:\Users\Yusuf\AndroidStudioProjects\PaperNx`, preserving local changes.
2. Perform clean Docker portable/native/sanitizers/release runs and *actual* 1280×720, 640×480, 720×1280 output snapshots.
3. Verify the status buttons persist through restart; invalid SD permissions show an error; deleting a disposable book still requires confirmation.
4. Validate EPUB background/publisher typography with realistic EPUBs and all nine palettes; do not recolor embedded illustrations accidentally.
5. Profile book-opening stages: MuPDF open, initial page_count, TOC, restore anchor, texture render/upload, foreground/background cover work, storage writes. Use separate cold/warm measurements, no assertions based only on SDL dummy.
6. Verify 35 physical hardware acceptance cases on the new candidate after rebuilding.
7. Build AArch64 ELF/NRO in `papernx-switch` Docker and inspect source identity receipt, metadata, icon, RomFS and ZIP SHA256.
8. Preserve previous distributions, all books, progress, catalogs and stats. No forced pushes, stable release or merge until explicitly approved.

## Smoke tests

- Mark Reading -> selected and Continue Reading filter; mark Unread -> Unread filter; mark Finished -> Completed filter, restart and repeat.
- Choose Warm Cream and OLED Black while reading: EPUB text *and canvas*, library top bar, modal surfaces and focus rings must match.
- Scroll the appearance options on a constrained viewport without controller; change color swatch by touch.
- With Reader Toolbar Always Visible, Left/Right still turn EPUB/PDF pages; Up focuses toolbar, A activates, B returns to reading focus without closing the book.
- Open the same large book twice; record first-page visible time and menu input responsiveness while loading.
