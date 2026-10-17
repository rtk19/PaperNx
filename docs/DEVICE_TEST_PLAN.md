# Fiziksel Switch kabul planı — v0.2.0-rc1

Bu oturumda Switch bağlı değildir. Önceki cihaz raporundaki sorunlar gereksinim olarak alınmıştır; yeni NRO için bütün fiziksel sonuçlar **UNVERIFIED**. Docker PASS bu tabloyu değiştirmez.

Önce microSD `switch/PaperNX/data/` klasörünü bilgisayara yedekleyin. Yeni ZIP’i klasörleri birleştirerek kurun. Bir resimli EPUB ve iki sayfadan uzun PDF hazırlayın. Cihaz modeli, HOS/hbmenu sürümü, applet/application modu, kitap ve NRO SHA256 değerlerini sonuçla birlikte kaydedin.

| # | Test | Kabul ölçütü | Sonuç |
|---|---|---|---|
| 1 | Custom icon | Homebrew Menu’de kullanıcı ikonu görünür. | UNVERIFIED |
| 2 | Başlatma | PaperNX çökmeden açılır. | UNVERIFIED |
| 3 | Kitaplık | Sekmeler, kapak oranları ve uzun başlıklar okunur. | UNVERIFIED |
| 4 | EPUB ve resimler | Resimli EPUB3 açılır; TOC ve resimler görünür. | UNVERIFIED |
| 5 | Dokunmatik sayfa | Joy-Con kullanmadan 10 sayfa ileri ve geri gidilir. | UNVERIFIED |
| 6 | Yavaş kaydırma | 1–2 saniyelik yatay kaydırma tek sayfa çevirir; hafif kayma dokunmayı bozmaz. | UNVERIFIED |
| 7 | Aa erişimi | Ortaya dokun → Aa; menü açılır ve X ile kapanır. | UNVERIFIED |
| 8 | Tipografi | Boyut, Serif/Sans/Yayıncı, satır, paragraf, kenar, hizalama, girinti çalışır. | UNVERIFIED |
| 9 | Tema/renk | Altı tema ve bağımsız yazı/zemin HEX gerçek EPUB sayfasını değiştirir. | UNVERIFIED |
| 10 | Ayarları yeniden açma | Kitap ayarları ve yeni kitap varsayılanları yeniden başlatmada korunur. | UNVERIFIED |
| 11 | PDF görünüm | Sayfaya/genişliğe sığdır ve %100 çalışır. | UNVERIFIED |
| 12 | PDF yakınlaştır/kaydır | −/+ ve tek parmak sürükleme ile bütün içerik erişilir; yanlış sayfa çevrilmez. | UNVERIFIED |
| 13 | PDF gece filtresi | Filtre açılır, fotoğrafa etkisi görülür, Orijinal ile geri alınır. | UNVERIFIED |
| 14 | Kategoriler/arama | Altı kategori, favori dokunması, liste/ızgara ve Türkçe yazılım klavyesi çalışır; boş Bitenler doğru mesajı verir. | UNVERIFIED |
| 15 | Kayıt güvenilirliği | 10 sayfa ve ayar değişikliğinden sonra Kitaplık’a dönülür; tekrarlanan eski uyarı yoktur. | UNVERIFIED |
| 16 | Konumu geri alma | Çıkış/başlatma sonrası EPUB ve PDF, iki farklı kitapta doğru konuma döner; yer imleri korunur. | UNVERIFIED |
| 17 | Kumandalar | Joy-Con/Pro Controller, takma/çıkarma ve dokunmatiğe geçiş çalışır. | UNVERIFIED |
| 18 | Uyku/uyanma | Açık kitapla uyku/uyanma; kayıt ve giriş çiftlenmez. | UNVERIFIED |
| 19 | Handheld/docked | 1280×720 ve docked çıktıda araçlar metni kapatmaz; PDF sınırları doğrudur. | UNVERIFIED |
| 20 | Çıkış | Kitaplıkta B ile Homebrew Menu’ye dönülür; yeniden açılabilir. | UNVERIFIED |

Kısa kayıt testi: EPUB’da 10 sayfa ilerle → Aa ile Sans ve Sepia seç → yer imi ekle → Kitaplık → en az 3 saniye bekle → çık → tekrar açıp aynı kitabı seç. Konum/tema/font/yer imi aynı olmalı. PDF’de ve ikinci kitapta tekrarla. Sonra uygulama kapalıyken bilgisayarda `progress.txt` dosyasını başka yere taşı ve `.bak` yerinde bırak; yeniden açıp yedekten kitap kayıtlarının geri geldiğini kontrol et. Silmek yerine dosyayı taşı ve ilk yedeği sakla. Bu son işlem son kayıttan bir önceki duruma dönmeyi bekler.

Hata sürerse Aa → Kontroller → Dokunmatik günlük aç; birkaç dokunma yapıp kapat. `logs/papernx.log` içindeki save operation/errno/platform_result ve touch satırlarını paylaş. SD çıkarma veya güç kesme deneyini açık uygulamada yapmayın. Yazılamayan yol testleri Docker’da hata enjeksiyonuyla yapılır.

Pinch için ek sonuç: UNVERIFIED. SDL çoklu dokunma olaylarının gerçek cihazda oluşup oluşmadığını kaydet; −/+ her durumda kullanılabilmelidir.
