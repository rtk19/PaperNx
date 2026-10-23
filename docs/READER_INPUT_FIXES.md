# PaperNX 0.17.1 — menu rollback and paper interaction

The library, file browser, settings, action/help sheets, catalogue actions, contents/bookmark menus, journal and loading presentation return to their 0.16.0 implementation. This is a forward patch release so installed 0.17.0 clients can discover it normally; published tags are not moved. The 0.17 compact toolbar remains on reading pages. Async folder import, saved scan roots, startup/manual scanning, themes/accents, expanded-area and landscape-spread switches, language-aware shoulders, Hebrew styling, zoom and page-number conventions remain available. No storage migration or reset is introduced.

## Touch and geometry

- Book drags may start in the top or bottom strip. Only actual visible control rectangles reserve input; invisible whole-width toolbar bands no longer reject interactive gestures.
- Comic gestures track outside the paper. Segment/rectangle intersection establishes contact with the actual sheet; movement wholly in a margin is consumed without a release-only page flip. A held curl never advances with elapsed time.
- Facing comic pages bend individual artwork rectangles. Naturally wide scans and pair/single transitions use their actual sheet bounds. Empty fullscreen background is not part of a turning texture. Different front/reverse dimensions interpolate onto the receiving leaf. Folios have separately owned small snapshots animated at the same progress.
- Enlarged spreads retain paired pages and support animated controller turns. Interior gestures continue panning; outer grips, margins and exhausted horizontal pan boundaries permit page turns. A second finger cancels a held turn before pinch zoom takes ownership.
- Both halves of the spine use their artwork's vertical bounds at rest and under zoom. The spine does not continue into the empty top/bottom space.

## Responsiveness

Idle preparation warms both images in each neighbouring comic group, one bounded texture per idle slice, using the exact cache keys consumed by the reader and animation. It no longer renders an unused PDF-viewport cache entry. Fit-size snapshots are capped at their required displayed resolution, while zoom snapshots remain bounded by source resolution. The event loop presents a newly started button turn before draining more queued events, and progress/catalogue disk flushes wait until the animation finishes. Cold archive/image decoding can still take time; desktop timings do not establish Switch performance.

## Verification

The reader-input suite sends actual SDL events for English/Hebrew books in portrait and landscape, at five vertical positions, and checks live/held/cancelled/committed state and visible-control ownership. Comic checks cover margin contact, gestures entirely outside artwork, zoomed spread turns, zero extra rasterizations after neighbour warming, paper geometry, and bounded spine pixels. Original generated `paper-bounds.cbz` art leaves visible gaps above and below the spread specifically to expose an overlong spine. Existing spread, comic, reader-caption, navigation, settings, library, updater and packaging suites remain required. Offscreen OpenGL screenshots use original generated art only. Real Switch interaction and cold-SD timing need device acceptance.
