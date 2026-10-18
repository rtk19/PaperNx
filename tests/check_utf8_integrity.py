#!/usr/bin/env python3
"""Check that documentation and source files have valid UTF-8 without mojibake."""
import os
import sys
from pathlib import Path

MOJIBAKE_PATTERNS = [
    bytes([0xc3, 0x83]),       # Double-encoded UTF-8 A-tilde
    bytes([0xc3, 0xa2, 0xe2]), # Double-encoded punctuation / quotes / bullets
    bytes([0xc3, 0x84, 0xc2]), # Double-encoded Turkish dotless-i
]

def check_file(path: Path) -> list:
    errors = []
    data = path.read_bytes()
    # 1. Must decode cleanly as valid UTF-8
    try:
        data.decode('utf-8')
    except UnicodeDecodeError as e:
        errors.append(f"Invalid UTF-8 sequence at offset {e.start}: {e.reason}")
        return errors
    # 2. Must not contain known double-encoded mojibake byte patterns
    for pat in MOJIBAKE_PATTERNS:
        if pat in data:
            errors.append(f"Mojibake pattern {pat!r} detected")
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

    if failed:
        sys.exit(1)
    print(f"UTF-8 integrity verified across {checked} files.")

if __name__ == '__main__':
    main()
