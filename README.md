# PaperNX

### Kitaplığın Switch’inde, okuma keyfin yanında.

**PaperNX**, Nintendo Switch için geliştirilen, Türkçe arayüzlü ve çevrimdışı bir **EPUB/PDF okuyucu**. Kitaplarını microSD kartına ekle, kitaplığından seç ve okumaya başla. Yazı boyutunu, sayfa düzenini ve temayı kendine göre ayarla; yer imleriyle sevdiğin bölümlere dön, kaldığın yerden devam et.

Kindle benzeri sade bir okuma deneyimini hedefleyen PaperNX, dokunmatik ekran ve Joy-Con/Pro Controller kontrolleri için tasarlanır. Okumak için hesap veya internet bağlantısı gerekmez.

> **v0.1.0-rc1 · Önizleme**
>
> Gerçek ARM64 derlemesi ve NRO paketi üretildi; çekirdek ve masaüstü entegrasyon testleri geçti. Fiziksel Nintendo Switch testleri henüz yapılmadı: **HARDWARE: UNVERIFIED**. Bu sürüm stable v1.0 değildir. [Doğrulama durumu](docs/STATUS.md) · [Test sonuçları](docs/TEST_RESULTS.md)

## PaperNX ile neler yapabilirsin?

| Özellik | Okuma deneyimi |
|---|---|
| **EPUB ve PDF** | Kitaplarını aynı uygulamadan aç. EPUB’da metni yeniden düzenle; PDF’de özgün sayfa görünümünü koru. |
| **Kişisel okuma görünümü** | EPUB yazı boyutunu, Serif/Sans seçimini, satır aralığını, kenar boşluklarını ve hizalamayı değiştir. |
| **Altı tema** | Açık kâğıt tonlarından koyu ve OLED Black görünüme geç. |
| **Kapaklı kitaplık** | Izgara veya liste görünümünde gez; favorilerini ve son okuduklarını bul. |
| **Türkçe arama** | Kitap adı, yazar veya dosya adıyla ara; Türkçe karakterleri kullan. |
| **Kaldığın yerden devam** | Okuma ilerlemeni ve kitap ayarlarını sakla; birden fazla yer imi oluştur. |
| **İçindekiler** | Belgenin sunduğu bölüm listesinden istediğin bölüme geç. |
| **PDF yakınlaştırma** | Sayfayı ekrana sığdır, 1–5× yakınlaştır ve sayfa üzerinde gezin. |
| **SD kart gezgini** | Ana kitap klasörünün dışındaki EPUB/PDF dosyalarını da seçerek aç. |

EPUB içindeki resim ve çizimler görüntülenir. Kapak bilgisi varsa gerçek kapak çıkarılır; kapak bulunamazsa ilk sayfa veya yedek görünüm kullanılır.

## Kurulum ve ilk kitap

PaperNX, **homebrew çalıştırabilen bir Nintendo Switch ortamı** gerektirir.

1. Önizleme ZIP paketini kullanıyorsan içeriğini microSD kartının köküne çıkar. Kaynaktan derliyorsan oluşturulan `PaperNX.nro` dosyasını aşağıdaki konuma kopyala.
2. `.epub` ve `.pdf` kitaplarını `switch/PaperNX/books/` klasörüne koy. Alt klasörler de taranır.
3. Homebrew Menu üzerinden **PaperNX** uygulamasını aç.
4. Yön tuşlarıyla bir kitap seçip **A**’ya bas veya dokunmatik ekranda kapağına dokun.
5. Kitabı açtıktan sonra **+ (Plus)** ile okuma görünümünü ayarla.

```text
microSD/
└── switch/
    └── PaperNX/
        ├── PaperNX.nro
        ├── books/
        │   ├── Roman.epub
        │   └── Dergi.pdf
        ├── data/
        └── fonts/
            ├── NotoSans-Regular.ttf
            ├── NotoSerif-Regular.ttf
            └── OFL.txt
```

`data/` okuma kayıtlarını tutar; güncelleme yaparken bu klasörü koru. Gerekli klasörler açılışta oluşturulur. Noto fontları NRO’nun RomFS bölümüne de eklenir; dağıtım paketi font dosyalarını ve lisanslarını ayrıca içerir.

Derleme çıktıları yerel `dist/` klasörüne yazılır. Henüz yayımlanmış bir binary indirme bağlantısı yoktur; kaynaktan üretmek için [Switch derleme rehberini](docs/BUILD_SWITCH.md) kullanabilirsin.

## Okuma görünümünü kişiselleştir

**Aradığın menü: kitap açıkken + (Plus) → Aa.** Dokunmatik kullanımda ekranın ortasına dokun, ardından üst sağdaki **Aa** düğmesine bas.

EPUB için Aa menüsünde dört sekme bulunur: **Tipografi · Düzen · Temalar · Kontroller**.

| Aa menüsünde | Kontrol |
|---|---|
| Sekme değiştir | **← / →** |
| Ayarlanacak alanı seç | **Y** |
| Seçili değeri değiştir | **↑ / ↓** |
| Menüyü kapat, okumaya dön | **B** veya **A** |

Menünün altındaki **Alan** bilgisi, hangi ayarı değiştirdiğini gösterir. Dokunmatik ekranda sekmelere ve ayar düğmelerine doğrudan dokunabilirsin.

### Arka plan ve yazı rengini değiştirme

1. Kitap açıkken **+** tuşuna bas.
2. **← / →** ile **Temalar** sekmesine geç.
3. **↑ / ↓** ile istediğin temayı seç veya renk örneğine dokun.
4. **B** ile okumaya dön.

| Tema | Görünüm |
|---|---|
| **Paper** | Açık kâğıt tonu |
| **Ivory** | Sıcak fildişi tonu |
| **Sepia** | Sepya okuma görünümü |
| **Slate** | Serin gri arka plan, koyu yazı |
| **Dark** | Koyu arka plan, açık yazı |
| **OLED Black** | Siyah arka plan, açık yazı |

**Arka plan ve yazı rengi tema olarak birlikte değişir.** Ayrı ayrı serbest renk seçimi mevcut değildir. Karanlık bir görünüm istiyorsan **Dark** veya **OLED Black** seç.

### Yazı boyutu ve font seçimi

1. **+ → Tipografi** sekmesini aç.
2. **Boyut** alanında **↑** ile yazıyı büyüt, **↓** ile küçült.
3. **Y** ile **Yazı tipi** alanına geç.
4. **↑ / ↓** ile **Serif** ve **Sans** arasında geçiş yap.

Serif, kitap hissi veren tırnaklı yazı tipidir; Sans daha sade, tırnaksız bir görünüm sunar. Dokunmatik kullanımda boyut için **− / +**, yazı tipi için **Serif / Sans** düğmesini kullan.

### Satır aralığı, kenar boşlukları ve hizalama

**+ → Düzen** sekmesinde **Y** ile sırasıyla **Satır aralığı**, **Kenar boşluğu** ve **Hizalama** alanları arasında geç. Seçtiğin alanı **↑ / ↓** ile değiştir.

Dokunmatik ekranda sunulan değerleri ve **İki Yana / Sola** hizalama seçeneklerini doğrudan seçebilirsin. EPUB yeni düzene göre yeniden sayfalanır; aynı bölümde önceki konumuna yaklaşık olarak dönülür. Toplam sayfa sayısının değişmesi normaldir.

### PDF görünümü

PDF’de **+**, ayrı bir **PDF Görünüm** menüsü açar:

- **← / →:** yakınlaştırmayı azalt/artır.
- **↑ / ↓:** arayüz temasını değiştir.
- **Y:** sayfayı ekrana sığdır.
- **B / A:** menüyü kapat.

Okuma sırasında **ZL / ZR** ile de küçültüp büyütebilirsin. Yakınlaştırılmış sayfada **sol analog çubuk**, yön tuşları veya **dokunarak sürükleme** ile gezin. **Sağ analog çubuğa basmak** sayfayı ekrana sığdırır. Sayfa değişince yakınlaştırma korunur, kaydırma konumu başlangıca döner.

**PDF’nin kendi fontu, yazı boyutu ve sayfa renkleri değiştirilmez.** EPUB ayarları PDF içeriğini yeniden biçimlendirmez. İki parmakla yakınlaştırma SDL çoklu dokunma olayı sağlandığında işlenir; Switch üzerinde cihaz doğrulaması bekler.

## Kitaplığını kullan

- **Izgara / liste:** kütüphanede **− (Minus)** tuşuna bas veya üstteki görünüm düğmesine dokun.
- **Kategoriler:** **L / R** ile tüm kitaplar, favoriler, okumaya devam, son okunanlar, okunmamış ve biten kitaplar arasında geç. Ekrandaki kategoriye de dokunabilirsin.
- **Favoriler:** kitabı seçip **sol analog çubuğa bas**; yeniden basarak favorilerden çıkar.
- **Arama:** **X**’e bas; kitap adı, yazar veya dosya adı gir. Switch için yerel yazılım klavyesi entegredir. `İ ı I i Ş ş Ğ ğ Ü ü Ö ö Ç ç` gibi Türkçe karakterler desteklenir.
- **SD gezgini:** **Y** ile dosya gezginini aç. Klasör veya kitap seçip **A**’ya bas; **B** ile üst klasöre dön.

Başlık ve yazar bilgileri kitap açıldığında belge metadatasından güncellenir. SD gezgininden açılan kitaplar da yol ve metadata bilgileriyle kitaplık kaydına alınır.

## Kontroller

Aşağıdaki tablolar uygulamadaki Switch düğme eşlemesini anlatır. Fiziksel Joy-Con/Pro Controller ve dokunmatik davranışlarının cihaz üzerindeki testleri henüz tamamlanmamıştır.

### Kütüphane

| Tuş | İşlev |
|---|---|
| **Yön tuşları / sol analog** | Kitap seç |
| **A** | Seçili kitabı aç |
| **X** | Ara |
| **Y** | SD kart gezginini aç |
| **− (Minus)** | Izgara / liste görünümü |
| **L / R** | Önceki / sonraki kategori |
| **Sol analog çubuğa bas** | Favori ekle / kaldır |
| **B** | Uygulamadan çık |

### Okuyucu

| Tuş | İşlev |
|---|---|
| **L / R** | Önceki / sonraki sayfa |
| **← / → veya sol analog** | Sayfa değiştir; yakınlaştırılmış PDF’de kaydır |
| **A** | Üst ve alt araç çubuklarını göster / gizle |
| **+ (Plus)** | Aa / PDF görünüm ayarları |
| **X** | İçindekiler |
| **Y** | Geçerli konuma yer imi ekle / kaldır |
| **− (Minus)** | Yer imleri listesi |
| **ZL / ZR** | PDF’de küçült / büyüt |
| **Sağ analog çubuğa bas** | PDF’yi sayfaya sığdır |
| **B** | Kitaplığa dön |

İçindekiler ve yer imleri listelerinde **↑ / ↓** ile seç, **A** ile git, **B** ile kapat. Menü kapatmak tek başına okuma konumunu değiştirmez.

### Dokunmatik kullanım

| Hareket | İşlev |
|---|---|
| Kitap kapağına veya liste satırına dokun | Kitabı aç |
| Okuma ekranının sol / sağ kenarına dokun | Önceki / sonraki sayfa |
| Ekranın ortasına dokun | Araç çubuğunu göster / gizle |
| Üst çubuktaki **Aa** | Görünüm ayarlarını aç |
| Üst çubuktaki **İçindekiler / Yer İmi** | İlgili listeyi aç |
| Alt çubuktaki ilerleme çizgisine dokun | Kitapta farklı bir konuma git |
| Yakınlaştırılmış PDF’yi sürükle | Sayfayı kaydır |

Araç çubuğu açıkken orta alana dokunmak çubuğu kapatır. Sayfa çevirmek için kenar dokunuşunu çubuk kapalıyken kullan.

### Masaüstü klavye kısayolları

| Tuş | İşlev |
|---|---|
| **Enter / Esc** | A / B karşılığı |
| **Yön tuşları** | Seçim, sayfa, menü ayarları veya PDF kaydırma |
| **PageUp / PageDown** | Önceki / sonraki sayfa; kütüphanede kategori |
| **X / Y** | İlgili ekrandaki X / Y işlevi |
| **F** | Kütüphanede favori |
| **−** | Kütüphanede ızgara / liste; PDF’de küçültme |
| **+** | EPUB’da Aa menüsü; PDF okurken yakınlaştırma |
| **B harfi** | Okuyucuda yer imleri listesi |
| **0** | PDF’yi sayfaya sığdır |
| **Backspace** | Aramada son karakteri sil |

Buradaki **B harfi**, Switch’teki geri düğmesiyle aynı işlevde değildir; masaüstünde geri/kapat için **Esc** kullan. Klavyeden `+` üretmek düzenine göre **Shift** gerektirebilir. PDF görünüm menüsüne fare/dokunmatik ile üstteki **Aa** düğmesinden de ulaşabilirsin.

## Kayıtlar ve bilinen sınırlar

Okuma ilerlemesi, yer imleri ve kitap görünüm ayarları `data/` altında saklanır. Değişiklikler aralıklarla kaydedilir; yazma başarısız olursa uygulama hata bildirir ve yeniden dener. Kayıtlar checksum ile denetlenir, kesintiye uğramış yazmalarda yedeklerden kurtarma denenir. Güncelleme öncesi `data/` klasörünün yedeğini almak kayıtlarını korumana yardımcı olur.

- EPUB konumu bölüm ve bölüm içindeki oranla saklanır. Font veya sayfa düzeni değişince aynı metne dönüş yaklaşık olabilir.
- Kitap dosyasını taşımak veya yeniden adlandırmak mevcut ilerleme kaydıyla eşleşmesini değiştirebilir.
- DRM ve parola korumalı belgeler desteklenmez. EPUB içindeki script, video ve etkileşimli içerikler çalıştırılmaz.
- Uygulamadaki arama **kitaplık aramasıdır**; kitap içi tam metin araması veya OCR mevcut değildir.
- Uyku/uyanma, handheld/docked, yazılım klavyesi ve fiziksel kontrolcü davranışları için [cihaz test matrisi](docs/DEVICE_TEST_PLAN.md) henüz tamamlanmamıştır.

Diğer sınırlar: [bilinen sorunlar](docs/KNOWN_ISSUES.md).

## Kısaca teknoloji

PaperNX, **C++17** ile yazılmış yerel bir uygulamadır. **SDL2 / SDL2_ttf** arayüzü ve yazıları, **MuPDF 1.23.11 C API** EPUB/PDF içeriğini işler. Nintendo Switch hedefinde **devkitA64 ve libnx** kullanılır. Tarayıcı, WebView veya HTTP sunucusu çalıştırmaz; kitap okuma tamamen cihaz üzerinde gerçekleşir.

Teknik ayrıntılar: [mimari](docs/ARCHITECTURE.md) · [Switch derlemesi](docs/BUILD_SWITCH.md) · [sürüm raporu](docs/RELEASE_REPORT.md).

## Geliştirme ve doğrulama

Yerel Docker ortamı çekirdek, SDL2/MuPDF masaüstü ve gerçek Switch çapraz derlemeleri için kullanılır:

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

`desktop` çekirdek/paketleme testlerini ve tam masaüstü uygulamasını derler, üç CTest testini çalıştırır. Ek betikler sabitlenmiş MuPDF 1.23.11 ve ASan/UBSan doğrulamasını çalıştırır. `switch` ARM64 MuPDF ile uygulamayı derler; ELF, NRO, NACP, RomFS ve paket bütünlüğünü kontrol eder. Loglar `verification/`, paketler `dist/` altında kalır.

Docker dışında çekirdek testleri:

```bash
cmake -S . -B build-core -G Ninja -DBUILD_TESTING=ON
cmake --build build-core --parallel
ctest --test-dir build-core --output-on-failure
python3 tests/test_package.py
```

Masaüstü derlemesini başlatmak için:

```bash
./build-desktop/PaperNX ./PaperNX-data
```

PR’larda ilgili kaynak değişiklikleri için çekirdek/paketleme ve masaüstü testleri çalışır. Switch işi aynı depodaki PR’a `switch-build` etiketi eklenmesiyle veya manuel `workflow_dispatch` ve `confirm=BUILD` ile istenir. Gerçekleştirilmiş testlerin sonuçları ve kapsamı [TEST_RESULTS](docs/TEST_RESULTS.md) dosyasındadır. Main’e birleştirme, fiziksel cihaz doğrulaması veya stable sürüm onayı yerine geçmez.

## Lisans

PaperNX ve MuPDF **AGPL-3.0**, Noto fontları **SIL Open Font License** kapsamındadır. Paket kaynak/lisans yükümlülükleri ve bağımlılık bildirimleri için [LICENSE](LICENSE) ve [lisans rehberine](docs/LICENSES.md) bak.
