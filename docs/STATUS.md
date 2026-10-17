# PaperNX v0.3.0-rc1

Dal: `fix/v03-final-verification`. Başlangıç main: `03bbf513d311566245ea3efe6a7a5e85ecbeb98b`.

- **Yerel ve Docker Mühendislik Doğrulaması:**
  - Bağımsız C++17 taşınabilir çekirdek testleri: **PASS** (3/3).
  - Pinned MuPDF 1.23.11 tam native masaüstü testleri: **PASS** (6/6).
  - Tam kaynak AddressSanitizer (ASan) + UndefinedBehaviorSanitizer (UBSan) + LeakSanitizer (LSan): **PASS** (6/6).
  - Native Release optimizasyonlu build ve CTest: **PASS** (6/6).
  - Taşınabilir Release build ve CTest: **PASS** (3/3).
  - Gerçek Nintendo Switch devkitA64 cross-compilation (AArch64 ELF64, NRO0, NACP v0.3.0-rc1, 256x256 gömülü ikon, RomFS fontları): **PASS**.
  - Sürüm paketleme ve SD bütünlük doğrulaması (`dist/v0.3.0-rc1/`): **PASS**.

- **Fiziksel Switch Kabulü:**
  - **UNVERIFIED**. Cihazsız geliştirme ortamında gerçek donanım doğrulaması üretilemez; [26 maddelik donanım test planı](DEVICE_TEST_PLAN.md) kullanıcı tarafından fiziki konsolda yürütülmelidir.

- **Dağıtım Paketi:**
  - Yerel: `dist/v0.3.0-rc1/PaperNX-switch-v0.3.0-rc1.zip`. Kullanıcı `books/` ve `data/` klasörleri korunur.
  - Sürüm durumu: Fiziksel donanım kabulü tamamlanana kadar **CANDIDATE / UNVERIFIED**.
