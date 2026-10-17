# Mimari

PaperNX çevrimdışı C++17 uygulamasıdır. `main.cpp` SDL olaylarını ve ekran durumlarını yönetir;
`core.cpp` kitap tarama/arama ve kayıtları; `gestures.cpp` dokunma ve çubuk tekrarlarını;
`typography.cpp` SDL_ttf metinlerini; `page_cache.cpp` texture sahipliğini yönetir.
`mupdf_bridge.c` C11 sınırı MuPDF fz_try/fz_catch temizleme yollarını içerir.

## Belge ve konum

Her açık belge kendi sınırlı allocator'ına ve MuPDF context'ine sahiptir. Thumbnail worker
ayrı belge/context açar. SDL texture oluşturma ve silme ana thread üzerinde kalır.
Kapatmada worker join edilir, belge/font/cache bırakılır, ardından renderer/window kapanır.

EPUB3 nav ve cover-image/EPUB2 cover metadata ZIP/XML API ile okunur; NCX native
outline fallback olarak kalır. URI'ler MuPDF ile çözülür. CSS değişiminde belge yeniden
oluşturulur, TOC ve render cache geçersizleştirilir. Tüm EPUB yüzeyi tema arka planıyla temizlenir;
PDF görüntüleri EPUB CSS/renk dönüşümüne sokulmaz.

MuPDF 1.23.11 EPUB bookmark'ları HTML nesnelerine bağlıdır; ham değerleri süreçler arasında
saklamak güvenli değildir. Bridge bunun yerine sürüm işaretli bölüm/oran konumu saklar.
Bu düzen değişiminde yaklaşık konumdur; CFI veya karakter çapası değildir. Eski ham
bookmark'lar reddedilir; sayfa sayısı ve toplam ilerleme oranı fallback olarak kullanılır.

## Kalıcılık

CHECKSUM başlığı FNV-1a payload bütünlüğünü doğrular; güvenlik/imza mekanizması değildir.
PAPERNX-3, PAPERNX-2 alanlarına kitabın yol/başlık/yazar bilgisini ekler. V1/V2 okunur.
Bozuk tekil satırlar atlanır; dosya checksum kontrolü zayıflatılmaz. `.bak`, `.bak.old`
ve `.tmp` sırayla kurtarma adaylarıdır. Temp flush/fsync ve byte doğrulamasından sonra
geçerli primary yedeklenir. Başarısız yazmada dirty korunur ve 3 saniye sonra denenir.

Belge kimliği yol ve boyuta bağlıdır; yeniden adlandırma/taşıma otomatik eşleşmez.
Tema, düzen ve birden fazla yer imi kayıtla döner; dosya taramasıyla bulunmayan ama
kaydı geçerli olan dış klasör kitapları yeniden kütüphaneye alınır.

## Derleme

CMake taşınabilir core ve masaüstü hedeflerini; Makefile devkitA64/libnx NRO hedefini kurar.
MuPDF 1.23.11 kaynak SHA256 değeri sabittir. SDL_ttf ve MuPDF ortak sistem HarfBuzz/FreeType
kullanır; uyumsuz iki statik font engine kopyası bağlanmaz. Docker tarifleri, doğrulama
betikleri ve PR testleri aynı kaynakları kullanır. Hardware matrisi ayrı tutulur.
