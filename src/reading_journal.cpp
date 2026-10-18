#include "papernx/reading_journal.hpp"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <ctime>
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
    while(true){
        auto at=s.find(sep,from);
        out.push_back(s.substr(from,at==std::string::npos?at:at-from));
        if(at==std::string::npos)break;
        from=at+1;
    }
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
std::tm nowDate() {
    const auto n=std::time(nullptr);const auto *ptr=std::localtime(&n);
    return ptr?*ptr:std::tm{};
}
int daysInMonth(int year,int month) {
    std::tm d{};d.tm_year=year-1900;d.tm_mon=month+1;d.tm_mday=0;
    std::mktime(&d);return d.tm_mday;
}
std::string jsonEscape(const std::string &s) {
    std::string out;out.reserve(s.size()+8);
    for(char c:s){
        if(c=='"')out+="\\\"";
        else if(c=='\\')out+="\\\\";
        else if(c=='\b')out+="\\b";
        else if(c=='\f')out+="\\f";
        else if(c=='\n')out+="\\n";
        else if(c=='\r')out+="\\r";
        else if(c=='\t')out+="\\t";
        else if(static_cast<unsigned char>(c)<0x20){
            char b[8];std::snprintf(b,sizeof(b),"\\u%04x",static_cast<unsigned char>(c));out+=b;
        }else out+=c;
    }
    return out;
}
std::string csvEscape(const std::string &s) {
    if(s.find_first_of(",\"\n\r")==std::string::npos)return s;
    std::string out="\"";
    for(char c:s){if(c=='"')out+="\"\"";else out+=c;}
    out+="\"";
    return out;
}
}
bool ReadingJournal::validSession(const JournalSession &s) const {
    if(!s.id||s.bookId.empty()||s.bookId.size()>maxString||s.title.size()>maxString||
       s.author.size()>maxString||s.startedUtc<0||s.endedUtc<s.startedUtc||
       s.activeMilliseconds>maxMillis||!std::isfinite(s.startProgress)||
       !std::isfinite(s.endProgress)||s.startProgress<0||s.startProgress>1||
       s.endProgress<0||s.endProgress>1||s.days.size()>36600)return false;
    std::uint64_t sum=0;
    for(const auto &day:s.days){
        if(!validDay(day.first)||day.second>maxMillis||sum>maxMillis-day.second)
            return false;
        sum+=day.second;
    }
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
        const auto cid = canonicalId(s.bookId);
        auto &b=acc[cid];b.id=cid;
        if(s.endedUtc>=b.lastRead){b.title=s.title;b.author=s.author;b.lastRead=s.endedUtc;b.progress=s.endProgress;}
        b.activeMilliseconds+=s.activeMilliseconds;
        if(s.activeMilliseconds>=5000)++b.sessions;
    }
    for (auto &kv : acc) {
        auto metaIt = canonicalMetadata_.find(kv.first);
        if (metaIt != canonicalMetadata_.end()) {
            if (!metaIt->second.first.empty()) kv.second.title = metaIt->second.first;
            if (!metaIt->second.second.empty()) kv.second.author = metaIt->second.second;
        }
    }
    std::vector<JournalBookTotal> items;
    for(auto &kv:acc)items.push_back(std::move(kv.second));
    std::sort(items.begin(),items.end(),[](const JournalBookTotal &a,const JournalBookTotal &b){
        return a.lastRead!=b.lastRead?a.lastRead>b.lastRead:a.id<b.id;});
    return items;
}
std::uint64_t ReadingJournal::bookMilliseconds(const std::string &bookId) const {
    std::uint64_t ms=0;
    const auto cid = canonicalId(bookId);
    for(const auto &s:sessions_)if(canonicalId(s.bookId)==cid)ms+=s.activeMilliseconds;
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
    saveAliases();
    dirty_=false;diagnostic_.clear();return true;
}
bool ReadingJournal::load() {
    loadAliases();
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

std::string computeBookFingerprint(const fs::path &filePath, std::uintmax_t fileSize) {
    if (fileSize == 0) return {};
    std::ifstream file(filePath, std::ios::binary);
    if (!file) return {};

    char head[4096];
    file.read(head, sizeof(head));
    std::streamsize headBytes = file.gcount();

    char tail[4096];
    std::streamsize tailBytes = 0;
    if (fileSize > sizeof(head)) {
        std::streamoff tailOffset = static_cast<std::streamoff>(fileSize > 4096 ? fileSize - 4096 : 0);
        file.seekg(tailOffset);
        if (file) {
            file.read(tail, sizeof(tail));
            tailBytes = file.gcount();
        }
    }

    std::uint64_t h = 14695981039346656037ULL;
    auto addBytes = [&](const char *data, std::size_t len) {
        for (std::size_t i = 0; i < len; ++i) {
            h ^= static_cast<unsigned char>(data[i]);
            h *= 1099511628211ULL;
        }
    };
    std::string sizeStr = std::to_string(fileSize);
    addBytes(sizeStr.data(), sizeStr.size());
    if (headBytes > 0) addBytes(head, static_cast<std::size_t>(headBytes));
    if (tailBytes > 0) addBytes(tail, static_cast<std::size_t>(tailBytes));

    std::ostringstream oss;
    oss << "fp-" << std::hex << std::setfill('0') << std::setw(16) << h;
    return oss.str();
}

void ReadingJournal::registerBookAlias(const std::string &bookId, const std::string &fingerprint,
                                       const std::string &title, const std::string &author) {
    if (bookId.empty()) return;
    std::string cid = bookId;

    if (!fingerprint.empty()) {
        auto it = fingerprintToBook_.find(fingerprint);
        if (it != fingerprintToBook_.end()) {
            cid = it->second;
            bookAliases_[bookId] = cid;
        } else {
            fingerprintToBook_[fingerprint] = bookId;
            bookAliases_[bookId] = bookId;
        }
    } else {
        if (!bookAliases_.count(bookId)) bookAliases_[bookId] = bookId;
    }

    if (!title.empty() || !author.empty()) {
        auto &meta = canonicalMetadata_[cid];
        if (!title.empty()) meta.first = title;
        if (!author.empty()) meta.second = author;
    }
    dirty_ = true;
}

std::string ReadingJournal::canonicalId(const std::string &bookId) const {
    auto it = bookAliases_.find(bookId);
    return (it != bookAliases_.end()) ? it->second : bookId;
}

bool ReadingJournal::saveAliases() {
    if (readOnly_) return false;
    const auto path = directory_ / "aliases.v1";
    const auto tmp = directory_ / "aliases.v1.tmp";
    std::ofstream out(tmp, std::ios::binary | std::ios::trunc);
    if (!out) return false;
    out << "PAPERNX-ALIASES-1\n";
    for (const auto &alias : bookAliases_) {
        std::string fp;
        for (const auto &kv : fingerprintToBook_) {
            if (kv.second == alias.second) { fp = kv.first; break; }
        }
        std::string title, author;
        auto metaIt = canonicalMetadata_.find(alias.second);
        if (metaIt != canonicalMetadata_.end()) {
            title = metaIt->second.first;
            author = metaIt->second.second;
        }
        std::ostringstream oss;
        oss << "A\t" << hex(alias.first) << "\t" << hex(alias.second)
            << "\t" << hex(fp) << "\t" << hex(title) << "\t" << hex(author);
        out << signedLine(oss.str());
    }
    out.flush();
    if (!out) return false;
    out.close();
    std::error_code ec;
    fs::rename(tmp, path, ec);
    return !ec;
}

bool ReadingJournal::loadAliases() {
    const auto path = directory_ / "aliases.v1";
    std::error_code ec;
    if (!fs::exists(path, ec)) return true;
    std::ifstream in(path, std::ios::binary);
    if (!in) return true;
    std::string line;
    if (!std::getline(in, line) || line != "PAPERNX-ALIASES-1") return true;
    while (std::getline(in, line)) {
        if (line.empty() || line.size() > maxLine) continue;
        std::string body;
        if (!checkedLine(line, body)) continue;
        auto fields = split(body, '\t');
        if (fields.size() < 6 || fields[0] != "A") continue;
        std::string aliasId, canonicalId, fp, title, author;
        if (!unhex(fields[1], aliasId) || !unhex(fields[2], canonicalId) ||
            !unhex(fields[3], fp) || !unhex(fields[4], title) || !unhex(fields[5], author)) continue;
        bookAliases_[aliasId] = canonicalId;
        if (!fp.empty()) fingerprintToBook_[fp] = canonicalId;
        if (!title.empty() || !author.empty()) canonicalMetadata_[canonicalId] = {title, author};
    }
    return true;
}

bool ReadingJournal::deleteSession(std::uint64_t sessionId) {
    if (readOnly_) return false;
    auto it = std::find_if(sessions_.begin(), sessions_.end(), [sessionId](const JournalSession &s) {
        return s.id == sessionId;
    });
    if (it == sessions_.end()) return false;
    sessions_.erase(it);
    dirty_ = true;
    save();
    return true;
}

bool ReadingJournal::exportJson(const fs::path &targetPath) const {
    std::error_code ec;
    fs::create_directories(targetPath.parent_path(), ec);
    std::ofstream out(targetPath, std::ios::trunc);
    if (!out) return false;

    const auto t = totals();
    out << "{\n";
    out << "  \"version\": 1,\n";
    out << "  \"app\": \"PaperNX Reading Journal\",\n";
    out << "  \"exportedUtc\": " << std::time(nullptr) << ",\n";
    out << "  \"dailyGoalMinutes\": " << dailyGoalMinutes_ << ",\n";
    out << "  \"idleMinutes\": " << idleMinutes_ << ",\n";
    out << "  \"totals\": {\n";
    out << "    \"activeMilliseconds\": " << t.activeMilliseconds << ",\n";
    out << "    \"sessions\": " << t.sessions << ",\n";
    out << "    \"readDays\": " << t.readDays << "\n";
    out << "  },\n";
    out << "  \"sessions\": [\n";

    for (std::size_t i = 0; i < sessions_.size(); ++i) {
        const auto &s = sessions_[i];
        out << "    {\n";
        out << "      \"id\": " << s.id << ",\n";
        out << "      \"bookId\": \"" << jsonEscape(canonicalId(s.bookId)) << "\",\n";
        out << "      \"title\": \"" << jsonEscape(s.title) << "\",\n";
        out << "      \"author\": \"" << jsonEscape(s.author) << "\",\n";
        out << "      \"startedUtc\": " << s.startedUtc << ",\n";
        out << "      \"endedUtc\": " << s.endedUtc << ",\n";
        out << "      \"activeMilliseconds\": " << s.activeMilliseconds << ",\n";
        out << "      \"startProgress\": " << s.startProgress << ",\n";
        out << "      \"endProgress\": " << s.endProgress << ",\n";
        out << "      \"days\": {";
        std::size_t dCount = 0;
        for (const auto &d : s.days) {
            if (dCount++ > 0) out << ", ";
            out << "\"" << d.first << "\": " << d.second;
        }
        out << "}\n";
        out << "    }" << (i + 1 < sessions_.size() ? "," : "") << "\n";
    }

    out << "  ]\n";
    out << "}\n";
    out.flush();
    return !out.fail();
}

bool ReadingJournal::exportCsv(const fs::path &targetPath) const {
    std::error_code ec;
    fs::create_directories(targetPath.parent_path(), ec);
    std::ofstream out(targetPath, std::ios::trunc);
    if (!out) return false;

    out << "id,bookId,title,author,startedUtc,endedUtc,activeMinutes,startProgress,endProgress,days\n";
    for (const auto &s : sessions_) {
        out << s.id << ","
            << csvEscape(canonicalId(s.bookId)) << ","
            << csvEscape(s.title) << ","
            << csvEscape(s.author) << ","
            << s.startedUtc << ","
            << s.endedUtc << ","
            << (s.activeMilliseconds / 60000) << ","
            << s.startProgress << ","
            << s.endProgress << ",";
        std::string dayStr;
        for (const auto &d : s.days) {
            if (!dayStr.empty()) dayStr += ";";
            dayStr += d.first + "=" + std::to_string(d.second / 60000);
        }
        out << csvEscape(dayStr) << "\n";
    }
    out.flush();
    return !out.fail();
}

PeriodStats ReadingJournal::calculatePeriodStats(int period, int offset, bool en) const {
    PeriodStats st;
    const auto cal = calendar();
    const auto today = nowDate();

    const char *monthsTr[] = {"Ocak","Şubat","Mart","Nisan","Mayıs","Haziran","Temmuz","Ağustos","Eylül","Ekim","Kasım","Aralık"};
    const char *monthsEn[] = {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};

    if (period == 0) { // Week (Hafta)
        st.title = en ? "This week" : "Bu hafta";
        int mondayOffset = (today.tm_wday + 6) % 7;
        std::tm startOfWeek = today;
        startOfWeek.tm_mday -= mondayOffset + (offset * 7);
        startOfWeek.tm_sec = 0; startOfWeek.tm_min = 0; startOfWeek.tm_hour = 0;
        std::mktime(&startOfWeek);

        const char *daysTr[] = {"Pzt","Sal","Çar","Per","Cum","Cmt","Paz"};
        const char *daysEn[] = {"Mon","Tue","Wed","Thu","Fri","Sat","Sun"};

        st.totalPeriodDays = 7;
        st.barValues.resize(7, 0);
        st.barLabels.resize(7);

        std::tm endOfWeek = startOfWeek;
        endOfWeek.tm_mday += 6;
        std::mktime(&endOfWeek);

        char rangeBuf[64];
        if (startOfWeek.tm_mon == endOfWeek.tm_mon) {
            std::snprintf(rangeBuf, sizeof(rangeBuf), "%d–%d %s",
                          startOfWeek.tm_mday, endOfWeek.tm_mday,
                          en ? monthsEn[startOfWeek.tm_mon] : monthsTr[startOfWeek.tm_mon]);
        } else {
            std::snprintf(rangeBuf, sizeof(rangeBuf), "%d %s – %d %s",
                          startOfWeek.tm_mday, en ? monthsEn[startOfWeek.tm_mon] : monthsTr[startOfWeek.tm_mon],
                          endOfWeek.tm_mday, en ? monthsEn[endOfWeek.tm_mon] : monthsTr[endOfWeek.tm_mon]);
        }
        st.range = rangeBuf;

        std::size_t sessionCount = 0;
        for (int i = 0; i < 7; ++i) {
            std::tm day = startOfWeek;
            day.tm_mday += i;
            std::mktime(&day);
            char dk[48];
            std::snprintf(dk, sizeof(dk), "%04d-%02d-%02d", day.tm_year + 1900, day.tm_mon + 1, day.tm_mday);
            auto it = cal.find(dk);
            std::uint64_t ms = (it != cal.end()) ? it->second : 0;
            st.barValues[i] = ms / 60000;
            st.barLabels[i] = en ? daysEn[i] : daysTr[i];
            st.activeMilliseconds += ms;
            if (ms >= 60000) ++st.daysRead;
            if (dailyGoalMinutes_ > 0 && ms >= dailyGoalMinutes_ * 60000ULL) ++st.goalDaysMet;
            if (day.tm_year == today.tm_year && day.tm_mon == today.tm_mon && day.tm_mday == today.tm_mday) {
                st.highlightBar = i;
            }
        }

        std::tm prevWeek = startOfWeek;
        prevWeek.tm_mday -= 7;
        std::mktime(&prevWeek);
        for (int i = 0; i < 7; ++i) {
            std::tm day = prevWeek;
            day.tm_mday += i;
            std::mktime(&day);
            char dk[48];
            std::snprintf(dk, sizeof(dk), "%04d-%02d-%02d", day.tm_year + 1900, day.tm_mon + 1, day.tm_mday);
            auto it = cal.find(dk);
            if (it != cal.end()) st.previousPeriodMs += it->second;
        }

        for (const auto &s : sessions_) {
            for (int i = 0; i < 7; ++i) {
                std::tm day = startOfWeek;
                day.tm_mday += i;
                std::mktime(&day);
                char dk[48];
                std::snprintf(dk, sizeof(dk), "%04d-%02d-%02d", day.tm_year + 1900, day.tm_mon + 1, day.tm_mday);
                if (s.days.count(dk)) { ++sessionCount; break; }
            }
        }
        if (sessionCount > 0) st.averageSessionMinutes = static_cast<unsigned>((st.activeMilliseconds / sessionCount) / 60000);

        if (st.previousPeriodMs > 0) {
            st.hasPreviousPeriod = true;
            std::int64_t diff = (static_cast<std::int64_t>(st.activeMilliseconds) - static_cast<std::int64_t>(st.previousPeriodMs)) * 100 / static_cast<std::int64_t>(st.previousPeriodMs);
            if (diff >= 0) st.trendText = en ? ("↗ " + std::to_string(diff) + "% more than last week") : ("↗ Geçen haftaya göre %" + std::to_string(diff) + " fazla");
            else st.trendText = en ? ("↘ " + std::to_string(-diff) + "% less than last week") : ("↘ Geçen haftaya göre %" + std::to_string(-diff) + " az");
        } else {
            st.hasPreviousPeriod = false;
            st.trendText = en ? "No previous week data" : "Önceki hafta verisi yok";
        }
    } else if (period == 1) { // Month (Ay)
        st.title = en ? "This month" : "Bu ay";
        std::tm curMonth = today;
        curMonth.tm_mon += offset;
        curMonth.tm_mday = 1;
        std::mktime(&curMonth);
        int year = curMonth.tm_year + 1900, month = curMonth.tm_mon;
        int maxDays = daysInMonth(year, month);

        char rangeBuf[64];
        std::snprintf(rangeBuf, sizeof(rangeBuf), "%s %d", en ? monthsEn[month] : monthsTr[month], year);
        st.range = rangeBuf;

        int numWeeks = (maxDays <= 28) ? 4 : (maxDays <= 31 ? 5 : 4);
        st.totalPeriodDays = static_cast<unsigned>(maxDays);
        st.barValues.resize(numWeeks, 0);
        st.barLabels.resize(numWeeks);

        std::size_t sessionCount = 0;
        int currentWeekIdx = std::min(numWeeks - 1, (today.tm_mday - 1) / 7);
        if (offset == 0) st.highlightBar = currentWeekIdx;

        for (int w = 0; w < numWeeks; ++w) {
            st.barLabels[w] = std::to_string(w + 1) + (en ? ".Wk" : ".Hf");
            int dStart = w * 7 + 1;
            int dEnd = std::min(maxDays, (w + 1) * 7);
            for (int d = dStart; d <= dEnd; ++d) {
                char dk[48];
                std::snprintf(dk, sizeof(dk), "%04d-%02d-%02d", year, month + 1, d);
                auto it = cal.find(dk);
                std::uint64_t ms = (it != cal.end()) ? it->second : 0;
                st.barValues[w] += ms / 60000;
                st.activeMilliseconds += ms;
                if (ms >= 60000) ++st.daysRead;
                if (dailyGoalMinutes_ > 0 && ms >= dailyGoalMinutes_ * 60000ULL) ++st.goalDaysMet;
            }
        }

        std::tm prevMonth = curMonth;
        prevMonth.tm_mon -= 1;
        std::mktime(&prevMonth);
        int prevYear = prevMonth.tm_year + 1900, prevM = prevMonth.tm_mon;
        int prevMaxDays = daysInMonth(prevYear, prevM);
        for (int d = 1; d <= prevMaxDays; ++d) {
            char dk[48];
            std::snprintf(dk, sizeof(dk), "%04d-%02d-%02d", prevYear, prevM + 1, d);
            auto it = cal.find(dk);
            if (it != cal.end()) st.previousPeriodMs += it->second;
        }

        for (const auto &s : sessions_) {
            for (int d = 1; d <= maxDays; ++d) {
                char dk[48];
                std::snprintf(dk, sizeof(dk), "%04d-%02d-%02d", year, month + 1, d);
                if (s.days.count(dk)) { ++sessionCount; break; }
            }
        }
        if (sessionCount > 0) st.averageSessionMinutes = static_cast<unsigned>((st.activeMilliseconds / sessionCount) / 60000);

        if (st.previousPeriodMs > 0) {
            st.hasPreviousPeriod = true;
            std::int64_t diff = (static_cast<std::int64_t>(st.activeMilliseconds) - static_cast<std::int64_t>(st.previousPeriodMs)) * 100 / static_cast<std::int64_t>(st.previousPeriodMs);
            if (diff >= 0) st.trendText = en ? ("↗ " + std::to_string(diff) + "% more than last month") : ("↗ Geçen aya göre %" + std::to_string(diff) + " fazla");
            else st.trendText = en ? ("↘ " + std::to_string(-diff) + "% less than last month") : ("↘ Geçen aya göre %" + std::to_string(-diff) + " az");
        } else {
            st.hasPreviousPeriod = false;
            st.trendText = en ? "No previous month data" : "Önceki ay verisi yok";
        }
    } else { // Year (Yıl)
        st.title = en ? "This year" : "Bu yıl";
        int year = today.tm_year + 1900 + offset;
        st.range = std::to_string(year);
        st.totalPeriodDays = 365;
        st.barValues.resize(12, 0);
        st.barLabels.resize(12);

        if (offset == 0) st.highlightBar = today.tm_mon;

        std::size_t sessionCount = 0;
        for (int m = 0; m < 12; ++m) {
            st.barLabels[m] = en ? monthsEn[m] : monthsTr[m];
            int maxD = daysInMonth(year, m);
            for (int d = 1; d <= maxD; ++d) {
                char dk[48];
                std::snprintf(dk, sizeof(dk), "%04d-%02d-%02d", year, m + 1, d);
                auto it = cal.find(dk);
                std::uint64_t ms = (it != cal.end()) ? it->second : 0;
                st.barValues[m] += ms / 60000;
                st.activeMilliseconds += ms;
                if (ms >= 60000) ++st.daysRead;
                if (dailyGoalMinutes_ > 0 && ms >= dailyGoalMinutes_ * 60000ULL) ++st.goalDaysMet;
            }
        }

        int prevYear = year - 1;
        for (int m = 0; m < 12; ++m) {
            int maxD = daysInMonth(prevYear, m);
            for (int d = 1; d <= maxD; ++d) {
                char dk[48];
                std::snprintf(dk, sizeof(dk), "%04d-%02d-%02d", prevYear, m + 1, d);
                auto it = cal.find(dk);
                if (it != cal.end()) st.previousPeriodMs += it->second;
            }
        }

        for (const auto &s : sessions_) {
            char prefix[8];
            std::snprintf(prefix, sizeof(prefix), "%04d-", year);
            for (const auto &d : s.days) {
                if (d.first.rfind(prefix, 0) == 0) { ++sessionCount; break; }
            }
        }
        if (sessionCount > 0) st.averageSessionMinutes = static_cast<unsigned>((st.activeMilliseconds / sessionCount) / 60000);

        if (st.previousPeriodMs > 0) {
            st.hasPreviousPeriod = true;
            std::int64_t diff = (static_cast<std::int64_t>(st.activeMilliseconds) - static_cast<std::int64_t>(st.previousPeriodMs)) * 100 / static_cast<std::int64_t>(st.previousPeriodMs);
            if (diff >= 0) st.trendText = en ? ("↗ " + std::to_string(diff) + "% more than last year") : ("↗ Geçen yıla göre %" + std::to_string(diff) + " fazla");
            else st.trendText = en ? ("↘ " + std::to_string(-diff) + "% less than last year") : ("↘ Geçen yıla göre %" + std::to_string(-diff) + " az");
        } else {
            st.hasPreviousPeriod = false;
            st.trendText = en ? "Jan to date" : "Ocaktan bugüne";
        }
    }

    st.streakDays = 0;
    std::tm checkDay = today;
    char todayKey[32];
    std::snprintf(todayKey, sizeof(todayKey), "%04d-%02d-%02d", checkDay.tm_year + 1900, checkDay.tm_mon + 1, checkDay.tm_mday);
    bool todayRead = (cal.count(todayKey) && cal.at(todayKey) >= 60000);

    if (!todayRead) {
        checkDay.tm_mday -= 1;
        std::mktime(&checkDay);
    }
    while (true) {
        char kBuf[32];
        std::snprintf(kBuf, sizeof(kBuf), "%04d-%02d-%02d", checkDay.tm_year + 1900, checkDay.tm_mon + 1, checkDay.tm_mday);
        if (cal.count(kBuf) && cal.at(kBuf) >= 60000) {
            ++st.streakDays;
            checkDay.tm_mday -= 1;
            std::mktime(&checkDay);
        } else {
            break;
        }
    }

    auto allBooks = books();
    st.finishedBooks = 0;
    for (const auto &b : allBooks) {
        if (b.progress >= 0.99) ++st.finishedBooks;
    }

    return st;
}

std::vector<JournalSession> ReadingJournal::sessionsForDay(const std::string &day) const {
    std::vector<JournalSession> out;
    for (auto it = sessions_.rbegin(); it != sessions_.rend(); ++it) {
        if (it->days.count(day) && it->days.at(day) > 0) {
            out.push_back(*it);
        }
    }
    return out;
}

std::vector<JournalSession> ReadingJournal::sessionsForBook(const std::string &bookId) const {
    std::vector<JournalSession> out;
    const auto cid = canonicalId(bookId);
    for (auto it = sessions_.rbegin(); it != sessions_.rend(); ++it) {
        if (canonicalId(it->bookId) == cid) {
            out.push_back(*it);
        }
    }
    return out;
}

JournalBookTotal ReadingJournal::bookTotal(const std::string &bookId) const {
    const auto all = books();
    const auto cid = canonicalId(bookId);
    for (const auto &b : all) {
        if (b.id == cid || b.id == bookId) return b;
    }
    return {};
}

std::pair<std::string, double> ReadingJournal::currentlyReading() const {
    const auto all = books();
    if (all.empty()) return {"", 0.0};
    for (const auto &b : all) {
        if (b.progress < 0.99) return {b.id, b.progress};
    }
    return {all[0].id, all[0].progress};
}

} // namespace papernx
