# PaperNX v0.3.0-rc1 Sürüm Teslim Raporu

- **Başlangıç main SHA:** `03bbf513d311566245ea3efe6a7a5e85ecbeb98b`
- **Geliştirme dalı:** `fix/v03-final-verification`
- **Hedef Sürüm:** `v0.3.0-rc1`

---

## 1. Teslim Edilen Başlıca Özellikler

1. **Tamamen Dokunmatik Odaklı Arayüz:**
   - Kitaplık, okuyucu ve dosya tarayıcıda sağ üstte her zaman erişilebilir `+` hızlı işlem menüsü.
   - Joy-Con veya kumanda gerektirmeden tüm temel işlevlere erişim.
   - 56 px minimum dokunmatik hedef boyutu, dokunma anında görsel basılma (`pressed`) geri bildirimi.
2. **Kapsamlı ve Geniş Okuma Ayarları:**
   - 6 sekmeli ayar paneli: Okuma, Sayfa düzeni, Görünüm, Navigasyon, PDF, Uygulama.
   - Canlı Türkçe metin önizleme alanı (font, boyut, satır yüksekliği, kenar boşluğu, hizalama, paragraf aralığı ve girinti).
   - Genel okuma ayarları ve kitap başına bağımsız geçersiz kılma (`styleOverride`, `modeOverride`). Önceden okunmuş kitapların genel temayı devralması.
3. **Gerçek Dikey Sürekli EPUB Kaydırma:**
   - Sahte sayfa kaydırma yerine dikey piksel içerik kaydırma.
   - Ayarlanabilir kaydırma hassasiyeti ve momentum.
   - Bölüm sınırlarında pürüzsüz geçiş ve mantıksal konum (`anchor`) koruma.
4. **Kalıcı ve Sınırlı Kapak Önbelleği:**
   - Disk üzerinde 32 MiB / 512 dosya ile sınırlandırılmış 150×210 RGB kapak önbelleği.
   - Dosya boyutu ve değiştirilme tarihi ile geçersiz kılma, bozulma tespiti ve kurtarma.
   - Yükleme öncesinde anında görünen şık vektörel yer tutucu kapaklar.
5. **Türkçe ve İngilizce Arayüz Dil Desteği:**
   - Uygulama ayarları altından anında dil seçimi.
   - Kitap içeriği ve başlıklarının özgünlüğünü koruyan, yalnızca arayüz etiketlerini çeviren lokalizasyon.

---

## 2. Mühendislik ve Doğrulama Durumu

- **PASS:**
  - Tam C++17 taşınabilir çekirdek testleri (3/3).
  - Pinned MuPDF 1.23.11 tam masaüstü testleri (6/6).
  - Tam kaynak ASan/UBSan/LeakSanitizer denetimi (6/6).
  - Native Release optimizasyonlu build ve CTest (6/6).
  - devkitA64 çapraz derleme ile gerçek AArch64 ELF64 ve NRO0 çıktısı.
  - Sürüm paketleme ve SD kart bütünlük kontrolü.
- **UNVERIFIED:**
  - Fiziksel Nintendo Switch üzerinde gerçek donanım kabulü.

---

## 3. Dağıtım Dosyaları

- **Dağıtım Dizini:** `dist/v0.3.0-rc1/`
  - `PaperNX.nro`
  - `PaperNX.nro.sha256`
  - `PaperNX-switch-v0.3.0-rc1.zip`
  - `PaperNX-switch-v0.3.0-rc1.zip.sha256`
  - `manifest.json`

---

## 4. Yayın Kararı

**READY FOR PHYSICAL TESTING (Fiziksel Teste Hazır Aday Sürüm)**

Bu sürüm masaüstü ve Docker mühendislik aşamalarından %100 başarıyla geçmiştir. Fiziksel konsol testleri [DEVICE_TEST_PLAN.md](DEVICE_TEST_PLAN.md) uyarınca kullanıcı tarafından doğrulanana kadar "stable" (kararlı) sürüm olarak adlandırılmamalıdır.
