# PaperNX interactive page curl

Fork version **0.9.0-curl.2**, based on `ysfbz/PaperNx` commit
`fc1612fe7c3fbe9d6cc25d33dccbd3ea1202ae9d` and the supplied page-curl prototype.

## Touch behavior

Drag horizontally anywhere in the reading area to pick up a page. The fold
follows the finger while it is held; it does not advance on a timer. The initial
height of the grip and subsequent vertical movement incline the crease. A drag
near a corner folds that corner; a central grip produces a more upright roll.
Reverse the drag to put the sheet back. Release beyond the distance threshold,
or with a short flick, to finish the turn. A short slow drag returns to the
original page. Reading progress is committed only when a dragged turn completes.

Taps, shoulder buttons and page keys still turn pages automatically. Rapid
button/tap turns are queued. Paginated EPUB and fit-to-page PDF support the
curl; continuous EPUB and zoomed PDF retain scrolling and panning. Toolbars,
scrubbers and modal controls keep their own hit areas. A second finger,
backgrounding, layout/zoom/size changes and book closure cancel a live curl.

## Rendering

The paper is wrapped around an inclined, tapered cylinder and projected from
actual 3D height. The moving sheet is depth-sorted, with a translucent paper
back, subdued reversed ink, directional lighting, a narrow highlight and soft
contact shadows. A single flat polygon keeps untouched text sharp. Texture
sampling is linear on supported SDL versions. The crease angle and radius
vanish at rest, so starting or cancelling a small drag does not pop a large
corner into view.

Owned front, destination and back textures remain independent of the page LRU
cache and normalize mixed PDF page sizes. Renderer clipping, colors, scale and
texture blending are preserved. SDL drivers without geometry fall back to a
scaled reveal; if render targets are unavailable, turns remain immediate.

## Build and verification

```sh
docker compose build desktop switch
docker compose run --rm desktop bash scripts/verify-desktop.sh
docker compose run --rm switch bash scripts/verify-switch.sh
```

The NRO and verified distribution are written to `dist/v0.9.0-curl.2/`.
Release evidence includes the source/binary build receipt, hashes and test logs.
The native tests exercise real SDL geometry and fallback, grip/vertical-travel
pixel changes, both endpoint limits, renderer state, live holds, reversal,
commit/cancel, flick speed, multi-touch, portrait transforms, light/OLED themes,
EPUB/PDF and cache eviction. Preview frames come from the native SDL renderer,
not a Nintendo Switch. Tests and artifact checks do not establish Switch
hardware compatibility or frame rate.

## Installation

Back up `sdmc:/switch/PaperNX/data/`, then replace only
`sdmc:/switch/PaperNX/PaperNX.nro`. Keep existing books and data directories.
Open through Homebrew Menu. The previous NRO can be restored to revert.
AGPL-3.0 and bundled dependency license notices continue to apply.
