#include "papernx/core.hpp"
#include "papernx/gestures.hpp"
#include <cassert>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

using namespace papernx;

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

    fs::remove_all(root);
    std::cout << "[PASS] All PaperNX core and gesture tests succeeded without error.\n";
    return 0;
}
