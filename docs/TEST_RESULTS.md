# PaperNX — gerçek test sonuçları

17 Ekim 2026; Windows üzerinde yerel Docker Desktop/Engine. Docker 29.8.2,
Compose v5.6.0, linux/amd64 container'lar. Windows PATH'te Docker bulunmadı;
istemci kullanıcı kurulumundaki `DockerDesktop/resources/bin/docker.exe` yolundan
bulundu ve credential helper için PATH düzeltildi. Mevcut test-nginx container'ı korunmuştur.

Uygulama/build revizyonu: `5c9cd44fe6d27457b8096a0452ea8d75974ed1f1`.
Sonuçlar terminalde gerçekten çalıştırılmıştır; fiziksel konsol sonucu değildir.

## Çalıştırılan komutlar

```bash
docker version
docker info
docker compose version
docker compose build desktop
docker compose build switch
docker compose run --rm desktop bash scripts/verify-final.sh
docker compose run --rm switch
python scripts/verify_distribution.py
```

`verify-final.sh` içinde sırayla gerçekten yürütülen kapılar:

```bash
cmake -S . -B build-core -G Ninja -DBUILD_TESTING=ON
cmake --build build-core --parallel 4
ctest --test-dir build-core --output-on-failure
python3 tests/test_package.py
python3 tests/generate_fixtures.py
cmake -S . -B build-desktop -G Ninja -DPAPERNX_NATIVE=ON -DBUILD_TESTING=ON
cmake --build build-desktop --parallel 4
ctest --test-dir build-desktop --output-on-failure
bash scripts/verify-pinned-desktop.sh
bash scripts/verify-sanitizers.sh
cmake -S . -B build-core-release -G Ninja -DBUILD_TESTING=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build-core-release --parallel 4
ctest --test-dir build-core-release --output-on-failure
```

Pinned script MuPDF 1.23.11 SHA256 kontrolü, statik kaynak derlemesi ve `build-pinned`
içinde tam uygulama + üç CTest çalıştırır. Sanitizer script C/C++ ve linker'da
`-fsanitize=address,undefined -fno-omit-frame-pointer`, çalıştırmada
`ASAN_OPTIONS=detect_leaks=1` kullanır. Test hedeflerinde `-UNDEBUG` uygulanır;
Release core testindeki NDEBUG kontrolü de derlenip çalışmıştır.

| Test | Son koşu | Toplam süre |
|---|---|---|
| Portable core | PASS 1/1 | 0,13 s |
| Native distro MuPDF | PASS 3/3 | 1,82 s |
| Kaynaktan MuPDF 1.23.11 | PASS 3/3 | 1,86 s |
| ASan/UBSan/leak detection | PASS 3/3 | 2,70 s |
| Release core | PASS 1/1 | 0,12 s |
| Python paket testleri | PASS | ayrı benchmark ölçülmedi |
| Gerçek dağıtım kontrolü | PASS | ayrı benchmark ölçülmedi |

Distro MuPDF paketi 1.23.10'dur; pkg-config metadata'sı 1.19.0 bildirir. Bu yüzden
hedef 1.23.11 ayrıca SHA256 sabitli kaynaktan derlenmiştir. Paket sürümleri
`verification/desktop-packages.txt` içindedir. Bu süreler Docker masaüstü test süreleridir;
Switch performans ölçümü değildir.

## Kapsam ve doğrudan render kanıtı

- Core: bozuk checksum, truncation, geçerli checksum altında bozuk/karışık satırlar;
  integer overflow/underflow, geçersiz float, NaN/inf; `.bak`, `.bak.old`, `.tmp`,
  kesilmiş save akışı, yazma hatası, boş/escape edilmiş Türkçe yer imi başlıkları;
  V1/V2 okuma ve V3 metadata round-trip; 1.000 kitap tarama/arama.
- EPUB3: gerçek ZIP/XML/PNG fixture, 20 başlangıç sayfası, iki bölüm, metadata;
  font relayout, serif/sans/satır/kenar/hizalama kontrolleri, altı ayrı render hash'i
  ve altı tema arka planının gerçek RGB kontrolü; 96×144 kapak ve bilinen renk pikseli.
- PDF: iki sayfa, resim XObject'i, farklı sayfa render hash'leri, iki TOC girişi,
  1,5× viewport çizimi; klavye/trigger/çubuk/touch drag/synthetic pinch ve sığdır kontrolleri.
- Uygulama: SDL_TEXTINPUT ile Türkçe giriş, tam UTF-8 backspace, kategori/favori,
  modal aç/kapat ve TOC seçimi; gerçek dosya gezgininden kitap açma; yeniden başlatmada
  kütüphane/metadata/tema/font/yer imleri; başarısız periyodik save'de dirty korunması.
- Kaynak/worker: font-family cache anahtarı, LRU replacement ve oversized texture reddi,
  tamamlanan işin pending'den çıkması ve eviction sonrası thumbnail yeniden yükleme;
  tekrar open/close ve 1024 byte MuPDF bütçesinde güvenli context açma hatası.
- SDL sanal aygıtları: yanlış instance removal'ın birinci controller'ı kapatmaması,
  ikinciye geçiş ve açık Switch ham düğme eşlemesi. Fiziksel donanım testi değildir.

Doğrudan `build-pinned/mupdf_render_tests` çıktısı: `verification/native-render.log`.
EPUB page0 hash `3cc1554f83ce714b`; relayout `42192d6f3d7abef8`.
PDF page0 `9bc7b06382cca0d4`, page1 `569dd902cfa9449b`, zoom `35a4a4bcae8ab652`.
İkinci EPUB TOC girişi, büyütülmüş düzende sayfa 30'a çözülmüştür.

## Gerçek ARM64 ve paket

`aarch64-none-elf-gcc` 16.1.0/devkitA64 r30, libnx 4.12.0, SDL2 2.28.5,
SDL2_ttf 2.22.0, HarfBuzz 10.0.1. MuPDF 1.23.11 kaynak SHA256:
`478f2a167feae2a291c8b8bc5205f2ce2f09f09b574a6eb0525bfad95a3cfe66`.

İki MuPDF statik arşivinin tüm üyeleri AArch64 olarak denetlendi. C++17/C11 uygulama,
`-fno-exceptions -fno-rtti`, libnx ve SDL2/TTF ile gerçekten linklendi; `elf2nro` çalıştı.
ELF64 little-endian AArch64 DYN çıktı. Gerçek `PaperNX.nro`: 47.042.549 byte.
NRO0 segmentleri, ASET/NACP 0.1.0-rc1, RomFS dizinleri ve gömülü font byte'ları doğrulandı.
ZIP CRC, 68 üye, doğru SD konumları, 59 lisans bildirimi ve SHA256 eşleşmeleri PASS.

Kanıtlar: `verification/switch.log`, `artifacts.json`, `package-integrity.json`,
`package-manifest.json`, `switch-packages.txt`, `mupdf-build-fingerprint.txt`.
Sentetik NRO yalnızca paket birim testinde kullanılır; gerçek dağıtım çıktısı değildir.

## İlk başarısız denemeler ve sınırlar

İlk core koşusu analog ilk hareket gecikmesinde FAIL oldu ve kod düzeltildi. Native
linkte olmayan MuPDF API'leri/HarfBuzz eksikliği; EPUB3 nav eksikliği; farklı iki
HarfBuzz/FreeType kopyasının pinned build'de çakışması; Switch'te yanlış include sırası,
`timegm` ve POSIX `fileno` görünürlüğü gerçek compiler/test loglarıyla giderildi.
Yeni restart testi eklenirken PowerShell stdin dönüşümü Türkçe literal'i bozdu;
UTF-8 dosya içeriği düzeltilip test tekrar PASS aldı. Yukarıdaki tablo son koşulardır.

ASan sonuçları yalnızca bu testlere aittir; distro/harici statik MuPDF'nin bütün kodu
sanitizer ile yeniden derlenmiş değildir. ThreadSanitizer/Valgrind ve fiziksel Switch
çalıştırılmadı. Donanım performansı, uyku/dock/applet, gerçek swkbd ve fiziksel pinch:
**UNVERIFIED**. GitHub CI sonuçları yerel testlerin yerine yazılmayacaktır.

## GitHub Actions ek kanıtı

Push sonrası otomatik PR koşusu `39035561321` core ve tam native Docker build/test
adımlarını başarıyla yürüttü. Workflow yalnızca `actions/upload-artifact` adımında
`Artifact storage quota has been hit` hatasıyla FAIL oldu; bu uygulama/test hatası değildir.
Mevcut GitHub artifact'leri silinmedi. PR koşularında job logu ve step summary korunur;
yükleme `workflow_dispatch` için `upload_artifacts=true` açık seçeneğine taşındı.
Hiçbir başarısız test skip edilmedi. Düzeltilen workflow'un
[39035754051 numaralı koşusu](https://github.com/ysfbz/PaperNx/actions/runs/39035754051)
başarıyla tamamlandı: portable ve desktop PASS; isteğe bağlı Switch işi tetiklenmedi.
Gerçek ARM64/NRO kanıtı yukarıdaki yerel Docker doğrulamasıdır.
