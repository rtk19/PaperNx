#pragma once
#include <string>
#include <vector>
#include <cstdint>

namespace papernx {
// Logical strings remain untouched in the catalog and on disk. Runs are ordered
// for display only; each run must be shaped in its original logical order.
struct BidiRun {
    std::string text;
    bool rtl{false}, hebrew{false};
};
struct BidiLine {
    bool rtl{false};
    std::vector<BidiRun> runs;
};
bool containsRtl(const std::string &text);
bool baseRtl(const std::string &text);
BidiLine bidiLine(const std::string &text, int baseDirection = -1);
// UTF-8 cut positions that retain Hebrew vowel/diacritic clusters.
std::vector<std::size_t> textClusterBoundaries(const std::string &text);
}
