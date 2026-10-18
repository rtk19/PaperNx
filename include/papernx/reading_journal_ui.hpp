#pragma once
#include "papernx/reading_journal.hpp"
#include "papernx/catalog.hpp"
#include "papernx/theme.hpp"
#include "papernx/typography.hpp"
#include <string>

namespace papernx {
class ReadingJournalUI {
public:
    enum class Action { None, Back, OpenFull, ClosePreview };
    void open(const std::string &bookId={});
    void showPreview() { preview_=true; }
    void hidePreview() { preview_=false; }
    bool previewVisible() const { return preview_; }
    Action tap(int x,int y,int w,int h,ReadingJournal &journal,
               const ProgressStore *progress=nullptr,const CatalogStore *catalog=nullptr);
    Action activate(int w,int h,ReadingJournal &journal,
                    const ProgressStore *progress=nullptr,const CatalogStore *catalog=nullptr);
    void navigate(int dx,int dy,const ReadingJournal &journal,
                  const ProgressStore *progress=nullptr,const CatalogStore *catalog=nullptr);
    void scroll(int dy);
    void nextTab();
    void prevTab();
    void back();
    bool hasSubpage() const { return goals_ || exportModal_ || deleteModal_ || !detailId_.empty(); }
    void render(SDL_Renderer *,Typography &,const ThemePalette &,const ReadingJournal &,
                const ProgressStore &,const CatalogStore &,int w,int h,bool english);
    void renderPreview(SDL_Renderer *,Typography &,const ThemePalette &,const ReadingJournal &,
                       const std::string &bookId,int w,int h,bool english,const ProgressStore *progress=nullptr);
    void openGoals() { goals_ = true; exportModal_ = false; deleteModal_ = false; }
    void openExport() { exportModal_ = true; goals_ = false; deleteModal_ = false; exportMessage_.clear(); }
    void openDeleteConfirm(std::uint64_t sessionId) { deleteModal_ = true; sessionToDelete_ = sessionId; goals_ = false; exportModal_ = false; }
    int currentTab() const { return tab_; }
    void setTab(int t) { tab_ = std::clamp(t, 0, 3); scrollY_ = 0; offset_ = 0; detailId_.clear(); }
    int scrollY() const { return scrollY_; }
    void setScrollY(int y) { scrollY_ = std::max(0, y); }
    int maxScrollY() const { return maxScrollY_; }
    int period() const { return period_; }
    void setPeriod(int p) { period_ = std::clamp(p, 0, 2); }
    const std::string &detailBookId() const { return detailId_; }
    void setDetailBookId(const std::string &id) { detailId_ = id; scrollY_ = 0; }

private:
    int tab_{0};            // 0=Overview, 1=Calendar, 2=Books, 3=History
    int period_{0};         // 0=Week, 1=Month, 2=Year
    int offset_{0};         // item index offset for fallback pagination
    int scrollY_{0};        // vertical scroll in pixels for content area
    int maxScrollY_{0};     // maximum scroll extent
    int focus_{0};          // keyboard/gamepad focus index
    int monthDelta_{0};     // calendar month offset
    int selectedDay_{0};    // selected day in calendar
    bool goals_{false};     // goals modal
    bool exportModal_{false}; // export modal
    bool deleteModal_{false}; // session delete modal
    std::uint64_t sessionToDelete_{0}; // session id pending deletion
    std::string exportMessage_; // status message after export
    bool preview_{false};   // reader mini preview
    std::string detailId_;  // selected book id for drilldown
};
}
