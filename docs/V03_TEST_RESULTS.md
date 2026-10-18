# v0.3.0-rc1 Doğrulama ve Test Raporu — 18 Ekim 2026

- **Başlangıç main:** `03bbf513d311566245ea3efe6a7a5e85ecbeb98b`
- **Çalışma dalı:** `fix/v03-final-verification`
- **Sürüm:** `0.3.0-rc1`

---

## 1. Test Matrisi ve Doğrulama Özeti

| Test Konfigürasyonu | Derleme Türü | Kapsam | Sonuç | Süre |
|---|---|---|---|---|
| **Pinned Native Desktop** | Debug/Default | 6 suite: localization, cover repo, core, mupdf bridge, stress fixtures, app workflow | **PASS** (6/6) | 21.69 sn |
| **Full Sanitizer (ASan/UBSan/LSan)** | Debug (`-O1 -g -fsanitize=address,undefined`) | 6 suite: bellek taşması, tanımsız davranış ve sızıntı denetimi | **PASS** (6/6) | 28.66 sn |
| **Native Release** | Release (`-O3 -DNDEBUG`, assertions aktif) | 6 suite: optimize tam native masaüstü çalıştırması | **PASS** (6/6) | 21.17 sn |
| **Portable Release** | Release (`-DPAPERNX_NATIVE=OFF`) | 3 suite: localization, covers, core | **PASS** (3/3) | 0.43 sn |
| **Nintendo Switch Cross-Build** | devkitA64 (`-O2 -march=armv8-a -fno-exceptions -fno-rtti`) | AArch64 ELF64, NRO0 binary, NACP, RomFS, Branding | **PASS** | Derleme & link başarılı |
| **Paket ve Dağıtım Bütünlüğü** | Python 3 doğrulayıcılar | ZIP CRC/SHA256, NRO hash, NACP sürüm, fontlar, lisanslar | **PASS** | Bütünlük onaylandı |
| **Fiziksel Switch Kabulü** | Donanım (Joy-Con'suz, dokunmatik, uyku/uyanma) | 26 senaryo | **UNVERIFIED** | Cihazda kullanıcı testi bekleniyor |

---

## 2. Test Paketleri ve Ayrıntıları

1. `papernx_localization_tests` (0.01 sn):
   - Türkçe/İngilizce UI metin çevirileri, eksik anahtar fallback davranışı, kitap başlığı ve içeriklerinin çeviriden muaf tutulması.
2. `papernx_cover_tests` (0.03 sn):
   - Disk üzeri kalıcı kapak önbelleği (maksimum 150×210 RGB24, 32 MiB/512 dosya sınırı).
   - Dosya boyutu/mtime invalidation, checksum koruması, bozulma tespiti, `sdmc:/` yolu desteği, sembolik link reddi, komşu dosyaların korunması.
3. `papernx_core_tests` (0.38 sn):
   - Format tespiti (EPUB/PDF), benzersiz belge kimliği oluşturma.
   - PAPERNX-1..5 legacy kayıtlarının PAPERNX-6'ya güvenli yükseltilmesi; yükseltme öncesi değişmez `.pre-v6-vN.bak` yedeği.
   - Hata enjeksiyonu altında atomik yazma ve kurtarma.
   - Genel okuma tercihlerinin ve kitap başına geçersiz kılmaların (`styleOverride`, `modeOverride`) bağımsız çözülmesi.
   - Uygulama ayarlarının (`AppSettingsStore`) bağımsız dosya üzerinde ayrık saklanması.
4. `mupdf_render_tests` (0.24 sn):
   - Bozuk EPUB ve PDF'lerin güvenli reddi.
   - EPUB reflow, 6 tema paleti, font boyutları, Serif/Sans font ayrımı.
   - PDF fit-page, fit-width, zoom (%100..%500) ve sınırlı dokunmatik pan.
   - Bütün sayfa renk filtreleri (Orijinal, Koyu, Sepya, Ters).
5. `v03_stress_fixtures` (0.51 sn):
   - 24 bölümlü geniş EPUB üretimi.
   - 8 adet 2048×1536 görsel içeren resimli EPUB.
   - 320 sayfalık geniş PDF belgesi.
6. `papernx_app_tests` (20.48 sn):
   - Kumandasız (Joy-Con'suz) %100 dokunmatik iş akışları.
   - Sağ üst `+` hızlı işlem menüsü ve alt menü yönlendirmeleri.
   - Açık kitap olmadan kütüphaneden genel okuma ve uygulama ayarlarına erişim.
   - Canlı Türkçe önizleme alanı (tipografi, kenar boşlukları, paragraf aralıkları ve girinti).
   - Gerçek dikey sürekli EPUB kaydırma, hassasiyet ve momentum, bölüm geçişleri, mantıksal konum (`anchor`) koruma.
   - Asenkron EPUB düzeni hazırlama, eski isteklerin iptali (`obsolete generations`) ve son isteğin kabulü.
   - 640×480, 1280×720 ve 1920×1080 ekran boyutu uyarlamaları.
   - Dokunmatik güvenli çıkış.

---

## 3. Nintendo Switch Çıktı Doğrulaması

- **ELF Dosyası:** `PaperNX.elf`
  - Sınıf: ELF64, Little Endian, DYN (PIE)
  - Mimari: AArch64 (Machine 183)
  - GCC: devkitA64 (14.2.0) / libnx 4.12.0
- **NRO Dosyası:** `PaperNX.nro`
  - Başlık: `NRO0`
  - Bildirilen Boyut: 45.977.600 bayt
  - Gerçek Boyut: 47.197.587 bayt
  - `.text` Segmenti: 8.708.096 bayt
  - `.rodata` Segmenti: 36.737.024 bayt
  - `.data` Segmenti: 532.480 bayt
  - NACP Başlığı: Uygulama adı `PaperNX`, Yazar `ysfbz`, Sürüm `0.3.0-rc1`
  - Gömülü İkon: 256×256 baseline JPEG (`assets/branding/PaperNX-icon-switch.jpg` ile bayt bayt özdeş)
  - RomFS İçeriği: `fonts/NotoSans-Regular.ttf`, `fonts/NotoSerif-Regular.ttf`, `fonts/OFL.txt`
  - NRO SHA256: `faa2984a2227f9ffae607252ab4d38e723b04a11472e2a8ee7edf35bb0b34d60`

---

## 4. Dağıtım Arşivi

- **Konum:** `dist/v0.3.0-rc1/PaperNX-switch-v0.3.0-rc1.zip`
- **Boyut:** ~32,6 MiB
- **Bütünlük:** ZIP CRC ve testzip doğrulaması hatasız.
- **Kullanıcı Verisi Güvenliği:** Arşiv kullanıcıya ait `books/` veya `data/` dosyası barındırmaz ve bunların üzerine yazmaz.
- **Lisans Bildirimleri:** 55'ten fazla açık kaynak lisans bildirimi (libnx, SDL2, MuPDF ve bağımlılıklar) `licenses/` altında eksiksiz yer alır.
