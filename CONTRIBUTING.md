# Contributing to PaperNX

Thank you for your interest in contributing to **PaperNX**! PaperNX is an open-source, native EPUB and PDF reader for the Nintendo Switch.

---

## Code of Conduct

Please help maintain a welcoming, respectful, and productive community. Be courteous, open to constructive feedback, and considerate of fellow developers and users.

---

## Development Environment & Tech Stack

- **Core Technologies:** C++17, C11, SDL2, SDL2_ttf, MuPDF 1.23.11, FreeType, HarfBuzz.
- **Target Platform:** Nintendo Switch (devkitA64, libnx, horizon OS).
- **Desktop Target:** Linux / POSIX environment for native testing and automated CI.
- **Build Systems:** CMake (Desktop) and Make (Switch NRO via devkitA64).
- **Containers:** Docker and Docker Compose provide sealed, reproducible compilation environments.

---

## Getting Started

### 1. Clone the Repository

```bash
git clone https://github.com/ysfbz/PaperNx.git
cd PaperNx
```

### 2. Build and Test Locally

We strongly recommend using the provided Docker containers to ensure consistent toolchains.

```bash
# Run all unit, integration, and UI component tests on Desktop
docker compose run --rm desktop bash scripts/verify-v04.sh native

# Run AddressSanitizer and UndefinedBehaviorSanitizer checks
docker compose run --rm desktop bash scripts/verify-v04.sh sanitizers

# Build and verify the Nintendo Switch AArch64 NRO binary
docker compose run --rm switch bash scripts/verify-switch.sh
```

---

## Coding Guidelines

1. **Language Standards:** C++17 for application and UI logic; C11 for MuPDF bridge wrappers.
2. **Memory Safety & RAII:** Manage dynamic resources using RAII principles. All tests must pass cleanly under ASan and UBSan without memory leaks or undefined behavior.
3. **Bilingual Localization:** Any user-facing string must be localized in both English and Turkish in `include/papernx/localization.hpp`. English is the default language.
4. **Touch & Ergonomics:** Ensure touch targets have a minimum dimension of 56px. Maintain controller shortcuts as secondary inputs.
5. **Data Integrity:** Reading bookmarks, user settings, and library databases must be saved atomically (`.tmp` write followed by atomic filesystem rename) to prevent corruption if the console powers off during a write.

---

## Pull Request Process

1. Fork the repository and create your feature branch from `main`:
   ```bash
   git checkout -b feature/your-feature-name
   ```
2. Write clean, readable code and accompany new functionality with automated tests under `tests/`.
3. Verify that all test suites pass:
   - `verify-v04.sh native` (11/11 tests pass)
   - `verify-v04.sh sanitizers` (11/11 tests pass, 0 leaks)
   - `verify-switch.sh` (AArch64 ELF, NACP, and NRO verified)
4. Submit your pull request with a descriptive title and detailed summary of changes.

---

## Maintainer

PaperNX is maintained by **Yusuf Buzrul** ([@ysfbz](https://github.com/ysfbz)). Feel free to open issues or discussions for feature ideas and bug reports.
