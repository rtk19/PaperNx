#include "papernx/core.hpp"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <system_error>

namespace papernx {
namespace {

std::uint64_t fnv1a(const std::string &s) {
    std::uint64_t h = 14695981039346656037ull;
    for (unsigned char c : s) {
        h ^= c;
        h *= 1099511628211ull;
    }
    return h;
}

std::string hex(std::uint64_t h) {
    std::ostringstream out;
    out << std::hex << std::setw(16) << std::setfill('0') << h;
    return out.str();
}

std::string escapeString(const std::string &s) {
    std::string res;
    for (char c : s) {
        if (c == '\t') res += "\\t";
        else if (c == '\n') res += "\\n";
        else if (c == '\\') res += "\\\\";
        else res += c;
    }
    return res;
}

std::string unescapeString(const std::string &s) {
    std::string res;
    for (std::size_t i = 0; i < s.size(); ++i) {
        if (s[i] == '\\' && i + 1 < s.size()) {
            if (s[i + 1] == 't') { res += '\t'; ++i; }
            else if (s[i + 1] == 'n') { res += '\n'; ++i; }
            else if (s[i + 1] == '\\') { res += '\\'; ++i; }
            else res += s[i];
        } else {
            res += s[i];
        }
    }
    return res;
}

bool parseSafeInt(const std::string &s, int &out) {
    if (s.empty()) return false;
    char *end = nullptr;
    long val = std::strtol(s.c_str(), &end, 10);
    if (!end || *end != '\0') return false;
    out = static_cast<int>(val);
    return true;
}

bool parseSafeLL(const std::string &s, std::int64_t &out) {
    if (s.empty()) return false;
    char *end = nullptr;
    long long val = std::strtoll(s.c_str(), &end, 10);
    if (!end || *end != '\0') return false;
    out = static_cast<std::int64_t>(val);
    return true;
}

bool parseSafeDouble(const std::string &s, double &out) {
    if (s.empty()) return false;
    char *end = nullptr;
    double val = std::strtod(s.c_str(), &end);
    if (!end || *end != '\0') return false;
    out = val;
    return true;
}

std::string payloadV2(const std::map<std::string, Progress> &records) {
    std::ostringstream out;
    out << "PAPERNX-2\n";
    for (const auto &[id, p] : records) {
        if (id.size() > 128 || id.find_first_of("\t\r\n") != std::string::npos) continue;
        out << id << '\t'
            << p.page << '\t'
            << p.pageCount << '\t'
            << std::setprecision(17) << p.fraction << '\t'
            << p.fontSize << '\t'
            << p.theme << '\t'
            << (p.favorite ? 1 : 0) << '\t'
            << p.muBookmark << '\t'
            << p.chapter << '\t'
            << p.lineHeight << '\t'
            << p.margin << '\t'
            << p.textAlign << '\t'
            << p.fontFamily << '\t'
            << p.lastRead << '\t'
            << p.bookmarks.size();
        for (const auto &bm : p.bookmarks) {
            out << '\t' << bm.page << '\t' << bm.timestamp << '\t'
                << bm.muBookmark << '\t' << escapeString(bm.title);
        }
        out << '\n';
    }
    return out.str();
}

} // namespace

std::string turkishCaseFold(const std::string &s) {
    std::string out;
    out.reserve(s.size());
    const unsigned char *p = reinterpret_cast<const unsigned char*>(s.data());
    const std::size_t n = s.size();

    for (std::size_t i = 0; i < n; ) {
        // Turkish uppercase 'İ' is 0xC4 0xB0 -> becomes lowercase 'i' (0x69)
        if (p[i] == 0xC4 && i + 1 < n && p[i + 1] == 0xB0) {
            out.push_back('i');
            i += 2;
        }
        // ASCII uppercase 'I' (0x49) in Turkish becomes 'ı' (0xC4 0xB1)
        else if (p[i] == 'I') {
            out.push_back(static_cast<char>(0xC4));
            out.push_back(static_cast<char>(0xB1));
            i += 1;
        }
        // Turkish 'Ğ' is 0xC4 0x9E -> 'ğ' (0xC4 0x9F)
        else if (p[i] == 0xC4 && i + 1 < n && p[i + 1] == 0x9E) {
            out.push_back(static_cast<char>(0xC4));
            out.push_back(static_cast<char>(0x9F));
            i += 2;
        }
        // Turkish 'Ü' is 0xC3 0x9C -> 'ü' (0xC3 0xBC)
        else if (p[i] == 0xC3 && i + 1 < n && p[i + 1] == 0x9C) {
            out.push_back(static_cast<char>(0xC3));
            out.push_back(static_cast<char>(0xBC));
            i += 2;
        }
        // Turkish 'Ş' is 0xC5 0x9E -> 'ş' (0xC5 0x9F)
        else if (p[i] == 0xC5 && i + 1 < n && p[i + 1] == 0x9E) {
            out.push_back(static_cast<char>(0xC5));
            out.push_back(static_cast<char>(0x9F));
            i += 2;
        }
        // Turkish 'Ö' is 0xC3 0x96 -> 'ö' (0xC3 0xB6)
        else if (p[i] == 0xC3 && i + 1 < n && p[i + 1] == 0x96) {
            out.push_back(static_cast<char>(0xC3));
            out.push_back(static_cast<char>(0xB6));
            i += 2;
        }
        // Turkish 'Ç' is 0xC3 0x87 -> 'ç' (0xC3 0xA7)
        else if (p[i] == 0xC3 && i + 1 < n && p[i + 1] == 0x87) {
            out.push_back(static_cast<char>(0xC3));
            out.push_back(static_cast<char>(0xA7));
            i += 2;
        }
        // Standard ASCII A-Z -> a-z
        else if (p[i] >= 'A' && p[i] <= 'Z') {
            out.push_back(static_cast<char>(p[i] - 'A' + 'a'));
            i += 1;
        }
        else {
            out.push_back(static_cast<char>(p[i]));
            i += 1;
        }
    }
    return out;
}

std::string utf8SafeTruncate(const std::string &s, std::size_t maxBytes) {
    if (s.size() <= maxBytes) return s;
    if (maxBytes == 0) return "";
    std::size_t end = maxBytes;
    while (end > 0 && (static_cast<unsigned char>(s[end]) & 0xC0) == 0x80) {
        --end;
    }
    return s.substr(0, end);
}

std::size_t utf8CodepointCount(const std::string &s) {
    std::size_t count = 0;
    for (unsigned char c : s) {
        if ((c & 0xC0) != 0x80) ++count;
    }
    return count;
}

Format detectFormat(const fs::path &path) {
    std::string ext = path.extension().string();
    for (char &c : ext) {
        if (c >= 'A' && c <= 'Z') c = static_cast<char>(c - 'A' + 'a');
    }
    if (ext == ".epub") return Format::Epub;
    if (ext == ".pdf") return Format::Pdf;
    return Format::Unsupported;
}

std::string defaultTitle(const fs::path &path) {
    std::string title = path.stem().string();
    for (char &c : title) {
        if (c == '_') c = ' ';
    }
    return title.empty() ? "Untitled" : title;
}

std::string documentId(const std::string &path, std::uintmax_t size) {
    return hex(fnv1a(path + "\n" + std::to_string(size)));
}

std::vector<Book> scan(const fs::path &root, std::size_t maxBooks) {
    std::vector<Book> books;
    std::error_code ec;
    if (!fs::is_directory(root, ec)) return books;

    fs::recursive_directory_iterator it(root, fs::directory_options::skip_permission_denied, ec), end;
    for (; it != end && books.size() < maxBooks; it.increment(ec)) {
        if (ec) { ec.clear(); continue; }
        if (it.depth() > 16) { it.disable_recursion_pending(); continue; }
        if (it->is_symlink(ec)) { it.disable_recursion_pending(); continue; }
        if (!it->is_regular_file(ec)) continue;

        const auto path = it->path();
        const auto type = detectFormat(path);
        if (type == Format::Unsupported) continue;

        auto size = it->file_size(ec);
        if (ec) { ec.clear(); continue; }

        Book b;
        b.path = path.string();
        b.bytes = size;
        b.id = documentId(b.path, size);
        b.title = defaultTitle(path);
        b.format = type;

        const auto mtime = it->last_write_time(ec);
        if (!ec) {
            b.modified = std::chrono::duration_cast<std::chrono::seconds>(mtime.time_since_epoch()).count();
        }
        ec.clear();
        books.push_back(std::move(b));
    }

    std::sort(books.begin(), books.end(), [](const Book &a, const Book &b) {
        return a.title < b.title;
    });
    return books;
}

std::vector<Book> filterByCategory(const std::vector<Book> &books, const ProgressStore &store, Category category) {
    if (category == Category::All) return books;
    std::vector<Book> result;
    result.reserve(books.size());

    for (const auto &b : books) {
        auto p = store.get(b.id);
        switch (category) {
            case Category::ContinueReading:
                if (p.pageCount > 0 && p.page > 0 && p.page < p.pageCount - 1 && p.lastRead > 0)
                    result.push_back(b);
                break;
            case Category::Recent:
                if (p.lastRead > 0)
                    result.push_back(b);
                break;
            case Category::Favorites:
                if (p.favorite)
                    result.push_back(b);
                break;
            case Category::Unread:
                if (p.page == 0 && p.fraction <= 0.001)
                    result.push_back(b);
                break;
            case Category::Completed:
                if (p.pageCount > 0 && (p.page >= p.pageCount - 1 || p.fraction >= 0.98))
                    result.push_back(b);
                break;
            default:
                result.push_back(b);
                break;
        }
    }
    return result;
}

std::vector<Book> search(const std::vector<Book> &books, const std::string &needle, Sort sort) {
    std::vector<Book> result;
    const std::string q = turkishCaseFold(needle);

    for (const auto &b : books) {
        if (q.empty() ||
            turkishCaseFold(b.title).find(q) != std::string::npos ||
            turkishCaseFold(b.author).find(q) != std::string::npos ||
            turkishCaseFold(fs::path(b.path).filename().string()).find(q) != std::string::npos) {
            result.push_back(b);
        }
    }

    std::sort(result.begin(), result.end(), [sort](const Book &a, const Book &b) {
        if (sort == Sort::Modified) {
            return a.modified == b.modified ? a.id < b.id : a.modified > b.modified;
        }
        if (sort == Sort::Author) {
            auto la = turkishCaseFold(a.author), lb = turkishCaseFold(b.author);
            if (la != lb) return la < lb;
        }
        if (sort == Sort::Filename) {
            auto la = turkishCaseFold(fs::path(a.path).filename().string());
            auto lb = turkishCaseFold(fs::path(b.path).filename().string());
            return la == lb ? a.id < b.id : la < lb;
        }
        auto l = turkishCaseFold(a.title), r = turkishCaseFold(b.title);
        return l == r ? a.id < b.id : l < r;
    });

    return result;
}

std::vector<Book> searchEx(const std::vector<Book> &books, const ProgressStore &store,
                           const std::string &needle, Sort sort, Category category) {
    auto filtered = filterByCategory(books, store, category);
    auto res = search(filtered, needle, sort);

    if (sort == Sort::LastRead) {
        std::sort(res.begin(), res.end(), [&store](const Book &a, const Book &b) {
            auto pa = store.get(a.id), pb = store.get(b.id);
            if (pa.lastRead != pb.lastRead) return pa.lastRead > pb.lastRead;
            return a.id < b.id;
        });
    }
    return res;
}

int restorePage(const Progress &p, int n) {
    if (n <= 0) return 0;
    if (p.pageCount == n) return std::clamp(p.page, 0, n - 1);
    const double f = std::isfinite(p.fraction) ? std::clamp(p.fraction, 0.0, 1.0) : 0.0;
    return std::clamp(static_cast<int>(std::round(f * (n - 1))), 0, n - 1);
}

ProgressStore::ProgressStore(fs::path p) : path_(std::move(p)) {}

Progress ProgressStore::get(const std::string &id) const {
    auto it = records_.find(id);
    return it == records_.end() ? Progress{} : it->second;
}

void ProgressStore::put(const std::string &id, Progress p) {
    p.page = std::max(0, p.page);
    p.pageCount = std::max(0, p.pageCount);
    p.fraction = std::isfinite(p.fraction) ? std::clamp(p.fraction, 0.0, 1.0) : 0.0;
    p.fontSize = std::clamp(p.fontSize, 12, 48);
    p.theme = std::clamp(p.theme, 0, 5);
    p.lineHeight = std::clamp(p.lineHeight, 100, 220);
    p.margin = std::clamp(p.margin, 10, 120);
    p.textAlign = std::clamp(p.textAlign, 0, 1);
    p.fontFamily = std::clamp(p.fontFamily, 0, 3);
    if (p.lastRead <= 0) {
        p.lastRead = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
    }
    records_[id] = std::move(p);
}

std::size_t ProgressStore::count() const {
    return records_.size();
}

bool ProgressStore::load() {
    auto readFile = [&](const fs::path &candidate) -> bool {
        std::ifstream in(candidate, std::ios::binary);
        if (!in) return false;

        std::string encoded((std::istreambuf_iterator<char>(in)), {});
        if (encoded.size() < 20 || encoded.size() > 8 * 1024 * 1024) return false;

        const auto split = encoded.find('\n');
        if (split == std::string::npos) return false;

        const std::string header = encoded.substr(0, split);
        const std::string body = encoded.substr(split + 1);

        if (header != "CHECKSUM=" + hex(fnv1a(body))) return false;

        std::istringstream rows(body);
        std::string versionLine;
        if (!std::getline(rows, versionLine)) return false;

        std::map<std::string, Progress> restored;

        if (versionLine == "PAPERNX-2") {
            std::string line;
            while (std::getline(rows, line)) {
                if (line.empty()) continue;
                std::vector<std::string> tokens;
                std::string token;
                std::istringstream tokenStream(line);
                while (std::getline(tokenStream, token, '\t')) {
                    tokens.push_back(token);
                }
                if (tokens.size() < 15) continue;

                Progress p;
                std::string id = tokens[0];
                int fav = 0, bmCount = 0;

                bool valid = parseSafeInt(tokens[1], p.page) &&
                             parseSafeInt(tokens[2], p.pageCount) &&
                             parseSafeDouble(tokens[3], p.fraction) &&
                             parseSafeInt(tokens[4], p.fontSize) &&
                             parseSafeInt(tokens[5], p.theme) &&
                             parseSafeInt(tokens[6], fav) &&
                             parseSafeLL(tokens[7], p.muBookmark) &&
                             parseSafeInt(tokens[8], p.chapter) &&
                             parseSafeInt(tokens[9], p.lineHeight) &&
                             parseSafeInt(tokens[10], p.margin) &&
                             parseSafeInt(tokens[11], p.textAlign) &&
                             parseSafeInt(tokens[12], p.fontFamily) &&
                             parseSafeLL(tokens[13], p.lastRead) &&
                             parseSafeInt(tokens[14], bmCount);

                if (!valid) continue;
                p.favorite = (fav != 0);

                std::size_t cursor = 15;
                bool bookmarksValid = true;
                for (int b = 0; b < bmCount && cursor + 3 < tokens.size(); ++b) {
                    Bookmark bm;
                    if (!parseSafeInt(tokens[cursor++], bm.page) ||
                        !parseSafeLL(tokens[cursor++], bm.timestamp) ||
                        !parseSafeLL(tokens[cursor++], bm.muBookmark)) {
                        bookmarksValid = false;
                        break;
                    }
                    bm.title = unescapeString(tokens[cursor++]);
                    p.bookmarks.push_back(std::move(bm));
                }

                if (bookmarksValid && id.size() == 16 && p.page >= 0 && std::isfinite(p.fraction)) {
                    restored[id] = p;
                }
            }
        } else if (versionLine == "PAPERNX-1") {
            // Backward-compatible schema migration
            std::string line;
            while (std::getline(rows, line)) {
                if (line.empty()) continue;
                std::replace(line.begin(), line.end(), '\t', ' ');
                std::istringstream row(line);
                std::string id;
                Progress p;
                int fav = 0;
                if (!(row >> id >> p.page >> p.pageCount >> p.fraction >> p.fontSize >> p.theme >> fav)) {
                    continue;
                }
                if (id.size() != 16 || p.page < 0 || p.pageCount < 0 || !std::isfinite(p.fraction)) {
                    continue;
                }
                p.favorite = (fav != 0);
                p.lastRead = 0;
                p.muBookmark = -1;
                p.chapter = -1;
                restored[id] = p;
            }
        } else {
            return false;
        }

        records_.swap(restored);
        return true;
    };

    if (readFile(path_)) return true;

    fs::path backup = path_;
    backup += ".bak";
    if (readFile(backup)) return true;

    fs::path backupOld = path_;
    backupOld += ".bak.old";
    if (readFile(backupOld)) return true;

    fs::path temp = path_;
    temp += ".tmp";
    return readFile(temp);
}

bool ProgressStore::save() const {
    std::error_code ec;
    fs::create_directories(path_.parent_path(), ec);
    if (ec) return false;

    const auto data = payloadV2(records_);
    fs::path temp = path_;
    temp += ".tmp";

    {
        std::ofstream out(temp, std::ios::binary | std::ios::trunc);
        if (!out) return false;
        out << "CHECKSUM=" << hex(fnv1a(data)) << '\n' << data;
        out.flush();
        if (!out.good()) return false;
    }

    // Verify temp file exists and has data before rotating
    auto tempSize = fs::file_size(temp, ec);
    if (ec || tempSize == 0) {
        ec.clear();
        fs::remove(temp, ec);
        return false;
    }

    fs::path backup = path_;
    backup += ".bak";
    fs::path backupOld = path_;
    backupOld += ".bak.old";

    // Step 1: Preserve existing backup into backup.old
    if (fs::exists(backup, ec)) {
        ec.clear();
        fs::remove(backupOld, ec);
        ec.clear();
        fs::rename(backup, backupOld, ec);
    }
    ec.clear();

    // Step 2: Rotate active primary file to backup
    if (fs::exists(path_, ec)) {
        ec.clear();
        fs::rename(path_, backup, ec);
        if (ec) {
            // Restore backup from backup.old if rotation failed
            if (fs::exists(backupOld, ec)) {
                ec.clear();
                fs::rename(backupOld, backup, ec);
            }
            return false;
        }
    }
    ec.clear();

    // Step 3: Promote verified temp to active primary file
    fs::rename(temp, path_, ec);
    if (ec) {
        ec.clear();
        // Rollback: restore primary from backup
        if (fs::exists(backup, ec)) {
            ec.clear();
            fs::rename(backup, path_, ec);
        }
        return false;
    }

    // Step 4: Transaction committed; safe to purge stale backup.old
    ec.clear();
    fs::remove(backupOld, ec);
    return true;
}

} // namespace papernx
