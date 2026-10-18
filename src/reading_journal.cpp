#include "papernx/reading_journal.hpp"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <limits>
#include <sstream>
#include <system_error>
#include <utility>

namespace papernx {
namespace {
constexpr const char *magic="PAPERNX-JOURNAL-1";
constexpr std::size_t maxSessions=100000, maxLine=8192, maxString=1024;
constexpr std::uint64_t maxMillis=100ULL*365*24*60*60*1000;
std::uint64_t checksum(const std::string &s) {
    std::uint64_t h=14695981039346656037ULL;
    for(unsigned char c:s){h^=c;h*=1099511628211ULL;}
    return h;
}
std::string hex(const std::string &s) {
    static constexpr char alphabet[]="0123456789abcdef";
    std::string out;out.reserve(s.size()*2);
    for(unsigned char c:s){out+=alphabet[c>>4];out+=alphabet[c&15];}
    return out;
}
int digit(char c) {
    if(c>='0'&&c<='9')return c-'0';
    if(c>='a'&&c<='f')return 10+c-'a';
    if(c>='A'&&c<='F')return 10+c-'A';
    return -1;
}
bool unhex(const std::string &s,std::string &out) {
    if(s.size()%2 || s.size()>maxString*2)return false;
    out.clear();out.reserve(s.size()/2);
    for(std::size_t i=0;i<s.size();i+=2){
        const int a=digit(s[i]),b=digit(s[i+1]);
        if(a<0||b<0)return false;
        out.push_back(char((a<<4)|b));
    }
    return true;
}
bool validDay(const std::string &day) {
    if(day.size()!=10||day[4]!='-'||day[7]!='-')return false;
    for(std::size_t i=0;i<day.size();++i)
        if(i!=4&&i!=7&&(day[i]<'0'||day[i]>'9'))return false;
    int m=(day[5]-'0')*10+day[6]-'0';
    int d=(day[8]-'0')*10+day[9]-'0';
    return m>=1&&m<=12&&d>=1&&d<=31;
}
std::vector<std::string> split(const std::string &s,char sep) {
    std::vector<std::string> out;std::size_t from=0;
    while(true){auto at=s.find(sep,from);out.push_back(s.substr(from,at==std::string::npos?at:at-from));
        if(at==std::string::npos)break;from=at+1;}
    return out;
}
template<typename T> bool number(const std::string &s,T &v) {
    if(s.empty())return false;
    std::istringstream in(s);in>>v;
    return !in.fail() && in.peek()==std::char_traits<char>::eof();
}
std::string signedLine(const std::string &body) {
    return body+"\t"+std::to_string(checksum(body))+"\n";
}
bool checkedLine(const std::string &line,std::string &body) {
    auto p=line.rfind('\t');if(p==std::string::npos)return false;
    body=line.substr(0,p);std::uint64_t digest=0;
    return number(line.substr(p+1),digest)&&digest==checksum(body);
}
std::string dayWire(const std::map<std::string,std::uint64_t> &days) {
    std::string result;
    for(const auto &e:days){if(!result.empty())result+=';';result+=e.first+'='+std::to_string(e.second);}
    return result;
}
bool parseDays(const std::string &s,std::map<std::string,std::uint64_t> &days) {
    if(s.empty())return true;
    for(const auto &part:split(s,';')){
        auto p=part.find('=');if(p==std::string::npos)return false;
        const auto day=part.substr(0,p);std::uint64_t ms=0;
        if(!validDay(day)||!number(part.substr(p+1),ms)||ms>maxMillis||!days.emplace(day,ms).second)return false;
    }
    return true;
}
}
bool ReadingJournal::validSession(const JournalSession &s) const {
    if(!s.id||s.bookId.empty()||s.bookId.size()>maxString||s.title.size()>maxString||
       s.author.size()>maxString||s.startedUtc<0||s.endedUtc<s.startedUtc||
       s.activeMilliseconds>maxMillis||!std::isfinite(s.startProgress)||
       !std::isfinite(s.endProgress)||s.startProgress<0||s.startProgress>1||
       s.endProgress<0||s.endProgress>1||s.days.size()>36600)return false;
    std::uint64_t sum=0;
    for(const auto &day:s.days){if(!validDay(day.first)||day.second>maxMillis||
        sum>maxMillis-day.second)return false;sum+=day.second;}
    return sum==s.activeMilliseconds;
}
void ReadingJournal::interaction(std::uint64_t nowMs) {
    lastInteraction_=nowMs;interactionReady_=true;
}
void ReadingJournal::setIdleMinutes(unsigned n) {
    if(n==2||n==5||n==10){idleMinutes_=n;dirty_=true;}
}
void ReadingJournal::setDailyGoalMinutes(unsigned n) {
    if(n<=720){dailyGoalMinutes_=n;dirty_=true;}
}
void ReadingJournal::finish(std::int64_t epochSeconds) {
    if(!sessions_.empty()&&clockReady_) {
        auto &last=sessions_.back();
        last.endedUtc=std::max(last.startedUtc,epochSeconds);
        if(last.activeMilliseconds<5000)sessions_.pop_back();
        else dirty_=true;
    }
    clockReady_=false;
}
void ReadingJournal::pause(std::uint64_t nowMs,std::int64_t epochSeconds) {
    (void)nowMs;finish(epochSeconds);
    interactionReady_=false;
}
void ReadingJournal::tick(std::uint64_t nowMs,std::int64_t epochSeconds,const std::string &day,
                          const std::string &bookId,const std::string &title,const std::string &author,
                          double progress,bool visible) {
    const bool valid=visible&&!bookId.empty()&&bookId.size()<=maxString&&
        title.size()<=maxString&&author.size()<=maxString&&validDay(day)&&
        std::isfinite(progress)&&progress>=0&&progress<=1&&epochSeconds>=0;
    const bool awake=interactionReady_&&nowMs>=lastInteraction_&&
        nowMs-lastInteraction_<idleMinutes_*60000ULL;
    if(!valid||!awake||readOnly_) { finish(epochSeconds);return; }
    if(clockReady_&&(sessions_.empty()||sessions_.back().bookId!=bookId))finish(epochSeconds);
    if(!clockReady_) {
        if(sessions_.size()>=maxSessions){diagnostic_="Journal session limit";return;}
        JournalSession s;s.id=nextId_++;s.bookId=bookId;s.title=title;s.author=author;
        s.startedUtc=s.endedUtc=epochSeconds;s.startProgress=s.endProgress=progress;
        sessions_.push_back(std::move(s));lastTick_=nowMs;clockReady_=true;dirty_=true;return;
    }
    auto &s=sessions_.back();
    const auto delta=nowMs>=lastTick_?nowMs-lastTick_:0;
    if(delta<=3000 && delta<=maxMillis-s.activeMilliseconds) {
        // Attribute only measured active time, not sleep or clock jumps. The caller
        // supplies the current local day; frequent ticks bound midnight granularity.
        s.activeMilliseconds+=delta;s.days[day]+=delta;
        if(delta)dirty_=true;
    }
    s.endedUtc=std::max(s.startedUtc,epochSeconds);
    s.endProgress=progress;lastTick_=nowMs;
}
JournalTotals ReadingJournal::totals() const {
    JournalTotals t;const auto days=calendar();t.readDays=0;
    for(const auto &d:days)if(d.second>=60000)++t.readDays;
    for(const auto &s:sessions_){t.activeMilliseconds+=s.activeMilliseconds;
        if(s.activeMilliseconds>=5000)++t.sessions;}
    return t;
}
std::map<std::string,std::uint64_t> ReadingJournal::calendar() const {
    std::map<std::string,std::uint64_t> days;
    for(const auto &s:sessions_)for(const auto &day:s.days)days[day.first]+=day.second;
    return days;
}
std::vector<JournalBookTotal> ReadingJournal::books() const {
    std::map<std::string,JournalBookTotal> acc;
    for(const auto &s:sessions_) {
        auto &b=acc[s.bookId];b.id=s.bookId;
        if(s.endedUtc>=b.lastRead){b.title=s.title;b.author=s.author;b.lastRead=s.endedUtc;b.progress=s.endProgress;}
        b.activeMilliseconds+=s.activeMilliseconds;
        if(s.activeMilliseconds>=5000)++b.sessions;
    }
    std::vector<JournalBookTotal> items;
    for(auto &kv:acc)items.push_back(std::move(kv.second));
    std::sort(items.begin(),items.end(),[](const JournalBookTotal &a,const JournalBookTotal &b){
        return a.lastRead!=b.lastRead?a.lastRead>b.lastRead:a.id<b.id;});
    return items;
}
std::uint64_t ReadingJournal::bookMilliseconds(const std::string &bookId) const {
    std::uint64_t ms=0;for(const auto &s:sessions_)if(s.bookId==bookId)ms+=s.activeMilliseconds;
    return ms;
}
bool ReadingJournal::save() {
    if(readOnly_){diagnostic_="Journal read-only after corrupt input";return false;}
    if(!dirty_)return true;
    std::error_code ec;fs::create_directories(directory_,ec);
    if(ec){diagnostic_="Cannot create journal directory";return false;}
    const auto path=directory_/"sessions.v1";
    const auto tmp=directory_/"sessions.v1.tmp",bak=directory_/"sessions.v1.bak";
    std::ofstream out(tmp,std::ios::binary|std::ios::trunc);
    if(!out){diagnostic_="Cannot write journal checkpoint";return false;}
    out<<magic<<"\n"<<signedLine("P\t"+std::to_string(idleMinutes_)+"\t"+std::to_string(dailyGoalMinutes_));
    for(const auto &s:sessions_){
        if(!validSession(s)){diagnostic_="Invalid journal record";return false;}
        std::ostringstream f;f<<std::setprecision(16);
        f<<"S\t"<<s.id<<"\t"<<hex(s.bookId)<<"\t"<<hex(s.title)<<"\t"<<hex(s.author)
         <<"\t"<<s.startedUtc<<"\t"<<s.endedUtc<<"\t"<<s.activeMilliseconds
         <<"\t"<<s.startProgress<<"\t"<<s.endProgress<<"\t"<<dayWire(s.days);
        const auto line=signedLine(f.str());
        if(line.size()>maxLine){diagnostic_="Journal record too large";return false;}
        out<<line;
    }
    out.flush();if(!out){diagnostic_="Journal write failed";return false;}
    out.close();if(!out){diagnostic_="Journal close failed";return false;}
    bool old=fs::exists(path,ec);
    if(old){
        fs::remove(bak,ec);ec.clear();
        fs::rename(path,bak,ec);
        if(ec){diagnostic_="Cannot rotate journal backup";return false;}
    }
    fs::rename(tmp,path,ec);
    if(ec){
        if(old){std::error_code recover;fs::rename(bak,path,recover);}
        diagnostic_="Cannot publish journal checkpoint";return false;
    }
    dirty_=false;diagnostic_.clear();return true;
}
bool ReadingJournal::load() {
    const auto path=directory_/"sessions.v1";
    std::error_code ec;fs::path source=path;
    if(!fs::exists(source,ec)) {
        const auto bak=directory_/"sessions.v1.bak";
        if(fs::exists(bak,ec))source=bak;
        else {diagnostic_.clear();return true;}
    }
    std::ifstream in(source,std::ios::binary);
    if(!in){diagnostic_="Cannot read journal";readOnly_=true;return false;}
    std::string line;
    if(!std::getline(in,line)||line!=magic){diagnostic_="Unknown journal version";readOnly_=true;return false;}
    if(!std::getline(in,line)||line.size()>maxLine){diagnostic_="Invalid journal preferences";readOnly_=true;return false;}
    std::string body;if(!checkedLine(line,body)){diagnostic_="Invalid journal header checksum";readOnly_=true;return false;}
    auto pref=split(body,'\t');unsigned idle=0,goal=0;
    if(pref.size()!=3||pref[0]!="P"||!number(pref[1],idle)||!number(pref[2],goal)||
       (idle!=2&&idle!=5&&idle!=10)||goal>720){diagnostic_="Invalid journal preferences";readOnly_=true;return false;}
    std::vector<JournalSession> loaded;std::uint64_t maxId=0;
    while(std::getline(in,line)){
        if(line.empty()||line.size()>maxLine||!checkedLine(line,body)){readOnly_=true;break;}
        auto fields=split(body,'\t');JournalSession s;
        if(fields.size()!=11||fields[0]!="S"||!number(fields[1],s.id)||
           !unhex(fields[2],s.bookId)||!unhex(fields[3],s.title)||
           !unhex(fields[4],s.author)||!number(fields[5],s.startedUtc)||
           !number(fields[6],s.endedUtc)||!number(fields[7],s.activeMilliseconds)||
           !number(fields[8],s.startProgress)||!number(fields[9],s.endProgress)||
           !parseDays(fields[10],s.days)||!validSession(s)||s.id<=maxId||
           loaded.size()>=maxSessions){readOnly_=true;break;}
        maxId=s.id;loaded.push_back(std::move(s));
    }
    sessions_=std::move(loaded);nextId_=maxId+1;idleMinutes_=idle;dailyGoalMinutes_=goal;
    clockReady_=false;interactionReady_=false;dirty_=false;
    if(readOnly_){diagnostic_="Corrupt journal tail: intact records recovered in read-only mode";return false;}
    diagnostic_.clear();return true;
}
} // namespace papernx
