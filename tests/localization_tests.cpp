#include "papernx/localization.hpp"
#include <cassert>
#include <iostream>
using papernx::localizeUiText;
int main() {
    assert(localizeUiText("İçindekiler",0)=="İçindekiler");
    assert(localizeUiText("İçindekiler",1)=="Contents");
    assert(localizeUiText("Sürekli Kaydırma · Yukarı / aşağı",1)=="Continuous Scroll · Up / down");
    assert(localizeUiText("Yayıncı stilleri: Kapalı",1)=="Publisher styles: Off");
    assert(localizeUiText("Sıralama: Değiştirilme",1)=="Sort: Modified");
    assert(localizeUiText("Yazı tipi: Yayıncı",1)=="Font: Publisher");
    assert(localizeUiText("PDF filtresi: Ters renkler",1)=="PDF filter: Invert colors");
    assert(localizeUiText("−    Kaydırma hassasiyeti: %125    +",1)=="−    Scroll sensitivity: %125    +");
    assert(localizeUiText("-    Yazı boyutu: 26 px    +",1)=="-    Font size: 26 px    +");
    assert(localizeUiText("Vazgeç",1)=="Cancel");
    assert(localizeUiText("Bölüm 2 · 34 / 152",1)=="Chapter 2 · 34 / 152");
    assert(localizeUiText("12 kitap · EPUB ve PDF · Çevrimdışı",1)=="12 books · EPUB and PDF · Offline");
    assert(localizeUiText("Klasör   İstanbul / Şiir",1)=="Folder   İstanbul / Şiir");
    assert(localizeUiText("Kitap    İçindekiler.epub",1)=="Book    İçindekiler.epub");
    assert(localizeUiText("Kitap açılamadı: çığlık.epub",1)=="Could not open book: çığlık.epub");
    // Unknown proper titles and document content must remain unchanged; callers
    // must additionally bypass this UI-only API for titles identical to UI labels.
    const std::string title="İstanbul Masalları — Çığlık ve Şiir";
    assert(localizeUiText(title,1)==title);
    assert(localizeUiText("EPUB",1)=="EPUB");
    assert(localizeUiText("PaperNX'ten çık",1)=="Exit PaperNX");
    assert(localizeUiText("Yazı boyutu",1)=="Font size");
    assert(localizeUiText("Satır aralığı",1)=="Line height");
    assert(localizeUiText("Ekran yönü",1)=="Display orientation");
    assert(localizeUiText("Günlüğüm",1)=="My journal");
    assert(localizeUiText("Kitabı sil",1)=="Delete book");
    assert(localizeUiText("Kitaplık",42)=="Kitaplık");
    assert(localizeUiText("Kitaplık",0)=="Kitaplık");
    std::cout << "localization tests passed\n";
}
