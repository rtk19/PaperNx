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
    "source/html/html-parse.c": [
        ("#define DEFAULT_DIR FZ_BIDI_LTR", "#define DEFAULT_DIR FZ_BIDI_NEUTRAL"),
    ],
    "source/html/html-layout.c": [
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
print("MuPDF: automatic paragraph direction, RTL final lines and indentation applied")
