# v0.2.0-rc1 doğrulama sonuçları — 17 Ekim 2026

Başlangıç main: `338fa80aab0cbeb5f27d3d04dce2c9efacd3c566`.
Dal: `feat/touch-reader-settings-v2`. Uygulama kaynağı: `69bb7ad2bff3fccf8ee9abf3838c4e51511c0511`.
Kaynak uygulama SHA256: `3ee3ceeec1652a144ca634a4ef8fed39c6bd0e06d39e64962eb7804b76e9d5de`.
Sonraki dokümantasyon commitleri uygulama baytlarını değiştirmez.

| Kontrol | Sonuç | Kanıt |
|---|---|---|
| Portable C++17 çekirdek | PASS — 1/1 | verification/desktop.log |
| Tam native SDL/MuPDF uygulaması | PASS — build ve 3/3 CTest | verification/desktop.log |
| MuPDF 1.23.11 kaynak hash'i + native entegrasyon | PASS — 3/3 | verification/pinned-desktop.log |
| ASan/UBSan + leak detection | PASS — 3/3 | verification/sanitizers.log |
| Release çekirdek, assertions açık | PASS — 1/1 | verification/core-release.log |
| Paket/SD yükseltme regresyonları | PASS — kişisel kitap/kayıt/günlük korunur | tests/test_package.py, verification/desktop.log |
| devkitA64/libnx ile gerçek ELF/NRO | PASS — AArch64 ELF64 DYN, NRO0 segmentleri | verification/switch.log |
| ASET/NACP/RomFS/fontlar | PASS — ad PaperNX, yazar ysfbz, sürüm 0.2.0-rc1 | verification/artifacts.json |
| Kullanıcı ikonu | PASS — 256×256 RGB baseline JPEG; gömülü 13.795 bayt kaynakla aynı | verification/artifacts.json |
| ZIP CRC, SHA256, font/lisanslar, temiz kullanıcı dizinleri | PASS | verification/package-integrity.json |
| Fiziksel Switch kabulü | UNVERIFIED — 20 senaryo | docs/DEVICE_TEST_PLAN.md |
| GitHub sosyal önizleme ayarı | UNVERIFIED — manuel yükleme gerekir | README.md |

Kayıt regresyonları normal/çoklu kitap, V1/2/3 geçişi, V4 bağımsız renkler ve varsayılanlar; gerçek yazılamayan `/sys` yolu, bozuk dizin, write/fflush/fclose/fsync/verify/backup/remove/rename hata enjeksiyonu, Switch'in üzerine yazmayan rename davranışı ve yalnız temp dosyasından kurtarmayı kapsar. Desteklenmeyen fsync başarı sayılmaz. Tekrar denemede dirty state korunur.

Dokunmatik uygulama testi 10 sayfa, sol/sağ tap/swipe, 900 ms yavaş kaydırma, 550 ms uzun dokunma, küçük parmak kayması, iki parmak iptali, finger/mouse çiftlenmesi ve mouse-only fallback; Aa/sekme/ayar/yer imi, kategori mesajı, iki kitapta yeniden başlatma, PDF fit-width/pan/filter ve suspend olaylarını kapsar. Sanal SDL kumandalarıyla ayırma/yeniden bağlama da test edilir. Yazılım klavyesinin fiziksel Switch uygulaması UNVERIFIED.

EPUB gerçek render testleri altı eski ve altı yeni CSS presetini, ayrı metin/zemin renklerini, Serif/Sans görüntü farkını, Türkçe metni, kapak/resim ve EPUB3 TOC'yi doğrular. Mantıksal konum relayout sonrası korunur; hatalı relayout argümanları geçerli belgeyi bozmaz. PDF viewportu maksimum zoom/aşırı pan altında 640×480 kalır; Orijinal, Koyu, Sepya ve iki kez invert geri dönüşü doğrulanır.

Çalıştırılan Docker komutları (PowerShell'de docker.exe kullanıcı kurulumundaki tam yoluyla çağrıldı):

```powershell
& "$env:LOCALAPPDATA/Programs/DockerDesktop/resources/bin/docker.exe" version
& "$env:LOCALAPPDATA/Programs/DockerDesktop/resources/bin/docker.exe" info --format '{{.ServerVersion}}'
& "$env:LOCALAPPDATA/Programs/DockerDesktop/resources/bin/docker.exe" compose version
& "$env:LOCALAPPDATA/Programs/DockerDesktop/resources/bin/docker.exe" run --rm -v "${PWD}:/work" papernx-desktop bash scripts/verify-desktop.sh
& "$env:LOCALAPPDATA/Programs/DockerDesktop/resources/bin/docker.exe" run --rm -v "${PWD}:/work" papernx-desktop bash scripts/verify-pinned-desktop.sh
& "$env:LOCALAPPDATA/Programs/DockerDesktop/resources/bin/docker.exe" run --rm -v "${PWD}:/work" papernx-desktop bash scripts/verify-final.sh
& "$env:LOCALAPPDATA/Programs/DockerDesktop/resources/bin/docker.exe" run --rm -v "${PWD}:/work" papernx-switch bash scripts/verify-switch.sh
& "$env:LOCALAPPDATA/Programs/DockerDesktop/resources/bin/docker.exe" run --rm -v "${PWD}:/work" papernx-switch bash -lc 'python3 scripts/verify_artifacts.py && python3 scripts/package_release.py PaperNX.nro --destination dist --version v0.2.0-rc1'
& "$env:LOCALAPPDATA/Programs/DockerDesktop/resources/bin/docker.exe" run --rm -v "${PWD}:/work" papernx-desktop bash -lc 'SDL_VIDEODRIVER=dummy build-pinned/papernx_app_tests > verification/performance.log 2>&1'
```

Mevcut `papernx-desktop`/`papernx-switch` imajları kullanıldı; bu oturumda `docker compose build` çalıştırılmadı. Toolchain/imaj kimlikleri verification altında kayıtlıdır. Switch: devkitA64 GCC 16.1.0, libnx 4.12.0, SDL2 2.28.5-4. Desktop: Ubuntu 24.04, GCC 13.2, SDL2 2.30, sistem MuPDF 1.23.10; ayrıca kaynak MuPDF 1.23.11 ayrı derlenip test edildi. ASan/UBSan uygulama/bridge kodunu kapsar; sistem MuPDF arşivinin tamamının sanitizer ile yeniden derlendiği iddia edilmez.

Ölçüm: Docker Linux amd64, SDL dummy, MuPDF 1.23.11, 3.551 bayt sentetik EPUB, 600×600 viewport, altı preset. Relayout 36.94–40.80 ms, render 2.64–4.01 ms. Bunlar Switch performansı veya büyük kitap ölçümü değildir. Donanım giriş gecikmesi ve gerçek RAM tepe ölçümü UNVERIFIED. Dokunmatik senaryo süreleri kasıtlı beklemeleri ve screenshot kaydını içerir.

Doğrulamada yakalanıp giderilenler: varsayılan kayıt anahtarının 16 karakter sınırı, Windows satır sonunun shell scriptini bozması, test modül import yolu, özel sayfa renklerinin menü kontrastına sızması, eksik eksi glifi ve PDF yardım metninin sayfa üstüne gelmesi. Bir Docker/Windows bind-mount JSON yazımı geçici errno 22 verdi; bu deneme verification/switch-retry-error.log içinde korunur. Aynı doğrulama ve tam Switch scripti sonraki çalıştırmada PASS. Hiçbir assertion kapatılmadı, test atlanmadı.

Çıktı: `dist/PaperNX-switch-v0.2.0-rc1.zip` (32,633,618 bayt), `dist/PaperNX.nro` (47,074,707 bayt).

ZIP SHA256: `52ba7d5bd4e272b5b562c705a3ec7bdb2a5289385c71c072865f4f1a9d03eca4`.
NRO SHA256: `7d86f0c80c492693c81c7890b27c88be8da0c29c1bcb58a5c373ab7ea809ae81`.

Yayın kararı: yerel mühendislik kapıları PASS; cihaz kabulü ve stable yayın hazırlığı UNVERIFIED. PR, donanım sonuçları beklenerek draft tutulur; main'e otomatik merge yapılmaz. Açık sınırlamalar [READER_V2](READER_V2.md) ve [KNOWN_ISSUES](KNOWN_ISSUES.md) içinde.
