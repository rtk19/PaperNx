#include "papernx/mupdf_bridge.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

static uint64_t fnv1a_hash(const unsigned char *data, size_t len) {
    uint64_t hash = 14695981039346656037ULL;
    for (size_t i = 0; i < len; ++i) {
        hash ^= (uint64_t)data[i];
        hash *= 1099511628211ULL;
    }
    return hash;
}

static void save_ppm(const char *path, const PxImage *img) {
    if (!img || !img->pixels || img->w <= 0 || img->h <= 0) return;
    FILE *f = fopen(path, "wb");
    if (!f) return;
    fprintf(f, "P6\n%d %d\n255\n", img->w, img->h);
    if (img->components == 3) {
        fwrite(img->pixels, 1, (size_t)img->w * img->h * 3, f);
    } else if (img->components == 4) {
        // Strip alpha for P6 PPM
        unsigned char *rgb = (unsigned char *)malloc((size_t)img->w * img->h * 3);
        if (rgb) {
            for (int i = 0; i < img->w * img->h; ++i) {
                rgb[i * 3 + 0] = img->pixels[i * 4 + 0];
                rgb[i * 3 + 1] = img->pixels[i * 4 + 1];
                rgb[i * 3 + 2] = img->pixels[i * 4 + 2];
            }
            fwrite(rgb, 1, (size_t)img->w * img->h * 3, f);
            free(rgb);
        }
    }
    fclose(f);
    printf("Saved render snapshot to %s (%dx%d, %d components)\n", path, img->w, img->h, img->components);
}

int main(int argc, char **argv) {
    const char *epub_path = "tests/fixtures/sample_book.epub";
    const char *pdf_path = "tests/fixtures/sample_doc.pdf";
    const char *corrupt_epub = "tests/fixtures/corrupt.epub";
    const char *corrupt_pdf = "tests/fixtures/corrupt.pdf";

    if (argc >= 3) {
        epub_path = argv[1];
        pdf_path = argv[2];
    }

    printf("==========================================================\n");
    printf("PaperNX — Native MuPDF Rendering & Integration Test Suite\n");
    printf("==========================================================\n");

    // 1. Error Resiliency: Corrupt EPUB & PDF
    {
        char err[256] = {0};
        PxDocument *bad_epub = px_document_open(corrupt_epub, err, sizeof(err));
        assert(bad_epub == NULL);
        assert(strlen(err) > 0);
        printf("[PASS] Corrupted EPUB rejected safely without crash: %s\n", err);

        err[0] = '\0';
        PxDocument *bad_pdf = px_document_open(corrupt_pdf, err, sizeof(err));
        assert(bad_pdf == NULL);
        assert(strlen(err) > 0);
        printf("[PASS] Corrupted PDF rejected safely without crash: %s\n", err);
    }

    // 2. Real EPUB Document Verification
    {
        char err[256] = {0};
        PxDocument *epub = px_document_open(epub_path, err, sizeof(err));
        if (!epub) {
            fprintf(stderr, "FATAL: Failed to open %s: %s\n", epub_path, err);
            return 1;
        }

        assert(px_document_is_epub(epub) == 1);
        int page_count = px_document_page_count(epub);
        printf("EPUB opened successfully. Page count: %d\n", page_count);
        assert(page_count >= 1);

        char title[128] = {0};
        char author[128] = {0};
        px_document_get_title(epub, title, sizeof(title));
        px_document_get_author(epub, author, sizeof(author));
        printf("EPUB Title: '%s', Author: '%s'\n", title, author);
        assert(strstr(title, "İstanbul") != NULL);
        assert(strstr(author, "Tanpınar") != NULL);

        // Layout EPUB for 1280x720 screen
        int layout_ok = px_document_layout_epub(epub, 1280.0f, 720.0f, 24.0f, 150, 40, 0, err, sizeof(err));
        assert(layout_ok == 0);
        int pages_after_layout = px_document_page_count(epub);
        printf("EPUB pages after layout: %d\n", pages_after_layout);
        assert(pages_after_layout >= 1);

        // Render Page 0
        PxImage img0 = {0};
        int r0 = px_document_render_epub(epub, 0, 1280, 720, 0, &img0, err, sizeof(err));
        assert(r0 == 0);
        assert(img0.pixels != NULL);
        assert(img0.w > 0 && img0.h > 0);
        uint64_t hash_epub0 = fnv1a_hash(img0.pixels, (size_t)img0.w * img0.h * img0.components);
        printf("[PASS] EPUB Page 0 rendered: %dx%d, hash: 0x%016llx\n", img0.w, img0.h, (unsigned long long)hash_epub0);
        save_ppm("tests/fixtures/rendered_epub_page0.ppm", &img0);
        px_image_free(&img0);

        // Layout with different font size & verify reflow changes hash
        px_document_layout_epub(epub, 1280.0f, 720.0f, 36.0f, 180, 80, 0, err, sizeof(err));
        PxImage img_reflow = {0};
        int r_reflow = px_document_render_epub(epub, 0, 1280, 720, 0, &img_reflow, err, sizeof(err));
        assert(r_reflow == 0);
        uint64_t hash_reflow = fnv1a_hash(img_reflow.pixels, (size_t)img_reflow.w * img_reflow.h * img_reflow.components);
        printf("[PASS] EPUB Page 0 reflowed (larger font): %dx%d, hash: 0x%016llx\n", img_reflow.w, img_reflow.h, (unsigned long long)hash_reflow);
        assert(hash_epub0 != hash_reflow);
        px_image_free(&img_reflow);

        // Test TOC outline
        PxTocEntry toc[16];
        int toc_count = px_document_get_toc(epub, toc, 16);
        printf("EPUB TOC count: %d\n", toc_count);
        assert(toc_count >= 2);
        printf("TOC[0]: '%s' -> page %d\n", toc[0].title, toc[0].page);
        printf("TOC[1]: '%s' -> page %d\n", toc[1].title, toc[1].page);
        assert(strstr(toc[0].title, "Birinci Bölüm") != NULL);
        assert(strstr(toc[1].title, "İkinci Bölüm") != NULL);

        // Test Bookmark creation and lookup
        int64_t bm = px_document_make_bookmark(epub, 0);
        assert(bm != -1);
        int looked_up = px_document_lookup_bookmark(epub, bm);
        printf("Bookmark for page 0 resolved to page: %d\n", looked_up);
        assert(looked_up == 0);

        px_document_close(epub);
        printf("[PASS] EPUB tests completed successfully!\n");
    }

    // 3. Real Multi-Page PDF Document Verification
    {
        char err[256] = {0};
        PxDocument *pdf = px_document_open(pdf_path, err, sizeof(err));
        if (!pdf) {
            fprintf(stderr, "FATAL: Failed to open %s: %s\n", pdf_path, err);
            return 1;
        }

        assert(px_document_is_epub(pdf) == 0);
        int page_count = px_document_page_count(pdf);
        printf("PDF opened successfully. Page count: %d\n", page_count);
        assert(page_count == 2);

        // Render Page 0 (Fit Page)
        PxImage pdf_img0 = {0};
        int r0 = px_document_render_pdf(pdf, 0, 1.0f, 0, 0, 1280, 720, &pdf_img0, err, sizeof(err));
        assert(r0 == 0);
        assert(pdf_img0.pixels != NULL);
        uint64_t hash_pdf0 = fnv1a_hash(pdf_img0.pixels, (size_t)pdf_img0.w * pdf_img0.h * pdf_img0.components);
        printf("[PASS] PDF Page 0 rendered: %dx%d, hash: 0x%016llx\n", pdf_img0.w, pdf_img0.h, (unsigned long long)hash_pdf0);
        save_ppm("tests/fixtures/rendered_pdf_page0.ppm", &pdf_img0);
        px_image_free(&pdf_img0);

        // Render Page 1 (Must differ from Page 0)
        PxImage pdf_img1 = {0};
        int r1 = px_document_render_pdf(pdf, 1, 1.0f, 0, 0, 1280, 720, &pdf_img1, err, sizeof(err));
        assert(r1 == 0);
        uint64_t hash_pdf1 = fnv1a_hash(pdf_img1.pixels, (size_t)pdf_img1.w * pdf_img1.h * pdf_img1.components);
        printf("[PASS] PDF Page 1 rendered: %dx%d, hash: 0x%016llx\n", pdf_img1.w, pdf_img1.h, (unsigned long long)hash_pdf1);
        assert(hash_pdf0 != hash_pdf1);
        px_image_free(&pdf_img1);

        // Render Zoomed & Panned Page 0 (1.5x zoom, panX = 100, panY = 50)
        PxImage pdf_zoom = {0};
        int rz = px_document_render_pdf(pdf, 0, 1.5f, 100, 50, 1280, 720, &pdf_zoom, err, sizeof(err));
        assert(rz == 0);
        uint64_t hash_zoom = fnv1a_hash(pdf_zoom.pixels, (size_t)pdf_zoom.w * pdf_zoom.h * pdf_zoom.components);
        printf("[PASS] PDF Page 0 Zoomed 1.5x: %dx%d, hash: 0x%016llx\n", pdf_zoom.w, pdf_zoom.h, (unsigned long long)hash_zoom);
        assert(hash_pdf0 != hash_zoom);
        px_image_free(&pdf_zoom);

        // PDF TOC
        PxTocEntry pdf_toc[8];
        int pdf_toc_count = px_document_get_toc(pdf, pdf_toc, 8);
        printf("PDF TOC count: %d\n", pdf_toc_count);
        assert(pdf_toc_count == 2);
        printf("PDF TOC[0]: '%s' -> page %d\n", pdf_toc[0].title, pdf_toc[0].page);
        printf("PDF TOC[1]: '%s' -> page %d\n", pdf_toc[1].title, pdf_toc[1].page);
        assert(strstr(pdf_toc[0].title, "Sayfa 1") != NULL);
        assert(strstr(pdf_toc[1].title, "Sayfa 2") != NULL);
        assert(pdf_toc[0].page == 0);
        assert(pdf_toc[1].page == 1);

        px_document_close(pdf);
        printf("[PASS] PDF tests completed successfully!\n");
    }

    printf("==========================================================\n");
    printf("ALL NATIVE MUPDF EPUB & PDF TESTS PASSED WITH PROVEN RENDERING!\n");
    printf("==========================================================\n");
    return 0;
}
