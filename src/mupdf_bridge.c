#include "papernx/mupdf_bridge.h"
#include <mupdf/fitz.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct PxDocument {
    fz_context *ctx;
    fz_document *doc;
    int pages;
    int epub;
};

static void report(char *dst, int cap, const char *msg) {
    if (dst && cap > 0) {
        snprintf(dst, (size_t)cap, "%s", msg ? msg : "Document error");
    }
}

void px_document_close(PxDocument *d) {
    if (d) {
        if (d->ctx) {
            if (d->doc) {
                fz_drop_document(d->ctx, d->doc);
                d->doc = NULL;
            }
            fz_drop_context(d->ctx);
            d->ctx = NULL;
        }
        free(d);
    }
}

PxDocument *px_document_open(const char *path, int w, int h, float em, const char *css, char *err, int cap) {
    if (!path || !path[0]) {
        report(err, cap, "Invalid document path");
        return NULL;
    }

    PxDocument *d = (PxDocument*)calloc(1, sizeof(*d));
    if (!d) {
        report(err, cap, "Out of memory allocating document wrapper");
        return NULL;
    }

    d->ctx = fz_new_context(NULL, NULL, 128 << 20);
    if (!d->ctx) {
        report(err, cap, "Cannot create MuPDF context");
        free(d);
        return NULL;
    }

    fz_try(d->ctx) {
        fz_register_document_handlers(d->ctx);
        d->doc = fz_open_document(d->ctx, path);
        if (!d->doc) {
            fz_throw(d->ctx, FZ_ERROR_GENERIC, "Failed to open document format");
        }
        if (fz_needs_password(d->ctx, d->doc)) {
            fz_throw(d->ctx, FZ_ERROR_GENERIC, "Password-protected documents are not supported");
        }
        d->epub = fz_is_document_reflowable(d->ctx, d->doc);
        if (d->epub) {
            fz_style_document(d->ctx, d->doc, 1, css ? css : "");
            float layout_w = (w > 100) ? (float)w : 1180.0f;
            float layout_h = (h > 100) ? (float)h : 600.0f;
            float layout_em = (em > 8.0f) ? em : 22.0f;
            fz_layout_document(d->ctx, d->doc, layout_w, layout_h, layout_em);
        }
        d->pages = fz_count_pages(d->ctx, d->doc);
    }
    fz_catch(d->ctx) {
        report(err, cap, fz_caught_message(d->ctx));
        px_document_close(d);
        return NULL;
    }

    return d;
}

int px_document_pages(PxDocument *d) {
    return d ? d->pages : 0;
}

int px_document_is_epub(PxDocument *d) {
    return d ? d->epub : 0;
}

int px_document_relayout(PxDocument *d, int w, int h, float em, const char *css, char *err, int cap) {
    if (!d || !d->ctx || !d->doc || !d->epub) return 0;
    int ok = 1;
    fz_try(d->ctx) {
        fz_style_document(d->ctx, d->doc, 1, css ? css : "");
        float layout_w = (w > 100) ? (float)w : 1180.0f;
        float layout_h = (h > 100) ? (float)h : 600.0f;
        float layout_em = (em > 8.0f) ? em : 22.0f;
        fz_layout_document(d->ctx, d->doc, layout_w, layout_h, layout_em);
        d->pages = fz_count_pages(d->ctx, d->doc);
    }
    fz_catch(d->ctx) {
        report(err, cap, fz_caught_message(d->ctx));
        ok = 0;
    }
    return ok;
}

int px_document_metadata(PxDocument *d, const char *key, char *out, int cap) {
    if (!d || !d->ctx || !d->doc || !out || cap <= 0 || !key) return 0;
    out[0] = 0;
    int result = 0;
    fz_try(d->ctx) {
        result = fz_lookup_metadata(d->ctx, d->doc, key, out, cap);
    }
    fz_catch(d->ctx) {
        out[0] = 0;
        result = 0;
    }
    return result;
}

static int copy_pixmap_to_image(fz_context *ctx, fz_pixmap *pix, PxImage *out, char *err, int cap) {
    int pw = fz_pixmap_width(ctx, pix);
    int ph = fz_pixmap_height(ctx, pix);
    int n = fz_pixmap_components(ctx, pix);
    int stride = fz_pixmap_stride(ctx, pix);

    if (pw < 1 || ph < 1 || pw > 4096 || ph > 4096) {
        report(err, cap, "Rendered dimensions exceed budget");
        return 0;
    }

    out->rgb = (unsigned char*)malloc((size_t)pw * (size_t)ph * 3);
    if (!out->rgb) {
        report(err, cap, "Memory allocation failed for pixel buffer");
        return 0;
    }

    const unsigned char *samples = fz_pixmap_samples(ctx, pix);
    if (n == 3) {
        for (int y = 0; y < ph; ++y) {
            memcpy(out->rgb + (size_t)y * pw * 3, samples + (size_t)y * stride, (size_t)pw * 3);
        }
    } else if (n == 4) {
        // Drop alpha channel to standard RGB24
        for (int y = 0; y < ph; ++y) {
            const unsigned char *src = samples + (size_t)y * stride;
            unsigned char *dst = out->rgb + (size_t)y * pw * 3;
            for (int x = 0; x < pw; ++x) {
                dst[x * 3 + 0] = src[x * 4 + 0];
                dst[x * 3 + 1] = src[x * 4 + 1];
                dst[x * 3 + 2] = src[x * 4 + 2];
            }
        }
    } else {
        free(out->rgb);
        out->rgb = NULL;
        report(err, cap, "Unsupported pixmap component format");
        return 0;
    }

    out->width = pw;
    out->height = ph;
    out->stride = pw * 3;
    return 1;
}

int px_document_render(PxDocument *d, int index, int max_w, int max_h, PxImage *out, char *err, int cap) {
    if (!d || !d->ctx || !d->doc || !out || index < 0 || index >= d->pages || max_w <= 0 || max_h <= 0) {
        report(err, cap, "Invalid render arguments");
        return 0;
    }
    memset(out, 0, sizeof(*out));
    fz_page *page = NULL;
    fz_pixmap *pix = NULL;
    int ok = 1;

    fz_var(page);
    fz_var(pix);

    fz_try(d->ctx) {
        page = fz_load_page(d->ctx, d->doc, index);
        fz_rect bounds = fz_bound_page(d->ctx, page);
        const float w = bounds.x1 - bounds.x0;
        const float h = bounds.y1 - bounds.y0;
        if (!(w > 0 && h > 0 && isfinite(w) && isfinite(h))) {
            fz_throw(d->ctx, FZ_ERROR_GENERIC, "Invalid page bounds");
        }

        const float scale = fminf((float)max_w / w, (float)max_h / h);
        if (!(scale > 0 && isfinite(scale))) {
            fz_throw(d->ctx, FZ_ERROR_GENERIC, "Invalid page scale");
        }

        pix = fz_new_pixmap_from_page(d->ctx, page, fz_scale(scale, scale), fz_device_rgb(d->ctx), 0);
        if (!copy_pixmap_to_image(d->ctx, pix, out, err, cap)) {
            ok = 0;
        }
    }
    fz_catch(d->ctx) {
        report(err, cap, fz_caught_message(d->ctx));
        px_image_free(out);
        ok = 0;
    }

    if (pix) fz_drop_pixmap(d->ctx, pix);
    if (page) fz_drop_page(d->ctx, page);
    return ok;
}

int px_document_render_view(PxDocument *d, int page_idx, float zoom, int view_x, int view_y,
                            int view_w, int view_h, PxImage *out, char *err, int cap) {
    if (!d || !d->ctx || !d->doc || !out || page_idx < 0 || page_idx >= d->pages ||
        view_w <= 0 || view_h <= 0 || zoom <= 0.05f || zoom > 10.0f) {
        report(err, cap, "Invalid view render arguments");
        return 0;
    }
    memset(out, 0, sizeof(*out));
    fz_page *page = NULL;
    fz_pixmap *pix = NULL;
    int ok = 1;

    fz_var(page);
    fz_var(pix);

    fz_try(d->ctx) {
        page = fz_load_page(d->ctx, d->doc, page_idx);
        fz_rect bounds = fz_bound_page(d->ctx, page);
        const float w = bounds.x1 - bounds.x0;
        const float h = bounds.y1 - bounds.y0;
        if (!(w > 0 && h > 0 && isfinite(w) && isfinite(h))) {
            fz_throw(d->ctx, FZ_ERROR_GENERIC, "Invalid page bounds");
        }

        float base_scale = fminf((float)view_w / w, (float)view_h / h);
        float final_scale = base_scale * zoom;

        // Apply scale, then translate viewport
        fz_matrix ctm = fz_concat(fz_scale(final_scale, final_scale), fz_translate((float)-view_x, (float)-view_y));
        fz_irect irect = fz_make_irect(0, 0, view_w, view_h);

        pix = fz_new_pixmap_from_page_with_cut(d->ctx, page, ctm, fz_device_rgb(d->ctx), 0, irect);
        if (!copy_pixmap_to_image(d->ctx, pix, out, err, cap)) {
            ok = 0;
        }
    }
    fz_catch(d->ctx) {
        report(err, cap, fz_caught_message(d->ctx));
        px_image_free(out);
        ok = 0;
    }

    if (pix) fz_drop_pixmap(d->ctx, pix);
    if (page) fz_drop_page(d->ctx, page);
    return ok;
}

int px_document_render_thumbnail(PxDocument *d, int max_w, int max_h, PxImage *image, char *err, int cap) {
    if (!d || d->pages <= 0) return 0;
    return px_document_render(d, 0, max_w, max_h, image, err, cap);
}

void px_image_free(PxImage *i) {
    if (i) {
        free(i->rgb);
        i->rgb = NULL;
        i->width = 0;
        i->height = 0;
        i->stride = 0;
    }
}

int64_t px_document_make_bookmark(PxDocument *d, int page_idx) {
    if (!d || !d->ctx || !d->doc || page_idx < 0 || page_idx >= d->pages) return -1;
    int64_t mark = -1;
    fz_try(d->ctx) {
        fz_location loc = fz_location_from_page_number(d->ctx, d->doc, page_idx);
        mark = (int64_t)fz_make_bookmark(d->ctx, d->doc, loc);
    }
    fz_catch(d->ctx) {
        mark = -1;
    }
    return mark;
}

int px_document_lookup_bookmark(PxDocument *d, int64_t bookmark) {
    if (!d || !d->ctx || !d->doc || bookmark == -1) return -1;
    int page = -1;
    fz_try(d->ctx) {
        fz_location loc = fz_lookup_bookmark(d->ctx, d->doc, (fz_bookmark)bookmark);
        page = fz_page_number_from_location(d->ctx, d->doc, loc);
    }
    fz_catch(d->ctx) {
        page = -1;
    }
    return page;
}

int px_document_get_location(PxDocument *d, int page_idx, int *chapter, int *p) {
    if (!d || !d->ctx || !d->doc || page_idx < 0 || page_idx >= d->pages) return 0;
    if (chapter) *chapter = -1;
    if (p) *p = -1;
    int ok = 0;
    fz_try(d->ctx) {
        fz_location loc = fz_location_from_page_number(d->ctx, d->doc, page_idx);
        if (chapter) *chapter = loc.chapter;
        if (p) *p = loc.page;
        ok = 1;
    }
    fz_catch(d->ctx) {
        ok = 0;
    }
    return ok;
}

int px_document_from_location(PxDocument *d, int chapter, int p) {
    if (!d || !d->ctx || !d->doc) return -1;
    int page_idx = -1;
    fz_try(d->ctx) {
        fz_location loc;
        loc.chapter = chapter;
        loc.page = p;
        page_idx = fz_page_number_from_location(d->ctx, d->doc, loc);
    }
    fz_catch(d->ctx) {
        page_idx = -1;
    }
    return page_idx;
}

static void fill_toc_recursive(fz_context *ctx, fz_document *doc, fz_outline *node,
                               PxTocEntry *entries, int max_entries, int level, int *count) {
    while (node && *count < max_entries) {
        PxTocEntry *entry = &entries[*count];
        if (node->title) {
            snprintf(entry->title, sizeof(entry->title), "%s", node->title);
        } else {
            snprintf(entry->title, sizeof(entry->title), "%s", "Untitled Section");
        }
        entry->level = level;
        entry->page = node->page.page; // Or location conversion
        (*count)++;

        if (node->down) {
            fill_toc_recursive(ctx, doc, node->down, entries, max_entries, level + 1, count);
        }
        node = node->next;
    }
}

int px_document_toc(PxDocument *d, PxTocEntry *entries, int max_entries) {
    if (!d || !d->ctx || !d->doc || !entries || max_entries <= 0) return 0;
    int count = 0;
    fz_outline *outline = NULL;
    fz_var(outline);

    fz_try(d->ctx) {
        outline = fz_load_outline(d->ctx, d->doc);
        if (outline) {
            fill_toc_recursive(d->ctx, d->doc, outline, entries, max_entries, 0, &count);
        }
    }
    fz_catch(d->ctx) {
        count = 0;
    }

    if (outline) {
        fz_drop_outline(d->ctx, outline);
    }
    return count;
}
