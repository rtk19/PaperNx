#pragma once
#include <cstdint>
#include <filesystem>
#include <map>
#include <string>
#include <vector>
namespace papernx {
namespace fs = std::filesystem;
enum class Format { Epub, Pdf, Unsupported };
enum class Sort { Title, Filename, Modified };
struct Book {
 std::string id, path, title, author;
 Format format{Format::Unsupported};
 std::uintmax_t bytes{};
 std::int64_t modified{};
};
struct Progress {
 int page{}, pageCount{};
 double fraction{};
 int fontSize{22}, theme{};
 bool favorite{};
};
Format detectFormat(const fs::path &path);
std::string documentId(const std::string &path, std::uintmax_t size);
std::string defaultTitle(const fs::path &path);
std::vector<Book> scan(const fs::path &root, std::size_t maxBooks=10000);
std::vector<Book> search(const std::vector<Book> &books, const std::string &needle, Sort sort=Sort::Title);
int restorePage(const Progress &progress, int newPageCount);
class ProgressStore {
public:
 explicit ProgressStore(fs::path path);
 bool load();
 bool save() const;
 Progress get(const std::string &id) const;
 void put(const std::string &id, Progress progress);
 std::size_t count() const;
private:
 fs::path path_;
 std::map<std::string, Progress> records_;
};
}
