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
    fs::remove_all(dir,ec);
}
}
int main(){run();std::cout<<"reading journal portable tests OK\n";}
