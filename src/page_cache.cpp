#include "papernx/page_cache.hpp"
#include <sstream>

namespace papernx {

PageCache::PageCache(std::size_t maxBytes) : maxBytes_(maxBytes) {}

PageCache::~PageCache() {
    clear();
}

std::string PageCache::makeKey(const std::string &docId, int page, float zoom, int fontSize, int theme) {
    std::ostringstream ss;
    ss << docId << "_" << page << "_" << static_cast<int>(zoom * 100) << "_" << fontSize << "_" << theme;
    return ss.str();
}

SDL_Texture *PageCache::get(const std::string &key, int *outW, int *outH) {
    auto it = items_.find(key);
    if (it == items_.end()) return nullptr;

    // Move to front of LRU order list
    order_.splice(order_.begin(), order_, it->second);

    if (outW) *outW = it->second->width;
    if (outH) *outH = it->second->height;
    return it->second->texture;
}

void PageCache::put(const std::string &key, SDL_Texture *texture, int w, int h) {
    if (!texture || w <= 0 || h <= 0) return;

    // If key already exists, update and move to front
    auto it = items_.find(key);
    if (it != items_.end()) {
        currentBytes_ -= it->second->bytes;
        if (it->second->texture != texture) {
            SDL_DestroyTexture(it->second->texture);
        }
        it->second->texture = texture;
        it->second->width = w;
        it->second->height = h;
        it->second->bytes = static_cast<std::size_t>(w * h * 4);
        currentBytes_ += it->second->bytes;
        order_.splice(order_.begin(), order_, it->second);
        return;
    }

    std::size_t bytes = static_cast<std::size_t>(w * h * 4);
    while (currentBytes_ + bytes > maxBytes_ && !order_.empty()) {
        evictOldest();
    }

    CachedTexture entry;
    entry.key = key;
    entry.texture = texture;
    entry.width = w;
    entry.height = h;
    entry.bytes = bytes;

    order_.push_front(entry);
    items_[key] = order_.begin();
    currentBytes_ += bytes;
}

void PageCache::evictOldest() {
    if (order_.empty()) return;
    auto last = std::prev(order_.end());
    if (last->texture) {
        SDL_DestroyTexture(last->texture);
    }
    currentBytes_ -= last->bytes;
    items_.erase(last->key);
    order_.pop_back();
}

void PageCache::evictDocument(const std::string &docId) {
    auto it = order_.begin();
    while (it != order_.end()) {
        if (it->key.rfind(docId, 0) == 0) { // starts with docId
            if (it->texture) {
                SDL_DestroyTexture(it->texture);
            }
            currentBytes_ -= it->bytes;
            items_.erase(it->key);
            it = order_.erase(it);
        } else {
            ++it;
        }
    }
}

void PageCache::clear() {
    for (auto &item : order_) {
        if (item.texture) {
            SDL_DestroyTexture(item.texture);
        }
    }
    order_.clear();
    items_.clear();
    currentBytes_ = 0;
}

} // namespace papernx
