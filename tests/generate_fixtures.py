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

def generate_cover_png() -> bytes:
    """Real RGB PNG with recognizable colors, no optional Python dependencies."""
    import struct
    def chunk(kind, data):
        return struct.pack(">I", len(data)) + kind + data + struct.pack(">I", zlib.crc32(kind + data) & 0xffffffff)
    width, height = 96, 144
    pixels = b"".join(b"\0" + bytes((24, 80 + y % 80, 180)) * width for y in range(height))
    return (b"\x89PNG\r\n\x1a\n" + chunk(b"IHDR", struct.pack(">IIBBBBB", width, height, 8, 2, 0, 0, 0))
            + chunk(b"IDAT", zlib.compress(pixels)) + chunk(b"IEND", b""))

def write_entry(archive, name, data, compress_type=zipfile.ZIP_STORED):
    info = zipfile.ZipInfo(name, date_time=(2026, 1, 1, 0, 0, 0))
    info.compress_type = compress_type
    archive.writestr(info, data)

def create_valid_epub(output_path: Path):
    """Create a fully compliant EPUB3 container with 2 chapters, TOC, cover, and Turkish text."""
    output_path.parent.mkdir(parents=True, exist_ok=True)
    cover_png = generate_cover_png()

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
    <dc:creator>Yusuf &#x130;nce</dc:creator>
    <dc:subject>Roman</dc:subject>
    <dc:subject>T&#xFC;rk Edebiyat&#x131;</dc:subject>
    <meta name="calibre:series" content="Bo&#x11F;azi&#xE7;i"/>
    <meta name="calibre:series_index" content="2.5"/>
    <meta property="dcterms:modified">2026-10-17T00:00:00Z</meta>
  </metadata>
  <manifest>
    <item id="toc" href="toc.xhtml" media-type="application/xhtml+xml" properties="nav"/>
    <item id="cover-img" href="cover.png" media-type="image/png" properties="cover-image"/>
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

    paragraph = "<p>İ ı I i Ş ş Ğ ğ Ü ü Ö ö Ç ç. Boğaziçi kıyısında okuma, anılar ve yağmur. " + ("Kitap sayfalarında İstanbul'un ışığı. " * 15) + "</p>"
    ch1_xhtml = ch1_xhtml.replace("</body>", '<img src="cover.png" alt="Kitap kapağı" width="96" height="144"/>' + paragraph * 20 + "</body>")
    ch2_xhtml = ch2_xhtml.replace("</body>", paragraph * 16 + "</body>")
    with zipfile.ZipFile(output_path, "w") as z:
        # EPUB spec requirement: mimetype must be the first uncompressed entry
        write_entry(z, "mimetype", "application/epub+zip", compress_type=zipfile.ZIP_STORED)
        write_entry(z, "META-INF/container.xml", container_xml, compress_type=zipfile.ZIP_DEFLATED)
        write_entry(z, "OEBPS/content.opf", content_opf, compress_type=zipfile.ZIP_DEFLATED)
        write_entry(z, "OEBPS/toc.xhtml", toc_xhtml, compress_type=zipfile.ZIP_DEFLATED)
        write_entry(z, "OEBPS/cover.png", cover_png, compress_type=zipfile.ZIP_DEFLATED)
        write_entry(z, "OEBPS/ch1.xhtml", ch1_xhtml, compress_type=zipfile.ZIP_DEFLATED)
        write_entry(z, "OEBPS/ch2.xhtml", ch2_xhtml, compress_type=zipfile.ZIP_DEFLATED)

def create_metadata_variants(source: Path):
    """Reuse real EPUB content for namespace, bounds and EPUB3-series checks."""
    with zipfile.ZipFile(source) as archive:
        entries = [(item.filename, archive.read(item.filename)) for item in archive.infolist()]
    variants = {
        "metadata_epub3.epub": '''<metadata xmlns:d="http://purl.org/dc/elements/1.1/">
          <d:title>Namespace test</d:title><d:creator>&#x130;nce</d:creator>
          <d:subject>&#x15E;iir</d:subject>
          <meta property="belongs-to-collection" id="set">Not a series</meta>
          <meta refines="#set" property="collection-type">set</meta>
          <meta property="belongs-to-collection" id="series">Okuma Dizisi</meta>
          <meta refines="#series" property="group-position">3</meta>
          <meta refines="#series" property="collection-type">series</meta></metadata>''',
        "metadata_bounded.epub": '<metadata xmlns:d="http://purl.org/dc/elements/1.1/">'
            '<d:title>Bounded test</d:title><d:creator>&#x130;nce</d:creator>'
            '<d:subject>' + '&#x131;' * 5000 + '</d:subject>'
            '<meta name="calibre:series_index" content="NaN"/></metadata>',
        "metadata_missing.epub": '<metadata><title>Missing optional fields</title></metadata>',
    }
    import re
    for filename, metadata in variants.items():
        with zipfile.ZipFile(FIXTURES_DIR / filename, "w") as out:
            for name, data in entries:
                if name == "OEBPS/content.opf":
                    data = re.sub(r'<metadata\b.*?</metadata>', metadata,
                                  data.decode("utf-8"), flags=re.S).encode("utf-8")
                write_entry(out, name, data)

def create_valid_pdf(output_path: Path):
    """Create a fully compliant multi-page PDF with outlines and Turkish text."""
    output_path.parent.mkdir(parents=True, exist_ok=True)

    stream1 = b"q 96 0 0 144 50 400 cm /Im1 Do Q BT /F1 24 Tf 50 750 Td (PaperNX PDF Test - Sayfa 1) Tj 0 -36 Td /F1 14 Tf (Turkce Karakter Testi: Agac, Isik, Uzum, Omur, Cicek, Istanbul) Tj ET"
    stream2 = b"BT /F1 24 Tf 50 750 Td (PaperNX PDF Test - Sayfa 2) Tj 0 -36 Td /F1 14 Tf (Ikinci sayfa icerigi ve farkli koordinat duzeni.) Tj ET"

    objects = []
    
    # 1: Catalog
    objects.append(b"1 0 obj\n<< /Type /Catalog /Pages 2 0 R /Outlines 5 0 R >>\nendobj\n")
    # 2: Pages
    objects.append(b"2 0 obj\n<< /Type /Pages /Kids [3 0 R 4 0 R] /Count 2 >>\nendobj\n")
    # 3: Page 1
    objects.append(b"3 0 obj\n<< /Type /Page /Parent 2 0 R /MediaBox [0 0 595 842] /Contents 7 0 R /Resources << /Font << /F1 6 0 R >> /XObject << /Im1 11 0 R >> >> >>\nendobj\n")
    # 4: Page 2
    objects.append(b"4 0 obj\n<< /Type /Page /Parent 2 0 R /MediaBox [0 0 595 842] /Contents 8 0 R /Resources << /Font << /F1 6 0 R >> /XObject << /Im1 11 0 R >> >> >>\nendobj\n")
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

    pixels = bytes((24, 80, 180)) * (96 * 144)
    compressed = zlib.compress(pixels)
    objects.append(f"11 0 obj\n<< /Type /XObject /Subtype /Image /Width 96 /Height 144 /ColorSpace /DeviceRGB /BitsPerComponent 8 /Filter /FlateDecode /Length {len(compressed)} >>\nstream\n".encode("ascii") + compressed + b"\nendstream\nendobj\n")
    objects.append(b"12 0 obj\n<< /Title (PaperNX PDF Fixture) /Author (PDF Author) /Subject (PDF Genre) >>\nendobj\n")

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

    trailer = f"trailer\n<< /Size {len(xref)} /Root 1 0 R /Info 12 0 R >>\nstartxref\n{xref_pos}\n%%EOF\n"

    pdf_content = header + body + xref_str.encode("latin1") + trailer.encode("latin1")
    output_path.write_bytes(pdf_content)

def create_hebrew_variants(base):
    """Original mixed-language text, no direction metadata on the paragraphs."""
    with zipfile.ZipFile(base) as archive:
        source = {name: archive.read(name) for name in archive.namelist()}
    for name, language, spine in (("hebrew_auto", "he", ""),
                                   ("hebrew_legacy", "iw-IL", ""),
                                   ("hebrew_navigation", "he", ""),
                                   ("hebrew_publisher", "he", ""),
                                   ("hebrew_spine_ltr", "he", ' page-progression-direction="ltr"'),
                                   ("hebrew_spine_rtl", "en", ' page-progression-direction="rtl"')):
        entries = dict(source)
        opf = entries["OEBPS/content.opf"].decode()
        opf = opf.replace("İstanbul Masalları ve Şiirler", "הספר החדש PaperNX 123").replace("Ahmet Hamdi Tanpınar", "ישראל ישראלי")
        opf = opf.replace("<dc:language>tr</dc:language>", f"<dc:language>{language}</dc:language>").replace("<spine>", f"<spine{spine}>")
        entries["OEBPS/content.opf"] = opf.encode()
        entries["OEBPS/ch1.xhtml"] = ('<html xmlns="http://www.w3.org/1999/xhtml"><head><title>עברית</title></head><body>'
            '<p>שלום עולם</p><p>English 123</p><p dir="ltr">שלום עולם</p><p dir="rtl">English 123</p>'
            '<p>ספר עם מילים English 456 ומספרים 123 בשורה אחת.</p>'
            '<p>' + 'זהו ספר בעברית ובו פסקה ארוכה שמיועדת לבדוק מעבר שורות וסידור נכון של המילים. ' * 8 + '</p>'
            '<p>שורה ראשונה<br/>שורה אחרונה</p></body></html>').encode()
        entries["OEBPS/toc.xhtml"] = entries["OEBPS/toc.xhtml"].decode().replace("Birinci Bölüm", "פרק ראשון").encode()
        if name == "hebrew_navigation":
            nav = entries["OEBPS/toc.xhtml"].decode().replace("פרק ראשון", "פֶּרֶק ראשון &amp; English 123 — " + "כותרת ארוכה בעברית " * 24)
            nav = nav.replace("İkinci Bölüm", "פרק שני — ההמשך")
            nav = nav.replace('</a></li>\n      <li><a href="ch2.xhtml">', '</a><ol><li><a href="ch1.xhtml#later">כותרת משנה — המשך הפרק</a></li></ol></li>\n      <li><a href="ch2.xhtml">')
            entries["OEBPS/toc.xhtml"] = nav.encode()
            chapter = entries["OEBPS/ch1.xhtml"].decode().replace('<p>שורה ראשונה', '<h2 id="later">כותרת משנה — המשך הפרק</h2><p>' + 'תוכן נוסף אחרי הכותרת בתוך אותו פרק. ' * 12 + '</p><p>שורה ראשונה')
            entries["OEBPS/ch1.xhtml"] = chapter.encode()
        if name == "hebrew_publisher":
            chapter = entries["OEBPS/ch1.xhtml"].decode()
            chapter = chapter.replace('<p>שלום עולם</p>', '<p style="text-align:right">שלום עולם</p>')
            chapter = chapter.replace('<p>English 123</p>', '<p style="text-align:left">English 123</p>')
            chapter = chapter.replace('<p dir="ltr">', '<p dir="ltr" style="text-align:left">')
            chapter = chapter.replace('<p dir="rtl">', '<p dir="rtl" style="text-align:right">')
            entries["OEBPS/ch1.xhtml"] = chapter.encode()
        with zipfile.ZipFile(FIXTURES_DIR / (name + ".epub"), "w") as archive:
            for entry, data in entries.items():
                write_entry(archive, entry, data)

def create_emphasis_fixture(base):
    """Same original Hebrew text in semantic, class and inline emphasis styles."""
    with zipfile.ZipFile(base) as archive:
        entries = {name: archive.read(name) for name in archive.namelist()}
    text = "שָׁלוֹם עולם ספר חדש"
    variants = [text, f"<strong>{text}</strong>", f"<em>{text}</em>",
                f"<strong><em>{text}</em></strong>", f'<span class="bold">{text}</span>',
                f'<span class="italic">{text}</span>', f'<span style="font-weight:bold">{text}</span>',
                f'<span style="font-style:italic">{text}</span>', text,
                f'<u>{text}</u>', f'<s>{text}</s>', f'<sup>{text}</sup>']
    items, spine = [], []
    for index, content in enumerate(variants):
        name = f"emphasis{index}.xhtml"
        items.append(f'<item id="e{index}" href="{name}" media-type="application/xhtml+xml"/>')
        spine.append(f'<itemref idref="e{index}"/>')
        entries["OEBPS/" + name] = (f'<html xmlns="http://www.w3.org/1999/xhtml" lang="he"><head>'
            '<title>Emphasis</title><link rel="stylesheet" href="emphasis.css"/>'
            '<style>.italic {font-style:italic} p {font-size:1px; color:white; text-align:left}</style>'
            '</head><body><p style="font-size:1px;color:white;text-align:left">'
            + content + '</p></body></html>').encode()
    entries["OEBPS/emphasis.css"] = b'.bold {font-weight:bold} body {margin:300px}'
    opf = entries["OEBPS/content.opf"].decode()
    opf = opf.replace('</manifest>', ''.join(items) + '<item id="emcss" href="emphasis.css" media-type="text/css"/></manifest>')
    start, end = opf.index('<spine>'), opf.index('</spine>') + len('</spine>')
    entries["OEBPS/content.opf"] = (opf[:start] + '<spine>' + ''.join(spine) + '</spine>' + opf[end:]).encode()
    with zipfile.ZipFile(FIXTURES_DIR / "hebrew_emphasis.epub", "w") as archive:
        for entry, data in entries.items():
            write_entry(archive, entry, data)

def main():
    FIXTURES_DIR.mkdir(parents=True, exist_ok=True)
    
    epub_path = FIXTURES_DIR / "sample_book.epub"
    create_valid_epub(epub_path)
    create_metadata_variants(epub_path)
    create_hebrew_variants(epub_path)
    create_emphasis_fixture(epub_path)
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
