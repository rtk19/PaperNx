# PaperNX v0.9.0 Release Verification Report

**Release Target:** `v0.9.0`  
**Application Title:** `PaperNX`  
**Author / Copyright Holder:** `Yusuf Buzrul`  
**Date:** 2026-10-19  
**Platform:** Nintendo Switch (devkitA64, libnx, Horizon OS) / POSIX Desktop (Linux x86_64)

---

## Engineering Quality & Test Gates Summary

All automated engineering gates have completed with a **100% pass rate** across all execution modes:

| Test Gate | Scope | Status | Result Details |
|---|---|:---:|---|
| **Portable Suite** | Core logic & headless mock components | **PASS** | 7/7 tests passed (100%) |
| **Native Suite** | Full application, UI components, MuPDF document bridge | **PASS** | 11/11 tests passed (100%) |
| **Release Native Suite** | Optimized C++17 release binary build & full test pass | **PASS** | 11/11 tests passed (100%) |
| **Sanitizers Suite** | AddressSanitizer (ASan) & UndefinedBehaviorSanitizer (UBSan) | **PASS** | 11/11 tests passed (0 leaks, 0 errors) |
| **Switch AArch64 Cross-Build** | devkitA64 toolchain, libnx, static MuPDF 1.23.11, RomFS | **PASS** | AArch64 ELF PIE & NRO0 generated |
| **Packaging & Verification** | Minimal SD layout, license compliance ZIP, SHA256 hashes | **PASS** | Verified via `verify_distribution.py` |

---

## Release Artifacts

Distribution Directory: `dist/v0.9.0/`

| Artifact | Size (Bytes) | SHA256 Checksum |
|---|---|---|
| `PaperNX.nro` | 48,085,971 | `f05651ba40305460ebcfdd626d01d7a33f24aefdedfa57eeb31fc48e30decf81` |
| `PaperNX-switch-v0.9.0.zip` | 33,260,644 | `6022068562fb2f74e673162d54589c92f10c23635440f65d819127e2ae4c4ed8` |
| `PaperNX-licenses-v0.9.0.zip` | 190,942 | `9c4c0d1f2042479c4723cbbaa3a83d0ce9c0584180a53d94127c575170fff859` |
| `manifest.json` | 2,371 | Complete distribution descriptor |

---

## Binary Metadata & Architecture

- **Target Architecture:** ARM64 / AArch64 (ELF64 Position-Independent Executable, machine 183)
- **NACP Metadata:**
  - Name: `PaperNX`
  - Author: `Yusuf Buzrul`
  - Version: `0.9.0`
- **Embedded Icon:** 256×256 baseline JPEG (`b47198387614a9e66a570d7faaefff0fa496033d4481060316a086059b506bb5`)
- **RomFS Image:** Embedded fonts (`NotoSans-Regular.ttf`, `NotoSans-SemiBold.ttf`, `NotoSerif-Regular.ttf`, `OFL.txt`)
- **Implementation SHA256:** `8d87b5458e95f3a998b70c20f15549e4e8d05335af5d5e4fd34d41d45148513a`
- **Minimal SD Distribution:** Package includes only `switch/PaperNX/PaperNX.nro`, `fonts/`, and empty `books/`. No test artifacts, user databases (`data/`), or log files are bundled, guaranteeing zero data loss on existing user installations.

---

## Hardware Acceptance Note

- **Automated Validation:** All unit tests, rendering pipelines, memory checks, and format verifications passed.
- **Physical Device Status:** Handheld touch and portrait rotation features are verified via SDL desktop simulation; awaiting physical Nintendo Switch hardware test acceptance according to [DEVICE_TEST_PLAN.md](DEVICE_TEST_PLAN.md).
