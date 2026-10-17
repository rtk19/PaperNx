#!/usr/bin/env python3
"""Deterministic native-reader stress documents. Generated files stay under build/.

No network, dependencies, third-party content or user-book paths are used.
"""
import json
import struct
import zipfile
import zlib
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
OUTPUT = ROOT / "build" / "test-data" / "v03-stress"


def png(width=2048, height=1536, color=(24, 160, 80)):
    def chunk(kind, data):
        return struct.pack(">I", len(data)) + kind + data + struct.pack(">I", zlib.crc32(kind + data) & 0xffffffff)
    pixels = (b"\0" + bytes(color) * width) * height
    return (b"\x89PNG\r\n\x1a\n" + chunk(b"IHDR", struct.pack(">IIBBBBB", width, height, 8, 2, 0, 0, 0))
            + chunk(b"IDAT", zlib.compress(pixels, 9)) + chunk(b"IEND", b""))


def entry(archive, name, value, compressed=True):
    info = zipfile.ZipInfo(name, date_time=(2026, 1, 1, 0, 0, 0))
    info.compress_type = zipfile.ZIP_DEFLATED if compressed else zipfile.ZIP_STORED
    archive.writestr(info, value)


def epub(name, chapters, images=None):
    images = images or {}
    manifest = ['<item id="nav" href="nav.xhtml" media-type="application/xhtml+xml" properties="nav"/>']
    manifest += [f'<item id="c{i}" href="c{i}.xhtml" media-type="application/xhtml+xml"/>' for i in range(len(chapters))]
    manifest += [f'<item id="image{i}" href="{key}" media-type="image/png"/>' for i, key in enumerate(images)]
    spine = ''.join(f'<itemref idref="c{i}"/>' for i in range(len(chapters)))
    nav = ''.join(f'<li><a href="c{i}.xhtml">Bölüm {i+1}</a></li>' for i in range(len(chapters)))
    with zipfile.ZipFile(OUTPUT / name, "w") as archive:
        entry(archive, "mimetype", "application/epub+zip", False)
        entry(archive, "META-INF/container.xml", '<container version="1.0" xmlns="urn:oasis:names:tc:opendocument:xmlns:container"><rootfiles><rootfile full-path="OEBPS/content.opf" media-type="application/oebps-package+xml"/></rootfiles></container>')
        entry(archive, "OEBPS/content.opf", '<package xmlns="http://www.idpf.org/2007/opf" version="3.0" unique-identifier="id"><metadata xmlns:dc="http://purl.org/dc/elements/1.1/"><dc:identifier id="id">papernx-stress-' + name + '</dc:identifier><dc:title>' + name + '</dc:title><dc:language>tr</dc:language></metadata><manifest>' + ''.join(manifest) + '</manifest><spine>' + spine + '</spine></package>')
        entry(archive, "OEBPS/nav.xhtml", '<html xmlns="http://www.w3.org/1999/xhtml" xmlns:epub="http://www.idpf.org/2007/ops"><head><title>İçindekiler</title></head><body><nav epub:type="toc"><ol>' + nav + '</ol></nav></body></html>')
        for i, body in enumerate(chapters):
            entry(archive, f"OEBPS/c{i}.xhtml", '<html xmlns="http://www.w3.org/1999/xhtml" lang="tr"><head><title>Okuma deneyi</title></head><body>' + body + '</body></html>')
        for key, value in images.items():
            entry(archive, "OEBPS/" + key, value)


def pdf(pages=320):
    objects = [b'<< /Type /Catalog /Pages 2 0 R >>', None,
               b'<< /Type /Font /Subtype /Type1 /BaseFont /Helvetica >>']
    children = []
    for index in range(pages):
        page_number = len(objects) + 1
        children.append(f"{page_number} 0 R")
        objects.append(f'<< /Type /Page /Parent 2 0 R /MediaBox [0 0 612 792] /Resources << /Font << /F1 3 0 R >> >> /Contents {page_number+1} 0 R >>'.encode())
        lines = [f'BT /F1 18 Tf 50 750 Td (PaperNX academic PDF - page {index+1} / {pages}) Tj /F1 11 Tf']
        lines += [f'0 -22 Td (Research paragraph {line+1}: offline reading, typography, navigation and reliable storage.) Tj' for line in range(28)]
        content = ('\n'.join(lines) + '\nET').encode()
        objects.append(f'<< /Length {len(content)} >>\nstream\n'.encode() + content + b'\nendstream')
    objects[1] = ('<< /Type /Pages /Count ' + str(pages) + ' /Kids [' + ' '.join(children) + '] >>').encode()
    output = bytearray(b'%PDF-1.4\n%\xe2\xe3\xcf\xd3\n')
    offsets = [0]
    for index, obj in enumerate(objects, 1):
        offsets.append(len(output))
        output.extend(f'{index} 0 obj\n'.encode() + obj + b'\nendobj\n')
    xref = len(output)
    output.extend(f'xref\n0 {len(offsets)}\n0000000000 65535 f \n'.encode())
    for offset in offsets[1:]:
        output.extend(f'{offset:010} 00000 n \n'.encode())
    output.extend(f'trailer\n<< /Size {len(offsets)} /Root 1 0 R >>\nstartxref\n{xref}\n%%EOF\n'.encode())
    (OUTPUT / "academic-320.pdf").write_bytes(output)


def main():
    OUTPUT.mkdir(parents=True, exist_ok=True)
    paragraph = '<p>İstanbul’da ışık, yağmur ve çiçekler: “Şimdi oku!” ' + ('Türkçe kitap; düşünce, öğrenme ve özgürlük. ' * 12) + '</p>'
    special = ('<h1>Bölüm başlığı</h1><p><span style="font-size:6px">Yayıncıdan gelen küçücük yazı.</span></p>'
               '<pre>Birinci dize\n  İkinci dize\nÜçüncü dize</pre>'
               '<table><tr><th>Başlık</th><th>Değer</th></tr><tr><td>Çiçek</td><td>Ölçüm</td></tr></table>')
    epub("large.epub", [f'<h1>Bölüm {i+1}</h1>' + special + paragraph * 18 for i in range(24)])
    images = {f'picture{i}.png': png(color=(24 + i * 8, 160, 80)) for i in range(8)}
    epub("illustrated.epub", [f'<img src="picture{i}.png" alt="Özgün test resmi"/>' + paragraph * 3 for i in range(8)], images)
    marker = 'MMMMM İstanbul Çiçek'
    epub("tiny-inline.epub", ['<p><span style="font-size:6px">' + marker + '</span></p>'])
    epub("normal-inline.epub", ['<p><span>' + marker + '</span></p>'])
    pdf()
    manifest = {path.name: path.stat().st_size for path in sorted(OUTPUT.iterdir()) if path.suffix in {'.epub', '.pdf'}}
    (OUTPUT / 'manifest.json').write_text(json.dumps({'generated': 'deterministic', 'large_chapters': 24,
        'image_chapters': 8, 'image_width': 2048, 'image_height': 1536, 'pdf_pages': 320, 'files': manifest}, indent=2) + '\n')
    print(json.dumps(manifest))


if __name__ == '__main__':
    main()
