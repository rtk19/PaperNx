# PaperNX — sürüm teslim raporu

17 Ekim 2026. **Karar: NEEDS FIXES. Hazırlık puanı: 82/100** (mühendislik değerlendirmesi).
Gerçek RC önizleme derlemesi vardır; stable v1.0 onayı verilmemiştir.

## Kaynak ve PR

- Dal: `feat/native-reader-foundation`.
- Derlenen uygulama commit'i: `5c9cd44fe6d27457b8096a0452ea8d75974ed1f1`.
- [PR #1](https://github.com/ysfbz/PaperNx/pull/1): OPEN/DRAFT; main'e merge yapılmadı.
- Başlangıç HEAD'i GitHub fetch ile `204dc4b` olarak doğrulandı; daha yeni değişiklik üzerine yazılmadı.
- Mantıksal commit'ler: `58f40f3` core/kurtarma, `d4b6838` native/EPUB3,
  `3bda474` UI/PDF/worker, `322c9d2` tema/bellek/metadata, `2595548` Docker/ARM64/CI,
  `5c9cd44` Switch düğme eşlemesi. Sonraki kanıt/dokümantasyon commit'i uygulama kodunu değiştirmez.

## Değişiklikler

| Dosyalar | Giderilen sorun |
|---|---|
| `src/main.cpp` | Book::bytes, UTF-8 olayları, swkbd, PDF zoom/pan/drag/pinch, modal kontrolleri, controller instance/eşleme, save retry, thumbnail pending sınırları |
| `src/core.cpp`, `include/papernx/core.hpp` | errno/range/finite kontrolü; checksum fixture uyumu; güvenli temp/backup davranışı; PAPERNX-3 metadata ve kütüphane kalıcılığı |
| `src/mupdf_bridge.c`, bridge header | Gerçek MuPDF API'leri, sınırlı allocator, EPUB3 nav/kapak, süreçten bağımsız bölüm/oran yer imleri, tema yüzeyi, viewport temizliği |
| `src/page_cache.cpp`, cache header | Font ailesi anahtarı, güvenli ownership/replacement, oversized reddi, konservatif byte bütçesi |
| `src/gestures.cpp` | İlk analog hareketin gecikmeden işlenmesi |
| `CMakeLists.txt`, `Makefile`, `scripts/build-mupdf-switch.sh` | Tam uygulama linki; ortak HB/FT; ARM64 header/ABI ve POSIX uyumluluğu; C11/C++17; NACP/icon/RomFS |
| Docker/Compose, `scripts/verify-*`, artifact/package/notice betikleri | Yerel tekrarlanabilir yürütme, gerçek ELF/NRO/RomFS/ZIP kontrolü, hash/manifest/lisans bildirimi |
| `tests/*`, workflow, README/docs | Uzun EPUB/resimli PDF/PNG fixture; gerçek UI/worker/core regresyonları; PR testleri ve isteğe bağlı Switch tetikleyicisi; kanıta uygun durum |

## Son doğrulamalar

Portable 1/1, Release assertions açık core 1/1, native 3/3, MuPDF 1.23.11 pinned native
3/3 ve ASan/UBSan/leak detection 3/3 **PASS**. Paket testleri **PASS**.
Gerçek ARM64 ELF ve NRO derleme/paket kontrolü **PASS**.
Komutlar ve testlerin sınırları [TEST_RESULTS](TEST_RESULTS.md) içindedir.

Doğrudan native render testi 20 sayfalık EPUB açtı; Türkçe metadata, iki bölüm,
relayout ve TOC çözümlemesini; resimli iki sayfalık PDF ve zoom/pan çizimini doğruladı.
Uygulama testleri gerçek PNG kapağını, altı tema pikselini, yeniden açmayı, UI girdilerini,
worker reload ve başarısız kaydı ayrıca çalıştırdı. Cihazda çalıştığı iddia edilmez.

## Üretilen gerçek dosyalar

Yerel çıktı: `dist/PaperNX.nro`, `dist/PaperNX-switch-v0.1.0-rc1.zip`, iki SHA256
metni ve `dist/manifest.json`. GitHub stable release veya binary yayını yapılmadı.

```text
PaperNX.nro
472336e44501ab893237899a11e33f6c1e7cbedc1dce4b8efdf7a2f714fa2dfc
PaperNX-switch-v0.1.0-rc1.zip
a01d10029332644cb8b7242f5fd226047509cadbf7646aa153b7a565af494b5c
```

NRO 47.042.549 byte; ELF64 AArch64. NACP `0.1.0-rc1`, üç NRO segmenti, RomFS fontları,
ZIP yerleşimi/CRC ve 59 lisans bildirimi denetlendi. Manifest, kaynak revision ve
implementation SHA256 değerini içerir. `verification/package-integrity.json` bağımsız
son paket kontrolünün sonucudur. NRO0 başlığı konsolda çalıştırma kanıtı değildir.

## Kalan kabul engelleri

- **P1-HW / UNVERIFIED:** gerçek Switch üzerinde başlangıç, Joy-Con/Pro ve reconnect,
  swkbd, fiziksel pinch, uyku/uyanma, handheld/docked ve application/applet bellek/çıkış.
  Kod temasları: `src/main.cpp:45`, `src/main.cpp:220`, `src/main.cpp:437`,
  `src/main.cpp:807`, `src/main.cpp:869`. [Cihaz matrisi](DEVICE_TEST_PLAN.md) tamamlanmalıdır.
- Kalan doğrulanmış P0 derleme/test engeli yoktur. Sonlu testler bütün P1 ihtimallerinin
  yokluğunu ispatlamaz; fiziksel uyumluluk onayı verilmez.
- P2: konum düzen değişiminde yaklaşık (`src/mupdf_bridge.c:440`); dosya taşıma/rename
  kimliği değiştirir (`src/core.cpp:225`). DRM/parola desteklenmez.
- Büyük/karmaşık gerçek kullanıcı kitapları, bütün allocation hata noktaları ve gerçek RAM
  profili kapsam dışı kalmıştır. Rolling devkitPro paketleri için sürüm manifest'i korunmalıdır.

Stable v1.0 için fiziksel matrisin tamamlanması, yeni bulunan P0/P1'lerin giderilmesi
ve lisans/karşılık gelen kaynak erişiminin dağıtım boyunca korunması gereklidir.
RC dosyaları yalnızca **HARDWARE: UNVERIFIED** etiketiyle değerlendirilmelidir.
