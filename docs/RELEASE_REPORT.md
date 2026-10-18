# v0.4.0-rc1 release report

Baseline final verification: `28f2e0035f876ff1b86af0651e9124d19ad92c5c`. Branch: `feat/v04-professional-reader-ux`. Foundation: `e788e87`. Previous report: [V03_RELEASE_REPORT.md](V03_RELEASE_REPORT.md).

Final engineering gates/artifact hashes are **UNVERIFIED** pending source freeze and rebuild. Expected output: `dist/v0.4.0-rc1/PaperNX.nro`, its SHA256, `PaperNX-switch-v0.4.0-rc1.zip`, its SHA256 and `manifest.json`. Only a genuine new AArch64 link/NRO may be packaged. `verification/v04-switch-build-receipt.json` records source before/after and ELF/NRO hashes; mismatch requires rebuilding.

Package includes PaperNX branding, NACP version, RomFS fonts including Noto Sans SemiBold, OFL/license notices and docs. Empty books/data directories contain no user data. Preserve existing books/data and migration backups while merging an upgrade; do not replace an existing data directory with an empty one.

New hardware status: **UNVERIFIED**. Release readiness: **candidate under verification, not stable**. See [coverage](V04_REQUIREMENT_COVERAGE.md), [known limits](KNOWN_ISSUES.md) and [physical checklist](DEVICE_TEST_PLAN.md). No automatic merge or stable publication.
