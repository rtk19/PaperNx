# PaperNX v0.9.0 — Known limitations and validation gaps

Not every limitation here is a reproduced defect. Distinguish implemented behavior, postponed features and unverified hardware-dependent paths.

## P1 — Real Nintendo Switch validation

The published v0.9.0 NRO has **35/35 physical-device acceptance checks marked UNVERIFIED** in [DEVICE_TEST_PLAN.md](DEVICE_TEST_PLAN.md). Priority cases: Joy-Con-detached touch, both portrait modes/touch mapping, PDF gestures, restart and sleep/resume, docked/handheld, controller reconnect, SD write errors and Unicode glyph rendering. Passing native desktop tests and producing AArch64 NRO bytes do not prove physical compatibility.

## Known implementation constraints

- **Storage:** Temporary writes, backup recovery and retries lower corruption risk, but power failure, FAT/SD limitations and interrupted writes can still lose unsaved progress. Back up `books/` and `data/` before updating. No absolute zero-data-loss guarantee.
- **Book identity:** Primarily path and file-size based; moving or replacing books can produce a new identity without automatically merging records.
- **EPUB position restoration:** Malformed/image-only chapters can use approximate geometric fallbacks after relayout.
- **PDF:** Whole-page pixel transformations also affect images/illustrations. No built-in PDF crop, true text reflow, OCR or column-aware reading. Flick thresholds are not hardware-tuned.
- **File management:** Deletion uses separate explicit confirmation and path/type checks. No trash or undo; test only on disposable copies.
- **Reading statistics:** Active time and finish estimates use interaction-derived heuristics, not proof the screen was being read. Chapter ETA and some advanced calendar reports are not guaranteed.
- **Catalog metadata:** Before opening a document, normalized separate authors/subjects and automatic moved-file reconciliation remain limited.
- **Physical ergonomics:** Focus visibility, long labels, tablet-size layout, rotations and sleep/restart still require on-device confirmation.

## Planned / not included in v0.9.0

Dictionary, annotations/highlights, EPUB in-book search, CBZ/CBR, PDF reflow/OCR, arbitrary gesture remapping and optional connectivity are not claimed as shipping capabilities. See [roadmap](PRODUCT_ROADMAP_2026.md).

## Evidence

[Reported automated tests](TEST_RESULTS.md) · [Current published digests](RELEASE_REPORT.md) · [Hardware acceptance](DEVICE_TEST_PLAN.md). Removed `V03_*` files are not current references.
