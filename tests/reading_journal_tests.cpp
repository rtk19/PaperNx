#include "papernx/reading_journal.hpp"
#include <cassert>
#include <chrono>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <string>

using namespace papernx;
namespace {
void step(ReadingJournal &j,std::uint64_t ms,std::int64_t utc,const std::string &day,
          bool visible=true,double fraction=0.1) {
    j.tick(ms,utc,day,"book-01","A Book","An Author",fraction,visible);
}
void run(){
    const auto dir=fs::temp_directory_path()/"papernx-journal-unit-v1";
    std::error_code ec;fs::remove_all(dir,ec);
    ReadingJournal journal(dir);
    assert(journal.load());
    assert(journal.sessions().empty());
    journal.interaction(1000);
    step(journal,1000,1792367880,"2026-10-18");
    for(std::uint64_t t=2000;t<=121000;t+=1000)
        step(journal,t,1792367880+static_cast<std::int64_t>(t/1000),"2026-10-18");
    assert(journal.totals().activeMilliseconds==120000);
    assert(journal.totals().sessions==1);
    step(journal,122000,1792368000,"2026-10-18",false);
    assert(journal.save());
    ReadingJournal roundTrip(dir);
    assert(roundTrip.load());
    assert(roundTrip.totals().activeMilliseconds==120000);
    assert(roundTrip.calendar().at("2026-10-18")==120000);
    assert(roundTrip.books().at(0).title=="A Book");
    assert(!roundTrip.sessions().empty());

    // Idleness stops counting and modal/background time is excluded.
    journal.interaction(200000);
    step(journal,200000,1792368000,"2026-10-18");
    step(journal,201000,1792368001,"2026-10-18");
    step(journal,502000,1792368302,"2026-10-18");
    auto before=journal.totals().activeMilliseconds;
    step(journal,803000,1792368603,"2026-10-18");
    assert(journal.totals().activeMilliseconds==before);

    ReadingJournal midnight(dir/"midnight");
    midnight.interaction(0);
    step(midnight,0,1792367880,"2026-10-18");
    for(std::uint64_t i=1000;i<=10000;i+=1000) {
        const auto day=i<=5000?"2026-10-18":"2026-10-19";
        step(midnight,i,1792367880+std::int64_t(i/1000),day);
    }
    midnight.pause(11000,1792367900);
    assert(midnight.calendar().at("2026-10-18")==5000);
    assert(midnight.calendar().at("2026-10-19")==5000);
    assert(midnight.save());

    // Unknown schema is left untouched. Never overwrite with a new format.
    const auto broken=dir/"invalid";
    fs::create_directories(broken);
    {std::ofstream out(broken/"sessions.v1");out<<"PAPERNX-JOURNAL-999\n";}
    ReadingJournal invalid(broken);
    assert(!invalid.load());
    assert(invalid.isReadOnly());
    assert(!invalid.save());

    // 1. Fingerprint & Book Aliasing Tests
    const auto bookFile1 = dir / "dune.epub";
    const auto bookFile2 = dir / "dune_moved.epub";
    {
        std::ofstream f1(bookFile1, std::ios::binary);
        f1 << "PK\x03\x04EPUB-TEST-CONTENT-FOR-FINGERPRINTING-LONG-DATA-FIXTURE";
        std::ofstream f2(bookFile2, std::ios::binary);
        f2 << "PK\x03\x04EPUB-TEST-CONTENT-FOR-FINGERPRINTING-LONG-DATA-FIXTURE";
    }
    const auto size1 = fs::file_size(bookFile1);
    const auto size2 = fs::file_size(bookFile2);
    auto fp1 = computeBookFingerprint(bookFile1, size1);
    auto fp2 = computeBookFingerprint(bookFile2, size2);
    assert(!fp1.empty());
    assert(fp1 == fp2);

    ReadingJournal aliasJournal(dir / "alias_test");
    aliasJournal.registerBookAlias("path-id-1", fp1, "Dune", "Frank Herbert");
    assert(aliasJournal.canonicalId("path-id-1") == "path-id-1");

    // Moving book to path-id-2 with same fingerprint maps to canonical path-id-1
    aliasJournal.registerBookAlias("path-id-2", fp2, "Dune", "Frank Herbert");
    assert(aliasJournal.canonicalId("path-id-2") == "path-id-1");

    // Add session for path-id-2
    aliasJournal.interaction(1000);
    aliasJournal.tick(1000, 1792367880, "2026-10-18", "path-id-2", "Dune", "Frank Herbert", 0.5, true);
    for (std::uint64_t t = 2000; t <= 61000; t += 1000)
        aliasJournal.tick(t, 1792367880 + std::int64_t(t / 1000), "2026-10-18", "path-id-2", "Dune", "Frank Herbert", 0.5, true);
    aliasJournal.pause(62000, 1792367942);
    assert(aliasJournal.save());

    // Both path-id-1 and path-id-2 resolve to the same book statistics
    assert(aliasJournal.bookMilliseconds("path-id-1") == 60000);
    assert(aliasJournal.bookMilliseconds("path-id-2") == 60000);
    assert(aliasJournal.books().size() == 1);
    assert(aliasJournal.books()[0].id == "path-id-1");

    // Reload and verify aliases persisted
    ReadingJournal aliasReload(dir / "alias_test");
    assert(aliasReload.load());
    assert(aliasReload.canonicalId("path-id-2") == "path-id-1");
    assert(aliasReload.bookMilliseconds("path-id-1") == 60000);

    // 2. Session Deletion & Recalculation Tests
    assert(!aliasReload.sessions().empty());
    auto sid = aliasReload.sessions()[0].id;
    assert(aliasReload.totals().activeMilliseconds == 60000);
    assert(aliasReload.deleteSession(sid));
    assert(aliasReload.totals().activeMilliseconds == 0);
    const auto remCal = aliasReload.calendar();
    assert(remCal.find("2026-10-18") == remCal.end() || remCal.at("2026-10-18") == 0);
    assert(aliasReload.books().empty());
    assert(!aliasReload.deleteSession(99999)); // non-existent session

    // 3. Export JSON & CSV Tests
    ReadingJournal exportJournal(dir / "export_test");
    exportJournal.interaction(1000);
    exportJournal.setDailyGoalMinutes(30);
    exportJournal.setIdleMinutes(5);
    step(exportJournal, 1000, 1792367880, "2026-10-18");
    for (std::uint64_t t = 2000; t <= 61000; t += 1000)
        step(exportJournal, t, 1792367880 + std::int64_t(t / 1000), "2026-10-18");
    exportJournal.pause(62000, 1792367942);

    const auto jsonPath = dir / "export_test" / "export" / "test.json";
    const auto csvPath = dir / "export_test" / "export" / "test.csv";
    assert(exportJournal.exportJson(jsonPath));
    assert(exportJournal.exportCsv(csvPath));
    assert(fs::exists(jsonPath));
    assert(fs::exists(csvPath));

    std::ifstream jsonIn(jsonPath);
    std::string jsonContent((std::istreambuf_iterator<char>(jsonIn)), std::istreambuf_iterator<char>());
    assert(jsonContent.find("PaperNX Reading Journal") != std::string::npos);
    assert(jsonContent.find("A Book") != std::string::npos);

    std::ifstream csvIn(csvPath);
    std::string csvContent((std::istreambuf_iterator<char>(csvIn)), std::istreambuf_iterator<char>());
    assert(csvContent.find("id,bookId,title,author") != std::string::npos);
    assert(csvContent.find("A Book") != std::string::npos);

    // 4. Period Aggregations & Honest Trend Tests
    auto weekStats = exportJournal.calculatePeriodStats(0, 0, false);
    assert(weekStats.barValues.size() == 7);
    assert(weekStats.daysRead >= 1);
    // Honest trend: no previous period -> no synthetic +18%
    assert(!weekStats.hasPreviousPeriod);
    assert(weekStats.trendText == "Önceki hafta verisi yok");

    auto monthStats = exportJournal.calculatePeriodStats(1, 0, false);
    assert(!monthStats.barValues.empty());

    auto yearStats = exportJournal.calculatePeriodStats(2, 0, false);
    assert(yearStats.barValues.size() == 12);

    // 5. Idle setting bounds
    exportJournal.setIdleMinutes(2);
    assert(exportJournal.idleMinutes() == 2);
    exportJournal.setIdleMinutes(10);
    assert(exportJournal.idleMinutes() == 10);
    exportJournal.setIdleMinutes(7); // invalid, must remain 10
    assert(exportJournal.idleMinutes() == 10);

    fs::remove_all(dir,ec);
}
}
int main(){run();std::cout<<"reading journal portable tests OK\n";}
