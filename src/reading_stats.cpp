#include "papernx/reading_stats.hpp"
#include <algorithm>
#include <cmath>
#include <limits>

namespace papernx {
namespace {
bool validDay(const std::string &day) {
    if(day.size()!=10||day[4]!='-'||day[7]!='-')return false;
    for(std::size_t i=0;i<day.size();++i)if(i!=4&&i!=7&&(day[i]<'0'||day[i]>'9'))return false;
    const int month=(day[5]-'0')*10+day[6]-'0',date=(day[8]-'0')*10+day[9]-'0';
    return month>=1&&month<=12&&date>=1&&date<=31;
}
constexpr const char *schema="readingstats0001";
constexpr std::int64_t limit=100LL*365*24*60*60*1000;
}
BookReadingStats ReadingStatsStore::get(const std::string &id)const{auto it=books_.find(id);return it==books_.end()?BookReadingStats{}:it->second;}
void ReadingStatsStore::interaction(std::uint64_t now){lastActivity_=now;hasActivity_=true;}
void ReadingStatsStore::tick(std::uint64_t now,const std::string&id,double fraction,bool visible,const std::string&day){
    const bool valid=visible&&id.size()==16&&std::isfinite(fraction)&&fraction>=0&&fraction<=1&&validDay(day);
    const bool awake=hasActivity_&&now>=lastActivity_&&now-lastActivity_<120000;
    if(!initialized_){lastTick_=now;initialized_=true;}
    const auto delta=now>=lastTick_?now-lastTick_:0;lastTick_=now;
    const bool same=id==currentBook_;
    if(valid&&awake&&same&&active_&&delta<=3000){
        auto &b=books_[id];
        if(b.activeMilliseconds<limit&&days_[day]<limit){b.activeMilliseconds+=static_cast<std::int64_t>(delta);days_[day]+=static_cast<std::int64_t>(delta);}
        // Only small forward movement gives pace evidence. Jump/history and
        // relayout changes cannot contribute a book-sized instantaneous gain.
        const double gain=fraction-std::max(lastFraction_,b.furthestFraction);
        if(!skipNextSample_&&gain>0.000001&&gain<=0.03){b.measuredFraction=std::min(1.0,b.measuredFraction+gain);++b.progressSamples;}
        b.lastReadingDay=day;
    }
    const bool nextActive=valid&&awake;
    if(nextActive&&(!active_||!same)){auto &b=books_[id];if(b.sessions<std::numeric_limits<int>::max())++b.sessions;}
    if(valid)books_[id].furthestFraction=std::max(books_[id].furthestFraction,fraction);
    skipNextSample_=false;
    active_=nextActive;currentBook_=valid?id:std::string{};lastFraction_=std::isfinite(fraction)?fraction:0;
}
std::int64_t ReadingStatsStore::totalMilliseconds()const{std::int64_t total=0;for(const auto &b:books_)total+=b.second.activeMilliseconds;return total;}
std::int64_t ReadingStatsStore::dayMilliseconds(const std::string&day)const{auto it=days_.find(day);return it==days_.end()?0:it->second;}
bool ReadingStatsStore::setDailyGoal(int minutes){if(minutes<0||minutes>720)return false;dailyGoal_=minutes;return true;}
std::optional<int> ReadingStatsStore::remainingMinutes(const std::string&id,double fraction)const{
    const auto b=get(id);if(!std::isfinite(fraction)||fraction<0||fraction>1||b.activeMilliseconds<120000||b.progressSamples<3||b.measuredFraction<0.02)return {};
    const double minutes=(1-fraction)*static_cast<double>(b.activeMilliseconds)/(b.measuredFraction*60000.0);
    if(minutes>100000)return {};
    return static_cast<int>(std::ceil(minutes));
}
bool ReadingStatsStore::save()const{
    if(!writable_)return false;
    ProgressStore store(path_);Progress marker;marker.bookTitle="PAPERNX-STATS-1";marker.page=dailyGoal_;store.put(schema,marker);
    for(const auto &entry:books_){const auto &b=entry.second;Progress p;p.chapter=1;p.bookPath=entry.first;p.lastRead=b.activeMilliseconds;p.page=b.sessions;p.fraction=b.measuredFraction;p.pageCount=b.progressSamples;p.bookTitle=b.lastReadingDay;p.anchorOffset=b.furthestFraction;store.put(documentId("statistics/book/"+entry.first,0),p);}
    for(const auto &day:days_){Progress p;p.chapter=2;p.bookTitle=day.first;p.lastRead=day.second;store.put(documentId("statistics/day/"+day.first,0),p);}
    return store.save();
}
bool ReadingStatsStore::load(){
    ProgressStore store(path_);if(!store.load()){std::error_code ec;for(const auto *suffix:{"",".bak",".bak.old",".tmp"})if(fs::exists(path_.string()+suffix,ec)){writable_=false;break;}return false;}
    if(store.get(schema).bookTitle!="PAPERNX-STATS-1"){writable_=false;return false;}
    writable_=false;
    std::map<std::string,BookReadingStats> books;std::map<std::string,std::int64_t> days;
    const int goal=store.get(schema).page;if(goal<0||goal>720)return false;
    for(const auto &r:store.all()){const auto &p=r.second;
        if(p.chapter==1){if(p.bookPath.size()!=16||p.lastRead<0||p.lastRead>limit||p.fraction<0||p.fraction>1||p.anchorOffset<0||p.anchorOffset>1||(!p.bookTitle.empty()&&!validDay(p.bookTitle)))return false;
            books[p.bookPath]={p.lastRead,p.page,p.fraction,p.pageCount,p.bookTitle,p.anchorOffset};}
        if(p.chapter==2){if(!validDay(p.bookTitle)||p.lastRead<0||p.lastRead>limit)return false;days[p.bookTitle]=p.lastRead;}}
    books_.swap(books);days_.swap(days);dailyGoal_=goal;writable_=true;initialized_=active_=hasActivity_=false;return true;
}
} // namespace papernx
