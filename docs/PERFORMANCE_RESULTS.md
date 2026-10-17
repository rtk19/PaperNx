# Performans ve bellek kanıtlarının sınırı

Önceki sürümdeki milisaniye/RAM tabloları bu oturumda ölçülemediği için kaldırıldı.
Switch FPS, pil tüketimi, açılış süresi ve gerçek tepe RAM: **UNVERIFIED**.

Çekirdek testinde 1.000 dosya tarama/arama doğruluğu çalıştırılır. CTest süreleri test
hazırlığını ve birçok senaryoyu birlikte içerir; bunlar tek bir kitap açma benchmark'ı değildir.
Güncel süreler `verification/*.log` ve [TEST_RESULTS](TEST_RESULTS.md) içindedir.

Kaynakta uygulanan sınırlar:

- Sayfa texture cache: 64 MiB; thumbnail texture cache: 32 MiB.
- Texture hesabı RGB24 için olası backend genişlemesini dikkate alarak en az 4 byte/piksel sayar.
  Bu bir cache bütçesidir, GPU sürücüsünün gerçek tahsisi ölçülmemiştir.
- Thumbnail bekleyen/işlenen/yüklemeye hazır toplam iş sayısı en fazla 32.
- Başarısız thumbnail retry kaydı en fazla 512; yeniden deneme aralığı 30 saniye.
- Her MuPDF context'inin callback üzerinden geçen tahsisleri en fazla 96 MiB;
  MuPDF store sınırı 32 MiB. Bunlar bütün uygulama için RAM garantisi değildir.
- Kapak sıkıştırılmış veri sınırı 8 MiB; XML metadata sınırı 2 MiB; kapak boyutu en fazla
  16 milyon piksel. Büyük/bozuk varlıklar reddedilir veya ilk sayfa/placeholder'a düşer.
- Render boyutları en fazla 4096×4096; TOC en fazla 256 giriş; kütüphane en fazla 10.000 kitap.

ASan/UBSan/leak detection koşularında bildirilen hata olup olmadığı test raporunda yazılıdır.
Sonlu bir test kümesi bütün belgeler veya fiziksel cihaz için sıfır sızıntı kanıtı oluşturmaz.
