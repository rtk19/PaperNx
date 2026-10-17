# Touch controls and controller parity — v0.3

This mapping describes native application routes in `src/main.cpp`, not a hardware pass certificate. Actual controller bindings are optional shortcuts; every essential operation must have a touchscreen route. Physical acceptance is tracked in [DEVICE_TEST_PLAN.md](DEVICE_TEST_PLAN.md).

| Operation / controller intent | Touch route |
|---|---|
| Open contextual actions / Plus | Top-right `+`, including distraction-free reader |
| Close actions / Back | Tap outside the quick sheet; dialogs use their visible close control |
| Open a book / Confirm | Tap its library card or file-browser row |
| Previous/next EPUB page / Left–Right, shoulders | Paginated mode: tap left/right reading zones or swipe horizontally |
| Continuous EPUB movement / analog navigation | Navigation → Continuous Scroll Mode, then drag vertically; sensitivity and momentum are adjustable |
| Previous/next chapter | Reader `+` → previous/next chapter, when a TOC is available |
| Show/hide reader toolbar | Tap the center reading zone; `+` remains discoverable |
| Reading preferences / Aa shortcut | Reader `+` → Okuma ayarları · Aa, or visible Aa toolbar action |
| Global reading preferences | Library `+` → Okuma ayarları; no active book required |
| Application appearance | Library `+` → Uygulama görünümü; independent UI theme |
| Settings sections | Left-side Okuma, Düzen, Görünüm, Navigasyon, PDF and Uygulama sections |
| Preference scope | Header scope control: global inheritance or explicit current-book override |
| Reset / save preferences | Visible settings footer actions; close also preserves applied preferences according to the current save policy |
| Return to library / Back | Reader `+` → Kitaplığa dön, or toolbar library action |
| Browse SD / file picker shortcut | Library `+` → SD’ye gözat / kitap aç |
| Parent folder / Back in browser | Browser parent row/action or `+` → Üst klasör |
| Search library / search shortcut | Library search control or `+` → Kitaplıkta ara; Switch software keyboard on hardware |
| Sort library | Library `+` → Sırala; cycles actual supported sort modes |
| Filter categories | Tap All/Continue/Recent/Favorites/Unread/Completed category controls; drag navigation strip when clipped |
| Grid/list | Library `+` → Liste görünümüne geç / Kapak ızgarasına geç |
| Scroll library/browser/TOC/bookmarks | Drag the visible list or grid vertically; bounded movement and inertia where enabled |
| Favorite/unfavorite / favorite shortcut | Visible favorite action on library book card/list row |
| TOC / chapter shortcut | Reader `+` → İçindekiler, then tap chapter |
| Bookmarks / bookmark shortcut | Reader `+` → Yer imleri; visible add/remove controls and saved-location rows |
| Progress / jump to page | Reader `+` → İlerleme / sayfaya git, then use visible reader progress control |
| Temporary exploration back | After a TOC/bookmark/progress jump, reader `+` → Önceki konuma dön; history is bounded to 24 locations |
| PDF zoom / zoom shortcuts | Visible −/+; two-finger zoom where SDL supplies multi-touch |
| PDF pan / analog pan | Drag a zoomed PDF; captured pan must not turn a page |
| PDF fit-page / fit-width / 100% | Settings → PDF view mode controls |
| PDF original/dark/sepia/invert | Settings → PDF filters; whole-page effects include images |
| Help / book information | `+` → Help; reader includes current book title/author when known |
| Exit app / library Back | Library or file browser `+` → PaperNX’ten çık |

EPUB font/layout rules do not apply to PDF. Horizontal gestures turn pages only in paginated EPUB mode. In continuous mode a vertical drag moves real rendered content, including across layout segments; it must not swap whole pages as a swipe imitation. A PDF pan is separate from EPUB scroll.

SDL synthesized mouse events accompanying finger events must not activate twice. Modal z-order blocks underlying page taps. Closing a modal cancels pending gesture state. Controller disconnection must leave touch routes available. These are regression requirements; consult aggregate application tests for exact automated coverage rather than treating this mapping as a full coverage claim.

Not currently promised: in-book full-text search, annotations, statistics or sync. The library search route must not be confused with in-book search. Future functions remain in [PRODUCT_ROADMAP_2026.md](PRODUCT_ROADMAP_2026.md) and must not appear as dead usable buttons.
