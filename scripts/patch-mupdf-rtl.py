#!/usr/bin/env python3
"""Small, checked, idempotent fixes against pinned MuPDF 1.23.11.

Auto direction is resolved independently for each flow by MuPDF's Unicode bidi
engine. Explicit HTML dir remains authoritative. Justified final/hard-break
lines and first-line indentation must use that same resolved direction.
"""
from pathlib import Path
import sys

root = Path(sys.argv[1])
changes = {
    "source/html/html-imp.h": [
        ("enum { TA_LEFT, TA_RIGHT, TA_CENTER, TA_JUSTIFY };",
         "enum { TA_LEFT, TA_RIGHT, TA_CENTER, TA_JUSTIFY, TA_START, TA_END };"),
        ("unsigned int text_align : 2;", "unsigned int text_align : 3;"),
    ],
    "source/html/css-apply.c": [
        ("\tstyle->text_align = TA_LEFT;", "\tstyle->text_align = TA_START;"),
        ('else if (!strcmp(value->data, "justify")) style->text_align = TA_JUSTIFY;',
         'else if (!strcmp(value->data, "justify")) style->text_align = TA_JUSTIFY;\n'
         '\t\telse if (!strcmp(value->data, "start")) style->text_align = TA_START;\n'
         '\t\telse if (!strcmp(value->data, "end")) style->text_align = TA_END;'),
    ],
    "source/html/html-parse.c": [
        ("#define DEFAULT_DIR FZ_BIDI_LTR", "#define DEFAULT_DIR FZ_BIDI_NEUTRAL"),
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
        expected = 2 if old.startswith("(align") else 1
        if count != expected:
            raise SystemExit(f"Unexpected MuPDF source: {name}: {old!r} ({count})")
        text = text.replace(old, new)
    path.write_text(text)
print("MuPDF: auto direction, logical start/end, physical publisher alignment, RTL final lines/indent applied")
