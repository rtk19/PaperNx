#!/usr/bin/env python3
"""Small, checked, idempotent fixes against pinned MuPDF 1.23.11.

Auto direction is resolved independently for each flow by MuPDF's Unicode bidi
engine. Explicit HTML dir remains authoritative. Justified final/hard-break
lines and first-line indentation must use that same resolved direction.
"""
from pathlib import Path
import sys

root = Path(sys.argv[1])
emphasis_helpers = '''
/* Reader-controlled layout retains author emphasis, without author geometry. */
int fz_papernx_reader_emphasis(fz_context *ctx)
{
    const char *css = fz_user_css(ctx);
    return css && strstr(css, "papernx-preserve-emphasis");
}
int fz_papernx_emphasis_property(int name)
{
    return name == PRO_FONT_WEIGHT || name == PRO_FONT_STYLE ||
        name == PRO_FONT_VARIANT || name == PRO_TEXT_DECORATION ||
        name == PRO_VERTICAL_ALIGN;
}
'''
changes = {
    "source/html/html-imp.h": [
        ("const char *fz_css_property_name(int name);",
         "const char *fz_css_property_name(int name);\nint fz_papernx_reader_emphasis(fz_context *ctx);\nint fz_papernx_emphasis_property(int name);"),
        ("enum { TA_LEFT, TA_RIGHT, TA_CENTER, TA_JUSTIFY };",
         "enum { TA_LEFT, TA_RIGHT, TA_CENTER, TA_JUSTIFY, TA_START, TA_END };"),
        ("unsigned int text_align : 2;", "unsigned int text_align : 3;"),
    ],
    "source/html/css-apply.c": [
        ("if (fz_use_document_css(ctx))", "if (fz_use_document_css(ctx) || fz_papernx_reader_emphasis(ctx))"),
        ("add_property(match, prop->name, prop->value, INLINE_SPECIFICITY);",
         "if (fz_use_document_css(ctx) || fz_papernx_emphasis_property(prop->name))\n\t\t\t\t\t\tadd_property(match, prop->name, prop->value, INLINE_SPECIFICITY);"),
        ("\tstyle->text_align = TA_LEFT;", "\tstyle->text_align = TA_START;"),
        ('else if (!strcmp(value->data, "justify")) style->text_align = TA_JUSTIFY;',
         'else if (!strcmp(value->data, "justify")) style->text_align = TA_JUSTIFY;\n'
         '\t\telse if (!strcmp(value->data, "start")) style->text_align = TA_START;\n'
         '\t\telse if (!strcmp(value->data, "end")) style->text_align = TA_END;'),
    ],
    "source/html/html-parse.c": [
        ("#define DEFAULT_DIR FZ_BIDI_LTR", "#define DEFAULT_DIR FZ_BIDI_NEUTRAL"),
        ("if (fz_use_document_css(ctx))", "if (fz_use_document_css(ctx) || fz_papernx_reader_emphasis(ctx))"),
    ],
    "source/html/css-parse.c": [
        ("void fz_parse_css(fz_context *ctx, fz_css *css, const char *source, const char *file)",
         emphasis_helpers + "\nvoid fz_parse_css(fz_context *ctx, fz_css *css, const char *source, const char *file)"),
        ("css->rule = parse_stylesheet(&buf, css->rule);", '''fz_css_rule *previous = css->rule;
    while (previous && previous->next) previous = previous->next;
    css->rule = parse_stylesheet(&buf, css->rule);
    if (fz_papernx_reader_emphasis(ctx) && strcmp(file, "<user>") && strncmp(file, "<default:", 9))
    {
        fz_css_rule *rule;
        for (rule = previous ? previous->next : css->rule; rule; rule = rule->next)
        {
            fz_css_property **link = &rule->declaration;
            while (*link)
            {
                if (!fz_papernx_emphasis_property((*link)->name)) *link = (*link)->next;
                else link = &(*link)->next;
            }
        }
    }'''),
    ],
    "source/fitz/font.c": [
        ("struct { fz_font *serif, *sans; } fallback[256];", "struct { fz_font *serif[4], *sans[4]; } fallback[256];"),
        ("fz_drop_font(ctx, ctx->font->fallback[i].serif);\n\t\t\tfz_drop_font(ctx, ctx->font->fallback[i].sans);",
         "for (int style = 0; style < 4; ++style)\n\t\t\t{\n\t\t\t\tfz_drop_font(ctx, ctx->font->fallback[i].serif[style]);\n\t\t\t\tfz_drop_font(ctx, ctx->font->fallback[i].sans[style]);\n\t\t\t}"),
        ("/* TODO: bold and italic */", "/* Separate fallback faces: synthetic styles must not mutate regular runs. */\n\tint fallback_style = (!!bold) | ((!!italic) << 1);"),
        ("fontp = &ctx->font->fallback[index].serif;", "fontp = &ctx->font->fallback[index].serif[fallback_style];"),
        ("fontp = &ctx->font->fallback[index].sans;", "fontp = &ctx->font->fallback[index].sans[fallback_style];"),
        ("switch (script)\n\t{\n\tcase UCDN_SCRIPT_HANGUL:",
         "if (*fontp)\n\t{\n\t\t(*fontp)->flags.fake_bold = bold && !(*fontp)->flags.is_bold;\n\t\t(*fontp)->flags.fake_italic = italic && !(*fontp)->flags.is_italic;\n\t}\n\n\tswitch (script)\n\t{\n\tcase UCDN_SCRIPT_HANGUL:"),
    ],
    "source/html/html-layout.c": [
        ("\tif (box->markup_dir == FZ_BIDI_RTL)\n\t{\n\t\tif (align == TA_LEFT)\n\t\t\talign = TA_RIGHT;\n\t\telse if (align == TA_RIGHT)\n\t\t\talign = TA_LEFT;\n\t}",
         "\t/* Resolve logical alignment; explicit left/right stay physical. */\n"
         "\tif (align == TA_START)\n\t\talign = box->markup_dir == FZ_BIDI_RTL ? TA_RIGHT : TA_LEFT;\n"
         "\telse if (align == TA_END)\n\t\talign = box->markup_dir == FZ_BIDI_RTL ? TA_LEFT : TA_RIGHT;"),
        ("float x = box->s.layout.x + indent;",
         "float x = box->s.layout.x + (box->markup_dir == FZ_BIDI_RTL ? 0 : indent);"),
        ("(align == TA_JUSTIFY) ? TA_LEFT : align",
         "(align == TA_JUSTIFY) ? (box->markup_dir == FZ_BIDI_RTL ? TA_RIGHT : TA_LEFT) : align"),
    ],
}
for name, replacements in changes.items():
    path = root / name
    text = path.read_text()
    for old, new in replacements:
        if new in text:
            continue
        count = text.count(old)
        expected = 2 if old.startswith("(align") else 3 if name.endswith("html-parse.c") and old == "if (fz_use_document_css(ctx))" else 1
        if count != expected:
            raise SystemExit(f"Unexpected MuPDF source: {name}: {old!r} ({count})")
        text = text.replace(old, new)
    path.write_text(text)
print("MuPDF: RTL layout, reader-safe author emphasis and styled fallback fonts applied")
