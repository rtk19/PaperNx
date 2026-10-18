#pragma once
#include <algorithm>
#include <cmath>
#include <limits>
#include <map>
#include <string>
#include <vector>
namespace papernx {
// Stable IDs keep focus independent of selection and survive rendering and list updates.
struct FocusRect { int x,y,w,h; };
class FocusManager {
public:
    struct Item {std::string id;FocusRect rect;bool enabled;};
    void begin(const std::string &context) {context_=context;items_.clear();}
    void add(std::string id,FocusRect rect,bool enabled=true) {
        items_.push_back({std::move(id),rect,enabled});
        if(active().empty() && enabled) remembered_[context_]=items_.back().id;
    }
    void finish() {
        auto found=std::find_if(items_.begin(),items_.end(),[&](const Item &i){return i.enabled && i.id==active();});
        if(found==items_.end()) {remembered_[context_].clear();for(const auto &i:items_)if(i.enabled){remembered_[context_]=i.id;break;}}
    }
    const std::string &active() const {auto i=remembered_.find(context_);return i==remembered_.end()?empty_:i->second;}
    bool focused(const std::string &id) const {return active()==id;}
    bool select(const std::string &id) {for(const auto &i:items_)if(i.id==id && i.enabled){remembered_[context_]=id;return true;}return false;}
    bool touch(int x,int y) {for(const auto &i:items_)if(i.enabled && x>=i.rect.x && x<i.rect.x+i.rect.w && y>=i.rect.y && y<i.rect.y+i.rect.h)return select(i.id);return false;}
    bool move(int dx,int dy) {
        finish();auto current=std::find_if(items_.begin(),items_.end(),[&](const Item &i){return i.id==active();});
        if(current==items_.end())return false;
        const double x=current->rect.x+current->rect.w*.5,y=current->rect.y+current->rect.h*.5;
        double best=std::numeric_limits<double>::max();const Item *target=nullptr;
        for(const auto &i:items_)if(i.enabled && i.id!=current->id) {
            double rx=i.rect.x+i.rect.w*.5-x,ry=i.rect.y+i.rect.h*.5-y;
            double along=dx?rx*dx:ry*dy,across=dx?std::abs(ry):std::abs(rx);
            if(along<=0)continue;
            double score=along+across*3+across*across/std::max(1.0,along);
            if(score<best){best=score;target=&i;}
        }
        return target?select(target->id):false;
    }
    const std::vector<Item>&items() const{return items_;}
private:
    std::string context_,empty_;
    std::map<std::string,std::string> remembered_;
    std::vector<Item>items_;
};
}
