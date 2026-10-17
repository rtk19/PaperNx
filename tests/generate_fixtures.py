#!/usr/bin/env python3
"""
tests/generate_fixtures.py
Generates valid, standards-compliant EPUB and PDF test fixtures with Turkish typography,
outlines, multiple chapters/pages, cover images, and corrupt fixtures for error testing.
"""

import os
import sys
import zipfile
import zlib
from pathlib import Path

FIXTURES_DIR = Path(__file__).resolve().parent / "fixtures"

def generate_minimal_jpeg() -> bytes:
    """Generate a minimal valid 2x2 JPEG image binary."""
    # SOI (FF D8), APP0 (FF E0), DQT (FF DB), SOF0 (FF C0), DHT (FF C4), SOS (FF DA), EOI (FF D9)
    # 2x2 pure white JPEG
    return bytes.fromhex(
        "ffd8ffe000104a46494600010101004800480000ffdb00430008060607060508070707"
        "0909080a0c140d0c0b0b0c1912130f141d1a1f1e1d1a1c1c20242e2720222c231c1c28"
        "37292c30313434341f27393d38323c2e333432ffc0000b080002000201011100ffc400"
        "1f0000010501010101010100000000000000000102030405060708090a0bffda000801"
        "0100003f007fb2c0ffd9"
    )

def create_valid_epub(output_path: Path):
    """Create a fully compliant EPUB3 container with 2 chapters, TOC, cover, and Turkish text."""
    output_path.parent.mkdir(parents=True, exist_ok=True)
    cover_jpeg = generate_minimal_jpeg()

    container_xml = """<?xml version="1.0" encoding="UTF-8"?>
<container version="1.0" xmlns="urn:oasis:names:tc:opendocument:xmlns:container">
  <rootfiles>
    <rootfile full-path="OEBPS/content.opf" media-type="application/oebps-package+xml"/>
  </rootfiles>
</container>"""

    content_opf = """<?xml version="1.0" encoding="UTF-8"?>
<package xmlns="http://www.idpf.org/2007/opf" version="3.0" unique-identifier="pub-id">
  <metadata xmlns:dc="http://purl.org/dc/elements/1.1/">
    <dc:identifier id="pub-id">urn:uuid:papernx-test-fixture-001</dc:identifier>
    <dc:title>İstanbul Masalları ve Şiirler</dc:title>
    <dc:creator>Ahmet Hamdi Tanpınar</dc:creator>
    <dc:language>tr</dc:language>
    <meta property="dcterms:modified">2026-10-17T00:00:00Z</meta>
  </metadata>
  <manifest>
    <item id="toc" href="toc.xhtml" media-type="application/xhtml+xml" properties="nav"/>
    <item id="cover-img" href="cover.jpg" media-type="image/jpeg" properties="cover-image"/>
    <item id="ch1" href="ch1.xhtml" media-type="application/xhtml+xml"/>
    <item id="ch2" href="ch2.xhtml" media-type="application/xhtml+xml"/>
  </manifest>
  <spine>
    <itemref idref="ch1"/>
    <itemref idref="ch2"/>
  </spine>
</package>"""

    toc_xhtml = """<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE html>
<html xmlns="http://www.w3.org/1999/xhtml" xmlns:epub="http://www.idpf.org/2007/ops" lang="tr">
<head>
  <title>İçindekiler</title>
</head>
<body>
  <nav epub:type="toc" id="toc">
    <h1>İçindekiler</h1>
    <ol>
      <li><a href="ch1.xhtml">Birinci Bölüm: Boğaziçi ve Sabah Güneşi</a></li>
      <li><a href="ch2.xhtml">İkinci Bölüm: Sonbahar Ağaçları ve Yağmur</a></li>
    </ol>
  </nav>
</body>
</html>"""

    ch1_xhtml = """<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE html>
<html xmlns="http://www.w3.org/1999/xhtml" lang="tr">
<head>
  <title>Birinci Bölüm: Boğaziçi</title>
  <style>
    body { font-family: serif; margin: 5%; line-height: 1.6; }
    h1 { color: #333333; text-align: center; }
    p { text-indent: 1.5em; margin-bottom: 0.8em; }
  </style>
</head>
<body>
  <h1>Birinci Bölüm: Boğaziçi ve Sabah Güneşi</h1>
  <p>İstanbul'un serin sabahında, sislerin ardından yükselen altın sarısı güneş Boğaz'ın sularını aydınlatıyordu. Vapurlar ağır ağır iskeleye yanaşırken martıların çığlıkları duyuluyordu.</p>
  <p>Şehrin bin yıllık taş sokaklarında yürüyen insanlar, taze simit ve çay kokusuyla güne merhaba diyordu. Işık, gölgeler ve hatıralar iç içe geçmişti.</p>
</body>
</html>"""

    ch2_xhtml = """<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE html>
<html xmlns="http://www.w3.org/1999/xhtml" lang="tr">
<head>
  <title>İkinci Bölüm: Sonbahar</title>
  <style>
    body { font-family: serif; margin: 5%; line-height: 1.6; }
    h1 { color: #333333; text-align: center; }
    p { text-indent: 1.5em; margin-bottom: 0.8em; }
  </style>
</head>
<body>
  <h1>İkinci Bölüm: Sonbahar Ağaçları ve Yağmur</h1>
  <p>Yapraklar sararıp dökülürken, ince bir yağmur camlara vuruyordu. Eski ahşap konağın kütüphanesinde açılan her sayfa, geçmiş zamanın derinliklerinden fısıldıyordu.</p>
  <p>Çınar ağaçlarının altında ıslanan kaldırımlar, kışın habercisi olan soğuk rüzgârlarla ürperiyordu. Çiçekler solarak yerini sessiz bir bekleyişe bırakmıştı.</p>
</body>
</html>"""

    with zipfile.ZipFile(output_path, "w") as z:
        # EPUB spec requirement: mimetype must be the first uncompressed entry
        z.writestr("mimetype", "application/epub+zip", compress_type=zipfile.ZIP_STORED)
        z.writestr("META-INF/container.xml", container_xml, compress_type=zipfile.ZIP_DEFLATED)
        z.writestr("OEBPS/content.opf", content_opf, compress_type=zipfile.ZIP_DEFLATED)
        z.writestr("OEBPS/toc.xhtml", toc_xhtml, compress_type=zipfile.ZIP_DEFLATED)
        z.writestr("OEBPS/cover.jpg", cover_jpeg, compress_type=zipfile.ZIP_DEFLATED)
        z.writestr("OEBPS/ch1.xhtml", ch1_xhtml, compress_type=zipfile.ZIP_DEFLATED)
        z.writestr("OEBPS/ch2.xhtml", ch2_xhtml, compress_type=zipfile.ZIP_DEFLATED)

def create_valid_pdf(output_path: Path):
    """Create a fully compliant multi-page PDF with outlines and Turkish text."""
    output_path.parent.mkdir(parents=True, exist_ok=True)

    stream1 = b"BT /F1 24 Tf 50 750 Td (PaperNX PDF Test - Sayfa 1) Tj 0 -36 Td /F1 14 Tf (Turkce Karakter Testi: Agac, Isik, Uzum, Omur, Cicek, Istanbul) Tj ET"
    stream2 = b"BT /F1 24 Tf 50 750 Td (PaperNX PDF Test - Sayfa 2) Tj 0 -36 Td /F1 14 Tf (Ikinci sayfa icerigi ve farkli koordinat duzeni.) Tj ET"

    objects = []
    
    # 1: Catalog
    objects.append(b"1 0 obj\n<< /Type /Catalog /Pages 2 0 R /Outlines 5 0 R >>\nendobj\n")
    # 2: Pages
    objects.append(b"2 0 obj\n<< /Type /Pages /Kids [3 0 R 4 0 R] /Count 2 >>\nendobj\n")
    # 3: Page 1
    objects.append(b"3 0 obj\n<< /Type /Page /Parent 2 0 R /MediaBox [0 0 595 842] /Contents 7 0 R /Resources << /Font << /F1 6 0 R >> >> >>\nendobj\n")
    # 4: Page 2
    objects.append(b"4 0 obj\n<< /Type /Page /Parent 2 0 R /MediaBox [0 0 595 842] /Contents 8 0 R /Resources << /Font << /F1 6 0 R >> >> >>\nendobj\n")
    # 5: Outlines
    objects.append(b"5 0 obj\n<< /Type /Outlines /First 9 0 R /Last 10 0 R /Count 2 >>\nendobj\n")
    # 6: Font (Helvetica standard)
    objects.append(b"6 0 obj\n<< /Type /Font /Subtype /Type1 /BaseFont /Helvetica /Encoding /WinAnsiEncoding >>\nendobj\n")
    # 7: Contents Page 1
    objects.append(f"7 0 obj\n<< /Length {len(stream1)} >>\nstream\n".encode("latin1") + stream1 + b"\nendstream\nendobj\n")
    # 8: Contents Page 2
    objects.append(f"8 0 obj\n<< /Length {len(stream2)} >>\nstream\n".encode("latin1") + stream2 + b"\nendstream\nendobj\n")
    # 9: Outline item 1
    objects.append(b"9 0 obj\n<< /Title (Sayfa 1 - Giris) /Parent 5 0 R /Next 10 0 R /Dest [3 0 R /XYZ null null null] >>\nendobj\n")
    # 10: Outline item 2
    objects.append(b"10 0 obj\n<< /Title (Sayfa 2 - Gelisme) /Parent 5 0 R /Prev 9 0 R /Dest [4 0 R /XYZ null null null] >>\nendobj\n")

    header = b"%PDF-1.4\n%\xe2\xe3\xcf\xd3\n"
    body = b""
    xref = [0]
    
    pos = len(header)
    for obj in objects:
        xref.append(pos)
        pos += len(obj)
        body += obj

    xref_pos = pos
    xref_str = f"xref\n0 {len(xref)}\n0000000000 65535 f \n"
    for offset in xref[1:]:
        xref_str += f"{offset:010d} 00000 n \n"

    trailer = f"trailer\n<< /Size {len(xref)} /Root 1 0 R >>\nstartxref\n{xref_pos}\n%%EOF\n"

    pdf_content = header + body + xref_str.encode("latin1") + trailer.encode("latin1")
    output_path.write_bytes(pdf_content)

def main():
    FIXTURES_DIR.mkdir(parents=True, exist_ok=True)
    
    epub_path = FIXTURES_DIR / "sample_book.epub"
    create_valid_epub(epub_path)
    print(f"Generated EPUB fixture: {epub_path} ({epub_path.stat().st_size} bytes)")

    pdf_path = FIXTURES_DIR / "sample_doc.pdf"
    create_valid_pdf(pdf_path)
    print(f"Generated PDF fixture: {pdf_path} ({pdf_path.stat().st_size} bytes)")

    # Corrupt fixtures for error resiliency tests
    corrupt_epub = FIXTURES_DIR / "corrupt.epub"
    corrupt_epub.write_bytes(b"PK\x03\x04truncated_corrupt_epub_data_stream_garbage_test")
    print(f"Generated corrupt EPUB fixture: {corrupt_epub}")

    corrupt_pdf = FIXTURES_DIR / "corrupt.pdf"
    corrupt_pdf.write_bytes(b"%PDF-1.4\ncorrupted_body_stream_with_no_xref_or_trailer_garbage")
    print(f"Generated corrupt PDF fixture: {corrupt_pdf}")

if __name__ == "__main__":
    main()
