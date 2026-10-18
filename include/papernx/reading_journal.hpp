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
};

struct JournalTotals {
    std::uint64_t activeMilliseconds{};
    std::size_t sessions{}, readDays{};
};

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
private:
    fs::path directory_;
    std::vector<JournalSession> sessions_;
    std::uint64_t nextId_{1},lastTick_{0},lastInteraction_{0};
    unsigned idleMinutes_{5},dailyGoalMinutes_{0};
    bool clockReady_{false},interactionReady_{false},dirty_{false},readOnly_{false};
    std::string diagnostic_;
    void finish(std::int64_t epochSeconds);
    bool validSession(const JournalSession &s) const;
};
} // namespace papernx
