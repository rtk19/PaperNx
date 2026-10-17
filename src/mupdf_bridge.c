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
    size_t allocated;
    size_t memory_limit;
    unsigned char background[3];
    char *path;
    char *css;
    fz_buffer *cover;
    int nav_count;
    PxTocEntry nav[256];
    char nav_uri[256][2048];
};

static void load_epub_extras(PxDocument *d);

static void set_background(PxDocument *d, const char *css) {
    unsigned int r = 255, g = 255, b = 255;
    const char *color = css ? strstr(css, "background-color: #") : NULL;
    if (color && sscanf(color + strlen("background-color: #"), "%2x%2x%2x", &r, &g, &b) != 3)
        r = g = b = 255;
    d->background[0] = (unsigned char)r;
    d->background[1] = (unsigned char)g;
    d->background[2] = (unsigned char)b;
}

typedef union AllocationHeader {
    max_align_t alignment;
    size_t bytes;
} AllocationHeader;

static void *budget_malloc(void *user, size_t bytes) {
    PxDocument *d = (PxDocument *)user;
    if (bytes > SIZE_MAX - sizeof(AllocationHeader)) return NULL;
    size_t total = bytes + sizeof(AllocationHeader);
    if (total > d->memory_limit - d->allocated) return NULL;
    AllocationHeader *header = (AllocationHeader *)malloc(total);
    if (!header) return NULL;
    header->bytes = total;
    d->allocated += total;
    return header + 1;
}

static void budget_free(void *user, void *ptr) {
    if (!ptr) return;
    PxDocument *d = (PxDocument *)user;
    AllocationHeader *header = (AllocationHeader *)ptr - 1;
    d->allocated -= header->bytes;
    free(header);
}

static void *budget_realloc(void *user, void *ptr, size_t bytes) {
    if (!ptr) return budget_malloc(user, bytes);
    if (!bytes) { budget_free(user, ptr); return NULL; }
    PxDocument *d = (PxDocument *)user;
    AllocationHeader *header = (AllocationHeader *)ptr - 1;
    size_t old = header->bytes;
    if (bytes > SIZE_MAX - sizeof(AllocationHeader)) return NULL;
    size_t total = bytes + sizeof(AllocationHeader);
    if (total > d->memory_limit - (d->allocated - old)) return NULL;
    header = (AllocationHeader *)realloc(header, total);
    if (!header) return NULL;
    header->bytes = total;
    d->allocated = d->allocated - old + total;
    return header + 1;
}

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
            fz_drop_buffer(d->ctx, d->cover);
            fz_free(d->ctx, d->path);
            fz_free(d->ctx, d->css);
            fz_drop_context(d->ctx);
            d->ctx = NULL;
        }
        free(d);
    }
}

PxDocument *px_document_open(const char *path, int w, int h, float em, const char *css, char *err, int cap) {
    return px_document_open_limited(path, w, h, em, css, err, cap, 96u * 1024u * 1024u);
}

PxDocument *px_document_open_limited(const char *path, int w, int h, float em, const char *css, char *err, int cap, size_t memory_limit) {
    if (!path || !path[0]) {
        report(err, cap, "Invalid document path");
        return NULL;
    }

    PxDocument *d = (PxDocument*)calloc(1, sizeof(*d));
    if (!d) {
        report(err, cap, "Out of memory allocating document wrapper");
        return NULL;
    }

    d->memory_limit = memory_limit;
    set_background(d, css);
    fz_alloc_context allocator = {d, budget_malloc, budget_realloc, budget_free};
    d->ctx = fz_new_context(&allocator, NULL, 32 << 20);
    if (!d->ctx) {
        report(err, cap, "Cannot create MuPDF context");
        free(d);
        return NULL;
    }

    fz_try(d->ctx) {
        fz_register_document_handlers(d->ctx);
        d->path = fz_strdup(d->ctx, path);
        d->css = fz_strdup(d->ctx, css ? css : "");
        fz_set_user_css(d->ctx, css ? css : "");
        d->doc = fz_open_document(d->ctx, path);
        if (!d->doc) {
            fz_throw(d->ctx, FZ_ERROR_GENERIC, "Failed to open document format");
        }
        if (fz_needs_password(d->ctx, d->doc)) {
            fz_throw(d->ctx, FZ_ERROR_GENERIC, "Password-protected documents are not supported");
        }
        d->epub = fz_is_document_reflowable(d->ctx, d->doc);
        if (d->epub) {
            fz_set_user_css(d->ctx, css ? css : "");
        fz_set_use_document_css(d->ctx, 1);
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

    if (d->epub) load_epub_extras(d);
    return d;
}

int px_document_pages(PxDocument *d) {
    return d ? d->pages : 0;
}

int px_document_page_size(PxDocument *d, int index, float *width, float *height) {
    if (!d || !width || !height || index < 0 || index >= d->pages) return 0;
    fz_page *page = NULL;
    fz_var(page);
    int ok = 0;
    fz_try(d->ctx) {
        page = fz_load_page(d->ctx, d->doc, index);
        fz_rect bounds = fz_bound_page(d->ctx, page);
        *width = bounds.x1 - bounds.x0; *height = bounds.y1 - bounds.y0;
        ok = isfinite(*width) && isfinite(*height) && *width > 0 && *height > 0;
    }
    fz_always(d->ctx) { fz_drop_page(d->ctx, page); }
    fz_catch(d->ctx) { ok = 0; }
    return ok;
}

int px_document_is_epub(PxDocument *d) {
    return d ? d->epub : 0;
}

int px_document_relayout(PxDocument *d, int w, int h, float em, const char *css, char *err, int cap) {
    if (!d || !d->ctx || !d->doc || !d->epub || w <= 0 || h <= 0 || w > 4096 || h > 4096 || !isfinite(em) || em < 8 || em > 96) { report(err, cap, "Invalid relayout arguments"); return 0; }
    fz_document *replacement = NULL;
    char *new_css = NULL;
    int pages = 0, ok = 0;
    fz_var(replacement); fz_var(new_css); fz_var(pages); fz_var(ok);
    fz_try(d->ctx) {
        new_css = fz_strdup(d->ctx, css ? css : "");
        fz_set_user_css(d->ctx, new_css);
        fz_set_use_document_css(d->ctx, 1);
        replacement = fz_open_document(d->ctx, d->path);
        fz_layout_document(d->ctx, replacement, w > 100 ? w : 1180, h > 100 ? h : 600, em > 8 ? em : 22);
        pages = fz_count_pages(d->ctx, replacement);
        if (pages <= 0) fz_throw(d->ctx, FZ_ERROR_GENERIC, "Empty relayout");
        ok = 1;
    }
    fz_catch(d->ctx) { report(err, cap, fz_caught_message(d->ctx)); }
    if (ok) {
        fz_drop_document(d->ctx, d->doc); d->doc = replacement;
        fz_free(d->ctx, d->css); d->css = new_css;
        d->pages = pages; set_background(d, css);
    } else {
        fz_drop_document(d->ctx, replacement); fz_free(d->ctx, new_css);
        fz_set_user_css(d->ctx, d->css);
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
    if (!d || !d->ctx || !d->doc || !out || index < 0 || index >= d->pages || max_w <= 0 || max_h <= 0 || max_w > 4096 || max_h > 4096) {
        report(err, cap, "Invalid render arguments");
        return 0;
    }
    memset(out, 0, sizeof(*out));
    fz_page *page = NULL;
    fz_pixmap *pix = NULL;
    fz_device *device = NULL;
    int ok = 1;

    fz_var(page);
    fz_var(pix);
    fz_var(device);

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

        if (d->epub) {
            // MuPDF's generic page helper clears to white, including CSS margins.
            // Clear the complete EPUB surface to the selected theme before drawing.
            fz_irect box = fz_make_irect(0, 0, (int)ceilf(w * scale), (int)ceilf(h * scale));
            pix = fz_new_pixmap_with_bbox(d->ctx, fz_device_rgb(d->ctx), box, NULL, 0);
            unsigned char *samples = fz_pixmap_samples(d->ctx, pix);
            int stride = fz_pixmap_stride(d->ctx, pix);
            for (int y = 0; y < box.y1; ++y)
                for (int x = 0; x < box.x1; ++x)
                    memcpy(samples + (size_t)y * stride + x * 3, d->background, 3);
            device = fz_new_draw_device(d->ctx, fz_identity, pix);
            fz_matrix ctm = fz_make_matrix(scale, 0, 0, scale, -bounds.x0 * scale, -bounds.y0 * scale);
            fz_run_page(d->ctx, page, device, ctm, NULL);
            fz_close_device(d->ctx, device);
        } else {
            pix = fz_new_pixmap_from_page(d->ctx, page, fz_scale(scale, scale), fz_device_rgb(d->ctx), 0);
        }
        if (!copy_pixmap_to_image(d->ctx, pix, out, err, cap)) {
            ok = 0;
        }
    }
    fz_catch(d->ctx) {
        report(err, cap, fz_caught_message(d->ctx));
        px_image_free(out);
        ok = 0;
    }

    if (device) fz_drop_device(d->ctx, device);
    if (pix) fz_drop_pixmap(d->ctx, pix);
    if (page) fz_drop_page(d->ctx, page);
    return ok;
}

int px_document_render_view(PxDocument *d, int page_idx, float zoom, int view_x, int view_y,
                            int view_w, int view_h, PxImage *out, char *err, int cap) {
    if (!d || !d->ctx || !d->doc || !out || page_idx < 0 || page_idx >= d->pages ||
        view_w <= 0 || view_h <= 0 || view_w > 4096 || view_h > 4096 || !isfinite(zoom) || zoom < 0.1f || zoom > 5.0f) {
        report(err, cap, "Invalid view render arguments");
        return 0;
    }
    memset(out, 0, sizeof(*out));
    fz_page *page = NULL;
    fz_pixmap *pix = NULL;
    fz_device *device = NULL;
    int ok = 1;

    fz_var(page);
    fz_var(pix);
    fz_var(device);

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
        view_x = (int)fmaxf(0, fminf((float)view_x, ceilf(w * final_scale) - view_w));
        view_y = (int)fmaxf(0, fminf((float)view_y, ceilf(h * final_scale) - view_h));
        fz_matrix ctm = fz_make_matrix(final_scale, 0, 0, final_scale,
            -bounds.x0 * final_scale - view_x + fmaxf(0, (view_w - w * final_scale) / 2),
            -bounds.y0 * final_scale - view_y + fmaxf(0, (view_h - h * final_scale) / 2));
        fz_irect irect = fz_make_irect(0, 0, view_w, view_h);

        pix = fz_new_pixmap_with_bbox(d->ctx, fz_device_rgb(d->ctx), irect, NULL, 0);
        fz_clear_pixmap_with_value(d->ctx, pix, 255);
        device = fz_new_draw_device(d->ctx, fz_identity, pix);
        fz_run_page(d->ctx, page, device, ctm, NULL);
        fz_close_device(d->ctx, device);
        if (!copy_pixmap_to_image(d->ctx, pix, out, err, cap)) {
            ok = 0;
        }
    }
    fz_catch(d->ctx) {
        report(err, cap, fz_caught_message(d->ctx));
        px_image_free(out);
        ok = 0;
    }

    if (device) fz_drop_device(d->ctx, device);
    if (pix) fz_drop_pixmap(d->ctx, pix);
    if (page) fz_drop_page(d->ctx, page);
    return ok;
}

int px_document_render_thumbnail(PxDocument *d, int max_w, int max_h, PxImage *out, char *err, int cap) {
    if (!d || !out || max_w <= 0 || max_h <= 0 || max_w > 1024 || max_h > 1024) return 0;
    memset(out, 0, sizeof(*out));
    if (d->cover) {
        fz_image *image = NULL;
        fz_pixmap *pix = NULL;
        fz_pixmap *rgb = NULL;
        fz_var(image); fz_var(pix); fz_var(rgb);
        int ok = 0;
        fz_try(d->ctx) {
            image = fz_new_image_from_buffer(d->ctx, d->cover);
            if (image->w <= 0 || image->h <= 0 || (int64_t)image->w * image->h > 16000000)
                fz_throw(d->ctx, FZ_ERROR_GENERIC, "Cover dimensions exceed budget");
            float scale = fminf((float)max_w / image->w, (float)max_h / image->h);
            int w = (int)fmaxf(1, floorf(image->w * scale));
            int h = (int)fmaxf(1, floorf(image->h * scale));
            pix = fz_new_pixmap(d->ctx, fz_device_rgb(d->ctx), w, h, NULL, 0);
            fz_clear_pixmap_with_value(d->ctx, pix, 255);
            fz_device *device = fz_new_draw_device(d->ctx, fz_identity, pix);
            fz_try(d->ctx) {
                fz_fill_image(d->ctx, device, image, fz_scale(w, h), 1, fz_default_color_params);
                fz_close_device(d->ctx, device);
            }
            fz_always(d->ctx) { fz_drop_device(d->ctx, device); }
            fz_catch(d->ctx) { fz_rethrow(d->ctx); }
            ok = copy_pixmap_to_image(d->ctx, pix, out, err, cap);
        }
        fz_always(d->ctx) {
            fz_drop_pixmap(d->ctx, rgb); fz_drop_pixmap(d->ctx, pix); fz_drop_image(d->ctx, image);
        }
        fz_catch(d->ctx) { px_image_free(out); ok = 0; }
        if (ok) return 1;
    }
    return px_document_render(d, 0, max_w, max_h, out, err, cap);
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
    if (!d || page_idx < 0 || page_idx >= d->pages) return -1;
    int64_t mark = -1;
    fz_try(d->ctx) {
        fz_location loc = fz_location_from_page_number(d->ctx, d->doc, page_idx);
        int count = fz_count_chapter_pages(d->ctx, d->doc, loc.chapter);
        uint32_t fraction = count > 1 ? (uint32_t)llround((double)loc.page * 1000000 / (count - 1)) : 0;
        mark = INT64_C(0x4000000000000000) | ((int64_t)loc.chapter << 32) | fraction;
    }
    fz_catch(d->ctx) { mark = -1; }
    return mark;
}

int px_document_lookup_bookmark(PxDocument *d, int64_t bookmark) {
    // Older persisted fz_bookmark values are process-local and must be ignored.
    if (!d || bookmark < INT64_C(0x4000000000000000)) return -1;
    int page = -1;
    fz_try(d->ctx) {
        int chapter = (int)((bookmark >> 32) & 0x3fffffff);
        uint32_t fraction = (uint32_t)bookmark;
        if (chapter < fz_count_chapters(d->ctx, d->doc) && fraction <= 1000000) {
            int count = fz_count_chapter_pages(d->ctx, d->doc, chapter);
            fz_location loc = fz_make_location(chapter, (int)llround((double)fraction * (count - 1) / 1000000));
            page = fz_page_number_from_location(d->ctx, d->doc, loc);
        }
    }
    fz_catch(d->ctx) { page = -1; }
    return page >= 0 && page < d->pages ? page : -1;
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

static int resolve_outline_page(fz_context *ctx, fz_document *doc, fz_outline *node) {
    int result = -1;
    fz_try(ctx) {
        fz_location loc = node->page;
        if (node->uri && node->uri[0]) {
            float x = 0, y = 0;
            loc = fz_resolve_link(ctx, doc, node->uri, &x, &y);
        }
        result = fz_page_number_from_location(ctx, doc, loc);
    }
    fz_catch(ctx) { result = -1; }
    return result;
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
        entry->page = resolve_outline_page(ctx, doc, node);
        (*count)++;

        if (node->down && level < 32) {
            fill_toc_recursive(ctx, doc, node->down, entries, max_entries, level + 1, count);
        }
        node = node->next;
    }
}

int px_document_toc(PxDocument *d, PxTocEntry *entries, int max_entries) {
    if (!d || !d->ctx || !d->doc || !entries || max_entries <= 0) return 0;
    if (d->epub && d->nav_count) {
        int n = d->nav_count < max_entries ? d->nav_count : max_entries;
        for (int i = 0; i < n; ++i) {
            entries[i] = d->nav[i];
            fz_try(d->ctx) {
                float x = 0, y = 0;
                fz_location loc = fz_resolve_link(d->ctx, d->doc, d->nav_uri[i], &x, &y);
                entries[i].page = fz_page_number_from_location(d->ctx, d->doc, loc);
            }
            fz_catch(d->ctx) { entries[i].page = -1; }
        }
        return n;
    }
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
/* EPUB3 navigation and cover metadata are not exposed by MuPDF 1.23's
 * generic document interface. Read the container through its public ZIP/XML API. */
static int xml_tag(fz_xml *node, const char *name) {
    const char *tag = fz_xml_tag(node);
    if (!tag) return 0;
    const char *colon = strrchr(tag, ':');
    return strcmp(colon ? colon + 1 : tag, name) == 0;
}

static fz_xml *xml_child(fz_xml *node, const char *name) {
    for (fz_xml *p = fz_xml_down(node); p; p = fz_xml_next(p))
        if (xml_tag(p, name)) return p;
    return NULL;
}

static int has_word(const char *words, const char *word) {
    if (!words) return 0;
    size_t n = strlen(word);
    for (const char *p = words; *p;) {
        while (*p == ' ' || *p == '\t' || *p == '\n') ++p;
        const char *end = p;
        while (*end && *end != ' ' && *end != '\t' && *end != '\n') ++end;
        if ((size_t)(end - p) == n && !strncmp(p, word, n)) return 1;
        p = end;
    }
    return 0;
}

static int archive_path(char *out, size_t cap, const char *base, const char *href) {
    if (!href || !*href || strlen(base) + strlen(href) + 2 >= cap ||
        strchr(href, ':') || href[0] == '/' || strchr(href, '\\')) return 0;
    memcpy(out, base, strlen(base));
    memcpy(out + strlen(base), href, strlen(href) + 1);
    fz_urldecode(out);
    fz_cleanname(out);
    return strncmp(out, "../", 3) && out[0] != '/';
}

static void archive_base(char *out, size_t cap, const char *path) {
    snprintf(out, cap, "%s", path);
    char *slash = strrchr(out, '/');
    if (slash) slash[1] = 0; else out[0] = 0;
}

static fz_buffer *read_bounded(fz_context *ctx, fz_archive *archive, const char *path, size_t limit) {
    fz_stream *stream = NULL;
    fz_buffer *buffer = NULL;
    fz_var(stream); fz_var(buffer);
    fz_try(ctx) {
        stream = fz_open_archive_entry(ctx, archive, path);
        buffer = fz_new_buffer(ctx, 4096);
        unsigned char chunk[4096];
        size_t count, total = 0;
        while ((count = fz_read(ctx, stream, chunk, sizeof chunk)) != 0) {
            if (count > limit - total) fz_throw(ctx, FZ_ERROR_GENERIC, "EPUB asset exceeds budget");
            fz_append_data(ctx, buffer, chunk, count);
            total += count;
        }
    }
    fz_always(ctx) { fz_drop_stream(ctx, stream); }
    fz_catch(ctx) { fz_drop_buffer(ctx, buffer); fz_rethrow(ctx); }
    return buffer;
}

static fz_xml *read_xml_bounded(fz_context *ctx, fz_archive *archive, const char *path) {
    fz_buffer *buffer = read_bounded(ctx, archive, path, 2 * 1024 * 1024);
    fz_xml *xml = NULL;
    fz_try(ctx) { xml = fz_parse_xml(ctx, buffer, 0); }
    fz_always(ctx) { fz_drop_buffer(ctx, buffer); }
    fz_catch(ctx) { fz_rethrow(ctx); }
    return xml;
}

static void xml_text(fz_xml *node, char *out, size_t cap, int depth) {
    if (depth > 32) return;
    for (fz_xml *p = fz_xml_down(node); p; p = fz_xml_next(p)) {
        const char *text = fz_xml_text(p);
        if (text) fz_strlcat(out, text, cap);
        else xml_text(p, out, cap, depth + 1);
    }
}

static void read_nav(PxDocument *d, fz_xml *node, const char *base, int level) {
    if (!node || level > 32 || d->nav_count >= 256) return;
    for (fz_xml *p = fz_xml_down(node); p && d->nav_count < 256; p = fz_xml_next(p)) {
        if (xml_tag(p, "a")) {
            int i = d->nav_count;
            if (archive_path(d->nav_uri[i], sizeof d->nav_uri[i], base, fz_xml_att(p, "href"))) {
                xml_text(p, d->nav[i].title, sizeof d->nav[i].title, 0);
                d->nav[i].level = level;
                d->nav[i].page = -1;
                ++d->nav_count;
            }
        } else read_nav(d, p, base, level + (xml_tag(p, "ol") ? 1 : 0));
    }
}

static fz_xml *find_toc_nav(fz_xml *node, int depth) {
    if (!node || depth > 32) return NULL;
    if (xml_tag(node, "nav") && has_word(fz_xml_att(node, "epub:type"), "toc")) return node;
    for (fz_xml *p = fz_xml_down(node); p; p = fz_xml_next(p)) {
        fz_xml *found = find_toc_nav(p, depth + 1);
        if (found) return found;
    }
    return NULL;
}

static void load_epub_extras(PxDocument *d) {
    fz_archive *archive = NULL;
    fz_xml *container = NULL, *opf = NULL, *nav = NULL;
    fz_var(archive); fz_var(container); fz_var(opf); fz_var(nav);
    fz_try(d->ctx) {
        archive = fz_open_archive(d->ctx, d->path);
        container = read_xml_bounded(d->ctx, archive, "META-INF/container.xml");
        fz_xml *files = xml_child(fz_xml_root(container), "rootfiles");
        fz_xml *file = xml_child(files, "rootfile");
        const char *opf_name = fz_xml_att(file, "full-path");
        char path[2048], base[2048], nav_path[2048] = "", cover_path[2048] = "";
        if (!archive_path(path, sizeof path, "", opf_name)) fz_throw(d->ctx, FZ_ERROR_GENERIC, "Invalid OPF path");
        opf = read_xml_bounded(d->ctx, archive, path);
        archive_base(base, sizeof base, path);
        fz_xml *package = fz_xml_root(opf);
        fz_xml *manifest = xml_child(package, "manifest");
        fz_xml *metadata = xml_child(package, "metadata");
        const char *cover_id = NULL;
        for (fz_xml *p = fz_xml_down(metadata); p; p = fz_xml_next(p))
            if (xml_tag(p, "meta") && fz_xml_att_eq(p, "name", "cover")) cover_id = fz_xml_att(p, "content");
        for (fz_xml *p = fz_xml_down(manifest); p; p = fz_xml_next(p)) {
            if (!xml_tag(p, "item")) continue;
            const char *properties = fz_xml_att(p, "properties");
            const char *id = fz_xml_att(p, "id");
            if (has_word(properties, "nav")) archive_path(nav_path, sizeof nav_path, base, fz_xml_att(p, "href"));
            if (has_word(properties, "cover-image") || (cover_id && id && !strcmp(cover_id, id)))
                archive_path(cover_path, sizeof cover_path, base, fz_xml_att(p, "href"));
        }
        // Cover failure must not prevent navigation or document reading.
        if (cover_path[0]) {
            fz_try(d->ctx) { d->cover = read_bounded(d->ctx, archive, cover_path, 8 * 1024 * 1024); }
            fz_catch(d->ctx) { d->cover = NULL; }
        }
        if (nav_path[0]) {
            nav = read_xml_bounded(d->ctx, archive, nav_path);
            archive_base(base, sizeof base, nav_path);
            read_nav(d, find_toc_nav(fz_xml_root(nav), 0), base, -1);
        }
    }
    fz_always(d->ctx) {
        fz_drop_xml(d->ctx, nav); fz_drop_xml(d->ctx, opf);
        fz_drop_xml(d->ctx, container); fz_drop_archive(d->ctx, archive);
    }
    fz_catch(d->ctx) { /* Optional extras; native outline/page-zero remain available. */ }
}

/* Explicitly whole-page, reversible filters; no claim of image-safe recoloring. */
void px_image_filter(PxImage *image, int mode) {
    if (!image || !image->rgb || mode < 1 || mode > 3) return;
    for (int y = 0; y < image->height; ++y) for (int x = 0; x < image->width; ++x) {
        unsigned char *p = image->rgb + (size_t)y * image->stride + x * 3;
        int lum = (p[0] * 54 + p[1] * 183 + p[2] * 19) / 256;
        if (mode == 3) { p[0] = 255-p[0]; p[1] = 255-p[1]; p[2] = 255-p[2]; }
        else if (mode == 1) { p[0] = 230 - lum * 202 / 255; p[1] = 228 - lum * 198 / 255; p[2] = 222 - lum * 190 / 255; }
        else { p[0] = 65 + lum * 171 / 255; p[1] = 50 + lum * 174 / 255; p[2] = 36 + lum * 162 / 255; }
    }
}
