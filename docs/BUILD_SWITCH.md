# Switch derleme

Fiziksel Switch uyumluluğu UNVERIFIED; bu yönerge gerçek ARM64/NRO derlemesi içindir.

```bash
docker compose build switch
docker compose run --rm switch
```

İmaj devkitPro devkitA64 taban digest'ine sabitlenmiştir. `dkp-pacman` ile devkitA64/libnx
ve Switch portlibs kurulumu/güncellemesi yapılır. Gerçek paket sürümleri
`verification/switch-packages.txt` dosyasına yazılır; paket deposunun gelecekte aynı
sürümleri vermesi garanti edilmez. MuPDF arşivinin SHA256 değeri sabittir.

Docker içindeki işlem:

```bash
export PATH="$DEVKITPRO/portlibs/switch/bin:$PATH"
bash scripts/build-mupdf-switch.sh /work/prefix-mupdf-switch
export MUPDF_PREFIX=/work/prefix-mupdf-switch
make all -j4
python3 scripts/verify_artifacts.py
python3 scripts/package_release.py PaperNX.nro --destination dist --version v0.1.0-rc1
```

C++17, `-fno-exceptions`, `-fno-rtti` ve C11 kullanılır. ELF C++ linker ile bağlanır.
HarfBuzz ve FreeType, SDL2_ttf ile ortak portlibs sürümünden alınır. MuPDF'nin diğer
üçüncü taraf bağımlılıkları `libmupdf-third.a` içindedir. SDL2/SDL2_ttf, HarfBuzz,
FreeType, bzip2, PNG, JPEG, zlib, libm ve libnx linker grubuna dahildir.

MuPDF kaynağının kendi başlıkları önce seçilir. Sistem font bağımlılıklarının include
klasörleri ayrı verilir; eski portlibs MuPDF başlıkları derlemeye karışmaz. Arşivlerin
**tüm üyeleri** AArch64 kontrolünden geçer. Önbellek compiler/flag/uyumluluk ve font
arşiv hash'leriyle doğrulanır; arşiv SHA256 değerleri tekrar kontrol edilir.

NACP sürümü `0.1.0-rc1`; varsayılan libnx ikonu kullanılır. `elf2nro`, `romfs/`
klasörünü dahil eder. Doğrulayıcı ELF machine=183, NRO0 segment sınırları, ASET/NACP,
RomFS dizinleri ve gömülü fontların kaynakla eşitliğini denetler.
Bu kontroller konsolda başlatma veya fiziksel bellek kullanımını kanıtlamaz.

`dist/` gerçek NRO'yu, SD ZIP'ini, SHA256 dosyalarını ve manifest'i içerir.
Genel Docker prune komutları kullanılmaz.
