#!/usr/bin/env bash
# Build the exact vendored Unicode bidi implementation for desktop or Switch.
set -euo pipefail
root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
target="${1:-switch}"
archive="$root/third_party/fribidi/fribidi-1.0.12.tar.xz"
expected=0cd233f97fc8c67bb3ac27ce8440def5d3ffacf516765b91c2cc654498293495
printf '%s  %s\n' "$expected" "$archive" | sha256sum -c -
prefix="$root/prefix-fribidi-$target"
build="$root/build-fribidi-$target"
options=(--disable-shared --enable-static --disable-deprecated)
if [[ "$target" == switch ]]; then
    export PATH="${DEVKITPRO:?}/devkitA64/bin:$PATH"
    export CC=aarch64-none-elf-gcc AR=aarch64-none-elf-ar RANLIB=aarch64-none-elf-ranlib
    export CFLAGS='-O2 -march=armv8-a+crc+crypto -mtune=cortex-a57 -mtp=soft -fPIE -ffunction-sections -fdata-sections'
    options+=(--host=aarch64-none-elf)
elif [[ "$target" == desktop ]]; then
    export CC=cc AR=ar RANLIB=ranlib CFLAGS='-O2 -fPIC'
else
    echo 'Expected switch or desktop' >&2;exit 1
fi
fingerprint="$expected|$($CC --version | head -n1)|$CFLAGS|${options[*]}|$(sha256sum "${BASH_SOURCE[0]}" | cut -d' ' -f1)"
if [[ -f "$prefix/.build_fingerprint" && -f "$prefix/lib/libfribidi.a" && "$(cat "$prefix/.build_fingerprint")" == "$fingerprint" ]]; then
    (cd "$prefix" && sha256sum -c archive-sha256)
    echo "Verified FriBidi 1.0.12 cache: $target";exit
fi
mkdir -p "$build" "$prefix"
if [[ ! -f "$build/fribidi-1.0.12/configure" ]]; then tar -xJf "$archive" -C "$build";fi
mkdir -p "$build/out"
cd "$build/out"
../fribidi-1.0.12/configure --prefix="$prefix" "${options[@]}"
make -C lib -j"${JOBS:-4}"
make -C lib install
mkdir -p "$prefix/lib/pkgconfig"
cp fribidi.pc "$prefix/lib/pkgconfig/"
if [[ "$target" == switch ]]; then
    machines="$(aarch64-none-elf-readelf -h "$prefix/lib/libfribidi.a" | sed -n 's/.*Machine: *//p' | sort -u)"
    [[ "$machines" == AArch64 ]] || { echo "Unexpected FriBidi architecture: $machines" >&2;exit 1; }
fi
(cd "$prefix" && sha256sum lib/libfribidi.a > archive-sha256)
printf '%s\n' "$fingerprint" > "$prefix/.build_fingerprint"
echo "Built and verified FriBidi 1.0.12: $target"
