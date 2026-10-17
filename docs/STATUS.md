# PaperNX — doğrulanmış durum

Tarih: 17 Ekim 2026. Dal: `feat/native-reader-foundation`.
Uygulama/build kaynağı: `5c9cd44fe6d27457b8096a0452ea8d75974ed1f1`.
PR: [#1](https://github.com/ysfbz/PaperNx/pull/1), taslak; main'e merge yapılmadı.

| Kapı | Sonuç | Kanıt |
|---|---|---|
| C++17 çekirdek | PASS, 1/1 | `verification/desktop.log` |
| Release çekirdek, assertions açık | PASS, 1/1 | `verification/core-release.log` |
| Paketleme birim testleri | PASS | `verification/desktop.log` |
| Tam SDL2/MuPDF masaüstü uygulaması | PASS; 3/3 CTest | `verification/desktop.log` |
| Kaynaktan MuPDF 1.23.11 + uygulama | PASS; 3/3 CTest | `verification/pinned-desktop.log` |
| ASan/UBSan + leak detection | PASS; 3/3 CTest | `verification/sanitizers.log` |
| ARM64 MuPDF, uygulama, ELF/NRO | PASS | `verification/switch.log` |
| NRO0/NACP/RomFS/font/ZIP bütünlüğü | PASS | `verification/artifacts.json`, `verification/package-integrity.json` |
| Fiziksel Switch | UNVERIFIED | `docs/DEVICE_TEST_PLAN.md` |

P0 üye/API/link hataları, bozuk checksum fixture'ı, numeric overflow/underflow/NaN/inf,
UTF-8 giriş/silme, başarısız kayıt retry, thumbnail pending yaşam döngüsü, cache bütçesi,
font-family anahtarı ve EPUB3 nav/gerçek kapak çıkarma düzeltildi. Sayfa yüzeyinin tema
rengi RGB ile denetlenir. PDF zoom/pan gerçek kontrollerden erişilir. Kaydı olmayan dış
klasör kitapları PAPERNX-3 yol/metadata kaydıyla yeniden bulunur. MuPDF ham pointer
bookmark'ları yerine bölüm/oran konumu kullanılır. Switch SDL A/B-X/Y eşlemesi ayrıca düzeltildi.

Gerçek önizleme çıktıları yerel `dist/` klasöründedir. NRO: 47.042.549 byte.
ZIP SHA256: `a01d10029332644cb8b7242f5fd226047509cadbf7646aa153b7a565af494b5c`.
NRO SHA256: `472336e44501ab893237899a11e33f6c1e7cbedc1dce4b8efdf7a2f714fa2dfc`.

Karar: **NEEDS FIXES** — stable v1.0 onayı yok. Mühendislik değerlendirmesi: **82/100**.
Kalan P1 kabul engeli fiziksel cihaz matrisidir. Bilinen P2 sınırları: yaklaşık EPUB
konumu ve dosya taşıma/yeniden adlandırmada kimlik değişimi. Ayrıntılar:
[TEST_RESULTS](TEST_RESULTS.md), [RELEASE_REPORT](RELEASE_REPORT.md), [KNOWN_ISSUES](KNOWN_ISSUES.md).
