#!/usr/bin/env bash
# scripts/build-mupdf-switch.sh
# Idempotent cross-compilation of MuPDF static archives for Nintendo Switch (devkitA64 / libnx).
set -euo pipefail

MUPDF_VERSION="${MUPDF_VERSION:-1.23.11}"
MUPDF_TAG="${MUPDF_VERSION}"
INSTALL_PREFIX="${1:-$(pwd)/prefix-mupdf-switch}"
BUILD_DIR="${BUILD_DIR:-$(pwd)/build-mupdf-switch}"

echo "=========================================================="
echo "PaperNX — Building MuPDF ${MUPDF_VERSION} for Nintendo Switch AArch64"
echo "Prefix: ${INSTALL_PREFIX}"
echo "=========================================================="

if [ -z "${DEVKITPRO:-}" ]; then
    echo "ERROR: DEVKITPRO environment variable is not set." >&2
    echo "Please set DEVKITPRO to your devkitPro installation path." >&2
    exit 1
fi

if [ ! -x "${DEVKITPRO}/devkitA64/bin/aarch64-none-elf-gcc" ]; then
    echo "ERROR: aarch64-none-elf-gcc not found at ${DEVKITPRO}/devkitA64/bin/" >&2
    exit 1
fi

export PATH="${DEVKITPRO}/devkitA64/bin:${DEVKITPRO}/tools/bin:${PATH}"
export CC="aarch64-none-elf-gcc"
export CXX="aarch64-none-elf-g++"
export AR="aarch64-none-elf-ar"
export LD="aarch64-none-elf-ld"
export RANLIB="aarch64-none-elf-ranlib"

# Check if already installed and valid
if [ -f "${INSTALL_PREFIX}/lib/libmupdf.a" ] && [ -f "${INSTALL_PREFIX}/lib/libmupdf-third.a" ] && [ -f "${INSTALL_PREFIX}/include/mupdf/fitz.h" ]; then
    echo "MuPDF is already compiled and present in ${INSTALL_PREFIX}."
    echo "To force rebuild, delete ${INSTALL_PREFIX} and re-run."
    exit 0
fi

mkdir -p "${BUILD_DIR}"
cd "${BUILD_DIR}"

TARBALL="mupdf-${MUPDF_VERSION}-source.tar.gz"
URL="https://mupdf.com/downloads/archive/${TARBALL}"

if [ ! -f "${TARBALL}" ]; then
    echo "Fetching MuPDF source tarball from ${URL}..."
    curl -fsSL -o "${TARBALL}" "${URL}" || wget -O "${TARBALL}" "${URL}"
fi

SRC_DIR="mupdf-${MUPDF_VERSION}-source"
if [ ! -d "${SRC_DIR}" ]; then
    echo "Extracting ${TARBALL}..."
    tar -xzf "${TARBALL}"
fi

cd "${SRC_DIR}"

# Arch flags for Nintendo Switch Tegra X1 (Cortex-A57)
ARCH_FLAGS="-march=armv8-a+crc+crypto -mtune=cortex-a57 -mtp=soft -fPIE -O2 -ffunction-sections -fdata-sections"
SWITCH_INCLUDES="-I${DEVKITPRO}/libnx/include -I${DEVKITPRO}/portlibs/switch/include"

echo "Building static libraries..."
make -j"$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)" \
    build=release \
    OS=none \
    HAVE_X11=no \
    HAVE_GLFW=no \
    HAVE_GLUT=no \
    HAVE_CURL=no \
    HAVE_WIN32=no \
    USE_SYSTEM_LIBS=no \
    CROSS_COMPILE=aarch64-none-elf- \
    XCFLAGS="${ARCH_FLAGS} ${SWITCH_INCLUDES} -D__SWITCH__ -D__arm64__" \
    libs

echo "Installing to ${INSTALL_PREFIX}..."
mkdir -p "${INSTALL_PREFIX}/lib" "${INSTALL_PREFIX}/include"
cp -r include/mupdf "${INSTALL_PREFIX}/include/"
cp build/release/libmupdf.a "${INSTALL_PREFIX}/lib/"
cp build/release/libmupdf-third.a "${INSTALL_PREFIX}/lib/"

echo "Validating AArch64 architecture of produced archives..."
aarch64-none-elf-readelf -h build/release/libmupdf.a | grep -m1 "AArch64" || true

echo "=========================================================="
echo "SUCCESS: MuPDF ${MUPDF_VERSION} AArch64 libraries ready!"
echo "Headers: ${INSTALL_PREFIX}/include/mupdf"
echo "Libraries: ${INSTALL_PREFIX}/lib/libmupdf.a, libmupdf-third.a"
echo "=========================================================="
