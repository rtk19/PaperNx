#!/usr/bin/env python3
"""Package ONLY an authentic NRO0 image; does not build one."""
import argparse
import hashlib
import json
import struct
from pathlib import Path
from zipfile import ZIP_DEFLATED, ZipFile

def package(nro: Path, destination: Path, version: str) -> tuple[Path, str]:
    image = nro.read_bytes()
    if len(image) < 0x80 or image[0x10:0x14] != b"NRO0":
        raise ValueError("Not a valid NRO0 magic at offset 0x10")
    declared_size = struct.unpack_from("<I", image, 0x18)[0]
    if declared_size < 0x80 or declared_size > len(image):
        raise ValueError("Invalid NRO header size")
    destination.mkdir(parents=True, exist_ok=True)
    name = f"PaperNX-switch-{version}.zip"
    archive = destination / name
    with ZipFile(archive, "w", ZIP_DEFLATED, compresslevel=6) as z:
        z.writestr("switch/PaperNX/books/", "")
        z.writestr("switch/PaperNX/fonts/", "")
        z.writestr("switch/PaperNX/data/", "")
        z.writestr("switch/PaperNX/PaperNX.nro", image)
    digest = hashlib.sha256(archive.read_bytes()).hexdigest()
    (destination / (name + ".sha256")).write_text(f"{digest}  {name}\n", encoding="utf-8")
    (destination / "manifest.json").write_text(
        json.dumps({"name": name, "sha256": digest, "nro_sha256": hashlib.sha256(image).hexdigest(),
                    "nro_image_size": declared_size, "verified": "format-header-only",
                    "hardware_verified": False}, indent=2) + "\n", encoding="utf-8")
    return archive, digest

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("nro", type=Path)
    parser.add_argument("--destination", type=Path, default=Path("dist"))
    parser.add_argument("--version", default="v0.1.0")
    args = parser.parse_args()
    archive, digest = package(args.nro, args.destination, args.version)
    print(f"{archive}: SHA256 {digest}")
