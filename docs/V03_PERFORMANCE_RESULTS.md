# v0.3.0-rc1 Performans ve Bellek Verileri

> [!NOTE]
> Aşağıdaki ölçümler Linux masaüstü Docker ortamında SDL dummy video sürücüsü kullanılarak CPU duvar saati üzerinden alınmıştır. Bu değerler **masaüstü referans verileridir**; gerçek Nintendo Switch donanımındaki dokunmatik giriş gecikmesi, VSync ve kare süreleri **UNVERIFIED** (doğrulanmamış) olarak kalmaktadır.

---

## 1. UI ve Sayfa Render Başarım Verileri

| İşlem | Masaüstü Ölçümü (p50 / p95) | Açıklama |
|---|---|---|
| **Sağ Üst `+` Hızlı Menü Açılışı** | ~0.10 ms / 29.56 ms | Tam ekran saydam katman ve 10 adet yuvarlatılmış eylem öğesi çizimi |
| **Ayarlar Sekme Değişimi** | ~0.15 ms / 35.88 ms | Canlı Türkçe metin önizlemesi ve 6 kontrol satırının çizimi |
| **Soğuk Sayfa Render** | ~4.20 ms / 6.33 ms | Önbellekte olmayan EPUB sayfasının MuPDF ile sıfırdan rasterize edilmesi |
| **Önbellekten Sayfa Çizimi** | ~0.38 ms / 1.30 ms | 64 MiB LRU doku önbelleğinden anında GPU doku transferi |

---

## 2. Geniş Belge ve Stres Testi Verileri

`tests/generate_stress_fixtures.py` ve `papernx_app_tests` tarafından üretilen sentetik stres senaryoları:

- **24 Bölümlü Geniş EPUB:**
  - Açılış süresi: 32.64 ms
- **Resimli EPUB (8 adet 2048×1536 görsel):**
  - Açılış süresi: 4.24 ms
- **320 Sayfalık Geniş PDF:**
  - Belge açılışı: 2.59 ms
  - 150 rastgele sayfa örnekleme toplam render süresi: 151.88 ms
- **Asenkron Düzen İptali (In-flight Layout Cancellation):**
  - 3 ardışık hızlı düzen isteğinde (font/kenar boşluğu/aralık) ilk 2 eski nesil (`generation`) iptal edildi; yalnızca son geçerli düzen işlendi (Kabul: 1/3).
  - Toplam toplu süre: 341.71 ms
- **Doku ve Bellek Bütçesi:**
  - Zirve sayfa doku kullanımı: ~63.78 MiB (64 MiB tavan sınırına uyuldu).
  - Süreç zirve bellek kullanımı (RSS High Water Mark): 147.304 KiB (~143.8 MiB).
- **Yayıncı Küçük Font Geçersiz Kılma:**
  - Yayıncının sabit 6px inline fontu (64px genişlik), okuyucu denetimli modda seçilen gövde fontuyla 284px genişliğe ölçeklendi.

---

## 3. Donanım Kaynak Sınırları

- **Sayfa Dokusu Önbelleği:** 64 MiB (LRU tahliyesi).
- **Metin Dokusu Önbelleği (Typography):** 8 MiB / 256 giriş.
- **Kapak Önbelleği:** 32 MiB disk alanı, maksimum 512 kayıt, 150×210 RGB24 (her biri ~94,5 KB).
- **Asenkron İş Parçacığı Sayısı:** 1 adet MuPDF düzen işçisi + 1 adet kapak işçisi (kuyrukta en fazla 32 iş).
