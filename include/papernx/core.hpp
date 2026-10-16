#pragma once
#include <cstdint>
#include <filesystem>
#include <map>
#include <string>
#include <vector>

namespace papernx {
namespace fs = std::filesystem;

enum class Format { Epub, Pdf, Unsupported };
enum class Sort { Title, Author, Filename, Modified, LastRead };
enum class Category { All, ContinueReading, Recent, Favorites, Unread, Completed };

struct Book {
    std::string id;
    std::string path;
    std::string title;
    std::string author;
    Format format{Format::Unsupported};
    std::uintmax_t bytes{};
    std::int64_t modified{};
};

struct Bookmark {
    int page{0};
    std::string title;
    std::int64_t timestamp{0};
    std::int64_t muBookmark{-1};
};

struct Progress {
    int page{0};
    int pageCount{0};
    double fraction{0.0};
    int fontSize{22};
    int theme{0};
    bool favorite{false};

    // Extended Reader & Persistence Attributes (PAPERNX-2)
    std::int64_t muBookmark{-1};
    int chapter{-1};
    int lineHeight{140}; // percentage (140 = 1.4)
    int margin{40};      // margin in pixels
    int textAlign{0};   // 0: justify, 1: left
    int fontFamily{0};  // 0: Serif, 1: Sans, 2: Custom/Publisher
    std::int64_t lastRead{0};
    std::vector<Bookmark> bookmarks;
};

Format detectFormat(const fs::path &path);
std::string documentId(const std::string &path, std::uintmax_t size);
std::string defaultTitle(const fs::path &path);
std::vector<Book> scan(const fs::path &root, std::size_t maxBooks = 10000);

// Unicode & Turkish Support
std::string turkishCaseFold(const std::string &s);
std::string utf8SafeTruncate(const std::string &s, std::size_t maxBytes);
std::size_t utf8CodepointCount(const std::string &s);

// Categorization & Filtering
std::vector<Book> filterByCategory(const std::vector<Book> &books, const class ProgressStore &store, Category category);
std::vector<Book> search(const std::vector<Book> &books, const std::string &needle, Sort sort = Sort::Title);
std::vector<Book> searchEx(const std::vector<Book> &books, const class ProgressStore &store,
                           const std::string &needle, Sort sort = Sort::Title, Category category = Category::All);

int restorePage(const Progress &progress, int newPageCount);

class ProgressStore {
public:
    explicit ProgressStore(fs::path path);
    bool load();
    bool save() const;
    Progress get(const std::string &id) const;
    void put(const std::string &id, Progress progress);
    std::size_t count() const;
    const std::map<std::string, Progress> &all() const { return records_; }

private:
    fs::path path_;
    std::map<std::string, Progress> records_;
};

} // namespace papernx
