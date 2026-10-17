# Fiziksel Switch doğrulama matrisi

Tüm satırlar **UNVERIFIED**. Konsol bu oturuma bağlı değildir. NRO'nun varlığı veya
başlık doğrulaması bu satırları PASS yapmaz. Sonuç, tarih, HOS/libnx/hbmenu sürümü,
cihaz modeli, açılan kitabın SHA256 değeri ve log dosyasıyla kaydedilmelidir.

| Senaryo | Kabul ölçütü | Sonuç |
|---|---|---|
| Kurulum/başlatma | SD ZIP doğru dizine açılır; ikon/NACP, RomFS ve Türkçe UI görünür | UNVERIFIED |
| EPUB3 | Yerel kitap, resim, kapak, nav/NCX ve bütün bölümler açılır | UNVERIFIED |
| EPUB düzeni | Boyut, serif/sans, satır, kenar ve hizalama değişir; konum aynı bölümde kalır | UNVERIFIED |
| Altı tema | Metin ve sayfa arka planı değişir; resimler bozulmaz | UNVERIFIED |
| PDF | Çok sayfalı vektör/tarama PDF, sığdırma, 1–5× zoom ve sınırlı pan | UNVERIFIED |
| Touch | Tap/swipe ile PDF drag ayrılır; çift mouse olayı oluşmaz | UNVERIFIED |
| Pinch | SDL çoklu dokunma olayının konsolda oluştuğu ve zoom yaptığı görülür | UNVERIFIED |
| Joy-Con/Pro | A/B/X/Y, Plus/Minus, L/R, ZL/ZR, çubuk ve yönler tablodaki işlemleri yapar | UNVERIFIED |
| Bağlantı | İkinci kumandayı ayırmak birinciyi kapatmaz; yeniden bağlantı çalışır | UNVERIFIED |
| Arama | İ ı I i Ş ş Ğ ğ Ü ü Ö ö Ç ç yazılım klavyesiyle girilir ve aranır | UNVERIFIED |
| Yeniden başlatma | Kitap kimliği, sayfa, düzen, tema, metadata ve yer imleri geri gelir | UNVERIFIED |
| Uyku/uyanma | Konum kaydedilir; çizim, kontrol ve worker güvenli devam eder | UNVERIFIED |
| Handheld/docked | Yerleşim ve viewport çözünürlük değişiminde güncellenir | UNVERIFIED |
| Application/applet | Gerçek RAM ve büyük kitap davranışı her iki kipte ölçülür | UNVERIFIED |
| Çıkış | B/Esc kütüphaneden hbmenu'ye güvenli döner | UNVERIFIED |
| Yazma hatası | Yazılamayan SD'de uyarı/yeniden deneme; geçerli yedek korunur | UNVERIFIED |

Ani güç kesme testi yalnızca yedeklenmiş test SD'siyle yapılmalıdır. Simüle edilmiş
interrupted-save testi gerçek fiziksel storage arızalarının tümünü kapsamaz.
Kararlı v1.0 onayı bu matris tamamlanmadan verilmez.
