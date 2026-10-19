#!/usr/bin/env python3
"""Check that documentation and source files have valid UTF-8 without mojibake,
ensure NFC normalization, and verify Turkish Unicode character integrity."""
import os
import sys
import unicodedata
from pathlib import Path

MOJIBAKE_PATTERNS = [
    bytes([0xc3, 0x83]),       # Double-encoded UTF-8 A-tilde
    bytes([0xc3, 0xa2, 0xe2]), # Double-encoded punctuation / quotes / bullets
    bytes([0xc3, 0x84, 0xc2]), # Double-encoded Turkish dotless-i
]

MOJIBAKE_STRINGS = [
    "Ã§", "Ã‡", "ÄŸ", "Äž", "Ä±", "Ä°", "Ã¶", "Ã–", "ÅŸ", "Åž", "Ã¼", "Ãœ",
    "â€™", "â€œ", "â€", "â€“", "â€”", "\ufffd"
]

TURKISH_CHARS = set("çÇğĞıİöÖşŞüÜ")

def check_file(path: Path) -> list:
    errors = []
    data = path.read_bytes()
    # 1. Must decode cleanly as valid UTF-8
    try:
        text = data.decode('utf-8')
    except UnicodeDecodeError as e:
        errors.append(f"Invalid UTF-8 sequence at offset {e.start}: {e.reason}")
        return errors
    # Do not check pattern definitions against self
    if path.name == "check_utf8_integrity.py":
        return errors
    # 2. Must not contain known double-encoded mojibake byte patterns
    for pat in MOJIBAKE_PATTERNS:
        if pat in data:
            errors.append(f"Mojibake pattern {pat!r} detected")
    # 3. Must not contain mojibake strings or U+FFFD
    for s in MOJIBAKE_STRINGS:
        if s in text:
            errors.append(f"Mojibake string {s!r} detected")
    # 4. Must be normalized in NFC (no decomposed accents)
    if not unicodedata.is_normalized('NFC', text):
        errors.append("File is not NFC normalized")
    return errors

def main():
    root = Path(__file__).resolve().parent.parent
    failed = False
    checked = 0
    extensions = {'.md', '.txt', '.cpp', '.hpp', '.inc', '.c', '.h', '.py', '.sh'}

    for dirpath, dirnames, filenames in os.walk(root):
        dirnames[:] = [d for d in dirnames if not (d == '.git' or d.startswith('build') or d.startswith('prefix') or d.startswith('dist'))]
        for f in filenames:
            path = Path(dirpath) / f
            if path.suffix not in extensions:
                continue
            errs = check_file(path)
            checked += 1
            if errs:
                print(f"FAIL: {path.relative_to(root)}: {', '.join(errs)}", file=sys.stderr)
                failed = True

    # Check that key files have proper Turkish characters
    loc_file = root / "include" / "papernx" / "localization.hpp"
    if loc_file.exists():
        loc_text = loc_file.read_text(encoding='utf-8')
        missing = [c for c in TURKISH_CHARS if c not in loc_text]
        if missing:
            missing_hex = [f"U+{ord(c):04X}" for c in missing]
            print(f"FAIL: {loc_file.relative_to(root)} missing Turkish characters: {missing_hex}", file=sys.stderr)
            failed = True
        else:
            print(f"Verified all 12 Turkish characters (ç, Ç, ğ, Ğ, ı, İ, ö, Ö, ş, Ş, ü, Ü) in {loc_file.relative_to(root)}")

    if failed:
        sys.exit(1)
    print(f"UTF-8 integrity verified across {checked} files.")

if __name__ == '__main__':
    main()
