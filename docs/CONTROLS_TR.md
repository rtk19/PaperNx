# Kontroller

Güncel kontrol tablosu [README](../README.md#kontroller) içindedir.
Bu tablo SDL mantıksal eşlemesini tanımlar. Joy-Con, Pro Controller, yazılım klavyesi
ve çoklu dokunma fiziksel Switch üzerinde henüz doğrulanmamıştır.

Aa modalında Y seçili alanı değiştirir; sol/sağ sekmeyi, yukarı/aşağı değeri değiştirir.
PDF için ayrı yakınlaştırma/sığdırma/tema kontrolleri gösterilir.
Araç çubuğu açıkken orta dokunuş sayfa çevirmeden çubuğu kapatır.
Kütüphanedeki X araması UTF-8 SDL_TEXTINPUT olaylarını kullanır; Switch'te libnx
swkbd çağrılır. Geri silme tam UTF-8 kod noktasını siler.
