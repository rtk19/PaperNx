#!/usr/bin/env bash
set -euo pipefail
bash scripts/verify-desktop.sh
bash scripts/verify-pinned-desktop.sh
bash scripts/verify-sanitizers.sh
{
    cmake -S . -B build-core-release -G Ninja -DBUILD_TESTING=ON -DCMAKE_BUILD_TYPE=Release
    cmake --build build-core-release --parallel 4
    ctest --test-dir build-core-release --output-on-failure
} 2>&1 | tee verification/core-release.log
