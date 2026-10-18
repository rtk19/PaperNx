#pragma once
#include "papernx/core.hpp"
#include <optional>

namespace papernx {
enum class ReadingStatus { Automatic, NotStarted, Reading, Completed };
enum class CatalogGroup { Authors, Genres, Collections, Series, Formats };
struct CatalogEntry {
    std::string title, author, genre, series, tags;
    std::string sourceTitle, sourceAuthor, sourceGenre, sourceSeries, metadataFingerprint, sourceSeriesIndex;
    std::vector<std::string> collections;
    bool hidden{false};
    ReadingStatus status{ReadingStatus::Automatic};
};
// Independent, versioned sidecar. Never writes the user's progress/bookmark file.
// Reuses its checksummed, flushed, backup/recovery persistence transport.
class CatalogStore {
public:
    explicit CatalogStore(fs::path path);
    bool load();
    bool save() const;
    bool save(const ProgressStore::SaveOptions &) const;
    const std::string &diagnostic() const { return diagnostic_; }
    CatalogEntry get(const std::string &id) const;
    bool put(const std::string &id, CatalogEntry entry);
    Book apply(Book book) const;
    std::vector<Book> visible(const std::vector<Book> &) const;
    std::map<std::string, std::size_t> groups(const std::vector<Book> &, CatalogGroup) const;
    std::vector<Book> filter(const std::vector<Book> &, CatalogGroup, const std::string &value) const;
    const std::map<std::string, std::string> &collections() const { return collections_; }
    std::string createCollection(const std::string &name);
    bool renameCollection(const std::string &id, const std::string &name);
    bool deleteCollection(const std::string &id);
    bool addToCollection(const std::string &book, const std::string &collection);
    bool removeFromCollection(const std::string &book, const std::string &collection);
    // Selection must be a fresh scanner Book. Rejects links, directories, root
    // escapes, changed identity/size and all formats except EPUB/PDF.
    bool validateDeletion(const Book &, const fs::path &allowedRoot, std::string &error) const;
    bool deleteBookFile(const Book &, const fs::path &allowedRoot, bool confirmed, std::string &error);
private:
    fs::path path_;
    std::map<std::string, CatalogEntry> entries_;
    std::map<std::string, std::string> collections_;
    mutable std::string diagnostic_;
    bool writable_{true};
};
} // namespace papernx
