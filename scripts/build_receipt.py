#!/usr/bin/env python3
"""Capture/verify source before and after an isolated Switch build."""
import argparse
import hashlib
import json
from pathlib import Path
from artifact_validation import source_identity, validate_elf

parser = argparse.ArgumentParser()
parser.add_argument('action', choices=['start', 'finish'])
parser.add_argument('--root', type=Path, default=Path('.'))
parser.add_argument('--receipt', type=Path, default=Path('verification/v04-switch-build-receipt.json'))
args = parser.parse_args()
identity = source_identity(args.root)
if args.action == 'start':
    args.receipt.parent.mkdir(parents=True, exist_ok=True)
    args.receipt.write_text(json.dumps({'source_before': identity}, indent=2) + '\n')
else:
    receipt = json.loads(args.receipt.read_text())
    if receipt['source_before']['implementation_sha256'] != identity['implementation_sha256']:
        raise SystemExit('Source changed during build; rebuild before packaging')
    outputs = {}
    for name in ['PaperNX.elf', 'PaperNX.nro']:
        data = (args.root / name).read_bytes()
        outputs[name] = {'sha256': hashlib.sha256(data).hexdigest(), 'bytes': len(data)}
        if name.endswith('.elf'):
            outputs[name].update(validate_elf(data))
    receipt.update(source_after=identity, outputs=outputs)
    args.receipt.write_text(json.dumps(receipt, indent=2) + '\n')
print(args.receipt)
