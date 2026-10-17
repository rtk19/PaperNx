#!/usr/bin/env python3
import hashlib
import json
from pathlib import Path
from artifact_validation import validate_assets, read_romfs, validate_elf, source_identity
from package_release import validate_nro

elf = Path("PaperNX.elf").read_bytes()
nro = Path("PaperNX.nro").read_bytes()
meta = validate_nro(nro)
assets = validate_assets(nro, meta["declared_size"])
files = read_romfs(nro, assets["romfs"])
for name in ("NotoSans-Regular.ttf", "NotoSerif-Regular.ttf", "OFL.txt"):
    assert files["fonts/" + name] == (Path("fonts") / name).read_bytes(), name
result = {"source": source_identity(Path.cwd()), "elf": validate_elf(elf), "nro": meta, "assets": assets,
          "romfs_files": sorted(files), "nro_sha256": hashlib.sha256(nro).hexdigest(),
          "hardware": "UNVERIFIED"}
Path("verification/artifacts.json").write_text(json.dumps(result, indent=2) + "\n")
print(json.dumps(result, indent=2))
