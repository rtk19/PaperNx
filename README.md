# PaperNX — Nintendo Switch için yerel EPUB/PDF okuyucu

C++17, SDL2/SDL2_ttf, libnx ve MuPDF 1.23.11 C API kullanan çevrimdışı okuyucu.
EPUB/PDF dosyaları microSD üzerinden açılır. Tarayıcı, WebView veya HTTP sunucusu kullanılmaz.

**Sürüm: v0.1.0-rc1 önizleme. Fiziksel Nintendo Switch: UNVERIFIED.**
Gerçek ARM64/NRO derlemesi ile konsolda çalıştırma ayrı doğrulamalardır.
Güncel kanıtlar: [STATUS](docs/STATUS.md), [test sonuçları](docs/TEST_RESULTS.md),
[sürüm raporu](docs/RELEASE_REPORT.md).

## Özellikler

- EPUB metin/resim çizimi; EPUB3 nav/NCX içindekiler ve metadata ile kapak çıkarma.
- Altı tema; serif/sans, yazı boyutu, satır aralığı, kenar boşluğu ve hizalama.
- PDF sayfaya sığdırma; 1–5× yakınlaştırma ve gerçek sayfa boyutlarına göre sınırlandırılmış kaydırma.
- Izgara/liste, favoriler, okumaya devam, son okunanlar, okunmamış ve biten kitaplar.
- Türkçe UTF-8 başlık/yazar/dosya adı araması; Switch üzerinde libnx yazılım klavyesi.
- SD dosya gezgini ve dış klasörlerden açılan kitapların kalıcı kütüphane kaydı.
- Çoklu yer imleri; checksum korumalı PAPERNX-3 kayıtları, PAPERNX-1/2 okuma desteği.
- Sınırlı thumbnail kuyruğu, ana iş parçacığında SDL texture oluşturma, bellek bütçeli LRU.

Kalıcı EPUB konumu bölüm ve bölüm içindeki sayfa oranıyla saklanır. Düzen değiştiğinde
aynı bölüme yaklaşık dönüş sağlar; karakter/CFI düzeyinde kesin konum garantisi vermez.
Dosyayı yeniden adlandırmak veya taşımak belge kimliğini değiştirir.

## SD yerleşimi

```text
switch/PaperNX/
├── PaperNX.nro
├── books/
├── data/
└── fonts/
    ├── NotoSans-Regular.ttf
    ├── NotoSerif-Regular.ttf
    └── OFL.txt
```

Paket ayrıca README, AGPL lisansı ve bağımlılık lisans bildirimlerini içerir.

## Tekrarlanabilir yerel Docker doğrulaması

```bash
docker version
docker info
docker compose version
docker compose build desktop switch
docker compose run --rm desktop
docker compose run --rm desktop bash scripts/verify-pinned-desktop.sh
docker compose run --rm desktop bash scripts/verify-sanitizers.sh
docker compose run --rm switch
```

`desktop` çekirdek/paketleme testlerini ve SDL2/MuPDF uygulamasını derler, üç CTest testini çalıştırır.
`verify-pinned-desktop.sh` SHA256 ile doğrulanmış MuPDF 1.23.11 kaynağını derleyerek aynı testleri çalıştırır.
`switch` devkitA64/libnx/portlibs ile gerçek NRO oluşturur; ELF/NRO/NACP/RomFS ve font içeriklerini denetler.
Çıktılar bind mount üzerinde `verification/`, `build-*`, `prefix-*` ve `dist/` altında kalır.
Mevcut Docker kaynakları için genel temizleme yapılmaz.

MuPDF, SDL2_ttf ile aynı sistem HarfBuzz/FreeType kitaplıklarını kullanır; diğer gerekli
MuPDF üçüncü taraf modülleri statik arşive derlenir. PDF/EPUB modülleri kapatılmaz.

Docker dışındaki çekirdek testi:

```bash
cmake -S . -B build-core -G Ninja -DBUILD_TESTING=ON
cmake --build build-core --parallel
ctest --test-dir build-core --output-on-failure
python3 tests/test_package.py
```

Masaüstü uygulaması: `./build-desktop/PaperNX ./PaperNX-data`.
Switch için ayrıntılar: [BUILD_SWITCH](docs/BUILD_SWITCH.md).

## Kontroller

Aşağıdakiler SDL mantıksal düğme eşlemeleridir; fiziksel Joy-Con/Pro Controller eşlemesi cihazda doğrulanmalıdır.

| Ekran | Kontrol | İşlev |
|---|---|---|
| Kütüphane | A / Enter | Kitabı aç |
| Kütüphane | X / X | Ara; masaüstünde Esc aramayı kapatır |
| Kütüphane | Y / Y | SD gezgini |
| Kütüphane | Minus / klavye - | Izgara/liste |
| Kütüphane | L/R veya PageUp/PageDown | Kategori |
| Kütüphane | Sol çubuk tık / F | Favori |
| Okuyucu | A / Enter / orta dokunuş | Araç çubuğu |
| Okuyucu | L/R / PageUp/PageDown | Önceki/sonraki sayfa |
| Okuyucu | D-pad/sol çubuk | Sayfa; yakınlaştırılmış PDF'te kaydırma |
| Okuyucu | X / X | İçindekiler |
| Okuyucu | Y / Y | Yer imi ekle/kaldır |
| Okuyucu | Minus / B klavye | Yer imleri listesi |
| Okuyucu | Plus / klavye + | Aa/PDF görünüm ayarları |
| PDF | ZL/ZR / klavye - veya = | Küçült/büyüt |
| PDF | Sağ çubuk tık / 0 | Sayfaya sığdır |
| Aa | Sol/sağ; Y; yukarı/aşağı | Sekme; alan; değer |
| PDF ayarları | Sol/sağ; yukarı/aşağı; Y | Büyütme; tema; sığdır |
| Modal/okuyucu | B / Esc | Kapat/kütüphaneye dön |
| Kütüphane | B / Esc | Çıkış |

Dokunma: sol/sağ kenar sayfa, orta araç çubuğu; PDF yakınlaştırılmışken sürükleme kaydırır.
SDL çoklu dokunma olayı sağlarsa pinch işlenir; Switch'teki çoklu dokunma desteği UNVERIFIED.
PDF sayfa değişiminde büyütme korunur, kaydırma başlangıca sıfırlanır.

## CI ve lisans

PR kaynak değişikliklerinde hafif çekirdek/paketleme ve masaüstü testleri çalışır.
Pahalı Switch işi aynı depodaki PR'a `switch-build` etiketi eklenmesiyle istenir.
Default branch'e alındıktan sonra `workflow_dispatch` + `confirm=BUILD` kullanılabilir.
Yerel doğrulama önceliklidir; sonuç alınmadan merge veya stable yayın yapılmaz.

PaperNX ve MuPDF AGPL-3.0 kapsamındadır. Noto fontları OFL kapsamındadır.
Kaynak ve bağımlılık dağıtım yükümlülükleri için [LICENSES](docs/LICENSES.md) dosyasına bakın.
