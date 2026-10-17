#!/usr/bin/env bash
# scripts/build-mupdf-switch.sh
# Production cross-compilation of MuPDF static archives for Nintendo Switch (devkitA64 / libnx).
set -euo pipefail

MUPDF_VERSION="1.23.11"
MUPDF_PINNED_SHA256="478f2a167feae2a291c8b8bc5205f2ce2f09f09b574a6eb0525bfad95a3cfe66"
COMPAT_HEADER="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)/include/papernx/switch_compat.h"
INSTALL_PREFIX="${1:-$(pwd)/prefix-mupdf-switch}"
BUILD_DIR="${BUILD_DIR:-$(pwd)/build-mupdf-switch}"

echo "=========================================================="
echo "PaperNX - Building MuPDF ${MUPDF_VERSION} for Nintendo Switch AArch64"
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

# Arch flags for Nintendo Switch Tegra X1 (Cortex-A57)
ARCH_FLAGS="-march=armv8-a+crc+crypto -mtune=cortex-a57 -mtp=soft -fPIE -O2 -ffunction-sections -fdata-sections"
SWITCH_INCLUDES="-I${DEVKITPRO}/libnx/include"

FONT_DEPS_SHA="$(sha256sum "${DEVKITPRO}/portlibs/switch/lib/libharfbuzz.a" "${DEVKITPRO}/portlibs/switch/lib/libfreetype.a" | sha256sum | cut -d' ' -f1)"
COMPAT_SHA="$(sha256sum "${COMPAT_HEADER}" | cut -d' ' -f1)"
GCC_VER="$("${CC}" --version | head -n1)"
FINGERPRINT="MUPDF=${MUPDF_VERSION}:${MUPDF_PINNED_SHA256}|CC=${GCC_VER}|FLAGS=${ARCH_FLAGS}|COMPAT=${COMPAT_SHA}|BUNDLED_HEADERS_ONLY=1|SYSTEM_HB_FT=1:${FONT_DEPS_SHA}"

mkdir -p "${INSTALL_PREFIX}"
INSTALL_PREFIX="$(cd "${INSTALL_PREFIX}" && pwd)"
FINGERPRINT_FILE="${INSTALL_PREFIX}/.build_fingerprint"
verify_arch() {
    local machines
    machines="$(aarch64-none-elf-readelf -h "$1" | sed -n 's/.*Machine: *//p' | sort -u)"
    if [ "$machines" != "AArch64" ]; then
        echo "ERROR: archive contains non-AArch64 or unreadable members: $1: $machines" >&2
        exit 1
    fi
    echo "All archive members are AArch64: $1"
}

# Check if already installed, valid, and fingerprint matches
if [ -f "${INSTALL_PREFIX}/lib/libmupdf.a" ] && \
   [ -f "${INSTALL_PREFIX}/lib/libmupdf-third.a" ] && \
   [ -f "${INSTALL_PREFIX}/include/mupdf/fitz.h" ] && \
   [ -f "${INSTALL_PREFIX}/licenses/MuPDF-1.23.11/COPYING" ] && \
   [ -f "${INSTALL_PREFIX}/archive-sha256" ] && \
   [ -f "${FINGERPRINT_FILE}" ] && \
   [ "$(cat "${FINGERPRINT_FILE}")" = "${FINGERPRINT}" ]; then
    echo "MuPDF is already compiled and cache fingerprint verified in ${INSTALL_PREFIX}."
    echo "Validating AArch64 architecture..."
    verify_arch "${INSTALL_PREFIX}/lib/libmupdf.a"
    verify_arch "${INSTALL_PREFIX}/lib/libmupdf-third.a"
    (cd "${INSTALL_PREFIX}/lib" && sha256sum -c ../archive-sha256)
    exit 0
fi

mkdir -p "${BUILD_DIR}"
cd "${BUILD_DIR}"

TARBALL="mupdf-${MUPDF_VERSION}-source.tar.gz"
URL="https://mupdf.com/downloads/archive/${TARBALL}"

if [ ! -f "${TARBALL}" ]; then
    echo "Fetching MuPDF source tarball from ${URL}..."
    curl --connect-timeout 20 --max-time 600 --retry 3 -fsSL -o "${TARBALL}.download" "${URL}"
    mv "${TARBALL}.download" "${TARBALL}"
fi

echo "Verifying SHA256 checksum of downloaded tarball..."
if command -v sha256sum &> /dev/null; then
    ACTUAL_SHA="$(sha256sum "${TARBALL}" | awk '{print $1}')"
elif command -v shasum &> /dev/null; then
    ACTUAL_SHA="$(shasum -a 256 "${TARBALL}" | awk '{print $1}')"
else
    echo "ERROR: SHA256 verification tool required." >&2
    exit 1
fi

if [ "${ACTUAL_SHA}" != "${MUPDF_PINNED_SHA256}" ]; then
    echo "ERROR: SHA256 mismatch for ${TARBALL}!" >&2
    echo "Expected: ${MUPDF_PINNED_SHA256}" >&2
    echo "Actual:   ${ACTUAL_SHA}" >&2
    rm -f "${TARBALL}"
    exit 1
fi
echo "SHA256 checksum verified: ${ACTUAL_SHA}"

SRC_DIR="mupdf-${MUPDF_VERSION}-source"
if [ ! -d "${SRC_DIR}" ]; then
    echo "Extracting ${TARBALL}..."
    tar -xzf "${TARBALL}"
fi

cd "${SRC_DIR}"
OUT="build/papernx-switch-$(printf %s "${FINGERPRINT}" | sha256sum | cut -c1-16)"

echo "Building static libraries..."
make -j"${JOBS:-4}" \
    build=release OUT="${OUT}" \
    XCXXFLAGS="-fno-exceptions -fno-rtti" \
    OS=none \
    HAVE_X11=no \
    HAVE_GLFW=no \
    HAVE_GLUT=no \
    HAVE_CURL=no \
    HAVE_WIN32=no \
    USE_SYSTEM_LIBS=no \
    USE_SYSTEM_HARFBUZZ=yes USE_SYSTEM_FREETYPE=yes \
    SYS_HARFBUZZ_CFLAGS="-I${DEVKITPRO}/portlibs/switch/include/harfbuzz" \
    SYS_FREETYPE_CFLAGS="-I${DEVKITPRO}/portlibs/switch/include/freetype2" \
    CROSS_COMPILE=aarch64-none-elf- \
    XCFLAGS="-I$(pwd)/include ${ARCH_FLAGS} ${SWITCH_INCLUDES} -include ${COMPAT_HEADER} -Dtimegm=papernx_timegm -D__SWITCH__ -D__arm64__" \
    libs

echo "Installing to ${INSTALL_PREFIX}..."
mkdir -p "${INSTALL_PREFIX}/lib" "${INSTALL_PREFIX}/include"
cp -r include/mupdf "${INSTALL_PREFIX}/include/"
cp "${OUT}/libmupdf.a" "${INSTALL_PREFIX}/lib/"
cp "${OUT}/libmupdf-third.a" "${INSTALL_PREFIX}/lib/"

echo "Strictly verifying AArch64 target architecture of generated libraries..."
verify_arch "${INSTALL_PREFIX}/lib/libmupdf.a"
verify_arch "${INSTALL_PREFIX}/lib/libmupdf-third.a"
(cd "${INSTALL_PREFIX}/lib" && sha256sum libmupdf.a libmupdf-third.a > ../archive-sha256)

python3 "$(dirname "${COMPAT_HEADER}")/../../scripts/collect_licenses.py" "$PWD" "${INSTALL_PREFIX}/licenses"

# Write cache fingerprint
echo "${FINGERPRINT}" > "${FINGERPRINT_FILE}"

echo "=========================================================="
echo "SUCCESS: MuPDF ${MUPDF_VERSION} AArch64 libraries verified and ready!"
echo "Headers: ${INSTALL_PREFIX}/include/mupdf"
echo "Libraries: ${INSTALL_PREFIX}/lib/libmupdf.a, libmupdf-third.a"
echo "Fingerprint: $(cat "${FINGERPRINT_FILE}")"
echo "=========================================================="
