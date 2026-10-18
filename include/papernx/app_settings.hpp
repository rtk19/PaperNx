#pragma once
#include "papernx/core.hpp"

namespace papernx {

struct AppPreferences {
    int uiTheme{0}; // Same curated palette indices as the reader; independent selection.
    int language{0}; // 0 Turkish, 1 English
    int libraryLayout{0}; // 0 grid, 1 list
    int sort{0}; // papernx::Sort
    int toolbar{0}; // 0 auto-hide, 1 persistent
    int chromePolicy{0}; // 0 follow reading, 1 application, 2 custom
    int customUiTheme{0};
    int displayOrientation{0}; // landscape, clockwise, counterclockwise
    bool preservePdfZoom{true};
    bool shoulderPageTurns{true};
    int paperTexture{0}; // off, soft, natural; EPUB canvas only
    std::string defaultBrowserPath;
};

// Separate file from reading progress, using ProgressStore's checksummed, recoverable
// atomic writes. Reserved APP_PREFERENCES_ record maps theme/uiTheme, fontFamily/language,
// textAlign/layout, (fontSize-12)/sort, pdfViewMode/toolbar and bookPath/browser path.
// The wire schema is versioned by PAPERNX-6; no application preference alters a book.
class AppSettingsStore {
public:
    explicit AppSettingsStore(fs::path path);
    bool load();
    bool save() const;
    const AppPreferences &get() const { return preferences_; }
    void put(AppPreferences preferences);
    const std::string &saveDiagnostic() const { return store_.saveDiagnostic(); }
private:
    mutable ProgressStore store_;
    AppPreferences preferences_;
};

} // namespace papernx
