# Independent component verification — v0.3

Reviewed 2026-10-17, separately from the component implementers. Scope: `core.hpp`,
`core.cpp`, `app_settings.hpp`, `core_tests.cpp`, `cover_repository.hpp`,
`cover_repository.cpp`, and `cover_repository_tests.cpp`. Main/UI and native document
rendering are outside this component review.

## Core/preferences: passed

- Independently configured `build-v03-review-core` in `papernx-desktop:latest` with
  `PAPERNX_NATIVE=OFF` and `CMAKE_CXX_FLAGS="-fno-exceptions -fno-rtti -Werror"`.
  Build succeeded; CTest passed both core and cover tests (2/2).
- Independently compiled core tests with GCC C++17, `-Wall -Wextra -Werror`,
  `-fno-exceptions -fno-rtti`, ASan/UBSan, `-O1 -g`, and frame pointers. Tests passed
  with `ASAN_OPTIONS=detect_leaks=1`; no sanitizer report.
- Read migration/load/save implementation and regression cases. Versions 1–5
  migrate with exact original bytes in immutable `.pre-v6-vN.bak`; injected backup
  failure aborts before changing the original. Rotating saves preserve this copy,
  and immutable-copy recovery is covered. EPUB appearance defaults and explicit
  navigation overrides resolve independently; PDF view/color remain book-local.
- `APP_PREFERENCES_` is a legal 16-character record in a separate settings file.
  Theme/language/layout/sort/toolbar/path mappings have valid nonoverlapping fields,
  normalization, escaping, checksum verification, and backup recovery tests.
- Anti-pattern searches found no production exceptions, throwing number parsers,
  TODO/FIXME/stub implementations, or recursive production deletion in this scope.
  Production filesystem operations use error-code overloads; test cleanup targets
  test-owned temporary directories. The core test suite still uses a fixed named
  temporary directory, so concurrent runs of that same suite must be serialized.

## Cover cache: findings and verification

Initial independent strict compile/run and ASan/UBSan runs passed. Review nevertheless
found two gaps and sent them to the implementation owner before approval:

1. A symlinked cache root/ancestor allowed cache IO outside the intended directory.
   The implementation owner added component inspection and external-file regression
   tests. Follow-up review found that lexical normalization before inspection could
   hide `symlink/..` while IO used the unnormalized path; correction requested.
2. Startup inserted every existing cache entry before trimming, causing transient
   unbounded index growth. The implementation owner added incremental trimming;
   the index now holds at most the configured bound plus one insertion while scanning.

Final recheck passed: the owner removed lexical normalization before component
inspection and added a `linked/../redirected` regression. I independently rebuilt
`build-v03-review-core`, reran CTest (2/2 passed), and compiled/ran the final cover
tests with ASan/UBSan and the strict flags above (passed, no diagnostic). New tests
also verify unchanged-size/mtime-only invalidation and startup trimming of 20 old
entries to at most two files/148 bytes. No remaining blocking findings in this
component scope. Corruption is bounded before
allocation; images are at most 150×210 RGB, file length and checksums are validated,
keys contain only hashed hexadecimal identity plus current size/mtime, and corrupt
or stale artwork is a cache miss. Existing tests cover budgets, restart reuse,
interrupted replacement, malformed headers, corruption, and untouched unrelated files.

Cache writes verify flush/close and reread, but do not fsync. This regenerable cache
does not guarantee artwork survives power loss; progress/settings use the stronger
independent verified/fsynced storage implementation. FAT interruption tests simulate
file states on Linux and cannot prove physical Switch SD behavior. This review does
not certify touch responsiveness, native UI integration, or hardware behavior.

## Reproduction commands

All commands ran inside `papernx-desktop:latest` with this repository mounted at
`/work` and `/work` as the working directory:

```sh
cmake -S . -B build-v03-review-core -DPAPERNX_NATIVE=OFF -DCMAKE_CXX_FLAGS="-fno-exceptions -fno-rtti -Werror"
cmake --build build-v03-review-core -j2
ctest --test-dir build-v03-review-core --output-on-failure
g++ -std=c++17 -fno-exceptions -fno-rtti -Wall -Wextra -Werror -Iinclude src/core.cpp src/gestures.cpp src/cover_repository.cpp tests/cover_repository_tests.cpp -o /tmp/v03-review-cover
/tmp/v03-review-cover
g++ -std=c++17 -g -O1 -fno-exceptions -fno-rtti -Wall -Wextra -Werror -fsanitize=address,undefined -fno-omit-frame-pointer -Iinclude src/core.cpp src/gestures.cpp src/cover_repository.cpp tests/core_tests.cpp -o /tmp/v03-review-core-asan
ASAN_OPTIONS=detect_leaks=1 /tmp/v03-review-core-asan
g++ -std=c++17 -g -O1 -fno-exceptions -fno-rtti -Wall -Wextra -Werror -fsanitize=address,undefined -fno-omit-frame-pointer -Iinclude src/core.cpp src/gestures.cpp src/cover_repository.cpp tests/cover_repository_tests.cpp -o /tmp/v03-review-cover-final
ASAN_OPTIONS=detect_leaks=1 /tmp/v03-review-cover-final
```
