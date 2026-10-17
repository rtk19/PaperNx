#include "papernx/core.hpp"
#include "papernx/gestures.hpp"
#include <cassert>
#include <cerrno>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#ifdef NDEBUG
#error Tests require enabled assertions
#endif

using namespace papernx;

static void writeChecked(const fs::path &path, const std::string &body) {
    uint64_t h = 14695981039346656037ull;
    for (unsigned char c : body) { h ^= c; h *= 1099511628211ull; }
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    out << "CHECKSUM=" << std::hex << std::setw(16) << std::setfill('0') << h << '\n' << body;
}

int main() {
    std::cout << "[TEST] Running PaperNX comprehensive core suite...\n";

    const auto root = fs::temp_directory_path() / "papernx-core-test-suite";
    fs::remove_all(root);
    fs::create_directories(root / "nested");
    fs::create_directories(root / "data");

    // 1. Format Detection
    assert(detectFormat("book.EPUB") == Format::Epub);
    assert(detectFormat("doc.pdf") == Format::Pdf);
    assert(detectFormat("document.PDF") == Format::Pdf);
    assert(detectFormat("notes.txt") == Format::Unsupported);
    assert(detectFormat("image.png") == Format::Unsupported);

    // 2. Turkish Case-Folding & Search
    // Capital 'İ' -> 'i', Capital 'I' -> 'ı'
    assert(turkishCaseFold("İSTANBUL") == "istanbul");
    assert(turkishCaseFold("IŞIK") == "ışık");
    assert(turkishCaseFold("AĞAÇ") == "ağaç");
    assert(turkishCaseFold("ÖMÜR") == "ömür");
    assert(turkishCaseFold("ÜZÜM") == "üzüm");
    assert(turkishCaseFold("ÇİÇEK") == "çiçek");
    assert(turkishCaseFold("PaperNX Reader") == "papernx reader");

    // 3. UTF-8 Safe Truncation
    std::string turkishText = "İstanbul'da Bir Çiçek";
    // "İstanbul" is 9 bytes in UTF-8: İ (2 bytes) + s,t,a,n,b,u,l (7 bytes)
    std::string trunc1 = utf8SafeTruncate(turkishText, 1); // 1 byte cuts inside 'İ', so must truncate to 0
    assert(trunc1.empty());
    std::string trunc2 = utf8SafeTruncate(turkishText, 2); // exactly 'İ'
    assert(trunc2 == "İ");
    std::string trunc3 = utf8SafeTruncate(turkishText, 3);
    assert(trunc3 == "İs");
    assert(utf8CodepointCount("Çiçek") == 5);

    // 4. Document Scanning & Identity
    {
        std::ofstream(root / "İstanbul_Masalı.epub") << "fixture epub";
        std::ofstream(root / "nested" / "Tarama_Raporu.PDF") << "fixture pdf";
        std::ofstream(root / "ignored.txt") << "ignore";
    }
    const auto scanned = scan(root);
    assert(scanned.size() == 2);
    assert(scan(root / "nonexistent").empty());

    // Search with Turkish matching
    assert(search(scanned, "istanbul").size() == 1);
    assert(search(scanned, "İSTANBUL").size() == 1);
    assert(search(scanned, "tarama").size() == 1);
    assert(search(scanned, "TARAMA").size() == 1);
    assert(search(scanned, "bulunamayan").empty());

    // Identity stability
    assert(documentId("x", 100) != documentId("x", 101));
    assert(documentId("x", 100) == documentId("x", 100));

    // 5. Page Restoration
    Progress p;
    p.page = 49;
    p.pageCount = 100;
    p.fraction = 49.0 / 99.0;
    p.fontSize = 27;
    p.theme = 2;
    p.lineHeight = 150;
    p.margin = 50;
    p.muBookmark = 9876543210ll;
    p.chapter = 4;
    p.favorite = true;
    p.lastRead = 1700000000ll;
    p.bookPath = "sdmc:/Kitaplar/İstanbul.epub";
    p.bookTitle = "İstanbul Masalları";
    p.bookAuthor = "Yaşar Kemal";
    p.bookmarks.push_back({10, "Bölüm 1 Başlangıcı", 1700000100ll, 12345ll});
    p.bookmarks.push_back({49, "Önemli Alıntı", 1700000200ll, 67890ll});

    assert(restorePage(p, 100) == 49);
    assert(restorePage(p, 200) == 98);
    assert(restorePage(p, 0) == 0);

    // 6. Persistence Store (PAPERNX-2) Save & Load
    const fs::path storePath = root / "data" / "progress.txt";
    ProgressStore store(storePath);
    store.put("0123456789abcdef", p);
    assert(store.count() == 1);
    assert(store.save());

    ProgressStore loaded(storePath);
    assert(loaded.load());
    assert(loaded.count() == 1);
    auto lp = loaded.get("0123456789abcdef");
    assert(lp.page == 49);
    assert(lp.bookPath == p.bookPath && lp.bookTitle == p.bookTitle && lp.bookAuthor == p.bookAuthor);
    assert(lp.pageCount == 100);
    assert(lp.fontSize == 27);
    assert(lp.theme == 2);
    assert(lp.favorite == true);
    assert(lp.muBookmark == 9876543210ll);
    assert(lp.chapter == 4);
    assert(lp.lineHeight == 150);
    assert(lp.margin == 50);
    assert(lp.lastRead == 1700000000ll);
    assert(lp.bookmarks.size() == 2);
    assert(lp.bookmarks[0].page == 10);
    assert(lp.bookmarks[0].title == "Bölüm 1 Başlangıcı");
    assert(lp.bookmarks[1].page == 49);
    assert(lp.bookmarks[1].title == "Önemli Alıntı");

    // 7. Recovery from Primary File Corruption (.bak fallback)
    assert(loaded.save()); // This creates progress.txt.bak
    {
        // Corrupt primary file
        std::ofstream corrupt(storePath, std::ios::trunc);
        corrupt << "GARBAGE DATA THAT FAILS CHECKSUM\n";
    }
    ProgressStore backupStore(storePath);
    assert(backupStore.load()); // Must recover from backup
    assert(backupStore.get("0123456789abcdef").page == 49);
    assert(backupStore.get("0123456789abcdef").bookmarks.size() == 2);

    // 8. Backward-Compatible Migration from PAPERNX-1
    {
        // Write raw PAPERNX-1 format
        std::string v1data = "PAPERNX-1\nfedcba9876543210\t12\t50\t0.24\t20\t1\t1\n";
        uint64_t h = 14695981039346656037ull;
        for (unsigned char c : v1data) { h ^= c; h *= 1099511628211ull; }
        std::ostringstream hstr;
        hstr << std::hex << std::setw(16) << std::setfill('0') << h;

        std::ofstream v1file(storePath, std::ios::trunc | std::ios::binary);
        v1file << "CHECKSUM=" << hstr.str() << "\n" << v1data;
    }
    ProgressStore migrationStore(storePath);
    assert(migrationStore.load());
    auto migrated = migrationStore.get("fedcba9876543210");
    assert(migrated.page == 12);
    assert(migrated.pageCount == 50);
    assert(migrated.fontSize == 20);
    assert(migrated.theme == 1);
    assert(migrated.favorite == true);
    // Saving migrated store upgrades format to PAPERNX-2
    assert(migrationStore.save());

    // Verify it loads back cleanly
    ProgressStore v2CheckStore(storePath);
    assert(v2CheckStore.load());
    assert(v2CheckStore.get("fedcba9876543210").page == 12);

    // 9. Categories and Advanced Search Filtering
    Book b1; b1.id = "b1"; b1.title = "Kürk Mantolu Madonna"; b1.author = "Sabahattin Ali";
    Book b2; b2.id = "b2"; b2.title = "İnce Memed"; b2.author = "Yaşar Kemal";
    Book b3; b3.id = "b3"; b3.title = "Tutunamayanlar"; b3.author = "Oğuz Atay";
    std::vector<Book> testBooks = {b1, b2, b3};

    Progress pb1; pb1.page = 50; pb1.pageCount = 150; pb1.fraction = 0.33; pb1.lastRead = 200; pb1.favorite = true;
    Progress pb2; pb2.page = 0; pb2.pageCount = 300; pb2.fraction = 0.0; pb2.lastRead = 0; pb2.favorite = false;
    Progress pb3; pb3.page = 499; pb3.pageCount = 500; pb3.fraction = 1.0; pb3.lastRead = 300; pb3.favorite = false;
    ProgressStore catStore(root / "data" / "cat_test.txt");
    catStore.put("b1", pb1);
    catStore.put("b2", pb2);
    catStore.put("b3", pb3);

    auto cont = filterByCategory(testBooks, catStore, Category::ContinueReading);
    assert(cont.size() == 1 && cont[0].id == "b1");

    auto favs = filterByCategory(testBooks, catStore, Category::Favorites);
    assert(favs.size() == 1 && favs[0].id == "b1");

    auto unread = filterByCategory(testBooks, catStore, Category::Unread);
    assert(unread.size() == 1 && unread[0].id == "b2");

    auto completed = filterByCategory(testBooks, catStore, Category::Completed);
    assert(completed.size() == 1 && completed[0].id == "b3");

    auto byAuthor = search(testBooks, "Yaşar");
    assert(byAuthor.size() == 1 && byAuthor[0].id == "b2");

    // 10. Bulk 1,000-book performance test
    fs::create_directories(root / "bulk");
    for (int i = 0; i < 1000; ++i) {
        std::ofstream(root / "bulk" / ("Kitap_" + std::to_string(i) + ".epub")) << "x";
    }
    const auto bulkScanned = scan(root);
    assert(bulkScanned.size() == 1002);
    assert(scan(root, 50).size() == 50);
    assert(search(bulkScanned, "kitap").size() == 1000);

    // 11. Touch Gestures & Controller Navigation
    TouchGestureTracker tracker;
    // Tap left
    tracker.onFingerDown(0.1f, 0.5f, 1);
    auto tapLeft = tracker.onFingerUp(0.1f, 0.5f, 1);
    assert(tapLeft.type == GestureType::TapLeft);

    // Tap right
    tracker.onFingerDown(0.85f, 0.5f, 2);
    auto tapRight = tracker.onFingerUp(0.85f, 0.5f, 2);
    assert(tapRight.type == GestureType::TapRight);

    // Tap center
    tracker.onFingerDown(0.5f, 0.5f, 3);
    auto tapCenter = tracker.onFingerUp(0.5f, 0.5f, 3);
    assert(tapCenter.type == GestureType::TapCenter);

    // Swipe Left (deltaX < 0)
    tracker.onFingerDown(0.7f, 0.5f, 4);
    tracker.onFingerMotion(0.4f, 0.5f, 4);
    auto swipeLeft = tracker.onFingerUp(0.4f, 0.5f, 4);
    assert(swipeLeft.type == GestureType::SwipeLeft);

    // Swipe Right (deltaX > 0)
    tracker.onFingerDown(0.3f, 0.5f, 5);
    tracker.onFingerMotion(0.6f, 0.5f, 5);
    auto swipeRight = tracker.onFingerUp(0.6f, 0.5f, 5);
    assert(swipeRight.type == GestureType::SwipeRight);

    // Controller deadzone
    ControllerNavigator nav;
    int dx = 0, dy = 0;
    // Below deadzone (<12000)
    assert(!nav.processAxis(0, 5000, dx, dy));
    assert(dx == 0 && dy == 0);
    // Above deadzone (>12000)
    assert(nav.processAxis(0, 20000, dx, dy));
    assert(dx == 1 && dy == 0);

    // 12. Malformed Save Data & Power-Loss Edge Cases
    {
        const fs::path malformedPath = root / "data" / "malformed.txt";
        {
            std::ostringstream mf;
            mf << "PAPERNX-2\n";
            mf << "corrupt_id_too_short\tnot_an_int\t50\t0.5\t20\t0\t0\t0\t0\t140\t40\t0\t0\t0\t0\n";
            mf << "0123456789abcdef\t10\t100\t0.1\t24\t2\t1\t12345\t3\t150\t50\t1\t0\t1700000000\t0\n";
            mf << "another_bad_row\t0\t0\tnan\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\n";
            writeChecked(malformedPath, mf.str());
        }
        ProgressStore malformedStore(malformedPath);
        // Loading should parse valid lines and skip corrupt rows without throwing or aborting
        assert(malformedStore.load());
        assert(malformedStore.count() == 1);
        assert(malformedStore.get("0123456789abcdef").page == 10);
        const std::string valid = "0123456789abcdef\t10\t100\t0.1\t24\t2\t1\t12345\t3\t150\t50\t1\t0\t1700000000\t0\n";
        for (const auto &bad : {"2147483648", "-2147483649", "999999999999999999999", "abc", "12x"}) {
            auto row = valid;
            row.replace(17, 2, bad);
            writeChecked(malformedPath, "PAPERNX-2\n" + row);
            ProgressStore s(malformedPath);
            assert(s.load() && s.count() == 0);
        }
        for (const auto &bad : {"nan", "inf", "-inf", "1e9999", "1e-9999", "0.1x"}) {
            auto row = valid;
            row.replace(row.find("0.1"), 3, bad);
            writeChecked(malformedPath, "PAPERNX-2\n" + row);
            ProgressStore s(malformedPath);
            assert(s.load() && s.count() == 0);
        }
        for (const auto &suffix : {".bak", ".bak.old", ".tmp"}) {
            const fs::path recovery = root / "data" / (std::string("recover") + suffix);
            std::ofstream(recovery) << "CHECKSUM=0000000000000000\nPAPERNX-2\n";
            ProgressStore broken(recovery);
            assert(!broken.load());
            writeChecked(recovery.string() + suffix, "PAPERNX-2\n" + valid);
            assert(broken.load() && broken.get("0123456789abcdef").page == 10);
        }
        std::ofstream(malformedPath, std::ios::trunc) << "CHECKSUM=";
        ProgressStore truncated(malformedPath);
        assert(!truncated.load());
    }

    // Save failures retain primary and recovery copies; empty/escaped UTF-8 titles round-trip.
    {
        const auto path = root / "data" / "failure.txt";
        ProgressStore s(path);
        Progress p; p.pageCount = 20;
        p.bookmarks.push_back({3, "", 1, -1});
        p.bookmarks.push_back({4, "İ ı I i Ş ş Ğ ğ Ü ü Ö ö Ç ç\t\r\n\\", 2, -1});
        s.put("0123456789abcdef", p);
        assert(s.save());
        ProgressStore read(path); assert(read.load());
        assert(read.get("0123456789abcdef").bookmarks[0].title.empty());
        assert(read.get("0123456789abcdef").bookmarks[1].title == p.bookmarks[1].title);
        fs::create_directory(path.string() + ".bak");
        assert(!s.save());
        ProgressStore afterFailure(path); assert(afterFailure.load());
        assert(afterFailure.count() == 1);
        // Simulated crash after primary rotation, before .tmp promotion.
        fs::remove_all(path.string() + ".bak");
        fs::rename(path, path.string() + ".bak");
        ProgressStore interrupted(path); assert(interrupted.load());
        assert(interrupted.save());
        ProgressStore recovered(path); assert(recovered.load());
        assert(recovered.count() == 1);
    }

    // 13. Turkish Search & Diacritic Roundtrip
    {
        std::vector<Book> trBooks;
        Book tb1; tb1.id = "tr1"; tb1.title = "IŞIK DOĞUDAN YÜKSELİR"; tb1.author = "İSMAİL ÇAĞLAR";
        Book tb2; tb2.id = "tr2"; tb2.title = "ıslak kaldırımlar"; tb2.author = "ömer seyfettin";
        Book tb3; tb3.id = "tr3"; tb3.title = "Gül ve Bülbülün Şarkısı"; tb3.author = "Şemseddin Sami";
        trBooks = {tb1, tb2, tb3};

        // Query with lowercase 'i' must find 'İ'
        assert(search(trBooks, "ismail").size() == 1);
        // Query with lowercase 'ı' must find 'I'
        assert(search(trBooks, "ışık").size() == 1);
        assert(search(trBooks, "IŞIK").size() == 1);
        // Query with 'ş' and 'ç'
        assert(search(trBooks, "şarkısı").size() == 1);
        assert(search(trBooks, "çağlar").size() == 1);
    }


    // Storage fault injection exercises the Switch non-replacing rename path on Linux.
    {
        for (const auto &op : {"create_directories", "temporary_write", "write", "fflush", "fclose", "fsync", "verify", "backup_old", "backup_copy", "remove_primary", "rename"}) {
            const auto path = root / "data" / (std::string("fault-") + op);
            ProgressStore store(path);
            Progress first; first.page = 10; first.pageCount = 100; first.favorite = true;
            first.bookmarks.push_back({4, "Turkish İıŞşĞğÜüÖöÇç", 1, -1});
            store.put("0123456789abcdef", first); assert(store.save()); assert(store.save());
            Progress second = first; second.page = 20;
            store.put("0123456789abcdef", second);
            ProgressStore::SaveOptions fault;
            fault.noReplaceRename = true;
            fault.fault = [op](const char *operation) { return std::string(operation) == op ? EACCES : 0; };
            assert(!store.save(fault));
            assert(store.saveDiagnostic().find("operation=") != std::string::npos);
            assert(store.saveDiagnostic().find("retry_possible=1") != std::string::npos);
            ProgressStore recovery(path); assert(recovery.load());
            assert(recovery.get("0123456789abcdef").page == 10);
            assert(recovery.get("0123456789abcdef").favorite);
            fault.fault = {}; assert(store.save(fault));
            ProgressStore restart(path); assert(restart.load());
            assert(restart.get("0123456789abcdef").page == 20);
        }
        const auto path = root / "data" / "only-temp";
        ProgressStore store(path); Progress p; p.page = 7; p.pageCount = 40;
        store.put("0123456789abcdef", p); assert(store.save());
        fs::rename(path, path.string() + ".tmp");
        ProgressStore recovered(path); assert(recovered.load());
        ProgressStore::SaveOptions fault;
        fault.fault = [](const char *op) { return std::string(op) == "temporary_write" ? ENOSPC : 0; };
        assert(!recovered.save(fault));
        fs::remove(path.string() + ".tmp");
        ProgressStore backup(path); assert(backup.load() && backup.get("0123456789abcdef").page == 7);
        assert(recovered.save());
        fault.fault = [](const char *op) { return std::string(op) == "fsync" ? ENOSYS : 0; };
        assert(!recovered.save(fault)); // Unsupported flush is NEVER reported as success.
        assert(recovered.saveDiagnostic().find("operation=fsync") != std::string::npos);
        std::ofstream(root / "not-a-directory") << "keep";
        ProgressStore invalid(root / "not-a-directory" / "save"); assert(!invalid.save());
        assert(invalid.saveDiagnostic().find("create_directories") != std::string::npos);
#ifndef _WIN32
        ProgressStore readonly("/sys/papernx-progress"); assert(!readonly.save());
        assert(readonly.saveDiagnostic().find("operation=open") != std::string::npos);
#endif
    }
    // V4 saves preserve independent per-book settings and the explicit global-default record.
    {
        ProgressStore store(root / "data" / "styles");
        Progress a; a.fontFamily = 2; a.textColor = 0x123456; a.backgroundColor = 0xeeddbb;
        a.paragraphSpacing = 160; a.indentation = 0; a.touchZone = 40; a.publisherStyles = 1;
        a.pdfColorMode = 2; a.pdfViewMode = 1;
        store.put("0123456789abcdef", a); store.put("PAPERNX_DEFAULTS", a);
        Progress b; b.page = 11; b.theme = 5;
        store.put("fedcba9876543210", b); assert(store.save());
        ProgressStore restart(root / "data" / "styles"); assert(restart.load());
        const auto style = restart.get("0123456789abcdef");
        assert(style.textColor == a.textColor && style.backgroundColor == a.backgroundColor);
        assert(style.fontFamily == 2 && style.paragraphSpacing == 160 && style.indentation == 0);
        assert(style.touchZone == 40 && style.publisherStyles == 1 && style.pdfColorMode == 2 && style.pdfViewMode == 1);
        assert(restart.get("PAPERNX_DEFAULTS").textColor == a.textColor);
        assert(restart.get("fedcba9876543210").page == 11 && restart.get("fedcba9876543210").theme == 5);
    }
    tracker.onFingerDown(.25f, .5f, 1);
    assert(tracker.onFingerUp(.27f, .51f, 1).type == GestureType::TapLeft);
    tracker.onFingerDown(.75f, .5f, 2);
    assert(tracker.onFingerUp(.74f, .51f, 2).type == GestureType::TapRight);
    fs::remove_all(root);
    std::cout << "[PASS] All PaperNX core, Turkish unicode, and gesture tests succeeded without error.\n";
    return 0;
}

