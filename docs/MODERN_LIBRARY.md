# Menus and library (0.16.0)

Open **+ → Application appearance → Application** to configure the interface. Choose **Interface appearance** (Dark, Light, Follow reading theme) and **Accent color** (Purple, Blue, Teal, Green, Orange, Red, Pink). The colour family follows rtk19/NX-torrent-player. Reading paper and its book-specific overrides remain independent. Dark/purple is the initial interface; Follow reading theme restores the paper-based menu colours.

All screens share rounded controls, consistent focus, elevated sheets, tonal selections and larger controls. Settings use a category sidebar, numeric steppers, switches, a reading preview and a scroll indicator. Catalog, contents, bookmarks, journal, updates, choices, browser and quick actions retain their existing actions. Expanded reading area and Two pages in landscape remain separate default-on switches in Application.

## Add a whole folder

Open **+ → Browse SD / open book**, enter the desired folder and choose **Add this folder** (or press Y). All supported EPUB, PDF, CBZ and CBR files in its subfolders enter the library in a single batch. Files stay in their original location. Existing series, collections, tags, notes, covers and reading positions remain available.

**Application → Book folders** lists extra scan roots and offers Add books folder and Scan all folders. A configured folder can be browsed or removed from automatic scanning. Removing a scan root never deletes files, progress or catalog records, and already imported existing books remain accessible. The built-in `sdmc:/switch/PaperNX/books/` folder is always a scan root. Up to 32 additional folders and 10,000 inventory entries are supported; traversal retains its existing 16-level depth limit and skips symlinks.

**Scan on launch** is On initially. Disable it to use the saved inventory until manually scanning through **Scan library now**, **Scan all folders** or **+ → Refresh library**. Launch displays the saved inventory while the scan runs. New or removed files are reconciled after the worker finishes. A disappeared source leaves the current inventory; reading history and metadata remain stored. Changing file size yields a new document identity as before. Scanning does not replace or repaginate the active document, and publication waits until a held touch finishes.

## Efficiency and storage

Folder traversal runs on a cancellable worker, with nested/duplicate roots normalized before scanning. Results merge using document identifiers instead of repeated searches across the library. The main thread publishes a complete batch and preserves the selected book. Background metadata uploads are batched; EPUB metadata is opened without layout or page counting. Covers remain lazy and bounded to visible/nearby items with two texture uploads per frame. The interface gradient is cached, while reading-page and comic caches retain their budgets.

`data/library-index.txt` is a disposable checksummed inventory using the existing recoverable atomic store. User reading progress and catalog data stay in their existing files. Interface, folder and scan preferences live in independent reserved records in `data/settings.txt`, retaining the prior on-disk schema. Missing old preference records get the new defaults. An index cache failure reports an error and leaves the in-memory scan usable; manual rescan rebuilds it. Atomic rename reduces corruption risk without guaranteeing durability under SD failure or sudden power loss.

## Shoulder buttons

In landscape, **English: R next, L previous; Hebrew: L next, R previous**. Each opened EPUB determines its direction; switching books updates the controls immediately. Comic direction overrides apply too. Portrait shortcuts remain unchanged. The L/R shortcuts setting continues to enable or disable these actions.
