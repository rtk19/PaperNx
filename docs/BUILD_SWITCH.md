# Nintendo Switch Build Guide & Toolchain Reference

PaperNX is compiled as a native Nintendo Switch homebrew executable (`.nro`) using devkitPro (`devkitA64` / `libnx`).

---

## 1. Prerequisites

To cross-compile PaperNX for the Nintendo Switch:
1. **devkitPro & devkitA64**:
   - `aarch64-none-elf-gcc` and `aarch64-none-elf-g++` (GCC 13+ / 14+)
   - `libnx` (Nintendo Switch Homebrew Library)
   - `switch-tools` (`elf2nro`, `nacptool`)
2. **Switch Portlibs** (installable via `dkp-pacman`):
   ```bash
   dkp-pacman -S switch-sdl2 switch-sdl2_ttf switch-freetype switch-libjpeg-turbo switch-zlib switch-bzip2 switch-libpng
   ```
3. **Cross-Compiled MuPDF AArch64 Static Libraries**:
   - `libmupdf.a` and `libmupdf-third.a` built for `aarch64-none-elf` (Tegra X1 / Cortex-A57).

---

## 2. Compiling MuPDF for Switch (Automated Script)

Run the included idempotent cross-compilation script:
```bash
chmod +x scripts/build-mupdf-switch.sh
./scripts/build-mupdf-switch.sh "$(pwd)/prefix-mupdf-switch"
```
This builds static libraries targeting `-march=armv8-a+crc+crypto -mtune=cortex-a57` with X11, GLFW, and curl disabled, installing headers to `prefix-mupdf-switch/include` and archives to `prefix-mupdf-switch/lib`.

---

## 3. Building PaperNX.nro

Once the MuPDF prefix is prepared:
```bash
export MUPDF_PREFIX="$(pwd)/prefix-mupdf-switch"
make all -j$(nproc)
```
Outputs generated:
- `PaperNX.elf`: 64-bit ARM ELF executable.
- `PaperNX.nacp`: Title (`PaperNX`), author (`ysfbz`), and version (`0.1.0`) metadata.
- `PaperNX.nro`: Validated Switch homebrew binary with embedded RomFS containing default fonts.

---

## 4. Packaging for SD Card

```bash
python3 scripts/package_release.py PaperNX.nro --destination dist --version "v0.1.0"
```
Produces:
- `dist/PaperNX-switch-v0.1.0.zip`
- `dist/PaperNX-switch-v0.1.0.zip.sha256`
- `dist/manifest.json`

---

## 5. Automated CI via GitHub Actions

The repository includes a guarded GitHub Actions workflow (`.github/workflows/manual-switch-build.yml`):
- Run method: Manual trigger (`workflow_dispatch`).
- Confirmation gate: Requires typing `BUILD` to authorize compute minutes.
- Uses `devkitpro/devkita64:latest` container to compile and package the release.
