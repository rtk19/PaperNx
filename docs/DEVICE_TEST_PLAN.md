# PaperNX Physical Nintendo Switch Device Test Plan

This checklist outlines the testing procedure to be executed on a physical Nintendo Switch console to promote PaperNX from **HARDWARE UNVERIFIED — ALPHA** to **HARDWARE VERIFIED — STABLE**.

---

## 1. Environment & Setup

- **Hardware**: Nintendo Switch (V1, V2, Lite, or OLED).
- **Firmware**: Atmosphere 1.5.0+ running on HOS 16.0.0+.
- **Loader**: Homebrew Menu (via Title Override / NSP Forwarder or Album applet).
  > **Note**: For large EPUB/PDF rasterization, launch via Title Override (holding `R` while launching an installed game) to grant PaperNX full RAM access (up to 3 GB) instead of Applet Mode (approx. 400 MB limit).

---

## 2. Installation Steps

1. Download the verified release archive: `PaperNX-switch-v0.1.0.zip`.
2. Extract the archive directly to the root of your FAT32 or exFAT microSD card.
3. Verify files exist:
   - `sdmc:/switch/PaperNX/PaperNX.nro`
   - `sdmc:/switch/PaperNX/fonts/NotoSans-Regular.ttf`
   - `sdmc:/switch/PaperNX/fonts/NotoSerif-Regular.ttf`
4. Copy test books into `sdmc:/switch/PaperNX/books/`:
   - Sample EPUB book (Turkish text, chapters)
   - Sample PDF document (multi-page, vector text, scanned graphics)
5. Reinsert microSD card into Nintendo Switch and launch Atmosphere.

---

## 3. Step-by-Step Test Checklist

### A. Startup & RomFS
- [ ] Application icon and metadata render cleanly in Homebrew Menu.
- [ ] Application launches without black screen or immediate crash.
- [ ] Log file is created at `sdmc:/switch/PaperNX/logs/papernx.log`.
- [ ] RomFS mounts successfully; Turkish fonts render sharp in UI header.

### B. Library Screen
- [ ] `books/` directory is scanned; books appear in library grid with titles and authors.
- [ ] Switching categories (*Continue Reading*, *Favorites*, *Unread*, *Completed*) works via touch and Joy-Con (`L` / `R`).
- [ ] Turkish search via virtual keyboard filters titles accurately (`İ`, `ı`, `ş`, `ç`, `ö`, `ü`).
- [ ] Selecting a book opens the reader within 1.0 second.

### C. EPUB Reading Experience
- [ ] Full-screen reading surface displays crisp text with eye-friendly theme.
- [ ] Tap right (right 22%) or press `R` / `ZR` / `A` turns to next page.
- [ ] Tap left (left 22%) or press `L` / `ZL` / `Y` turns to previous page.
- [ ] Tap center opens header and footer toolbars without turning page.
- [ ] Opening "Aa" modal allows changing font size, margins, line height, and theme.
- [ ] Theme changes reflect immediately across the entire screen.
- [ ] Opening TOC modal displays chapters; selecting a chapter jumps to the exact section.
- [ ] Bookmarking current page adds entry to Bookmarks modal.

### D. PDF Reading Experience
- [ ] PDF renders in fit-page mode with correct aspect ratio.
- [ ] Zooming via Right Stick Up increases magnification cleanly.
- [ ] Panning via Right Stick or touchscreen drag scrolls around the zoomed page without visual tearing or stale tiles.
- [ ] Turning page resets zoom/pan cleanly to page fit.

### E. Persistence & Battery / Power Loss
- [ ] Read several pages, then press `Plus (+)` -> Exit to Homebrew Menu.
- [ ] Reopen PaperNX; the last opened book and page position are restored identically.
- [ ] Sleep mode: Put console to sleep while reading; wake console up; render surface resumes without texture corruption.

### F. Controller Lifecycle
- [ ] Detach Joy-Cons to play in tabletop mode; controller hotplugging is recognized.
- [ ] Connect Nintendo Switch Pro Controller; navigation functions without restart.
