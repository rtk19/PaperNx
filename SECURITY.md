# Security Policy

## Supported Versions

Security updates and bug fixes are provided for the following releases:

| Version | Supported          |
| ------- | ------------------ |
| 0.9.x   | :white_check_mark: |
| < 0.9.0 | :x:                |

---

## Threat Model & Security Posture

**PaperNX** is designed as a **completely offline** application for the Nintendo Switch:
- **No Network Connectivity:** PaperNX does not open TCP/UDP sockets, perform HTTP/HTTPS requests, or communicate with external servers.
- **No Cloud Services or Accounts:** No personal identifiers, credentials, or telemetry data are collected or transmitted.
- **MicroSD Isolation:** All reads and writes are restricted to document directories (`sdmc:/switch/PaperNX/books/`) and application state directories (`sdmc:/switch/PaperNX/data/`).
- **Memory Safety:** Memory-critical components (document parsing and layout via MuPDF, typography via SDL2_ttf/FreeType) are validated with GCC/Clang AddressSanitizer and UndefinedBehaviorSanitizer suites on every release.

---

## Reporting a Vulnerability

If you discover a security issue or memory corruption bug (e.g., a maliciously crafted EPUB or PDF file triggering an unchecked buffer overflow or denial of service), please report it responsibly:

1. **GitHub Security Advisory:** Submit a confidential report via the **Security** tab of the GitHub repository:  
   [https://github.com/ysfbz/PaperNx/security/advisories](https://github.com/ysfbz/PaperNx/security/advisories)
2. **Direct Contact:** Alternatively, reach out directly to the maintainer:  
   **Yusuf Buzrul** via GitHub: [@ysfbz](https://github.com/ysfbz)

Please include:
- A description of the vulnerability.
- A minimal proof-of-concept (sample document triggering the bug).
- The PaperNX version tested.

We will review reports promptly, reproduce the issue in an isolated environment, and publish a fix alongside an appropriate advisory.
