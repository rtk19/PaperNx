# Independent native UI code review — v0.3

Reviewed 2026-10-17 against the supplied professional touch reader requirements.
Scope: `src/main.cpp`, `include/papernx/ui.hpp`, `src/typography.cpp`,
`include/papernx/typography.hpp`, `include/papernx/localization.hpp`, and native
workflow regressions. This review is independent of the UI implementation owner.

## Findings returned to the implementation owner

1. **P1: Drawer gestures could turn the underlying reader page.** A drag outside
   the drawer bypassed list capture and reached reader swipe handling. The owner
   added a modal gesture guard; reviewed the guard in `handleGesture`.
2. **P1: Resize could publish stale prepared layout.** Resize performed synchronous
   relayout without invalidating an already-running layout worker. The owner now
   advances the generation and queues a debounced replacement. Old results are
   discarded before the newest request starts.
3. **P1: Reopening settings through the plus menu broke return navigation.**
   `settingsReturn_` could become `AaModal`, trapping a library user in settings or
   leading to an empty reader. The owner preserves the original return screen
   when settings are already open; reviewed the correction in `openSettings`.
4. **P1: Cold long-label truncation repeatedly measured successively shortened
   UTF-8 strings.** The owner replaced that loop with binary search over UTF-8
   codepoint boundaries; reviewed the final implementation in `renderText`.
5. **P2: Favorite symbols were missing in a real desktop screenshot.** The
   professional-library screenshot displayed replacement glyphs. Requested vector
   drawing or a readable label and a new screenshot using the actual bundled font.
   The owner now uses `ui::favorite` vector drawing in both grid and list, and
   reserves author-label space beside it. Final screenshot confirmation pending.
6. **P2: Bookmark maximum scroll differed from its viewport by eight pixels.**
   The owner now uses the actual `h-280` clipped viewport height so the final row
   is whole; reviewed the correction in `maxUiScroll`.

## Architecture checks

- Settings and quick actions have native SDL controls with explicit modal routing;
  all six setting tabs and scrollable content use practical 56-pixel hit targets.
  The 640×480 sidebar's final tab ends at y=432, inside its y=16..464 sheet.
- App appearance has independent persistent storage. The language picker is now
  wired to the application preference and UI-label localization. Metadata/TOC
  labels use untranslated controls; their content must remain byte-identical.
- Layout worker owns a separate limited MuPDF document/context (64 MiB). Its
  captured arguments are values; atomic completion publishes prepared fields.
  UI consumes them only after completion and joins the completed worker. Closing
  books invalidates results without joining a running pagination operation.
- Successful layout replacement samples the current logical anchor immediately
  before swapping documents. The old readable document survives preparation and
  failures. Document generations prevent old-book results replacing a new book.
- Text textures are bounded by 8 MiB and 256 entries with LRU eviction. Cache keys
  distinguish renderer, font, color, width and wrapping. Preview-font replacement
  clears cached textures before destroying the font; application teardown destroys
  typography before SDL renderer destruction.
- Thumbnail decoding/rendering uses the worker's separate document; only
  `processCompletedThumbnails` uploads SDL textures on the renderer thread.
- No production exceptions, WebView/Chromium dependency, or future-only clickable
  menu entries were found in this scope.

## Validation limits

This document is a code-review record, not an overall acceptance PASS. The UI
owner's final native build, regression tests and regenerated screenshots are still
required after these corrections. Existing benchmark JSON explicitly identifies
desktop SDL dummy/software timing; it must not be presented as Switch latency.
Physical Switch controller-free use, touch latency, memory and sleep/resume remain
UNVERIFIED until tested on a device.
