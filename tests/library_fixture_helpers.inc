// Legacy fixture workflows wait for the asynchronous inventory. Production
// startup and the dedicated library suite exercise the non-blocking path.
static bool initReady(PaperNxApp &app) {
    if(!app.init())return false;
    const auto deadline=SDL_GetTicks()+10000;
    while(app.libraryScanner_.pending() && SDL_GetTicks()<deadline){app.processLibraryScan();SDL_Delay(2);}
    return !app.libraryScanner_.pending();
}
// Snapshot/workflow tests explicitly wait for a requested cold comic sheet.
// The performance suite separately drives real input without this wait.
static void awaitComicTurn(PaperNxApp &app) {
    const auto deadline=SDL_GetTicks()+20000;
    while((app.comicTurnQueued_ || app.comicTouchWaiting_) && SDL_GetTicks()<deadline) {
        app.processComicPages();app.tickPageTurn();SDL_Delay(2);
    }
    assert(!app.comicTurnQueued_ && !app.comicTouchWaiting_);
}
static void preparedTurn(PaperNxApp &app,int delta) {app.turnPage(delta);awaitComicTurn(app);}
