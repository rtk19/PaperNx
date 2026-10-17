# PaperNX v0.3.0-rc1 — Bilinen Durumlar ve Açık Sınırlar

## 1. P1-HW: Fiziksel Nintendo Switch Doğrulaması

- Joy-Con'suz (çıkarılmış) kullanım, libnx yazılım klavyesi, donanım çoklu dokunma, uyku/uyanma (`sleep/resume`), Homebrew Menu'ye temiz çıkış, docked/handheld geçişi ve gerçek microSD okuma/yazma gecikmeleri **UNVERIFIED** (doğrulanmamış) durumdadır.
- Masaüstü SDL simülasyonları ve Docker testleri bu donanım doğrulamalarının yerine geçemez.
- İlgili prosedür [DEVICE_TEST_PLAN.md](DEVICE_TEST_PLAN.md) içerisinde tanımlanmıştır.

---

## 2. P2: Konum Hassasiyeti ve Geometrik Fallback

- Mantıksal EPUB konumları bölüm ve karakter/offset bazlıdır (`px_document_anchor`).
- İç etiket veya karakter bilgisi bulunmayan sıra dışı/bozuk XHTML bölümlerinde, geometrik bölüm yüksekliği oranına (1/8191 hassasiyet) geri düşülür.
- Çok uzun (on binlerce piksel) tek parça bölümlerde geometrik geri düşüş durumunda 1-2 piksel sapma görülebilir.

---

## 3. P2: Bütün Sayfa PDF Renk Filtreleri

- PDF Koyu, Sepya ve Ters renk modları tüm sayfayı raster piksel düzeyinde filtreler.
- Sayfa içindeki fotoğraflar ve renkli illüstrasyonlar da bu filtrelerden etkilenir. Vektör/metin ve görsel ayrıştırmalı PDF reflow bu sürümün kapsamında değildir.

---

## 4. Kapsam Dışı Bırakılan (Gelecek Yol Haritası) Özellikler

Aşağıdaki özellikler v0.3.0-rc1 kapsamında **kasıtlı olarak yer almamaktadır** ve [PRODUCT_ROADMAP_2026.md](PRODUCT_ROADMAP_2026.md) içerisinde planlanan sürümlere devredilmiştir:
- Kitap içi tam metin araması (v0.5)
- Metin seçimi, notlar ve altını çizme (v0.5)
- Çevrimdışı sözlük (v0.5)
- Okuma istatistikleri ve tahmini bitirme süresi (ETA) (v0.4)
- PDF OCR ve reflow (v0.7)
- Bulut / Calibre kablosuz senkronizasyonu (v0.6+)
- Sesli okuma (TTS) (Araştırma)

Arayüzde bu özellikler için tıklanabilir sahte/ölü butonlar yer almaz.
