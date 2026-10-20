# PaperNX page curl fork

Based on `ysfbz/PaperNx` commit `fc1612fe7c3fbe9d6cc25d33dccbd3ea1202ae9d`
and the supplied PaperNX-PageCurl-Patch prototype. Fork version: **0.9.0-curl.1**.

Adjacent page turns use a 460 ms textured cylindrical fold, with easing and
shading, in paginated EPUB and fit-to-page PDF. Shoulder buttons, page keys,
touch zones and the existing PDF page gestures all use the same transition.
Continuous EPUB scrolling and zoomed PDF retain their existing behavior.
Rapid turns are queued; reversing direction is supported. Settings, layout,
zoom, resize, backgrounding and book closure cancel the transition. Turning
beyond the first or last page does nothing.

The prototype's final-frame jump is corrected by moving the cylinder's contact
line as its radius shrinks. Both pages are captured into owned viewport textures
before the animation starts, so cache eviction, cold page rendering and different
PDF page dimensions cannot invalidate the transition. SDL drivers without
geometry use a clipped reveal; if render-target snapshots are unavailable,
page turns remain immediate. The flat sheet uses a normal texture copy to avoid software-rasterizer mesh cracks. Renderer clipping, color and blend state are restored.

## Build and test

```sh
docker compose build desktop switch
docker compose run --rm desktop bash scripts/verify-desktop.sh
docker compose run --rm switch bash scripts/verify-switch.sh
```

The Switch build produces `PaperNX.nro` and the verified distribution under
`dist/v0.9.0-curl.1/`. The source and build receipt identify the implementation
used for the binary. Existing AGPL-3.0 and bundled dependency notices apply.

The native CTest suite includes actual SDL software-renderer pixel checks for
forward/backward geometry and fallback, endpoint continuity, clipping and saved
renderer state. Application tests cover EPUB/PDF, queue/reversal, cache eviction,
cancellation and document boundaries. These checks do **not** establish Switch
hardware compatibility or frame rate; test the build on your console.

Validation for this build: all 7 portable CTest checks, all 14 native CTest
checks and the Python packaging regressions passed. Switch compilation,
AArch64 ELF/NRO structure, embedded branding/fonts, source receipt and
distribution checks are verified separately in the release evidence.

## Installation

Back up `sdmc:/switch/PaperNX/data/` and replace only
`sdmc:/switch/PaperNX/PaperNX.nro`. Keep your existing books and data directories.
Open PaperNX through Homebrew Menu. To revert, restore the upstream NRO.
