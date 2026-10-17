#!/usr/bin/env python3
"""Verify the actual local release ZIP against its NRO, ELF, manifest and fonts."""
import hashlib
import json
from pathlib import Path
from zipfile import ZipFile

from artifact_validation import validate_assets, read_romfs, validate_elf
from package_release import validate_nro

if not __debug__:
    raise RuntimeError("Artifact checks require enabled assertions")

manifest = json.loads(Path("dist/manifest.json").read_text(encoding="utf-8"))
archive = Path("dist") / manifest["name"]
assert hashlib.sha256(archive.read_bytes()).hexdigest() == manifest["sha256"]
nro = Path("dist/PaperNX.nro").read_bytes()
assert nro == Path("PaperNX.nro").read_bytes()
assert hashlib.sha256(nro).hexdigest() == manifest["nro"]["sha256"]
assets = validate_assets(nro, validate_nro(nro)["declared_size"])
version = nro[assets["nacp"]["offset"] + 0x3060:assets["nacp"]["offset"] + 0x3070].split(b"\0")[0].decode()
assert version == "0.1.0-rc1", version
files = read_romfs(nro, assets["romfs"])
for name in ("NotoSans-Regular.ttf", "NotoSerif-Regular.ttf", "OFL.txt"):
    assert files["fonts/" + name] == (Path("fonts") / name).read_bytes()
with ZipFile(archive) as zipped:
    assert zipped.testzip() is None
    assert zipped.read("switch/PaperNX/PaperNX.nro") == nro
    for name in ("NotoSans-Regular.ttf", "NotoSerif-Regular.ttf", "OFL.txt"):
        assert zipped.read("switch/PaperNX/fonts/" + name) == (Path("fonts") / name).read_bytes()
    notices = [name for name in zipped.namelist() if "/licenses/" in name]
    assert len(notices) >= 55
    assert "switch/PaperNX/licenses/SDK-notices/libnx-4.12.0-LICENSE.md" in notices
    members = len(zipped.namelist())
result = {
    "result": "PASS", "zip_sha256": manifest["sha256"],
    "nro_sha256": manifest["nro"]["sha256"], "nacp_version": version,
    "license_files": len(notices), "zip_members": members,
    "elf": validate_elf(Path("PaperNX.elf").read_bytes()), "hardware": "UNVERIFIED"
}
Path("verification/package-integrity.json").write_text(json.dumps(result, indent=2) + "\n", encoding="utf-8")
print(json.dumps(result, indent=2))
