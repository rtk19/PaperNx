# Automatic updates — rtk19/PaperNx

If your installed build already has **+ → Check for update**, use it to install
0.13.0. Otherwise install the current NRO manually once. PaperNX checks the fork
in the background every time it starts. New versions show **Later**, **What's new**
and **Update**. No update or an offline startup leaves the reader available
without a popup. Use **+ → Check for update** for an explicit result at any time;
the action is at the end of the scrollable + menu.

**What's new** shows the release notes with touch, D-pad and stick scrolling.
**Update** downloads the NRO with progress and verifies it. **Restart** finishes
installation and requests a relaunch through hbloader. If the loader cannot
relaunch applications, reopen PaperNX from Homebrew Menu. Books, progress,
settings and the library stay in place. Existing older versions require a
manual upgrade to obtain the updater itself.

## Source and versions

The fixed source is
`https://api.github.com/repos/rtk19/PaperNx/releases?per_page=30`.
New releases use plain semantic versions and normal GitHub releases: `0.13.0`,
then `0.13.1` for a fix or `0.14.0` for a feature release. Git tags use the
conventional `v` prefix, for example `v0.13.0`; application versions contain no
suffix. Legacy prereleases remain readable for migration. Drafts and invalid
tags are ignored. Complete semantic version precedence prevents downgrades and
lets old `0.12.0-reader.*` builds recognize `0.13.0` as newer.

Releases need a `PaperNX.nro` asset, its byte size, and either GitHub's SHA256
asset digest or `PaperNX.nro.sha256`. The existing packaging/publication scripts
produce these files. Only this repository's HTTPS release download URLs are
accepted. Publish future releases after all assets have been uploaded; keep
`APP_VERSION`, NACP and tag versions in sync. `/releases/latest` intentionally
is not used so older prerelease builds can continue to upgrade through the same endpoint.

## Installation

The loader's actual executable path selects the update target, including renamed
NROs. An unknown/non-NRO path disables installation. A separate worker streams
to `<executable>.part`; it checks size, SHA256, NRO segment/resource boundaries,
PaperNX identity and NACP version before staging `<executable>.new` with durable
`.ready` metadata. HTTPS validates against the embedded Mozilla roots; redirects
cannot downgrade HTTPS. There is no archive extraction or whole-NRO RAM buffer.

The running executable remains unchanged during download. Clean exit stops the
worker, releases documents/fonts/SDL/RomFS, then applies a validated pending
update. The previous executable is retained as `.old` until replacement succeeds;
a failed replacement attempts restoration. A complete verified staged download
can be recovered on the next launch. Partial or tampered files are never applied.
On loaders that cannot restart, the same installation completes on exit.

## Verification

`scripts/verify-v04.sh native` and `scripts/verify-v04.sh sanitizers` include the
updater unit and HTTP/HTTPS integration suites. The test server uses loopback,
temporary synthetic NROs and a temporary TLS certificate; it does not publish
or install a GitHub release. See [TEST_RESULTS.md](TEST_RESULTS.md).
Physical Switch networking, FAT replacement and hbloader relaunch still need
device testing.
