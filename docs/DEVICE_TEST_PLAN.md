# Physical Nintendo Switch acceptance — v0.3.0-rc1

All results below are **UNVERIFIED** until a new device session is recorded. Desktop simulation, component tests and successful AArch64 linking do not change physical results. The six earlier videos are acceptance input, not acceptance of this new build.

Back up `switch/PaperNX/data/` and preserve `books/`. Install only the new package application assets, merging directories. Record NRO SHA256, package version, Switch LCD/OLED model, HOS/hbmenu version, applet/application mode and test-book names. Start with both controllers detached. Keep a text-heavy EPUB, an image-heavy EPUB and a multi-hundred-page PDF available if possible. Never remove SD or cut power while the app is writing.

| # | Short procedure | Acceptance | Result |
|---|---|---|---|
| 1 | Launch with no controller connected | Opens and accepts touch; branding/icon correct | UNVERIFIED |
| 2 | Tap library top-right plus | Readable contextual sheet; every action reachable by touch | UNVERIFIED |
| 3 | Open global settings before opening any book; repeat with empty library | No document required; Turkish sample preview visible; no crash | UNVERIFIED |
| 4 | Select Sepia and 26px globally; close settings | Immediate feedback; selection persists | UNVERIFIED |
| 5 | Open previously-read EPUB A, then previously-read B | Both inherit new global settings; original positions retained | UNVERIFIED |
| 6 | Customize B only, change theme/font, reopen A; remove B override | A/global unchanged by B; B inherits after override removal; restart repeats correctly | UNVERIFIED |
| 7 | Drag a long library in grid and list views | Finger tracking, bounded inertia, readable categories and no accidental book open | UNVERIFIED |
| 8 | Cold launch library with many uncached covers | Intentional covers immediately; stable card size; no raw loading blocks | UNVERIFIED |
| 9 | Allow covers to load, exit and relaunch | Warm covers reuse disk cache; no long sequential regeneration; book changes invalidate artwork | UNVERIFIED |
| 10 | Open EPUB; turn 20 pages forward/back with taps and horizontal swipes | Each intended gesture turns once; record latency/frame gaps | UNVERIFIED |
| 11 | Open Aa using plus while reading | Large panel appears promptly, controls readable without guessing center tap | UNVERIFIED |
| 12 | Rapidly change font, spacing, themes and tabs; close/reopen | UI remains responsive; latest settings win; position preserved; no stale result replaces new state | UNVERIFIED |
| 13 | Open PDF; select fit-width; zoom and drag | Readable width, entire page reachable, pan does not turn page; filters reversible | UNVERIFIED |
| 14 | Search library with Turkish characters | Switch software keyboard appears; correct search/cancel and no double input | UNVERIFIED |
| 15 | Browse SD, enter folder, return parent, open book, return library | All operations touch-only; no stuck gesture or required controller | UNVERIFIED |
| 16 | Open TOC; jump chapter; add/open/delete bookmark | Correct location, touch-only routes; previous-location return works | UNVERIFIED |
| 17 | Read, bookmark, exit by touch, restart and reopen EPUB/PDF | Logical location and bookmarks restored; test two already-read books | UNVERIFIED |
| 18 | Change preferences/turn pages repeatedly and return library | No recurring save error; logs show successful save or useful diagnostic | UNVERIFIED |
| 19 | Library plus → exit; relaunch | Clean touch-only return to hbmenu and successful relaunch | UNVERIFIED |
| 20 | Inspect Paper/Dark/Sepia on LCD/OLED and handheld/docked | Labels, contrast, covers, focus/pressed states and margins readable | UNVERIFIED |
| 21 | Sleep/resume during EPUB, PDF and open settings; disconnect/reconnect controller | No crash, stale texture, lost position, double activation or disabled touch | UNVERIFIED |
| 22 | Read large/image-heavy EPUB and large PDF; repeatedly switch books/settings | Record steady/peak memory; no unbounded growth, allocation crash or UI freeze | UNVERIFIED |
| 23 | Settings → Navigation → Continuous Scroll Mode; close and drag vertically | Genuine finger-tracked content scroll; no menu/page turn on drag; horizontal swipe does not paginate | UNVERIFIED |
| 24 | Scroll across chapters; change sensitivity and momentum; switch paginated/continuous; restart | Coherent chapter transition and accurate logical position; preference persists globally and with explicit book override | UNVERIFIED |
| 25 | Open plus/TOC/bookmarks/settings and tap/drag over reader underneath | Frontmost overlay owns input; no behind-dialog page turns or stuck gesture on dismissal | UNVERIFIED |
| 26 | TOC/bookmark/progress jump then plus → previous location | Original logical location restored, including after a reflow | UNVERIFIED |

Record PASS/FAIL/UNVERIFIED per row, installed NRO hash and book identity. For latency record tap-to-first-visible-feedback and full-render latency separately; use median/p95/worst frame gaps where possible. Record continuous-scroll cold/warm behavior separately from paginated turns. Warm-cover comparison must use a real restart.

Safe recovery check: with the app closed and the original data backed up, copy the current progress file aside, leave the known-good backup, then relaunch to verify recovery. Never delete the only saved copy. Corruption/write failures are primarily covered by deterministic desktop fault injection rather than deliberate power-loss experiments on a personal SD card.

The user's earlier report of missing continuous scrolling is not closed by desktop tests. Film the settings route and actual vertical drag on the new installed build. Stable publication and merge acceptance remain pending explicit hardware results; do not relabel this candidate as hardware verified without evidence.
