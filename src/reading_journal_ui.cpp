#include "papernx/reading_journal_ui.hpp"
#include "papernx/ui.hpp"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <ctime>
#include <map>
#include <vector>

namespace papernx {
namespace {
std::string label(bool en, const char *tr, const char *english) { return en ? english : tr; }

std::string duration(std::uint64_t ms, bool en) {
    if(ms<60000) return std::to_string(ms/1000)+(en?" sec":" sn");
    auto m = ms / 60000;
    if (m >= 60) return std::to_string(m / 60) + (en ? " h " : " sa ") + std::to_string(m % 60) + (en ? " min" : " dk");
    return std::to_string(m) + (en ? " min" : " dk");
}

std::string key(int year, int month, int day) {
    char b[32]; std::snprintf(b, sizeof b, "%04d-%02d-%02d", year, month, day); return b;
}

std::tm nowDate() {
    const auto n = std::time(nullptr); const auto *ptr = std::localtime(&n);
    return ptr ? *ptr : std::tm{};
}

std::tm dateAt(int monthDelta) {
    auto d = nowDate(); d.tm_mday = 1; d.tm_mon += monthDelta;
    std::mktime(&d); return d;
}

int daysInMonth(int year, int month) {
    std::tm d{}; d.tm_year = year - 1900; d.tm_mon = month + 1; d.tm_mday = 0;
    std::mktime(&d); return d.tm_mday;
}

std::string localDate(std::int64_t epoch) {
    const auto raw = static_cast<std::time_t>(epoch);
    const auto *d = std::localtime(&raw);
    if (!d) return "?";
    char b[32]; std::strftime(b, sizeof b, "%Y-%m-%d", d); return b;
}

std::string localTime(std::int64_t epoch) {
    const auto raw = static_cast<std::time_t>(epoch);
    const auto *d = std::localtime(&raw);
    if (!d) return "??:??";
    char b[16]; std::snprintf(b, sizeof b, "%02d:%02d", d->tm_hour, d->tm_min); return b;
}

void text(SDL_Renderer *r, Typography &font, const std::string &s, int x, int y,
          SDL_Color color, int maxw = 0, bool large = false) {
    font.renderText(r, large ? font.titleFont() : font.uiFont(), s, x, y, color, maxw);
}

void textSmall(SDL_Renderer *r, Typography &font, const std::string &s, int x, int y,
               SDL_Color color, int maxw = 0) {
    font.renderText(r, font.smallFont(), s, x, y, color, maxw);
}

void line(SDL_Renderer *r, SDL_Color color, int x, int y, int w) {
    SDL_SetRenderDrawColor(r, color.r, color.g, color.b, 255);
    SDL_RenderDrawLine(r, x, y, x + w, y);
}

void panel(SDL_Renderer *r, const ThemePalette &pal, SDL_Rect rc) {
    ui::surface(r, rc, pal.card, 16);
    SDL_SetRenderDrawColor(r, pal.cardBorder.r, pal.cardBorder.g, pal.cardBorder.b, 255);
    SDL_RenderDrawRect(r, &rc);
}

struct Geometry { int left, top, width, navTop; bool sidebar; };
Geometry geometry(int w) {
    return {w >= 900 ? 244 : 18, w >= 900 ? 104 : 160, w - (w >= 900 ? 262 : 36), w >= 900 ? 116 : 94, w >= 900};
}

void drawRing(SDL_Renderer *r, const ThemePalette &pal, int cx, int cy, int radius, int thickness, float fraction) {
    fraction = std::clamp(fraction, 0.0f, 1.0f);
    SDL_Color track = blendColor(pal.card, pal.cardBorder, 60);
    SDL_SetRenderDrawColor(r, track.r, track.g, track.b, 255);
    for (int angle = 0; angle < 360; ++angle) {
        float rad = angle * 3.14159265f / 180.0f;
        for (int t = 0; t < thickness; ++t) {
            int px = cx + static_cast<int>((radius - t) * std::cos(rad));
            int py = cy + static_cast<int>((radius - t) * std::sin(rad));
            SDL_RenderDrawPoint(r, px, py);
        }
    }
    if (fraction > 0.01f) {
        SDL_SetRenderDrawColor(r, pal.accent.r, pal.accent.g, pal.accent.b, 255);
        int maxAngle = static_cast<int>(fraction * 360.0f);
        for (int angle = -90; angle < -90 + maxAngle; ++angle) {
            float rad = angle * 3.14159265f / 180.0f;
            for (int t = 0; t < thickness; ++t) {
                int px = cx + static_cast<int>((radius - t) * std::cos(rad));
                int py = cy + static_cast<int>((radius - t) * std::sin(rad));
                SDL_RenderDrawPoint(r, px, py);
            }
        }
    }
}

void drawCover(SDL_Renderer *r, const ThemePalette &pal, Typography &font, SDL_Rect rc,
               const std::string &title, const std::string &author, int seed = 0) {
    (void)pal;
    SDL_Color cBg = (seed % 3 == 0) ? SDL_Color{112, 75, 59, 255} :
                    (seed % 3 == 1) ? SDL_Color{174, 132, 89, 255} :
                                      SDL_Color{81, 104, 117, 255};
    ui::surface(r, rc, cBg, 8);
    SDL_SetRenderDrawColor(r, 255, 255, 255, 40);
    SDL_Rect spine{rc.x, rc.y, 6, rc.h};
    SDL_RenderFillRect(r, &spine);
    SDL_SetRenderDrawColor(r, 0, 0, 0, 60);
    SDL_RenderDrawRect(r, &rc);
    SDL_Color tCol{245, 229, 201, 255};
    if (rc.w >= 45 && rc.h >= 65) {
        font.renderText(r, font.smallFont(), author, rc.x + 8, rc.y + 8, tCol, rc.w - 12);
        font.renderText(r, font.buttonFont(), title, rc.x + 8, rc.y + rc.h / 2 - 8, tCol, rc.w - 12);
    }
}

void header(SDL_Renderer *r, Typography &font, const ThemePalette &pal, int w, int tab, bool en) {
    ui::surface(r, {0, 0, w, 76}, pal.card, 0);
    ui::button(r, font, {16, 10, 150, 56}, label(en, "Kitaplık", "Library"), pal);
    text(r, font, label(en, "Kütüphane / Günlüğüm", "Library / Journal"), 180, 12, pal.muted, w - 440);
    text(r, font, label(en, "Okuma Günlüğüm", "My Reading Journal"), 180, 36, pal.fg, w - 440, true);
    // Position header actions to the left of the global Quick Menu '+' button at {w - 80, 8, 64, 64}
    ui::button(r, font, {w - 240, 10, 150, 56}, label(en, "Hedeflerim", "Goals"), pal);
    if (tab == 3) {
        ui::button(r, font, {w - 400, 10, 150, 56}, label(en, "Dışa aktar", "Export"), pal);
    }
    line(r, pal.cardBorder, 0, 76, w);
}

void sidebar(SDL_Renderer *r, Typography &font, const ThemePalette &pal, int w, int h, int active, int focus, bool en) {
    auto g = geometry(w);
    if (g.sidebar) {
        panel(r, pal, {14, 86, 214, h - 98});
        // Brand mark
        SDL_Rect brandMark{28, 102, 38, 38};
        ui::surface(r, brandMark, blendColor(pal.card, pal.accent, 25), 10);
        text(r, font, "P", brandMark.x + 12, brandMark.y + 6, pal.accent, 24, true);
        text(r, font, "PaperNx", 74, 102, pal.fg, 140, true);
        textSmall(r, font, label(en, "OKUMA ALANI", "READING SPACE"), 74, 144, pal.muted, 140);
        line(r, pal.cardBorder, 24, 174, 194);

        const char *tr[] = {"Genel Bakış", "Takvim", "Kitaplarım", "Geçmiş"};
        const char *eng[] = {"Overview", "Calendar", "My books", "History"};
        for (int i = 0; i < 4; ++i) {
            SDL_Rect rc{24, 190 + i * 66, 194, 56};
            ui::button(r, font, rc, en ? eng[i] : tr[i], pal, i == active, false, false, focus == i + 1);
            const auto ink=i==active?pal.bg:pal.fg;
            SDL_SetRenderDrawColor(r,ink.r,ink.g,ink.b,255);
            const int bx=rc.x+12,by=rc.y+18;
            if(i==0){for(int k=0;k<4;++k){SDL_Rect q{bx+(k%2)*9,by+(k/2)*9,6,6};SDL_RenderDrawRect(r,&q);}}
            else if(i==1){SDL_Rect q{bx,by,20,19};SDL_RenderDrawRect(r,&q);SDL_RenderDrawLine(r,bx,by+6,bx+19,by+6);}
            else if(i==2){SDL_Rect q{bx,by,9,18};SDL_RenderDrawRect(r,&q);q.x+=10;SDL_RenderDrawRect(r,&q);}
            else{SDL_Rect q{bx,by,17,19};SDL_RenderDrawRect(r,&q);for(int k=0;k<3;++k)SDL_RenderDrawLine(r,bx+4,by+5+5*k,bx+13,by+5+5*k);}
        }

        // Side footer offline status
        SDL_Rect foot{24, h - 90, 194, 68};
        panel(r, pal, foot);
        SDL_Color dotCol{78, 125, 98, 255};
        ui::surface(r, {foot.x + 12, foot.y + 16, 8, 8}, dotCol, 4);
        textSmall(r, font, label(en, "Çevrimdışı kayıt", "Offline journal"), foot.x + 26, foot.y + 12, pal.fg, foot.w - 32);
        textSmall(r, font, label(en, "microSD kartında", "Saved on microSD"), foot.x + 12, foot.y + 38, pal.muted, foot.w - 24);
    } else {
        // Horizontal top tabs
        const char *tr[] = {"Genel", "Takvim", "Kitaplar", "Geçmiş"};
        const char *eng[] = {"Overview", "Calendar", "Books", "History"};
        int tw = (w - 32) / 4;
        for (int i = 0; i < 4; ++i) {
            SDL_Rect rc{16 + i * tw, 86, tw - 4, 56};
            ui::button(r, font, rc, en ? eng[i] : tr[i], pal, i == active, false, false, focus == i + 1);
        }
    }
}

void empty(SDL_Renderer *r, Typography &font, const ThemePalette &pal, SDL_Rect rc, bool en) {
    panel(r, pal, rc);
    text(r, font, label(en, "Henüz okuma kaydı yok.", "No reading records yet."),
         rc.x + 24, rc.y + 24, pal.fg, rc.w - 48, true);
    text(r, font, label(en, "Bir kitap açıp okuduğunda aktif sürelerin burada görünecektir.",
         "Your reading activity will appear here once you open and read a book."),
         rc.x + 24, rc.y + 74, pal.muted, rc.w - 48);
    text(r, font, label(en, "Kitaplığa dön: sol üst düğme (veya B)", "Return to library: upper-left button (or B)"),
         rc.x + 24, rc.y + 124, pal.accent, rc.w - 48);
}

// Journal sessions are historical facts; the live progress indicator is the
// authoritative library state, including explicit unread/finished overrides.
double liveProgress(const std::string &bookId,double historic,
                    const ProgressStore &progress,const CatalogStore &catalog) {
    const auto status=catalog.get(bookId).status;
    if(status==ReadingStatus::NotStarted)return 0.;
    if(status==ReadingStatus::Completed)return 1.;
    const auto &all=progress.all();
    const auto it=all.find(bookId);
    if(it==all.end())return std::clamp(historic,0.,1.);
    return std::clamp(it->second.fraction,0.,1.);
}
std::uint64_t dayMs(const std::map<std::string, std::uint64_t> &cal, int y, int m, int d) {
    const auto it = cal.find(key(y, m, d)); return it == cal.end() ? 0 : it->second;
}
} // namespace

void ReadingJournalUI::open(const std::string &bookId) {
    tab_ = bookId.empty() ? 0 : 2;
    detailId_ = bookId;
    period_ = 0;
    offset_ = 0;
    monthDelta_ = 0;
    selectedDay_ = 0;
    focus_ = 0;
    goals_ = false;
    exportModal_ = false;
    deleteModal_ = false;
    sessionToDelete_ = 0;
    exportMessage_.clear();
    preview_ = false;
}

void ReadingJournalUI::nextTab() {
    tab_ = (tab_ + 1) % 4;
    offset_ = 0;
    focus_ = tab_ + 1;
    detailId_.clear();
}

void ReadingJournalUI::prevTab() {
    tab_ = (tab_ + 3) % 4;
    offset_ = 0;
    focus_ = tab_ + 1;
    detailId_.clear();
}

void ReadingJournalUI::back() {
    if (deleteModal_) { deleteModal_ = false; sessionToDelete_ = 0; }
    else if (exportModal_) { exportModal_ = false; exportMessage_.clear(); }
    else if (goals_) { goals_ = false; }
    else if (!detailId_.empty()) { detailId_.clear(); offset_ = 0; }
}

void ReadingJournalUI::navigate(int dx, int dy, const ReadingJournal &journal) {
    if (goals_ || exportModal_ || deleteModal_) return;
    if (!detailId_.empty()) {
        if (dy) offset_ = std::max(0, offset_ + dy);
        return;
    }
    if (tab_ == 1 && selectedDay_ > 0 && dx) {
        const auto d = dateAt(monthDelta_);
        selectedDay_ = std::clamp(selectedDay_ + dx, 1, daysInMonth(d.tm_year + 1900, d.tm_mon));
        return;
    }
    if (dx) {
        tab_ = std::clamp(tab_ + dx, 0, 3);
        offset_ = 0;
    }
    if (dy && (tab_ == 2 || tab_ == 3)) {
        offset_ = std::clamp(offset_ + dy, 0, std::max(0, int(tab_ == 2 ? journal.books().size() : journal.sessions().size()) - 1));
    }
    if (dy && tab_ == 1) {
        const auto d = dateAt(monthDelta_);
        selectedDay_ = std::clamp(std::max(1, selectedDay_) + dy * 7, 1, daysInMonth(d.tm_year + 1900, d.tm_mon));
    }
    focus_ = tab_ + 1;
}

ReadingJournalUI::Action ReadingJournalUI::activate(int w, int h, ReadingJournal &j) {
    (void)w; (void)h;
    if (deleteModal_) {
        if (sessionToDelete_) j.deleteSession(sessionToDelete_);
        deleteModal_ = false; sessionToDelete_ = 0;
        return Action::None;
    }
    if (exportModal_) { exportModal_ = false; return Action::None; }
    if (goals_) { goals_ = false; return Action::None; }
    if (preview_) return Action::OpenFull;
    if (!detailId_.empty()) { back(); return Action::None; }
    if (tab_ == 2) {
        const auto b = j.books();
        if (offset_ < int(b.size())) { detailId_ = b[offset_].id; offset_ = 0; }
    }
    return Action::None;
}

ReadingJournalUI::Action ReadingJournalUI::tap(int x, int y, int w, int h, ReadingJournal &journal) {
    // 1. Modals Tap Handling
    if (deleteModal_) {
        const int pw = 480, ph = 260, px = (w - pw) / 2, py = (h - ph) / 2;
        if (y >= py + 180 && y <= py + 236) {
            if (x >= px + 24 && x <= px + 224) {
                if (sessionToDelete_) journal.deleteSession(sessionToDelete_);
                deleteModal_ = false; sessionToDelete_ = 0;
                return Action::None;
            } else if (x >= px + 240 && x <= px + 440) {
                deleteModal_ = false; sessionToDelete_ = 0;
                return Action::None;
            }
        }
        if (x < px || x > px + pw || y < py || y > py + ph) {
            deleteModal_ = false; sessionToDelete_ = 0;
        }
        return Action::None;
    }

    if (exportModal_) {
        const int pw = 520, ph = 320, px = (w - pw) / 2, py = (h - ph) / 2;
        if (y >= py + 170 && y <= py + 226) {
            auto today = nowDate();
            char dateBuf[32];
            std::snprintf(dateBuf, sizeof dateBuf, "%04d-%02d-%02d", today.tm_year + 1900, today.tm_mon + 1, today.tm_mday);
            if (x >= px + 24 && x <= px + 244) {
                auto outPath = journal.exportDirectory() / ("reading_journal_" + std::string(dateBuf) + ".json");
                if (journal.exportJson(outPath)) exportMessage_ = "JSON: " + outPath.string();
                else exportMessage_ = "Hata: JSON aktarılamadı";
                return Action::None;
            } else if (x >= px + 260 && x <= px + 480) {
                auto outPath = journal.exportDirectory() / ("reading_journal_" + std::string(dateBuf) + ".csv");
                if (journal.exportCsv(outPath)) exportMessage_ = "CSV: " + outPath.string();
                else exportMessage_ = "Hata: CSV aktarılamadı";
                return Action::None;
            }
        }
        if (y >= py + 240 && y <= py + 296) { exportModal_ = false; return Action::None; }
        if (x < px || x > px + pw || y < py || y > py + ph) { exportModal_ = false; }
        return Action::None;
    }

    if (goals_) {
        const int pw = 520, ph = 320, px = (w - pw) / 2, py = (h - ph) / 2;
        if (y >= py + 160 && y <= py + 216) {
            if (x >= px + 24 && x <= px + 134) {
                journal.setDailyGoalMinutes(journal.dailyGoalMinutes() >= 5 ? journal.dailyGoalMinutes() - 5 : 0);
            } else if (x >= px + 146 && x <= px + 256) {
                journal.setDailyGoalMinutes(std::min(720u, journal.dailyGoalMinutes() + 5));
            } else if (x >= px + 268 && x <= px + 373) {
                journal.setDailyGoalMinutes(0);
            } else if (x >= px + 385 && x <= px + 490) {
                journal.setDailyGoalMinutes(30);
            }
            return Action::None;
        }
        if (y >= py + 236 && y <= py + 292) { goals_ = false; return Action::None; }
        if (x < px || x > px + pw || y < py || y > py + ph) { goals_ = false; }
        return Action::None;
    }

    if (preview_) {
        const int pw = std::min(620, w - 24), px = (w - pw) / 2, py = (h - 320) / 2;
        if (x < px || x > px + pw || y < py || y > py + 320) return Action::ClosePreview;
        if (y > py + 244) return x < px + pw / 2 ? Action::OpenFull : Action::ClosePreview;
        return Action::None;
    }

    // 2. Top Header Bar Actions
    if (y < 76) {
        if (x < 180) {
            if (!detailId_.empty()) { detailId_.clear(); return Action::None; }
            return Action::Back;
        }
        if (x >= w - 240 && x < w - 90) { openGoals(); return Action::None; }
        if (tab_ == 3 && x >= w - 400 && x < w - 250) { openExport(); return Action::None; }
    }

    // 3. Navigation (Sidebar or Top Nav)
    auto g = geometry(w);
    if (g.sidebar) {
        for (int i = 0; i < 4; ++i) {
            SDL_Rect rc{24, 190 + i * 66, 194, 56};
            if (ui::contains(rc, x, y)) {
                tab_ = i; offset_ = 0; focus_ = i + 1; detailId_.clear();
                return Action::None;
            }
        }
    } else {
        int tw = (w - 32) / 4;
        for (int i = 0; i < 4; ++i) {
            SDL_Rect rc{16 + i * tw, 86, tw - 4, 56};
            if (ui::contains(rc, x, y)) {
                tab_ = i; offset_ = 0; focus_ = i + 1; detailId_.clear();
                return Action::None;
            }
        }
    }

    // 4. Tab 0: Overview Interactive Areas
    if (tab_ == 0) {
        const int px = g.left, py = g.top, cw = g.width;
        // Period segmented toggle: Hafta | Ay | Yıl
        int segW = 90, segX = px + cw - 3 * segW;
        if (y >= py && y <= py + 52) {
            for (int p = 0; p < 3; ++p) {
                if (x >= segX + p * segW && x < segX + (p + 1) * segW) {
                    period_ = p; return Action::None;
                }
            }
        }
        // Touch targets come from the exact same geometry as the render pass.
        const bool stacked = h >= 900 && cw < 900;
        const int botY = py + 70 + 224 + (stacked ? 248 : 12) + (stacked ? 200 : 100);
        const int halfW = stacked ? cw : (cw - 14) / 2;
        const int botH = stacked ? std::min(164,std::max(112,(h-botY-34)/2))
                                 : std::min(160,h-botY-16);
        const int currentX = px, recentX=stacked?px:px+halfW+14;
        const int recentY = stacked?botY+botH+12:botY;
        if (ui::contains({currentX+halfW-122,botY+64,110,56},x,y)) {
            const auto cr=journal.currentlyReading();
            if(!cr.first.empty()){detailId_=cr.first;tab_=2;offset_=0;}
            return Action::None;
        }
        if (ui::contains({currentX+halfW-160,botY+8,148,56},x,y)) {
            tab_=2;offset_=0;return Action::None;
        }
        if (ui::contains({recentX+halfW-160,recentY+8,148,56},x,y)) {
            tab_=3;offset_=0;return Action::None;
        }
    }

    // 5. Tab 1: Calendar Interactive Areas
    if (tab_ == 1) {
        const int px = g.left, py = g.top, cw = g.width;
        int calW = (cw >= 700 && !(h>=900 && cw<900)) ? (cw*62/100) : cw;
        // Month navigation buttons line up with calRc.y+8.
        if (y >= py + 62 && y < py + 118) {
            if (x >= px + calW - 148 && x <= px + calW - 80) { monthDelta_--; selectedDay_ = 0; return Action::None; }
            if (x >= px + calW - 76 && x <= px + calW - 8) { monthDelta_++; selectedDay_ = 0; return Action::None; }
        }
        // Day cells
        auto d = dateAt(monthDelta_);
        int first = (d.tm_wday + 6) % 7;
        int cellW = std::max(56, (calW - 28) / 7), y0 = py + 146;
        if (y >= y0) {
            if(x<px+10 || x>=px+10+7*cellW)return Action::None;
            int col = (x - px - 10) / cellW;
            int row = (y - y0) / 58;
            int day = row * 7 + col - first + 1;
            if (col >= 0 && col < 7 && day >= 1 && day <= daysInMonth(d.tm_year + 1900, d.tm_mon)) {
                selectedDay_ = day; return Action::None;
            }
        }
    }

    // 6. Tab 2: Books Interactive Areas
    if (tab_ == 2) {
        if (!detailId_.empty()) {
            // Drilldown back button
            if (y >= g.top && y <= g.top + 56 && x >= g.left && x <= g.left + 220) {
                detailId_.clear(); return Action::None;
            }
        } else {
            const auto b = journal.books();
            int n = 0;
            for (std::size_t i = std::size_t(offset_); i < b.size(); ++i) {
                int rowY = g.top + 52 + n++ * 104;
                if (rowY + 98 > h) break;
                // "Detay" button
                if (y >= rowY + 16 && y <= rowY + 76 &&
                    x >= g.left + g.width - 156 && x <= g.left + g.width - 14) {
                    detailId_ = b[i].id; offset_ = 0; return Action::None;
                }
            }
        }
    }

    // 7. Tab 3: History Interactive Areas
    if (tab_ == 3) {
        int n = 0;
        for (auto it = journal.sessions().rbegin(); it != journal.sessions().rend(); ++it) {
            if (n++ < offset_) continue;
            int rowY = g.top + 52 + (n - offset_ - 1) * 88;
            if (rowY + 80 > h) break;
            // Delete button [Sil]
            if (y >= rowY + 12 && y <= rowY + 68 && x >= g.left + g.width - 84 && x <= g.left + g.width - 12) {
                sessionToDelete_ = it->id;
                deleteModal_ = true;
                return Action::None;
            }
        }
    }

    return Action::None;
}

void ReadingJournalUI::render(SDL_Renderer *r, Typography &font, const ThemePalette &pal,
                             const ReadingJournal &journal,const ProgressStore &progress,
                             const CatalogStore &catalog,int w, int h, bool en) {
    header(r, font, pal, w, tab_, en);
    sidebar(r, font, pal, w, h, tab_, focus_, en);

    const auto g = geometry(w);
    const int px = g.left, py = g.top, cw = g.width;

    if (!detailId_.empty()) {
        // Book Details Drilldown
        auto bTotal = journal.bookTotal(detailId_);
        bTotal.progress=liveProgress(detailId_,bTotal.progress,progress,catalog);
        ui::button(r, font, {px, py, 200, 56}, label(en, "Kitaplarıma dön", "Back to books"), pal);
        SDL_Rect headRc{px, py + 66, cw, 140};
        panel(r, pal, headRc);
        drawCover(r, pal, font, {headRc.x + 16, headRc.y + 14, 76, 112}, bTotal.title, bTotal.author);
        text(r, font, bTotal.title.empty() ? label(en, "Kitap Detayı", "Book Details") : bTotal.title,
             headRc.x + 104, headRc.y + 16, pal.fg, headRc.w - 120, true);
        text(r, font, bTotal.author, headRc.x + 104, headRc.y + 48, pal.muted, headRc.w - 120);

        font.drawProgressBar(r, headRc.x + 104, headRc.y + 80, headRc.w - 130, 8, static_cast<float>(bTotal.progress),
                             blendColor(pal.card, pal.cardBorder, 60), pal.accent);
        char metaBuf[128];
        std::snprintf(metaBuf, sizeof metaBuf, "%%%d %s  ·  %s  ·  %zu %s",
                      int(bTotal.progress * 100.0), label(en, "tamamlandı", "completed").c_str(),
                      duration(bTotal.activeMilliseconds, en).c_str(),
                      bTotal.sessions, label(en, "oturum", "sessions").c_str());
        textSmall(r, font, metaBuf, headRc.x + 104, headRc.y + 98, pal.muted, headRc.w - 120);

        text(r, font, label(en, "Bu Kitabın Oturum Geçmişi", "Session History for this Book"),
             px, py + 220, pal.fg, cw, true);
        const auto bSessions = journal.sessionsForBook(detailId_);
        if (bSessions.empty()) {
            panel(r, pal, {px, py + 260, cw, 100});
            text(r, font, label(en, "Kayıtlı oturum bulunamadı.", "No recorded sessions found."), px + 20, py + 295, pal.muted, cw - 40);
        } else {
            int n = 0;
            for (std::size_t i = std::size_t(offset_); i < bSessions.size(); ++i) {
                int rowY = py + 260 + n++ * 76;
                if (rowY + 70 > h) break;
                panel(r, pal, {px, rowY, cw, 68});
                text(r, font, "•", px + 18, rowY + 18, pal.accent);
                text(r, font, localDate(bSessions[i].endedUtc) + " · " + localTime(bSessions[i].startedUtc) + "–" + localTime(bSessions[i].endedUtc),
                     px + 48, rowY + 14, pal.fg, cw - 260);
                char pBuf[64];
                std::snprintf(pBuf, sizeof pBuf, "%%%d -> %%%d", int(bSessions[i].startProgress * 100.0), int(bSessions[i].endProgress * 100.0));
                text(r, font, pBuf, px + 48, rowY + 40, pal.muted, cw - 260);
                text(r, font, duration(bSessions[i].activeMilliseconds, en), px + cw - 180, rowY + 22, pal.accent, 160);
            }
        }
        return;
    }

    if (journal.totals().sessions == 0) {
        empty(r, font, pal, {px, py, cw, std::min(h - py - 24, 260)}, en);
        return;
    }

    // Tab 0: Genel Bakış (Overview)
    if (tab_ == 0) {
        auto stats = journal.calculatePeriodStats(period_, 0, en);
        // Hero row: Eyebrow + Title & Segmented Period Toggle
        text(r, font, label(en, "OKUMA ÖZETİ", "READING SUMMARY"), px, py - 4, pal.accent);
        text(r, font, stats.title, px, py + 27, pal.fg, 260, true);

        int segW = 90, segX = px + cw - 3 * segW;
        const char *pTr[] = {"Hafta", "Ay", "Yıl"};
        const char *pEn[] = {"Week", "Month", "Year"};
        for (int p = 0; p < 3; ++p) {
            SDL_Rect rc{segX + p * segW, py, segW - 2, 52};
            ui::button(r, font, rc, en ? pEn[p] : pTr[p], pal, p == period_, false, false);
        }

        // Top Grid: Left Chart Panel & Right Goal Panel
        const bool portraitStack = (h >= 900 && cw < 900);
        int wide = (!portraitStack && cw >= 700) ? (cw * 62 / 100) : cw;
        int gridY = py + 70;
        int topCardH = 224;

        // Panel 1: Active Reading Time & Chart
        SDL_Rect chartRc{px, gridY, wide, topCardH};
        panel(r, pal, chartRc);
        textSmall(r, font, label(en, "Toplam aktif okuma süresi", "Total active reading time"), chartRc.x + 20, chartRc.y + 12, pal.muted, wide - 40);
        text(r, font, duration(stats.activeMilliseconds, en), chartRc.x + 20, chartRc.y + 36, pal.fg, wide - 40, true);

        // Trend Pill
        SDL_Rect trendPill{chartRc.x + 20, chartRc.y + 72, std::min(wide - 40, 260), 26};
        ui::surface(r, trendPill, blendColor(pal.card, pal.accent, 22), 13);
        textSmall(r, font, stats.trendText, trendPill.x + 10, trendPill.y + 4, pal.accent, trendPill.w - 20);

        // Bar Chart with clean vertical clearance
        if (!stats.barValues.empty()) {
            std::uint64_t maxVal = 1;
            for (auto v : stats.barValues) maxVal = std::max(maxVal, v);
            int barAreaY = chartRc.y + 104;
            int barCount = static_cast<int>(stats.barValues.size());
            int barStep = (wide - 48) / barCount;
            int barW = std::clamp(barStep - 10, 8, 36);

            for (int i = 0; i < barCount; ++i) {
                int bX = chartRc.x + 24 + i * barStep + (barStep - barW) / 2;
                int bH = static_cast<int>(50.0 * (double(stats.barValues[i]) / double(maxVal)));
                bH = std::max(4, bH);
                SDL_Rect bRc{bX, barAreaY + 50 - bH, barW, bH};
                SDL_Color bCol = (i == stats.highlightBar) ? pal.accent : blendColor(pal.card, pal.accent, 45);
                ui::surface(r, bRc, bCol, 5);
                textSmall(r, font, stats.barLabels[i], bX - 2, barAreaY + 54, pal.muted, barStep);
            }
        }
        line(r, pal.cardBorder, chartRc.x + 20, chartRc.y + topCardH - 28, wide - 40);
        textSmall(r, font, stats.range, chartRc.x + 20, chartRc.y + topCardH - 24, pal.muted, wide - 40);

        // Panel 2: Goal Widget
        if (wide < cw || portraitStack) {
            SDL_Rect goalRc = portraitStack ? SDL_Rect{px,gridY+topCardH+12,cw,topCardH}
                                            : SDL_Rect{px+wide+14,gridY,cw-wide-14,topCardH};
            panel(r, pal, goalRc);
            textSmall(r, font, label(en, "Günlük okuma hedefim", "Daily reading goal"), goalRc.x + 18, goalRc.y + 12, pal.muted, goalRc.w - 36);
            std::string goalBadge = journal.dailyGoalMinutes() ? (std::to_string(journal.dailyGoalMinutes()) + (en ? " min/day" : " dk/gün")) : label(en, "Kapalı", "Disabled");
            textSmall(r, font, goalBadge, goalRc.x + goalRc.w - 120, goalRc.y + 40, pal.accent, 110);

            // Ring with centered text
            int cx = goalRc.x + goalRc.w / 2, cy = goalRc.y + 92, radius = 48, thickness = 9;
            float gFrac = stats.totalPeriodDays ? float(stats.goalDaysMet) / float(stats.totalPeriodDays) : 0.f;
            drawRing(r, pal, cx, cy, radius, thickness, gFrac);
            char gNum[32];
            std::snprintf(gNum, sizeof gNum, "%u/%u", stats.goalDaysMet, stats.totalPeriodDays);
            text(r, font, gNum, cx - 24, cy - 18, pal.fg, 48, true);
            textSmall(r, font, label(en, "gün", "days"), cx - 18, cy + 12, pal.muted, 36);

            // Copy & progress dots
            std::string copy = journal.dailyGoalMinutes() ? (stats.goalDaysMet >= stats.totalPeriodDays / 2 ? label(en, "Harika gidiyorsun!", "Great progress!") : label(en, "Küçük adımlarla alışkanlık", "Building habits step by step")) : label(en, "Hedef belirlemek için dokun", "Tap Goals to set a daily target");
            textSmall(r, font, copy, goalRc.x + 14, goalRc.y + 154, pal.fg, goalRc.w - 28);

            // 7 dots for week
            int dotCount = std::min(7, int(stats.totalPeriodDays));
            int dotStep = 18;
            int dotStartX = cx - (dotCount * dotStep) / 2;
            for (int k = 0; k < dotCount; ++k) {
                SDL_Rect dRc{dotStartX + k * dotStep, goalRc.y + 190, 10, 8};
                SDL_Color dCol = (k < int(stats.goalDaysMet)) ? pal.accent : blendColor(pal.card, pal.cardBorder, 70);
                ui::surface(r, dRc, dCol, 4);
            }
        }

        // Mid Row: 4 Stat Tiles with live finished books count
        int tileY = gridY + topCardH + (portraitStack ? topCardH + 24 : 12);
        const int tileColumns = portraitStack ? 2 : 4;
        int tileW = (cw - (tileColumns - 1) * 12) / tileColumns;
        int tileH = 88;
        const std::string sLabels[] = {
            label(en, "Okunan gün", "Days read"),
            label(en, "Ortalama oturum", "Avg session"),
            label(en, "Bitirilen kitap", "Books finished"),
            label(en, "Okuma serisi", "Reading streak")
        };

        std::size_t finishedCount = 0;
        const auto &allBooks = journal.books();
        for (const auto &b : allBooks) {
            if (liveProgress(b.id, b.progress, progress, catalog) >= 0.99) finishedCount++;
        }
        for (const auto &pr : progress.all()) {
            if (liveProgress(pr.first, pr.second.fraction, progress, catalog) >= 0.99) {
                bool exists = false;
                for (const auto &b : allBooks) { if (b.id == pr.first) { exists = true; break; } }
                if (!exists) finishedCount++;
            }
        }

        const std::string sValues[] = {
            std::to_string(stats.daysRead) + (en ? " days" : " gün"),
            std::to_string(stats.averageSessionMinutes) + (en ? " min" : " dk"),
            std::to_string(finishedCount),
            std::to_string(stats.streakDays) + (en ? " days" : " gün")
        };
        for (int i = 0; i < 4; ++i) {
            SDL_Rect tRc{px + (i % tileColumns) * (tileW + 12),
                         tileY + (i / tileColumns) * (tileH + 12), tileW, tileH};
            panel(r, pal, tRc);
            textSmall(r, font, sLabels[i], tRc.x + 14, tRc.y + 10, pal.muted, tileW - 20);
            text(r, font, sValues[i], tRc.x + 14, tRc.y + 40, pal.fg, tileW - 20, true);
        }

        // Bottom Row: Currently Reading & Recent Activities
        int botY = tileY + (portraitStack ? 2 : 1) * (tileH + 12);
        int botH = portraitStack ? std::min(164,std::max(112,(h-botY-34)/2)) : std::min(160, h - botY - 16);
        int halfW = portraitStack ? cw : (cw - 14) / 2;

        // Card 1: Currently Reading
        SDL_Rect cRc{px, botY, halfW, botH};
        panel(r, pal, cRc);
        text(r, font, label(en, "Şu anda okuduğun", "Currently reading"), cRc.x + 16, cRc.y + 10, pal.fg, halfW - 170, true);
        ui::button(r, font, {cRc.x + halfW - 160, cRc.y + 8, 148, 56}, label(en, "Kitaplarım", "My books"), pal);

        auto cr = journal.currentlyReading();
        if (!cr.first.empty()) {
            cr.second=liveProgress(cr.first,cr.second,progress,catalog);
            auto bTot = journal.bookTotal(cr.first);
            drawCover(r, pal, font, {cRc.x + 16, cRc.y + 54, 52, 78}, bTot.title, bTot.author);
            text(r, font, bTot.title, cRc.x + 80, cRc.y + 54, pal.fg, halfW - 210);
            textSmall(r, font, bTot.author, cRc.x + 80, cRc.y + 80, pal.muted, halfW - 210);
            font.drawProgressBar(r, cRc.x + 80, cRc.y + 104, halfW - 210, 6, static_cast<float>(cr.second),
                                 blendColor(pal.card, pal.cardBorder, 60), pal.accent);
            char pStr[64];
            std::snprintf(pStr, sizeof pStr, "%%%d  ·  %s", int(cr.second * 100.0), duration(bTot.activeMilliseconds, en).c_str());
            textSmall(r, font, pStr, cRc.x + 80, cRc.y + 116, pal.muted, halfW - 210);
            ui::button(r, font, {cRc.x + halfW - 122, cRc.y + 64, 110, 56}, label(en, "Detay", "Details"), pal, true);
        }

        // Card 2: Recent Activity
        SDL_Rect rRc{portraitStack ? px : px + halfW + 14,
                    portraitStack ? botY + botH + 12 : botY, halfW, botH};
        panel(r, pal, rRc);
        text(r, font, label(en, "Son etkinlikler", "Recent activity"), rRc.x + 16, rRc.y + 10, pal.fg, halfW - 170, true);
        ui::button(r, font, {rRc.x + halfW - 160, rRc.y + 8, 148, 56}, label(en, "Tümünü Gör", "See all"), pal);

        const auto &allSessions = journal.sessions();
        int sShown = 0;
        for (auto it = allSessions.rbegin(); it != allSessions.rend() && sShown < 2; ++it, ++sShown) {
            int itemY = rRc.y + 54 + sShown * 46;
            text(r, font, "•", rRc.x + 16, itemY + 4, pal.accent);
            text(r, font, it->title, rRc.x + 40, itemY, pal.fg, halfW - 160);
            textSmall(r, font, localDate(it->endedUtc), rRc.x + 40, itemY + 22, pal.muted, halfW - 160);
            textSmall(r, font, duration(it->activeMilliseconds, en), rRc.x + halfW - 110, itemY + 8, pal.accent, 96);
        }
    } else if (tab_ == 1) {
        // Tab 1: Takvim (Calendar)
        text(r, font, label(en, "OKUMA TAKVİMİM", "READING CALENDAR"), px, py - 4, pal.accent);
        textSmall(r, font, label(en, "Hangi günler okuduğunu ve sürelerini gör", "Track your active reading days"), px, py + 25, pal.muted, cw - 200);

        const bool tallCalendar=(h>=900 && cw<900);
        int calW = (cw >= 700 && !tallCalendar) ? (cw * 62 / 100) : cw;
        int gridH = tallCalendar ? 500 : h - py - 58;
        SDL_Rect calRc{px, py + 54, calW, gridH};
        panel(r, pal, calRc);

        auto d = dateAt(monthDelta_);
        const char *trM[] = {"Ocak", "Şubat", "Mart", "Nisan", "Mayıs", "Haziran", "Temmuz", "Ağustos", "Eylül", "Ekim", "Kasım", "Aralık"};
        const char *enM[] = {"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};
        char mHead[64];
        std::snprintf(mHead, sizeof mHead, "%s %d", en ? enM[d.tm_mon] : trM[d.tm_mon], d.tm_year + 1900);
        text(r, font, mHead, calRc.x + 20, calRc.y + 14, pal.fg, calW - 180, true);
        ui::button(r, font, {calRc.x + calW - 148, calRc.y + 8, 64, 56}, "<", pal);
        ui::button(r, font, {calRc.x + calW - 76, calRc.y + 8, 64, 56}, ">", pal);

        const char *daysTr[] = {"Pzt", "Sal", "Çar", "Per", "Cum", "Cmt", "Paz"};
        const char *daysEn[] = {"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};
        int first = (d.tm_wday + 6) % 7;
        int cellW = std::max(56, (calW - 28) / 7);
        for (int i = 0; i < 7; ++i) {
            textSmall(r, font, en ? daysEn[i] : daysTr[i], calRc.x + 12 + i * cellW + 12, calRc.y + 64, pal.muted, cellW - 16);
        }

        const auto cal = journal.calendar();
        const auto now = nowDate();
        int maxD = daysInMonth(d.tm_year + 1900, d.tm_mon);
        for (int k = 1; k <= maxD; ++k) {
            int z = first + k - 1, row = z / 7, col = z % 7;
            std::uint64_t ms = dayMs(cal, d.tm_year + 1900, d.tm_mon + 1, k);
            int intensity = (ms == 0) ? 0 : (ms < 1800000) ? 20 : (ms < 3600000) ? 40 : (ms < 5400000) ? 65 : 90;
            SDL_Rect box{calRc.x + 10 + col * cellW, calRc.y + 92 + row * 58, cellW - 6, 52};
            SDL_Color fill = blendColor(pal.card, pal.accent, intensity);
            ui::surface(r, box, fill, 10);
            if (k == selectedDay_ || (selectedDay_ == 0 && k == now.tm_mday && monthDelta_ == 0)) {
                ui::focusRing(r, box, pal);
            }
            SDL_Color tC = (intensity > 70) ? pal.bg : pal.fg;
            text(r, font, std::to_string(k), box.x + 8, box.y + 4, tC, box.w - 12);
            if (ms >= 60000) {
                textSmall(r, font, std::to_string(ms / 60000) + (en ? "m" : " dk"), box.x + 6, box.y + 28, tC, box.w - 8);
            } else if (ms > 0) {
                textSmall(r, font, std::to_string(std::max<std::uint64_t>(1, ms / 1000)) + (en ? "s" : " sn"), box.x + 6, box.y + 28, tC, box.w - 8);
            }
        }

        // Legend below calendar
        int legY = calRc.y + gridH - 32;
        textSmall(r, font, label(en, "Az", "Less"), calRc.x + 16, legY, pal.muted, 30);
        for (int lvl = 0; lvl < 5; ++lvl) {
            int intVal = lvl == 0 ? 0 : lvl == 1 ? 20 : lvl == 2 ? 40 : lvl == 3 ? 65 : 90;
            SDL_Rect lBox{calRc.x + 46 + lvl * 20, legY + 2, 14, 14};
            ui::surface(r, lBox, blendColor(pal.card, pal.accent, intVal), 3);
        }
        textSmall(r, font, label(en, "Çok  0/30/60/90+ dk", "More  0/30/60/90+ min"), calRc.x + 154, legY, pal.muted, 180);

        // Daily Detail Panel (Right Side)
        if (calW < cw || tallCalendar) {
            int detX = tallCalendar ? px : px + calW + 14;
            int detW = tallCalendar ? cw : cw - calW - 14;
            SDL_Rect detRc = tallCalendar ? SDL_Rect{detX,calRc.y+gridH+16,detW,
                                                  std::max(200,h-(calRc.y+gridH+16)-24)}
                                           : SDL_Rect{detX,py+54,detW,gridH};
            panel(r, pal, detRc);

            int dDay = selectedDay_ ? selectedDay_ : (monthDelta_ == 0 ? now.tm_mday : 1);
            char dStr[64];
            std::snprintf(dStr, sizeof dStr, "%d %s %d", dDay, en ? enM[d.tm_mon] : trM[d.tm_mon], d.tm_year + 1900);
            textSmall(r, font, label(en, "GÜNLÜK AYRINTI", "DAILY DETAIL"), detX + 18, detRc.y + 14, pal.accent);
            text(r, font, dStr, detX + 18, detRc.y + 36, pal.fg, detW - 36, true);

            std::uint64_t selMs = dayMs(cal, d.tm_year + 1900, d.tm_mon + 1, dDay);
            textSmall(r, font, label(en, "Aktif okuma süresi:", "Active reading time:"), detX + 18, detRc.y + 80, pal.muted, detW - 36);
            text(r, font, duration(selMs, en), detX + 18, detRc.y + 104, pal.accent, detW - 36, true);
            line(r, pal.cardBorder, detX + 18, detRc.y + 148, detW - 36);

            text(r, font, label(en, "O gün okunan kitaplar", "Books read on this day"), detX + 18, detRc.y + 160, pal.fg, detW - 36);
            char dayK[32];
            std::snprintf(dayK, sizeof dayK, "%04d-%02d-%02d", d.tm_year + 1900, d.tm_mon + 1, dDay);
            auto dSessions = journal.sessionsForDay(dayK);
            if (dSessions.empty()) {
                textSmall(r, font, label(en, "Bu gün için kayıtlı oturum yok.", "No recorded reading sessions."), detX + 18, detRc.y + 196, pal.muted, detW - 36);
            } else {
                int nS = 0;
                for (const auto &s : dSessions) {
                    if (nS++ >= 4) break;
                    int sY = detRc.y + 196 + (nS - 1) * 64;
                    panel(r, pal, {detX + 18, sY, detW - 36, 56});
                    ui::surface(r, {detX + 30, sY + 24, 8, 8}, pal.accent, 4);
                    text(r, font, s.title, detX + 48, sY + 8, pal.fg, detW - 180);
                    textSmall(r, font, duration(s.activeMilliseconds, en), detX + detW - 120, sY + 18, pal.accent, 96);
                }
            }
        }
    } else if (tab_ == 2) {
        // Tab 2: Kitaplarım (Books)
        text(r, font, label(en, "KİTAPLARA GÖRE", "BOOKS READING TIME"), px, py - 4, pal.accent);
        textSmall(r, font, label(en, "İlerleme, aktif süre ve oturum geçmişi", "Progress, active time and sessions"), px, py + 25, pal.muted, cw - 200);

        auto bList = journal.books();
        for(auto &bk:bList)bk.progress=liveProgress(bk.id,bk.progress,progress,catalog);
        int n = 0;
        for (std::size_t i = std::size_t(offset_); i < bList.size(); ++i) {
            int rowY = py + 52 + n++ * 104;
            if (rowY + 98 > h) break;
            SDL_Rect bRc{px, rowY, cw, 96};
            panel(r, pal, bRc);
            drawCover(r, pal, font, {bRc.x + 14, bRc.y + 12, 46, 72}, bList[i].title, bList[i].author, int(i));
            text(r, font, bList[i].title, bRc.x + 72, bRc.y + 10, pal.fg, cw - 250, false);
            textSmall(r, font, bList[i].author, bRc.x + 72, bRc.y + 38, pal.muted, cw - 250);

            font.drawProgressBar(r, bRc.x + 72, bRc.y + 62, cw - 250, 6, static_cast<float>(bList[i].progress),
                                 blendColor(pal.card, pal.cardBorder, 60), pal.accent);
            char bInfo[128];
            std::snprintf(bInfo, sizeof bInfo, "%%%d  ·  %s  ·  %zu %s",
                          int(bList[i].progress * 100.0), duration(bList[i].activeMilliseconds, en).c_str(),
                          bList[i].sessions, label(en, "oturum", "sessions").c_str());
            textSmall(r, font, bInfo, bRc.x + 72, bRc.y + 72, pal.muted, cw - 250);
            ui::button(r, font, {bRc.x + cw - 156, bRc.y + 20, 142, 56}, label(en, "Detay", "Details"), pal);
        }
    } else {
        // Tab 3: Geçmiş (History / Journal)
        text(r, font, label(en, "OKUMA GEÇMİŞİM", "SESSION HISTORY"), px, py - 4, pal.accent);
        textSmall(r, font, label(en, "Kronolojik okuma oturumları ve kayıt silme", "Chronological reading sessions log"), px, py + 25, pal.muted, cw - 360);

        int n = 0;
        const auto &sList = journal.sessions();
        for (auto it = sList.rbegin(); it != sList.rend(); ++it) {
            if (n++ < offset_) continue;
            int rowY = py + 52 + (n - offset_ - 1) * 88;
            if (rowY + 80 > h) break;
            SDL_Rect sRc{px, rowY, cw, 80};
            panel(r, pal, sRc);
            text(r, font, "•", sRc.x + 18, rowY + 22, pal.accent);
            text(r, font, it->title, sRc.x + 48, rowY + 12, pal.fg, cw - 280, false);

            char subStr[128];
            std::snprintf(subStr, sizeof subStr, "%s · %s–%s · %%%d -> %%%d",
                          localDate(it->endedUtc).c_str(),
                          localTime(it->startedUtc).c_str(), localTime(it->endedUtc).c_str(),
                          int(it->startProgress * 100.0), int(it->endProgress * 100.0));
            textSmall(r, font, subStr, sRc.x + 48, rowY + 44, pal.muted, cw - 280);

            textSmall(r, font, duration(it->activeMilliseconds, en), sRc.x + cw - 180, rowY + 26, pal.accent, 90);
            ui::button(r, font, {sRc.x + cw - 84, rowY + 12, 72, 56}, label(en, "Sil", "Del"), pal, false, false, false, false, false, false, true);
        }
    }

    // 8. Modals
    if (goals_) {
        SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(r, 0, 0, 0, 160);
        SDL_Rect dim{0, 0, w, h}; SDL_RenderFillRect(r, &dim);

        const int pw = 520, ph = 320, pxM = (w - pw) / 2, pyM = (h - ph) / 2;
        panel(r, pal, {pxM, pyM, pw, ph});
        textSmall(r, font, label(en, "OKUMA HEDEFLERİ", "READING GOALS"), pxM + 24, pyM + 18, pal.accent);
        text(r, font, label(en, "Okuma hedeflerim", "My Reading Goals"), pxM + 24, pyM + 42, pal.fg, pw - 48, true);
        textSmall(r, font, label(en, "Hedefler isteğe bağlıdır. Günlük okuma hedefini buradan ayarla.",
                                     "Goals are optional. Adjust your daily reading target below."),
                  pxM + 24, pyM + 80, pal.muted, pw - 48);

        std::string curG = journal.dailyGoalMinutes() ? (std::to_string(journal.dailyGoalMinutes()) + (en ? " min / day" : " dakika / gün")) : label(en, "Kapalı (Hedef yok)", "Disabled (No goal)");
        text(r, font, curG, pxM + 24, pyM + 116, pal.fg, pw - 48, true);

        ui::button(r, font, {pxM + 24, pyM + 160, 110, 56}, "-5 dk", pal);
        ui::button(r, font, {pxM + 146, pyM + 160, 110, 56}, "+5 dk", pal);
        ui::button(r, font, {pxM + 268, pyM + 160, 105, 56}, label(en, "Kapalı", "Off"), pal, journal.dailyGoalMinutes() == 0);
        ui::button(r, font, {pxM + 385, pyM + 160, 105, 56}, "30 dk", pal, journal.dailyGoalMinutes() == 30);
        ui::button(r, font, {pxM + 24, pyM + 236, pw - 48, 56}, label(en, "Tamam", "Done"), pal, true);
    }

    if (exportModal_) {
        SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(r, 0, 0, 0, 160);
        SDL_Rect dim{0, 0, w, h}; SDL_RenderFillRect(r, &dim);

        const int pw = 520, ph = 320, pxM = (w - pw) / 2, pyM = (h - ph) / 2;
        panel(r, pal, {pxM, pyM, pw, ph});
        textSmall(r, font, label(en, "VERİ DIŞA AKTARMA", "DATA EXPORT"), pxM + 24, pyM + 18, pal.accent);
        text(r, font, label(en, "Okuma verilerini dışa aktar", "Export Reading Data"), pxM + 24, pyM + 42, pal.fg, pw - 48, true);
        textSmall(r, font, label(en, "Oturum geçmişini JSON ve CSV formatında microSD karta kaydeder.",
                                     "Saves reading session history in JSON and CSV format to microSD."),
                  pxM + 24, pyM + 80, pal.muted, pw - 48);
        std::string exportDirStr = journal.exportDirectory().string();
        textSmall(r, font, label(en, "Konum: ", "Path: ") + exportDirStr, pxM + 24, pyM + 108, pal.muted, pw - 48);

        if (!exportMessage_.empty()) {
            textSmall(r, font, exportMessage_, pxM + 24, pyM + 136, pal.accent, pw - 48);
        }

        ui::button(r, font, {pxM + 24, pyM + 170, 220, 56}, label(en, "JSON Olarak Aktar", "Export JSON"), pal);
        ui::button(r, font, {pxM + 260, pyM + 170, 220, 56}, label(en, "CSV Olarak Aktar", "Export CSV"), pal);
        ui::button(r, font, {pxM + 24, pyM + 240, pw - 48, 56}, label(en, "Kapat", "Close"), pal, true);
    }

    if (deleteModal_) {
        SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(r, 0, 0, 0, 160);
        SDL_Rect dim{0, 0, w, h}; SDL_RenderFillRect(r, &dim);

        const int pw = 480, ph = 260, pxM = (w - pw) / 2, pyM = (h - ph) / 2;
        panel(r, pal, {pxM, pyM, pw, ph});
        textSmall(r, font, label(en, "OTURUMU SİL", "DELETE SESSION"), pxM + 24, pyM + 18, pal.accent);
        text(r, font, label(en, "Bu oturumu silmek istiyor musun?", "Delete this reading session?"),
             pxM + 24, pyM + 44, pal.fg, pw - 48, true);
        font.renderWrapped(r, font.smallFont(),
                           label(en, "Bu işlem kalıcıdır. Gün ve kitap toplamları yeniden hesaplanacaktır.",
                                     "This action is permanent. All totals and calendar stats will update."),
                           {pxM + 24, pyM + 86, pw - 48, 64}, pal.muted);

        ui::button(r, font, {pxM + 24, pyM + 180, 200, 56}, label(en, "Sil (Kalıcı)", "Delete"), pal, false, false, false, false, false, false, true);
        ui::button(r, font, {pxM + 240, pyM + 180, 200, 56}, label(en, "Vazgeç", "Cancel"), pal);
    }
}

void ReadingJournalUI::renderPreview(SDL_Renderer *r, Typography &font, const ThemePalette &pal,
                                     const ReadingJournal &j, const std::string &bookId, int w, int h, bool en) {
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r, 0, 0, 0, 150);
    SDL_Rect cover{0, 0, w, h}; SDL_RenderFillRect(r, &cover);

    const int pw = std::min(620, w - 24), px = (w - pw) / 2, py = (h - 320) / 2;
    panel(r, pal, {px, py, pw, 320});
    text(r, font, label(en, "Bu kitabın istatistikleri", "This book's statistics"), px + 24, py + 22, pal.fg, pw - 48, true);
    text(r, font, label(en, "Bugün", "Today"), px + 24, py + 104, pal.muted, pw - 48);

    const auto today = nowDate();
    const auto days = j.calendar();
    text(r, font, duration(dayMs(days, today.tm_year + 1900, today.tm_mon + 1, today.tm_mday), en),
         px + 24, py + 138, pal.fg, pw / 2);

    text(r, font, label(en, "Bu kitap", "This book"), px + pw / 2, py + 104, pal.muted, pw / 2 - 24);
    text(r, font, duration(j.bookMilliseconds(bookId), en), px + pw / 2, py + 138, pal.fg, pw / 2 - 24);

    ui::button(r, font, {px + 18, py + 244, pw / 2 - 28, 56}, label(en, "Günlüğe git", "Open journal"), pal, true);
    ui::button(r, font, {px + pw / 2 + 10, py + 244, pw / 2 - 28, 56}, label(en, "Kapat", "Close"), pal);
}
} // namespace papernx
