#pragma once
#include "papernx/reading_journal.hpp"
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
    Action tap(int x,int y,int w,int h,ReadingJournal &journal);
    Action activate(int w,int h,ReadingJournal &journal);
    void navigate(int dx,int dy,const ReadingJournal &journal);
    void nextTab();
    void prevTab();
    void back();
    bool hasSubpage() const { return goals_ || exportModal_ || deleteModal_ || !detailId_.empty(); }
    void render(SDL_Renderer *,Typography &,const ThemePalette &,const ReadingJournal &,
                int w,int h,bool english);
    void renderPreview(SDL_Renderer *,Typography &,const ThemePalette &,const ReadingJournal &,
                       const std::string &bookId,int w,int h,bool english);
    void openGoals() { goals_ = true; exportModal_ = false; deleteModal_ = false; }
    void openExport() { exportModal_ = true; goals_ = false; deleteModal_ = false; exportMessage_.clear(); }
private:
    int tab_{0};            // 0=Overview, 1=Calendar, 2=Books, 3=History
    int period_{0};         // 0=Week, 1=Month, 2=Year
    int offset_{0};         // scroll offset
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
