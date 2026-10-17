# Lisanslar ve kaynak

- PaperNX: [AGPL-3.0](../LICENSE).
- MuPDF 1.23.11: dağıtım arşivindeki `COPYING` ve üçüncü taraf bildirimleri; kaynak
  [Artifex deposu](https://github.com/ArtifexSoftware/mupdf/tree/1.23.11).
- Noto Sans/Serif: [OFL](../fonts/OFL.txt); aynı dosya SD font dizininde ve RomFS'tedir.
- libnx 4.12.0: [upstream izin bildirimi](https://github.com/switchbrew/libnx/blob/v4.12.0/LICENSE.md).
- SDL2 2.28.5: [zlib bildirimi](https://github.com/libsdl-org/SDL/blob/release-2.28.5/LICENSE.txt).
- HarfBuzz 10.0.1: [COPYING](https://github.com/harfbuzz/harfbuzz/blob/10.0.1/COPYING).
- FreeType: portlibs içindeki FTL/GPLv2 seçenek bildirimleri pakete kopyalanır.
- MuPDF'nin JPEG, OpenJPEG, JBIG2, zlib, LCMS, Gumbo, MuJS, Extract ve diğer kaynak
  bildirimleri seçilmiş kaynak dağıtımından toplanır. SDL2_ttf ve diğer portlibs'in
  kurulu lisans dosyaları da dahil edilir.

`collect_licenses.py` kaynak ve SDK bildirimlerini toplar; sabit sürüm ek bildirimleri
`scripts/license-notices/` içindedir. ZIP, bunları `switch/PaperNX/licenses/` altında taşır.
`SOURCE.txt` upstream kaynak adreslerini içerir. Derlemede kullanılan paket sürümleri
`verification/switch-packages.txt` ile kaydedilir.

Genel dağıtımda AGPL'nin karşılık gelen kaynak, değişiklikler ve derleme tariflerine
ilişkin koşulları uygulanmalıdır; yalnızca bir lisans dosyası eklemek bu yükümlülükleri
tek başına yerine getirmez. Bu oturumda GitHub stable release yayınlanmamıştır.
Kaynak deposu ve kullanılan MuPDF arşivine erişim sürüm paketiyle birlikte korunmalıdır.
