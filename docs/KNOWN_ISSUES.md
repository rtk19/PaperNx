# Açık doğrulamalar ve sınırlamalar

## P1-HW: Fiziksel Switch doğrulaması eksik

Joy-Con/Pro Controller düğme eşlemesi ve yeniden bağlantı, libnx yazılım klavyesi,
çoklu dokunma, uyku/uyanma, hbmenu çıkışı, handheld/docked geçişi ve application/applet
RAM davranışı **UNVERIFIED**. Masaüstü sanal SDL kontrol testleri bu doğrulamaların
yerine geçmez. Kaynak: `src/main.cpp` olay döngüsü, `toggleSearch`, `handleEvent`;
[cihaz matrisi](DEVICE_TEST_PLAN.md). Stable v1.0 için engeldir.

## P2: Konum hassasiyeti ve dosya taşıma

Kalıcı EPUB konumu bölüm/oran yaklaşımıdır; düzen değişiminde yakın metne döner,
CFI/karakter kesinliği sağlamaz. Eski ham MuPDF pointer bookmark'ları kullanılmaz.
`src/mupdf_bridge.c`: `px_document_make_bookmark`, `px_document_lookup_bookmark`.

Belge kimliği yol/boyuttan türetilir. Taşınan/yeniden adlandırılan dosyaya otomatik
ilerleme eşleştirme yapılmaz (`src/core.cpp: documentId`). Aynı yol ve boyutla değişen
farklı içerik için de otomatik kimlik ayrımı yoktur.

## Belgeler, sınırlar ve dağıtım

DRM ve parola korumalı dosyalar desteklenmez. EPUB script/video/interaktivite çalıştırılmaz.
TOC 256 girişle, scanner 10.000 kitapla sınırlıdır. Kaynakta bellek sınırları vardır;
tüm büyük/bozuk belge türleri veya bütün allocation hata noktaları denenmemiştir.
Taranmış PDF'de metin araması veya OCR uygulama özelliği değildir.

Docker taban imajları ve MuPDF kaynak hash'i sabittir; devkitPro paket kurulumu rolling
depoya bağlıdır. Kullanılan sürümler loglanır; gelecekte byte düzeyinde aynı toolchain
kurulumu garanti edilmez. Genel yayımdan önce lisans/kaynak erişimi korunmalı ve cihaz
matrisi tamamlanmalıdır. [TEST_RESULTS](TEST_RESULTS.md) son yürütme kanıtıdır.
