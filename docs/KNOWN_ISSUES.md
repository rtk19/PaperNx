# PaperNX Known Issues & Technical Limitations

This document tracks known limitations, hardware-specific considerations, and architectural trade-offs in PaperNX.

---

## 1. Physical Hardware Unverified (ALPHA)
- **Status**: The application and all toolchain components have been validated on AArch64 Clang/GCC compilers and in headless CI container environments. However, physical Nintendo Switch execution (via Atmosphère / Homebrew Menu) has not yet been performed on a physical console.
- **Remediation**: Follow [docs/DEVICE_TEST_PLAN.md](DEVICE_TEST_PLAN.md) upon physical hardware availability.

## 2. Large Complex Fixed-Layout EPUBs
- **Status**: PaperNX is optimized for reflowable standard EPUB (novels, essays, articles). Fixed-layout EPUBs (e.g., interactive children's books or heavily styled comics) may fall back to default viewport dimension rasterization.
- **Workaround**: Use PDF format for fixed-layout visual media (comics, manga, scanned textbooks) to enable smooth hardware-accelerated zoom and pan.

## 3. Persistent Reading Locator across Format Changes
- **Status**: EPUB bookmarks generated via `fz_make_bookmark` are layout-dependent within the current rendering instance. When switching font size or margins, the application recalculates the logical location based on chapter index and text percentage fraction.
- **Planned Enhancement**: Implement CFI (Canonical Fragment Identifier) persistence across different document versions.

## 4. Docked / Handheld Resolution Transitions
- **Status**: PaperNX initializes at standard 1280x720 (720p). When docking the console to a 1080p TV, SDL2 scales the 720p presentation surface.
- **Impact**: Text remains sharp due to high-DPI font rendering, but native 1080p framebuffers are not dynamically recreated during runtime docking transitions.
