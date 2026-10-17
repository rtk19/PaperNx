#!/usr/bin/env python3
"""Require the supplied baseline, three-component 256px JPEG for hbmenu/elf2nro."""
import hashlib
import struct
from pathlib import Path

def validate_icon(path):
    data = Path(path).read_bytes()
    if not data.startswith(b"\xff\xd8") or not data.endswith(b"\xff\xd9"):
        raise ValueError("PaperNX custom icon must be a complete JPEG")
    pos = 2
    while pos < len(data):
        if data[pos] != 255:
            raise ValueError("Invalid JPEG marker")
        while data[pos] == 255:
            pos += 1
        marker = data[pos]; pos += 1
        if marker in (0xd9, 0xda):
            break
        length = struct.unpack_from(">H", data, pos)[0]
        if length < 2 or pos + length > len(data):
            raise ValueError("Truncated JPEG")
        if marker in (0xc0, 0xc1, 0xc2):
            bits, height, width, components = struct.unpack_from(">BHHB", data, pos + 2)
            if (marker, bits, width, height, components) != (0xc0, 8, 256, 256, 3):
                raise ValueError("Custom icon requires baseline 8-bit 256x256 three-component RGB-compatible JPEG")
            return {"width": width, "height": height, "components": components, "encoding": "baseline JPEG", "sha256": hashlib.sha256(data).hexdigest()}
        pos += length
    raise ValueError("JPEG frame missing")

if __name__ == "__main__":
    print(validate_icon("assets/branding/PaperNX-icon-switch.jpg"))
