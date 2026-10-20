#include "papernx/bidi.hpp"
#include <fribidi/fribidi.h>
#include <algorithm>

namespace papernx {
namespace {
std::vector<FriBidiChar> decode(const std::string &text) {
    std::vector<FriBidiChar> chars(text.size()+1);
    int n=fribidi_charset_to_unicode(FRIBIDI_CHAR_SET_UTF8,text.data(),int(text.size()),chars.data());
    chars.resize(std::max(0,n)); return chars;
}
bool hebrew(FriBidiChar c) { return (c>=0x590 && c<=0x5ff) || (c>=0xfb1d && c<=0xfb4f); }
std::string encode(const std::vector<FriBidiChar> &chars) {
    if(chars.empty()) return {};
    std::string result(chars.size()*4+1,'\0');
    int n=fribidi_unicode_to_charset(FRIBIDI_CHAR_SET_UTF8,chars.data(),int(chars.size()),result.data());
    result.resize(std::max(0,n)); return result;
}
}
bool containsRtl(const std::string &text) {
    for(auto c:decode(text)) if(FRIBIDI_IS_RTL(fribidi_get_bidi_type(c))) return true;
    return false;
}
bool baseRtl(const std::string &text) {
    auto chars=decode(text);
    for(auto c:chars) {
        auto type=fribidi_get_bidi_type(c);
        if(FRIBIDI_IS_STRONG(type)) return FRIBIDI_IS_RTL(type);
    }
    return false;
}
std::vector<std::size_t> textClusterBoundaries(const std::string &text) {
    auto chars=decode(text); std::vector<std::size_t> result{0}; std::size_t offset=0;
    for(std::size_t i=0;i<chars.size();++i) {
        auto c=chars[i]; offset+=c<0x80?1:(c<0x800?2:(c<0x10000?3:4));
        if(i+1==chars.size() || (fribidi_get_bidi_type(chars[i+1])!=FRIBIDI_TYPE_NSM && chars[i+1]!=0x200d && c!=0x200d))
            result.push_back(offset);
    }
    return result;
}
BidiLine bidiLine(const std::string &text,int baseDirection) {
    BidiLine result;
    auto chars=decode(text); if(chars.empty()) return result;
    FriBidiParType base=baseDirection<0?FRIBIDI_PAR_ON:(baseDirection?FRIBIDI_PAR_RTL:FRIBIDI_PAR_LTR);
    std::vector<FriBidiLevel> levels(chars.size());
    std::vector<FriBidiStrIndex> positions(chars.size());
    if(!fribidi_log2vis(chars.data(),int(chars.size()),&base,nullptr,positions.data(),nullptr,levels.data())) return result;
    result.rtl=FRIBIDI_IS_RTL(base);
    struct Run { int visual; BidiRun run; };
    std::vector<Run> runs;
    for(std::size_t i=0;i<chars.size();) {
        auto level=levels[i]; bool script=hebrew(chars[i]);
        std::vector<FriBidiChar> logical;
        int visual=positions[i];
        do {
            auto c=chars[i]; auto type=fribidi_get_bidi_type(c);
            // Formatting controls affect the algorithm, never become tofu glyphs.
            if(!FRIBIDI_IS_EXPLICIT_OR_BN(type) && !FRIBIDI_IS_ISOLATE(type) && c!=0x200e && c!=0x200f) logical.push_back(c);
            visual=std::min(visual,int(positions[i])); ++i;
        } while(i<chars.size() && levels[i]==level && hebrew(chars[i])==script);
        if(!logical.empty()) runs.push_back({visual,{encode(logical),bool(level&1),script}});
    }
    std::stable_sort(runs.begin(),runs.end(),[](const Run &a,const Run &b){return a.visual<b.visual;});
    for(auto &run:runs) result.runs.push_back(std::move(run.run));
    return result;
}
}
