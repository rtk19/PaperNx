#define main papernx_application_entry
#include "../src/main.cpp"
#undef main
#include <cassert>
#include <iostream>

namespace {
struct AppTest {
    static void preview(PaperNxApp &app, const char *direction) {
        fs::create_directories("verification/page-curl");
        for (int frame = 0; frame <= 10; ++frame) {
            app.pageTurnAnimation_.started = SDL_GetTicks() - frame * 40;
            app.render();
            int w, h; SDL_GetRendererOutputSize(app.renderer_, &w, &h);
            auto surface = SDL_CreateRGBSurfaceWithFormat(0, w, h, 32, SDL_PIXELFORMAT_ARGB8888);
            assert(surface);
            assert(SDL_RenderReadPixels(app.renderer_, nullptr, surface->format->format,
                                        surface->pixels, surface->pitch) == 0);
            auto path = std::string("verification/page-curl/epub-") + direction + "-" + std::to_string(frame) + ".bmp";
            assert(SDL_SaveBMP(surface, path.c_str()) == 0);
            SDL_FreeSurface(surface);
        }
    }
    static void finish(PaperNxApp &app) {
        assert(app.pageTurnAnimation_.active);
        app.pageTurnAnimation_.started = SDL_GetTicks() - PaperNxApp::PAGE_TURN_MS;
        app.tickPageTurn();
    }
    static void run(const fs::path &root) {
        PaperNxApp app(root); assert(app.init());
        auto epub = *std::find_if(app.books_.begin(), app.books_.end(), [](const Book &b) { return b.format == Format::Epub; });
        app.openBook(epub);
        app.extraStyle_.readingMode = 0;
        app.documentMode_ = 0;
        app.reflowEpub(22);
        assert(app.pageCount_ >= 3);
        app.currentPage_ = 0;
        app.turnPage(-1); assert(!app.pageTurnAnimation_.active && app.currentPage_ == 0);
        app.turnPage(1);
        assert(app.currentPage_ == 1 && app.pageTurnAnimation_.active);
        assert(app.pageTurnAnimation_.previous && app.pageTurnAnimation_.next);
        preview(app, "forward");
        // Snapshots survive complete LRU eviction while rendering a curl frame.
        app.pageCache_.clear();
        app.pageTurnAnimation_.started = SDL_GetTicks() - 200;
        app.render();
        app.turnPage(1);
        assert(app.currentPage_ == 1 && app.pageTurnAnimation_.queued == 1);
        finish(app); assert(app.currentPage_ == 2 && app.pageTurnAnimation_.active);
        finish(app); assert(!app.pageTurnAnimation_.active);
        app.turnPage(-1); assert(app.pageTurnAnimation_.direction == -1);
        preview(app, "backward");
        finish(app); assert(app.currentPage_ == 1);
        app.turnPage(1); app.turnPage(-1); finish(app);
        assert(app.currentPage_ == 1 && app.pageTurnAnimation_.direction == -1);
        finish(app);
        app.turnPage(1); app.openSettings();
        assert(!app.pageTurnAnimation_.active && !app.pageTurnAnimation_.previous);
        app.dismissSettings();
        app.turnPage(-1); app.layoutPending_ = true; app.tickPageTurn();
        assert(!app.pageTurnAnimation_.active); app.layoutPending_ = false;
        app.documentMode_ = 1; app.turnPage(1); assert(!app.pageTurnAnimation_.active);
        app.documentMode_ = 0;
        app.turnPage(-1); app.appBackground_ = true; app.tickPageTurn();
        assert(!app.pageTurnAnimation_.active); app.appBackground_ = false;
        app.turnPage(1); app.closeBook(); assert(!app.pageTurnAnimation_.next);

        auto pdf = *std::find_if(app.books_.begin(), app.books_.end(), [](const Book &b) { return b.format == Format::Pdf; });
        app.openBook(pdf); app.currentPage_ = 0; app.setPdfView(0);
        float firstW, firstH, nextW, nextH;
        assert(px_document_page_size(app.doc_, 0, &firstW, &firstH));
        assert(px_document_page_size(app.doc_, 1, &nextW, &nextH));
        assert(firstW != nextW && firstH != nextH);
        app.turnPage(1); assert(app.pageTurnAnimation_.active); finish(app);
        app.turnPage(-1); app.zoomPdf(2.f); assert(!app.pageTurnAnimation_.active);
        app.turnPage(1); assert(!app.pageTurnAnimation_.active);
        app.setPdfView(0); app.turnPage(-1); assert(app.pageTurnAnimation_.active);
        app.currentPage_ = app.pageCount_ - 1; app.tickPageTurn();
        assert(!app.pageTurnAnimation_.active);
        app.turnPage(1); assert(!app.pageTurnAnimation_.active);
        app.closeBook();
    }
};
}

int main() {
    const auto root = fs::temp_directory_path() / "papernx-page-curl-regressions";
    fs::remove_all(root); fs::create_directories(root / "books");
    fs::copy_file("tests/fixtures/sample_book.epub", root / "books" / "sample.epub");
    fs::copy_file("tests/fixtures/sample_doc.pdf", root / "books" / "sample.pdf");
    // Keep xref offsets valid while changing only the second page's aspect ratio.
    const auto pdfPath = root / "books" / "sample.pdf";
    std::ifstream input(pdfPath, std::ios::binary);
    std::string pdf((std::istreambuf_iterator<char>(input)), {}); input.close();
    auto media = pdf.rfind("MediaBox [0 0 595 842]");
    assert(media != std::string::npos);
    pdf.replace(media, 22, "MediaBox [0 0 842 595]");
    std::ofstream(pdfPath, std::ios::binary).write(pdf.data(), pdf.size());
    fs::create_directory_symlink(fs::absolute("fonts"), root / "fonts");
    AppTest::run(root);
    fs::remove_all(root);
    std::cout << "[PASS] page curl EPUB/PDF, queue/reversal, cache eviction, cancellation and boundaries\n";
}
