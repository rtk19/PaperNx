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
