#pragma once
#include <cstdint>
#include <filesystem>
#include <map>
#include <string>
#include <vector>

namespace papernx {
namespace fs = std::filesystem;

// Independent from the legacy reading-stats.txt and PAPERNX-6 progress/settings.
// All times come from the caller's monotonic clock, except local day assignment.
struct JournalSession {
    std::uint64_t id{};
    std::string bookId, title, author;
    std::int64_t startedUtc{}, endedUtc{};
    std::uint64_t activeMilliseconds{};
    double startProgress{}, endProgress{};
    std::map<std::string,std::uint64_t> days;
};

struct JournalBookTotal {
    std::string id,title,author;
    std::uint64_t activeMilliseconds{};
    std::size_t sessions{};
    std::int64_t lastRead{};
    double progress{};
    bool archived{false};
};

struct JournalTotals {
    std::uint64_t activeMilliseconds{};
    std::size_t sessions{}, readDays{};
};

struct PeriodStats {
    std::string title;
    std::string range;
    std::uint64_t activeMilliseconds{};
    std::uint64_t previousPeriodMs{};
    bool hasPreviousPeriod{false};
    std::string trendText;
    std::vector<std::uint64_t> barValues;
    std::vector<std::string> barLabels;
    int highlightBar{-1};
    unsigned daysRead{};
    unsigned averageSessionMinutes{};
    unsigned finishedBooks{};
    unsigned streakDays{};
    unsigned goalDaysMet{};
    unsigned totalPeriodDays{};
};

// Computes a fast, bounded fingerprint (max 8KB read) combining fileSize, head, and tail bytes.
std::string computeBookFingerprint(const fs::path &filePath, std::uintmax_t fileSize);

class ReadingJournal {
public:
    explicit ReadingJournal(fs::path directory) : directory_(std::move(directory)) {}
    // No existing file is an empty journal, not an error.
    bool load();
    bool save();
    // nowMs must be monotonic. epochSeconds is wall-clock UTC, used ONLY for calendar.
    void tick(std::uint64_t nowMs, std::int64_t epochSeconds, const std::string &localDay,
              const std::string &bookId, const std::string &title, const std::string &author,
              double progress, bool readerVisible);
    void interaction(std::uint64_t nowMs);
    void pause(std::uint64_t nowMs, std::int64_t epochSeconds);
    void setIdleMinutes(unsigned value);
    unsigned idleMinutes() const { return idleMinutes_; }
    void setDailyGoalMinutes(unsigned value);
    unsigned dailyGoalMinutes() const { return dailyGoalMinutes_; }
    const std::vector<JournalSession> &sessions() const { return sessions_; }
    std::map<std::string,std::uint64_t> calendar() const;
    std::vector<JournalBookTotal> books() const;
    JournalTotals totals() const;
    std::uint64_t bookMilliseconds(const std::string &bookId) const;
    bool hasUnsavedChanges() const { return dirty_; }
    bool isReadOnly() const { return readOnly_; }
    const std::string &diagnostic() const { return diagnostic_; }

    // Book identity & aliasing
    void registerBookAlias(const std::string &bookId, const std::string &fingerprint,
                           const std::string &title, const std::string &author);
    std::string canonicalId(const std::string &bookId) const;

    // Session management & export
    bool deleteSession(std::uint64_t sessionId);
    bool exportJson(const fs::path &targetPath) const;
    bool exportCsv(const fs::path &targetPath) const;

    // Period aggregations & detailed queries
    PeriodStats calculatePeriodStats(int period /*0=week, 1=month, 2=year*/, int offset=0, bool english=false) const;
    std::vector<JournalSession> sessionsForDay(const std::string &day) const;
    std::vector<JournalSession> sessionsForBook(const std::string &bookId) const;
    JournalBookTotal bookTotal(const std::string &bookId) const;
    std::pair<std::string, double> currentlyReading() const;

private:
    fs::path directory_;
    std::vector<JournalSession> sessions_;
    std::map<std::string, std::string> bookAliases_;       // aliasBookId -> canonicalId
    std::map<std::string, std::string> fingerprintToBook_; // fingerprint -> canonicalId
    std::map<std::string, std::pair<std::string, std::string>> canonicalMetadata_; // canonicalId -> {title, author}
    std::uint64_t nextId_{1},lastTick_{0},lastInteraction_{0};
    unsigned idleMinutes_{5},dailyGoalMinutes_{0};
    bool clockReady_{false},interactionReady_{false},dirty_{false},readOnly_{false};
    std::string diagnostic_;
    void finish(std::int64_t epochSeconds);
    bool validSession(const JournalSession &s) const;
    bool loadAliases();
    bool saveAliases();
};
} // namespace papernx
