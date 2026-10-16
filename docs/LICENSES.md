# Dependency and license responsibilities

- **PaperNX**: AGPL-3.0-or-later, full license in `LICENSE`.
- **MuPDF C library**: AGPL-3.0-or-later or separate paid Artifex license. Sources: https://github.com/ArtifexSoftware/mupdf and https://artifex.com/licensing . MuPDF is **not vendored** or shipped yet.
- **SDL2, SDL2_ttf**: verify the exact upstream zlib licenses for selected releases and their linked dependencies before release.
- **Fonts**: no third-party font binary included. End-user fonts must be legally usable.
- **Nintendo**: only public libnx/devkitPro tooling is contemplated; no proprietary Nintendo SDK.

Before distributing NRO binaries, provide full corresponding source, original licensing/NOTICE files, exact pinned dependencies and license information. A private source repository without access offered to binary recipients is insufficient.
