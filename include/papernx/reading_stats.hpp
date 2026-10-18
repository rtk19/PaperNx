#pragma once
#include "papernx/core.hpp"
#include <optional>

namespace papernx {
struct BookReadingStats {
    std::int64_t activeMilliseconds{};
    int sessions{};
    double measuredFraction{};
    int progressSamples{};
    std::string lastReadingDay;
    double furthestFraction{};
};
// Caller supplies monotonic milliseconds and a local ISO calendar date.
// No wall-clock duration arithmetic; idle/suspend/menu time never contributes.
class ReadingStatsStore {
public:
    explicit ReadingStatsStore(fs::path path) : path_(std::move(path)) {}
    bool load();
    bool save() const;
    void interaction(std::uint64_t now);
    // Call before jumps/history restoration/EPUB relayout. Retains elapsed time.
    void reanchor() { skipNextSample_ = true; }
    void tick(std::uint64_t now, const std::string &bookId, double fraction,
              bool readingVisible, const std::string &localDay);
    BookReadingStats get(const std::string &bookId) const;
    std::int64_t totalMilliseconds() const;
    std::int64_t dayMilliseconds(const std::string &localDay) const;
    std::optional<int> remainingMinutes(const std::string &bookId, double fraction) const;
    bool setDailyGoal(int minutes); // 0 disables; maximum 12 hours.
    int dailyGoal() const { return dailyGoal_; }
    const std::map<std::string,std::int64_t> &calendar() const { return days_; }
private:
    fs::path path_;
    std::map<std::string,BookReadingStats> books_;
    std::map<std::string,std::int64_t> days_;
    int dailyGoal_{0};
    std::uint64_t lastTick_{0}, lastActivity_{0};
    bool initialized_{false}, active_{false}, hasActivity_{false};
    bool skipNextSample_{false};
    bool writable_{true};
    std::string currentBook_;
    double lastFraction_{0};
};
} // namespace papernx
