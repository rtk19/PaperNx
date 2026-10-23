# Fork verification: 0.16.0

The native and ASan/UBSan/leak suites expand to 26 tests, with 8 portable tests. New coverage exercises batch scans, nested-root deduplication, cancellation, changed/missing files, atomic inventory, persisted folder/theme/scan settings, non-blocking startup, gesture-safe publication, manual and launch scans, removal of scan roots without deleting files, and an active reader surviving library publication. Metadata-only EPUB opening returns title/author without laying out any pages. The spread suite sends actual R/L controller and PageUp/PageDown events after repeatedly opening English and Hebrew books, checks reversed landscape direction and unchanged portrait direction. Modern menu screenshots cover dark/light/follow-paper, folder choices, settings, browser, contents, bookmarks, portrait and Turkish. Offscreen OpenGL exercises modern menus as well as spread/chrome rendering. Existing updater, Hebrew, archive, pagination and packaging regressions remain active.

Switch verification validates the AArch64 ELF, numeric version/developer in NACP, NRO resources, fonts/CA, source receipts, licenses and installation ZIP. Real-device performance, first-pinch latency and touch feel still require a Switch; desktop checks do not establish those timings. Release evidence contains the actual test/build logs and original-fixture screenshots.

# Fork verification: 0.15.0

The 24 native and 24 ASan/UBSan suites now also cover persisted default-on layout switches, real setting-row taps, smart wide/ComicInfo groups in both directions, no cold texture build on the first pinch, retained spread geometry and pinch focal point, touch/Y return-to-top without adding bookmarks, and full-screen drag progress. Legacy single-page comic zoom/pan controls remain exercised with the spread setting off. Existing geometry/fallback, Hebrew, updater, packaging, portable and OpenGL checks remain active. Physical Switch animation timing and touch feel require hardware acceptance.

# Fork verification: 0.14.0

The new spread suite checks actual EPUB/PDF/CBZ/CBR pixels in light/night themes, RTL/LTR leaf order, owned front/reverse folios, exact animation endpoints after cache eviction, mixed PDF aspect ratios, odd/single-page endings, queued and held/cancelled turns, portrait snapshots and rotation cancellation. Geometry and fallback tests check asymmetric reverse-page ink for mirroring and preservation of texture modulation state. Prior single-leaf touch regressions now explicitly use portrait; landscape workflows assert two-page advancement. Hebrew alignment and emphasis tests remain active with the tighter page margins.

Release verification uses 24 native CTests, the same 24 ASan/UBSan tests with leak detection, 7 portable tests, packaging validation and offscreen OpenGL spread/chrome tests. Switch checks validate AArch64, NRO/NACP version/developer, bundled fonts/CA, source/build receipts, licenses and the installation archive. Logs and original-fixture screenshots accompany the release. Physical Switch rendering, touch latency, FPS and update installation remain unverified.

# Fork verification: 0.13.0

Fresh verification: 23/23 native CTests, 23/23 ASan/UBSan CTests with leak detection, 7/7 portable tests and packaging validation. The expanded reader-caption suite also runs with offscreen OpenGL. Switch checks verify AArch64, numeric NACP/application version, embedded fonts/CA store, exact source receipts and installation ZIP integrity.

Actual CBZ/CBR frames now exercise hidden/simple and visible/detailed footer transitions in paginated/continuous modes, landscape/both portrait rotations and light/dark themes. The hidden frame must contain the numeral, the detail caption must replace it, and the document rectangle must remain pixel-identical. Coverage includes middle taps, first/last pages, manga, single-page comics, EPUB/PDF and continuous EPUB totals. Update integration scenarios now offer a normal 0.13.0 release to an old suffixed build; ordering checks cover 0.13.1 patches, 0.14.0 features and downgrade rejection. Physical Switch acceptance remains unverified; older release evidence follows.

# Fork verification: v0.12.0-reader.4

Fresh verification: 23/23 native CTests, 23/23 ASan/UBSan CTests with leak detection, 7/7 portable tests and packaging validation. The book-caption suite also runs with offscreen OpenGL. Switch checks verify AArch64, NRO/NACP developer/version, embedded fonts/CA store, exact source receipts and the installation ZIP.

Book tests now check `current` while the toolbar is hidden and `current of total • x%` while visible, including the persistent-toolbar preference. They compare actual content pixels across middle taps, both portrait rotations, landscape and light/dark paper. Paginated/continuous EPUB, PDF, total screen-page counts, final-page 100% and comic isolation are covered. Captions stay in the existing empty strips. Physical Switch acceptance remains unverified; previous release evidence follows.

# Fork verification: v0.12.0-reader.3

Fresh verification: 23/23 native CTests, 23/23 ASan/UBSan CTests with leak detection, 7/7 portable tests, packaging tests and the updater UI with offscreen OpenGL. The final updater suites were repeated after adding rotated LS/RS modal controls. A live verified-TLS request against rtk19/PaperNx selected the real reader.2 prerelease from a reader.1 baseline.

Updater coverage exercises full prerelease ordering (reader.10 versus reader.9), scoped/invalid JSON and Unicode notes, draft/foreign-asset rejection, and 20 local HTTP/HTTPS scenarios: valid/current/missing releases, HTTP errors, corrupt/truncated/oversized downloads, wrong NRO identity/version, cancellation, unwritable staging, redirects, pending recovery, tampered staged data, failed replacement, missing readiness metadata, trusted/untrusted certificates and HTTPS-to-HTTP downgrade rejection. SDL tests cover silent startup errors, explicit checks in the + menu, automatic prompts, virtualized scrollable notes, landscape/both portrait touch, both sticks, input isolation, download progress and restart requests. The running NRO must remain byte-identical until the deferred apply call.

Switch checks validate AArch64, NRO/NACP, developer/version, bundled CA certificates/fonts, source/build receipts and the installation ZIP. Evidence includes logs and original test screenshots. Physical Switch network, SD replacement and hbloader relaunch remain unverified. See [AUTO_UPDATE.md](AUTO_UPDATE.md) for behavior and reproduction; prior release evidence follows.

# Fork verification: v0.12.0-reader.2

Fresh verification: 21/21 native CTests with patched MuPDF 1.23.11, 21/21 ASan/UBSan CTests with leak detection, 7/7 portable tests, the packaging rejection/extraction suite, and the book-caption suite with offscreen OpenGL. Switch validation checks the AArch64 ELF, NRO/NACP version/developer, RomFS fonts, source receipt, license notices and installation ZIP. Logs and original fixture screenshots accompany the release.

The new book suite compares actual document pixels before/after a middle tap: the content rectangle must remain identical, the top caption changes with toolbar visibility, and the bottom numeral stays identical. It covers all three orientations, light/dark paper, nested chapters, spine fallback without a TOC, continuous screen-page numbering, PDF without an outline and comic control isolation. Hebrew-only render fixtures distinguish regular, bold, italic and combined styles in serif/sans fonts; external/embedded CSS and inline emphasis must match semantic tags. Later regular text must remain regular. Underline, strikethrough and superscript are checked while author font sizes/colors/margins cannot override reader layout.

Physical Switch acceptance remains unverified. Reproduce with `scripts/verify-v04.sh portable`, `scripts/verify-pinned-desktop.sh`, `scripts/verify-v04.sh sanitizers`, `python3 tests/test_package.py`, and `SDL_VIDEODRIVER=offscreen LIBGL_ALWAYS_SOFTWARE=1 build-pinned/papernx_book_chrome_tests`. Prior release evidence follows.

# Fork verification: v0.12.0-reader.1

The suite contains 20 native CTests, 7 portable tests and 6 focused ASan/UBSan tests with leak detection. The new page-navigation application test also runs with offscreen OpenGL. Switch validation checks AArch64, NRO/NACP developer/version, embedded fonts, source receipts and the actual installation ZIP.

New coverage exercises 200 rapid scrub motions without intermediate position saves; release, B/Esc, multi-touch, focus loss and live rotation cancellation; both portrait orientations; mouse and controller routes; page-grid scrolling and focus; current-page selection, RTL/manga order, chapter and bookmark jumps; long mixed Hebrew/niqqud titles and nested continuous EPUB fragment positions; full-page CBR/PDF thumbnails, single-page books, empty chapter/bookmark lists, damaged preview recovery and stale background results after switching books. Original test artwork and logs accompany the release. Desktop behavior does not establish physical Switch FPS or touch latency.

Reproduce using the native/portable workflows in BUILD_SWITCH.md, plus `SDL_VIDEODRIVER=offscreen LIBGL_ALWAYS_SOFTWARE=1 build-pinned/papernx_page_navigation_tests`. Include `papernx_page_navigation_tests` in the focused sanitizer targets and test filter. Prior release evidence follows.

# Fork verification: v0.11.0-reader.4

This developer-name update is checked by the real Switch build, NRO/NACP author and version validation, AArch64/RomFS/font inspection, source receipts, installation ZIP integrity and portable/package checks. The displayed developer must be **Refael Bar**. Reader behavior is inherited from reader.3; its native/OpenGL/sanitizer regression evidence follows below and is not a fresh rerun for this metadata change. Physical Switch acceptance remains unverified.

# Fork verification: v0.11.0-reader.3

The new reader suite passes 19 native CTests with patched MuPDF 1.23.11 and the
pinned FriBidi 1.0.12 source build, plus 7 portable tests and package/UTF-8 checks.
Five focused ASan/UBSan tests with leak detection cover Hebrew UI/layout, catalog
sorting, background series indexing, controller input and comic zoom/curls.
The comic reader also runs through the offscreen OpenGL renderer. Final build
and distribution receipts, original fixture screenshots and logs ship in the
release's verification-evidence.zip. Physical Nintendo Switch behavior, FPS and
touch latency remain unverified.

Rotation coverage checks keyboard arrows, D-pad, LS and RS in four physical
directions at all three display orientations. It compares rendered paper
displacement, release behavior, live held rotation and modal selection, including
a check against applying rotation twice to stick-generated actions. The shared
direction transform is checked against the pointer transform.

Existing controller coverage compares LS and RS from identical navigation states in
both axes and directions, including modal choices/palettes and held repeat.
Trigger tests cover stationary holds, release, opposing triggers, menus, zoom
limits and focus loss; 100 small zoom steps check focal drift. Comic and curl
tests check full sheet coordinates, a separately positioned incoming page,
viewport clipping and continuous first-frame D-pad movement. Software and
offscreen OpenGL paths run the same comic interactions.

Existing coverage includes Hebrew glyph/niqqud fallback in every font role, mixed
numbers/Latin text and isolation controls, unchanged Latin pixel output, RTL
start/justified/indented paragraphs, explicit direction, continuous EPUB layout
and restored anchors; zoomed comic snapshots and top-of-page navigation, held
D-pad/LS/RS movement; full-library metadata indexing, series shelf/back/search
behavior, numeric 9/10/22 and decimal/suffixed issues, and per-issue progress.
Legacy interaction tests open a series shelf before exercising its issue list.

Reproduce with scripts/verify-pinned-desktop.sh, the native/portable/sanitizer
workflows in the build guide, and scripts/verify-switch.sh. The latter validates
AArch64 output, NACP version, the branding icon, all embedded Latin/Hebrew fonts,
source receipts and the actual installation ZIP. No user databases are packaged.

Historical upstream validation follows.

# PaperNX v0.9.0 — Reported validation results

The release-preparation handoff reports:

| Gate | Reported result | Checked-in script |
|---|---|---|
| Portable C++17 | 7/7 PASS | `scripts/verify-v04.sh portable` |
| Pinned native SDL/MuPDF | 11/11 PASS | `scripts/verify-v04.sh native` |
| Optimized native Release | 11/11 PASS | `scripts/verify-v04.sh release` |
| ASan/UBSan with leak detection | 11/11 PASS | `scripts/verify-v04.sh sanitizers` |
| Large document stress | PASS reported | `v03_stress_fixtures` and `papernx_app_tests` (legacy test target names) |
| AArch64 NRO/NACP/RomFS | PASS reported | `scripts/verify-switch.sh` |
| ZIP and license archive verification | PASS reported | `scripts/verify_distribution.py` |
| Physical Nintendo Switch acceptance | **UNVERIFIED** | [35-item checklist](DEVICE_TEST_PLAN.md) |

The public v0.9.0 tag points to `135df6be80a622d12ce59c173b31068da1844e17`. These are **reported prior test outcomes**; documentation maintenance did not rerun Docker, compile an NRO or establish a new GitHub Actions pass for the tagged source. The `verification/` directory's historical logs and screenshots were intentionally removed from Git tracking. Static UTF-8/NFC checks do not independently demonstrate correct on-device glyph rendering.

See [Release Report](RELEASE_REPORT.md), [build and reproduction instructions](BUILD_SWITCH.md) and [known limitations](KNOWN_ISSUES.md).
