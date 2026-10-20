#include "papernx/catalog.hpp"
#include "papernx/series.hpp"
#include "papernx/reading_stats.hpp"
#include <cassert>
#include <cerrno>
#include <chrono>
#include <fstream>
#include <iostream>
using namespace papernx;
int main(){
    assert(naturalCompare("Issue 9","Issue 10")<0);
    assert(naturalCompare("Issue 009","Issue 9")==0);
    assert(naturalCompare("Issue 999999999999999999999999","Issue 1000000000000000000000000")<0);
    assert(naturalCompare("ספר 9","ספר 10")<0);
    const auto root=fs::temp_directory_path()/("papernx-catalog-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    fs::create_directories(root/"books");fs::create_directories(root/"outside");
    {std::ofstream out(root/"books"/"one.epub");out<<"disposable test copy";}
    {std::ofstream out(root/"books"/"two.pdf");out<<"disposable test PDF";}
    auto books=scan(root/"books");assert(books.size()==2);const auto first=books.front(),second=books.back();
    ProgressStore progress(root/"progress.db");Progress original;original.page=9;original.fraction=.37;original.bookmarks.push_back({9,"İstanbul",123,-1});progress.put(first.id,original);assert(progress.save());
    CatalogStore catalog(root/"catalog.db");assert(!catalog.load());
    const auto shelf=catalog.createCollection("Türkçe Edebiyat");assert(shelf.size()==16);
    assert(catalog.createCollection("TÜRKÇE EDEBİYAT")==shelf);
    const auto shelf2=catalog.createCollection("Okunacaklar");assert(!shelf2.empty());
    auto e=catalog.get(first.id);e.title="İnce Memed";e.author="Yaşar Kemal";e.genre="Roman";e.series="Çukurova";e.tags="Türkçe: tarih\töykü";e.status=ReadingStatus::Completed;
    assert(catalog.put(first.id,e));assert(catalog.addToCollection(first.id,shelf));assert(catalog.addToCollection(first.id,shelf2));
    assert(catalog.addToCollection(first.id,shelf));assert(catalog.get(first.id).collections.size()==2);
    assert(!catalog.addToCollection(first.id,"missing"));assert(catalog.save());
    CatalogStore restored(root/"catalog.db");assert(restored.load());assert(restored.get(first.id).tags==e.tags);assert(restored.get(first.id).title==e.title);assert(restored.get(first.id).status==ReadingStatus::Completed);
    assert(restored.get(first.id).collections.size()==2);assert(restored.apply(first).author==e.author);
    auto cached=restored.get(second.id);cached.sourceTitle="Embedded title";cached.sourceAuthor="Embedded author";cached.sourceGenre="Essays";cached.sourceSeries="Series";cached.sourceSeriesIndex="2.5";cached.metadataFingerprint=std::to_string(second.bytes)+":"+std::to_string(second.modified);
    assert(restored.put(second.id,cached));assert(restored.apply(second).title=="Embedded title");
    auto changed=second;++changed.modified;assert(restored.apply(changed).title==second.title);
    assert(restored.filter(books,CatalogGroup::Formats,"PDF").size()==1);
    cached={};assert(restored.put(second.id,cached));
    assert(restored.groups(books,CatalogGroup::Authors).at(e.author)==1);
    assert(restored.filter(books,CatalogGroup::Genres,"Roman").size()==1);
    assert(restored.filter(books,CatalogGroup::Collections,shelf).size()==1);
    // Same displayed title never conflates separate path identities.
    auto sameTitle=restored.get(second.id);sameTitle.title=e.title;assert(restored.put(second.id,sameTitle));assert(restored.visible(books).size()==2);
    e=restored.get(first.id);e.hidden=true;assert(restored.put(first.id,e));assert(restored.save());
    CatalogStore hidden(root/"catalog.db");assert(hidden.load());assert(hidden.visible(scan(root/"books")).size()==1);assert(fs::exists(first.path));
    assert(hidden.renameCollection(shelf,"Romanlar"));assert(!hidden.renameCollection(shelf,"Okunacaklar"));
    assert(hidden.deleteCollection(shelf));assert(hidden.get(first.id).collections.size()==1);assert(fs::exists(first.path));
    assert(hidden.removeFromCollection(first.id,shelf2));assert(hidden.get(first.id).collections.empty());assert(hidden.save());
    // Storage failure returns false and retains the last verified catalog.
    auto broken=hidden.get(second.id);broken.author="must not survive failed write";assert(hidden.put(second.id,broken));
    ProgressStore::SaveOptions readOnly;readOnly.fault=[](const char *op){return std::string(op)=="temporary_write"?EROFS:0;};
    assert(!hidden.save(readOnly));assert(hidden.diagnostic().find("errno=")!=std::string::npos);
    CatalogStore intact(root/"catalog.db");assert(intact.load());assert(intact.get(second.id).author.empty());
    // Checksum corruption / interrupted write recovers an earlier safe backup.
    {std::ofstream out(root/"catalog.db",std::ios::trunc);out<<"interrupted";}
    {std::ofstream out(root/"catalog.db.tmp",std::ios::trunc);out<<"interrupted";}
    CatalogStore recovered(root/"catalog.db");assert(recovered.load());
    // Catalog mutation never rewrites positions or bookmarks.
    ProgressStore progressAgain(root/"progress.db");assert(progressAgain.load());assert(progressAgain.get(first.id).page==9);assert(progressAgain.get(first.id).bookmarks[0].title=="İstanbul");
    std::string error;
    assert(catalog.validateDeletion(first,root/"books",error));
    assert(!catalog.deleteBookFile(first,root/"books",false,error));assert(fs::exists(first.path));
    assert(!catalog.validateDeletion(first,root/"outside",error));
    auto spoof=first;spoof.id=second.id;assert(!catalog.validateDeletion(spoof,root/"books",error));
    spoof=first;spoof.path=(root/"books").string();spoof.format=Format::Epub;assert(!catalog.validateDeletion(spoof,root/"books",error));
    spoof=first;spoof.path=(root/"progress.db").string();assert(!catalog.validateDeletion(spoof,root,error));
    {std::ofstream out(root/"outside"/"valuable.epub");out<<"never delete me";}
    std::error_code symlinkError;fs::create_symlink(root/"outside"/"valuable.epub",root/"books"/"link.epub",symlinkError);
    if(!symlinkError){spoof=first;spoof.path=(root/"books"/"link.epub").string();spoof.bytes=fs::file_size(spoof.path);spoof.id=documentId(spoof.path,spoof.bytes);assert(!catalog.validateDeletion(spoof,root/"books",error));}
    // Changed contents invalidate selection even though a suffix remains EPUB.
    {std::ofstream out(second.path,std::ios::app);out<<"changed";}
    assert(!catalog.validateDeletion(second,root/"books",error));
    assert(catalog.deleteBookFile(first,root/"books",true,error));assert(!fs::exists(first.path));assert(fs::exists(root/"outside"/"valuable.epub"));
    // Explicit status commands have actual position semantics. Journal/history,
    // bookmarks, and per-book styles must not be destroyed.
    Progress marked;marked.pageCount=20;marked.page=6;marked.fraction=.30;
    marked.bookmarks.push_back({6,"Resume point",123,-1});
    marked.theme=4;marked.styleOverride=1;
    applyManualReadingStatus(marked,ReadingStatus::NotStarted,1000);
    assert(marked.page==0 && marked.fraction==0.0 && marked.lastRead==0);
    assert(marked.bookmarks.size()==1 && marked.theme==4 && marked.styleOverride==1);
    marked.page=6;marked.fraction=.30;
    applyManualReadingStatus(marked,ReadingStatus::Reading,1001);
    assert(marked.page==6 && marked.fraction==.30);
    applyManualReadingStatus(marked,ReadingStatus::Completed,1002);
    assert(marked.fraction==1.0 && marked.page==19);
    applyManualReadingStatus(marked,ReadingStatus::Reading,1003);
    assert(marked.fraction==0.0 && marked.page==0);
    assert(marked.bookmarks.size()==1 && marked.theme==4);
    // Conservative real activity, menu/idle/suspend exclusion and evidence gate.
    ReadingStatsStore stats(root/"stats.db");assert(!stats.load());assert(stats.setDailyGoal(20));assert(!stats.setDailyGoal(-1));assert(!stats.setDailyGoal(721));
    stats.interaction(0);stats.tick(0,first.id,0,true,"2026-10-18");
    for(std::uint64_t now=1000;now<=150000;now+=1000){if(now%30000==0)stats.interaction(now);stats.tick(now,first.id,now/5000000.0,true,"2026-10-18");}
    assert(stats.get(first.id).activeMilliseconds==150000);assert(stats.get(first.id).sessions==1);assert(stats.remainingMinutes(first.id,.03));
    assert(!stats.remainingMinutes(second.id,.5));assert(stats.dayMilliseconds("2026-10-18")==150000);
    stats.tick(151000,first.id,.03,false,"2026-10-18");stats.tick(160000,first.id,.03,false,"2026-10-18");assert(stats.totalMilliseconds()==150000);
    stats.interaction(160001);stats.tick(160001,first.id,.03,true,"2026-10-18");stats.tick(161001,first.id,.03,true,"2026-10-18");assert(stats.totalMilliseconds()==151000);
    stats.tick(500000,first.id,.5,true,"2026-10-18");assert(stats.totalMilliseconds()==151000); // sleep gap excluded
    stats.interaction(500000);stats.tick(500001,first.id,.5,true,"2026-10-18");stats.tick(501001,first.id,.5,true,"2026-10-18");assert(stats.totalMilliseconds()==152000);
    stats.tick(502001,first.id,.5,true,"2026-10-19");assert(stats.dayMilliseconds("2026-10-19")==1000);
    assert(stats.save());ReadingStatsStore statsAgain(root/"stats.db");assert(statsAgain.load());assert(statsAgain.dailyGoal()==20);assert(statsAgain.totalMilliseconds()==153000);assert(statsAgain.get(first.id).lastReadingDay=="2026-10-19");
    // Resuming from persistent data never credits time between launches.
    statsAgain.tick(999999999,first.id,.5,true,"2026-10-19");assert(statsAgain.totalMilliseconds()==153000);
    ProgressStore future(root/"future-catalog.db");Progress futureMarker;futureMarker.bookTitle="PAPERNX-CATALOG-99";future.put("catalog000000001",futureMarker);assert(future.save());
    CatalogStore futureCatalog(root/"future-catalog.db");assert(!futureCatalog.load());assert(!futureCatalog.save());
    Book issue9{"0000000000000009","/books/The Walking Dead Deluxe 009 (2024).cbr","The Walking Dead Deluxe 009 (2024)","",Format::Cbr};
    Book issue10{"0000000000000010","/books/The Walking Dead Deluxe 010 (2024).cbr","The Walking Dead Deluxe 010 (2024)","",Format::Cbr};
    auto series9=identifySeries(issue9,{}),series10=identifySeries(issue10,{});
    assert(series9.key==series10.key && series9.title=="The Walking Dead Deluxe" && series9.issue=="009");
    auto capitals=issue10;capitals.title="THE WALKING DEAD DELUXE 010";assert(identifySeries(capitals,{}).key==series9.key);
    assert(seriesIssueLess(issue9,issue10,catalog));
    auto metadata=catalog.get(issue10.id);metadata.sourceSeries="Official Series";metadata.sourceSeriesIndex="0.5";metadata.metadataFingerprint="0:0:series-v1";assert(catalog.put(issue10.id,metadata));
    metadata=catalog.get(issue9.id);metadata.sourceSeries="Official Series";metadata.sourceSeriesIndex="9";metadata.metadataFingerprint="0:0";assert(catalog.put(issue9.id,metadata));
    assert(seriesIssueLess(issue10,issue9,catalog));
    metadata=catalog.get(issue10.id);metadata.sourceSeriesIndex="10A";assert(catalog.put(issue10.id,metadata));
    metadata=catalog.get(issue9.id);metadata.sourceSeriesIndex="11";assert(catalog.put(issue9.id,metadata));
    assert(seriesIssueLess(issue10,issue9,catalog));
    metadata.series="My edited series";assert(identifySeries(issue9,metadata).title=="My edited series");
    Book standalone{"book","1984.epub","1984","",Format::Epub};assert(identifySeries(standalone,{}).key.empty());
    auto naturallySorted=search({issue10,issue9},"",Sort::Title);assert(naturallySorted.front().id==issue9.id);
    fs::remove_all(root);
    std::cout<<"Catalog, safe deletion, recovery, Unicode, progress isolation, reading stats and goal tests passed\n";
    return 0;
}
