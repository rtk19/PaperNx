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
    void back();
    bool hasSubpage() const { return goals_||!detailId_.empty(); }
    void render(SDL_Renderer *,Typography &,const ThemePalette &,const ReadingJournal &,
                int w,int h,bool english);
    void renderPreview(SDL_Renderer *,Typography &,const ThemePalette &,const ReadingJournal &,
                       const std::string &bookId,int w,int h,bool english);
private:
    int tab_{0},offset_{0},focus_{0},monthDelta_{0},selectedDay_{0};
    bool goals_{false},preview_{false};
    std::string detailId_;
};
}
