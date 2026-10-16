#pragma once
#include <SDL.h>
#include <list>
#include <string>
#include <unordered_map>

namespace papernx {

struct CachedTexture {
    std::string key;
    SDL_Texture *texture{nullptr};
    int width{0};
    int height{0};
    std::size_t bytes{0};
};

class PageCache {
public:
    explicit PageCache(std::size_t maxBytes = 48 * 1024 * 1024);
    ~PageCache();

    static std::string makeKey(const std::string &docId, int page, float zoom = 1.0f, int fontSize = 0, int theme = 0);

    SDL_Texture *get(const std::string &key, int *outW = nullptr, int *outH = nullptr);
    void put(const std::string &key, SDL_Texture *texture, int w, int h);
    void evictDocument(const std::string &docId);
    void clear();

    std::size_t currentBytes() const { return currentBytes_; }
    std::size_t count() const { return items_.size(); }

private:
    std::size_t maxBytes_;
    std::size_t currentBytes_{0};

    std::list<CachedTexture> order_;
    std::unordered_map<std::string, std::list<CachedTexture>::iterator> items_;

    void evictOldest();
};

} // namespace papernx
