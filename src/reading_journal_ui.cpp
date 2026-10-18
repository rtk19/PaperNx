#include "papernx/reading_journal_ui.hpp"
#include "papernx/ui.hpp"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <ctime>
#include <map>
#include <vector>

namespace papernx {
namespace {
std::string label(bool en,const char *tr,const char *english) {return en?english:tr;}
std::string duration(std::uint64_t ms,bool en) {
    auto m=ms/60000;
    if(m>=60)return std::to_string(m/60)+(en?" h ":" sa ")+std::to_string(m%60)+(en?" min":" dk");
    return std::to_string(m)+(en?" min":" dk");
}
std::string key(int year,int month,int day){
    char b[24];std::snprintf(b,sizeof b,"%04d-%02d-%02d",year,month,day);return b;
}
std::tm nowDate() {
    const auto n=std::time(nullptr);const auto *ptr=std::localtime(&n);
    return ptr?*ptr:std::tm{};
}
std::tm dateAt(int monthDelta) {
    auto d=nowDate();d.tm_mday=1;d.tm_mon+=monthDelta;
    std::mktime(&d);return d;
}
int daysInMonth(int year,int month) {
    std::tm d{};d.tm_year=year-1900;d.tm_mon=month+1;d.tm_mday=0;
    std::mktime(&d);return d.tm_mday;
}
std::string localDate(std::int64_t epoch){
    const auto raw=static_cast<std::time_t>(epoch);
    const auto *d=std::localtime(&raw);
    if(!d)return "?";
    char b[32];std::strftime(b,sizeof b,"%Y-%m-%d",d);return b;
}
void text(SDL_Renderer *r,Typography &font,const std::string &s,int x,int y,
          SDL_Color color,int maxw=0,bool large=false){
    font.renderText(r,large?font.titleFont():font.uiFont(),s,x,y,color,maxw);
}
void line(SDL_Renderer *r,SDL_Color color,int x,int y,int w){
    SDL_SetRenderDrawColor(r,color.r,color.g,color.b,255);
    SDL_RenderDrawLine(r,x,y,x+w,y);
}
void panel(SDL_Renderer *r,const ThemePalette &pal,SDL_Rect rc){
    ui::surface(r,rc,pal.card,16);
    SDL_SetRenderDrawColor(r,pal.cardBorder.r,pal.cardBorder.g,pal.cardBorder.b,255);
    SDL_RenderDrawRect(r,&rc);
}
struct Geometry {int left,top,width,navTop;bool sidebar;};
Geometry geometry(int w){
    return {w>=900?244:18,w>=900?116:170,w-(w>=900?266:36),w>=900?124:94,w>=900};
}
void header(SDL_Renderer *r,Typography &font,const ThemePalette &pal,int w,bool en) {
    ui::surface(r,{0,0,w,80},pal.card,0);
    ui::button(r,font,{16,12,156,56},label(en,"Kitaplık","Library"),pal);
    text(r,font,label(en,"Okuma Günlüğüm","My Reading Journal"),194,18,pal.fg,w-420,true);
    ui::button(r,font,{w-248,12,160,56},label(en,"Hedeflerim","My goals"),pal);
    line(r,pal.cardBorder,0,80,w);
}
void nav(SDL_Renderer *r,Typography &font,const ThemePalette &pal,int w,int active,int focus,bool en){
    const char *tr[]={"Genel Bakış","Takvim","Kitaplarım","Geçmiş"};
    const char *eng[]={"Overview","Calendar","My books","History"};
    auto g=geometry(w);
    for(int i=0;i<4;++i){
        SDL_Rect rc=g.sidebar?SDL_Rect{16,g.navTop+i*68,208,56}:
                                SDL_Rect{18+i*(g.width/4),96,g.width/4-5,56};
        ui::button(r,font,rc,en?eng[i]:tr[i],pal,i==active,false,false,focus==i+1);
    }
}
void empty(SDL_Renderer *r,Typography &font,const ThemePalette &pal,SDL_Rect rc,bool en) {
    panel(r,pal,rc);
    text(r,font,label(en,"Henüz okuma kaydı yok.","No reading records yet."),
         rc.x+24,rc.y+24,pal.fg,rc.w-48);
    text(r,font,label(en,"İlk kitabını açtığında sürelerin burada görünecek.",
         "Your reading time appears here once you open a book."),
         rc.x+24,rc.y+74,pal.muted,rc.w-48);
    text(r,font,label(en,"Kitaplığa dön: sol üst düğme","Return to library: upper-left button"),
         rc.x+24,rc.y+124,pal.accent,rc.w-48);
}
std::uint64_t dayMs(const std::map<std::string,std::uint64_t> &cal,int y,int m,int d){
    const auto it=cal.find(key(y,m,d));return it==cal.end()?0:it->second;
}
}
void ReadingJournalUI::open(const std::string &bookId) {
    tab_=bookId.empty()?0:2;detailId_=bookId;monthDelta_=0;offset_=0;focus_=0;goals_=false;preview_=false;
}
void ReadingJournalUI::back(){
    if(goals_)goals_=false;
    else if(!detailId_.empty())detailId_.clear();
}
void ReadingJournalUI::navigate(int dx,int dy,const ReadingJournal &journal){
    if(goals_)return;
    if(!detailId_.empty()){if(dy)offset_=std::max(0,offset_+dy);return;}
    if(dx && tab_==1){monthDelta_+=dx;selectedDay_=0;}
    else if(dx)tab_=std::clamp(tab_+dx,0,3);
    if(dy && (tab_==2||tab_==3))offset_=std::clamp(offset_+dy,0,int(tab_==2?journal.books().size():journal.sessions().size()));
    if(dy && tab_==1)selectedDay_=std::clamp(selectedDay_+dy*7,1,31);
    focus_=tab_+1;
}
ReadingJournalUI::Action ReadingJournalUI::activate(int w,int h,ReadingJournal &j){
    if(goals_)return Action::None;
    if(preview_)return Action::OpenFull;
    if(!detailId_.empty()) {back();return Action::None;}
    (void)w;(void)h;(void)j;return Action::None;
}
ReadingJournalUI::Action ReadingJournalUI::tap(int x,int y,int w,int h,ReadingJournal &journal){
    if(preview_){
        const int pw=std::min(620,w-24),px=(w-pw)/2,py=(h-320)/2;
        if(x<px||x>px+pw||y<py||y>py+320)return Action::ClosePreview;
        if(y>py+244)return x<px+pw/2?Action::OpenFull:Action::ClosePreview;
        return Action::None;
    }
    if(goals_){
        const int px=(w-520)/2,py=(h-280)/2;
        if(y>py+192 && y<py+260){
            if(x<px+165){journal.setDailyGoalMinutes(journal.dailyGoalMinutes()>=5?journal.dailyGoalMinutes()-5:0);}
            else if(x<px+330){journal.setDailyGoalMinutes(std::min(720u,journal.dailyGoalMinutes()+5));}
            else goals_=false;
        }else if(x<px||x>px+520||y<py||y>py+280)goals_=false;
        return Action::None;
    }
    if(!detailId_.empty()&&y<90&&x<188){detailId_.clear();return Action::None;}
    if(y<80){if(x<180)return detailId_.empty()?Action::Back:(back(),Action::None);
        if(x>=w-248&&x<w-80){goals_=true;return Action::None;}
    }
    auto g=geometry(w);
    if(!detailId_.empty()){if(y>g.top+76)offset_=std::max(0,offset_+1);return Action::None;}
    for(int i=0;i<4;++i){
        SDL_Rect rc=g.sidebar?SDL_Rect{16,g.navTop+i*68,208,56}:
                                SDL_Rect{18+i*g.width/4,96,g.width/4-5,56};
        if(ui::contains(rc,x,y)){tab_=i;offset_=0;focus_=i+1;return Action::None;}
    }
    if(tab_==1){
        if(y>=g.top+62&&y<g.top+126){if(x>g.left+g.width-148){monthDelta_+=(x>g.left+g.width-76)?1:-1;selectedDay_=0;}return Action::None;}
        auto d=dateAt(monthDelta_);int year=d.tm_year+1900,month=d.tm_mon,first=(d.tm_wday+6)%7;
        const int cell=std::max(56,(g.width-24)/7),y0=g.top+164;
        if(y>=y0){const int c=(x-g.left-8)/cell,row=(y-y0)/64;
            const int day=row*7+c-first+1;
            if(c>=0&&c<7&&day>=1&&day<=daysInMonth(year,month))selectedDay_=day;
        }
    }
    if(tab_==2){
        const int row=(y-g.top-70)/84+offset_;
        const auto books=journal.books();
        if(y>=g.top+70&&row>=0&&row<int(books.size())){detailId_=books[row].id;offset_=0;}
    }
    if(tab_==3&&y>=g.top+80)offset_=std::max(0,offset_+1);
    return Action::None;
}
void ReadingJournalUI::render(SDL_Renderer *r,Typography &font,const ThemePalette &pal,
                               const ReadingJournal &journal,int w,int h,bool en){
    header(r,font,pal,w,en);nav(r,font,pal,w,tab_,focus_,en);
    const auto g=geometry(w);const int x=g.left,y=g.top,cw=g.width;
    const auto sum=journal.totals();
    const auto cal=journal.calendar();
    if(!detailId_.empty()){
        const auto books=journal.books();
        auto found=std::find_if(books.begin(),books.end(),[&](const JournalBookTotal &b){return b.id==detailId_;});
        if(found==books.end()){detailId_.clear();return;}
        text(r,font,found->title,x,y,pal.fg,cw,true);
        panel(r,pal,{x,y+70,cw,130});
        text(r,font,found->author,x+20,y+90,pal.muted,cw-40);
        text(r,font,duration(found->activeMilliseconds,en)+"  ·  "+std::to_string(found->sessions)+
             label(en," oturum"," sessions"),x+20,y+134,pal.fg,cw-40);
        text(r,font,label(en,"Oturum geçmişi","Session history"),x,y+224,pal.fg,cw);
        int count=0;const int start=y+272;
        for(auto it=journal.sessions().rbegin();it!=journal.sessions().rend();++it){
            if(it->bookId!=detailId_)continue;
            if(count++<offset_)continue;
            int yy=start+(count-offset_-1)*74;if(yy+65>h)break;
            panel(r,pal,{x,yy,cw,64});
            text(r,font,localDate(it->endedUtc),x+16,yy+18,pal.fg,cw/2);
            text(r,font,duration(it->activeMilliseconds,en),x+cw-240,yy+18,pal.muted,215);
        }
    }else if(sum.sessions==0){
        empty(r,font,pal,{x,y,cw,std::min(h-y-24,240)},en);
    }else if(tab_==0){
        text(r,font,label(en,"Okuma özeti","Reading summary"),x,y,pal.fg,cw,true);
        const int wide=cw>=620?(cw*64/100):cw;
        panel(r,pal,{x,y+68,wide,240});
        text(r,font,label(en,"Bu hafta · aktif okuma","This week · active reading"),x+24,y+88,pal.muted,wide-48);
        auto today=nowDate();std::uint64_t week=0,values[7]{};
        for(int k=0;k<7;++k){
            std::tm d=today;d.tm_mday-=6-k;std::mktime(&d);
            values[k]=dayMs(cal,d.tm_year+1900,d.tm_mon+1,d.tm_mday);week+=values[k];
        }
        text(r,font,duration(week,en),x+24,y+126,pal.fg,wide-48,true);
        std::uint64_t maxv=1;for(auto v:values)maxv=std::max(maxv,v);
        const int startX=x+32,base=y+276,step=std::max(25,(wide-64)/7);
        for(int i=0;i<7;++i){
            const int ht=int(76.*double(values[i])/maxv);
            ui::surface(r,{startX+i*step,base-ht,step-12,std::max(3,ht)},
                        i==6?pal.accent:blendColor(pal.bg,pal.accent,35),4);
            text(r,font,std::to_string(i+1),startX+i*step,base+4,pal.muted,step-4);
        }
        if(wide<cw){
            panel(r,pal,{x+wide+16,y+68,cw-wide-16,240});
            text(r,font,label(en,"Günlük hedef","Daily goal"),x+wide+32,y+90,pal.muted,cw-wide-44);
            text(r,font,journal.dailyGoalMinutes()?std::to_string(journal.dailyGoalMinutes())+
                 label(en," dk"," min"):label(en,"Kapalı","Disabled"),x+wide+32,y+140,pal.fg,cw-wide-40,true);
            auto n=dayMs(cal,today.tm_year+1900,today.tm_mon+1,today.tm_mday);
            text(r,font,label(en,"Bugün: ","Today: ")+duration(n,en),x+wide+32,y+206,pal.fg,cw-wide-40);
        }
        const int tileY=y+326,tileW=(cw-36)/3;
        const std::string labels[]={label(en,"Toplam süre","Total time"),label(en,"Oturumlar","Sessions"),label(en,"Okunan gün","Reading days")};
        const std::string valuesText[]={duration(sum.activeMilliseconds,en),std::to_string(sum.sessions),std::to_string(sum.readDays)};
        for(int i=0;i<3;++i){panel(r,pal,{x+i*(tileW+18),tileY,tileW,124});
            text(r,font,labels[i],x+i*(tileW+18)+15,tileY+18,pal.muted,tileW-24);
            text(r,font,valuesText[i],x+i*(tileW+18)+15,tileY+66,pal.fg,tileW-24);}
    }else if(tab_==1){
        text(r,font,label(en,"Okuma takvimim","Reading calendar"),x,y,pal.fg,cw,true);
        panel(r,pal,{x,y+62,cw,h-y-85});
        auto d=dateAt(monthDelta_);
        const char *tr[]={"Ocak","Şubat","Mart","Nisan","Mayıs","Haziran","Temmuz","Ağustos","Eylül","Ekim","Kasım","Aralık"};
        const char *english[]={"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
        text(r,font,std::string(en?english[d.tm_mon]:tr[d.tm_mon])+" "+std::to_string(d.tm_year+1900),x+20,y+80,pal.fg,cw-180);
        ui::button(r,font,{x+cw-148,y+66,64,56},"‹",pal);
        ui::button(r,font,{x+cw-76,y+66,64,56},"›",pal);
        const int first=(d.tm_wday+6)%7,cell=std::max(56,(cw-24)/7);
        const char *daysTr[]={"Pzt","Sal","Çar","Per","Cum","Cmt","Paz"};
        const char *daysEn[]={"Mon","Tue","Wed","Thu","Fri","Sat","Sun"};
        for(int i=0;i<7;++i)text(r,font,en?daysEn[i]:daysTr[i],x+8+i*cell+10,y+132,pal.muted,cell-18);
        const auto now=nowDate();
        for(int k=1;k<=daysInMonth(d.tm_year+1900,d.tm_mon);++k){
            const int z=first+k-1,row=z/7,col=z%7;
            const std::uint64_t ms=dayMs(cal,d.tm_year+1900,d.tm_mon+1,k);
            const int intensity=ms==0?0:ms<1800000?15:ms<3600000?28:ms<5400000?45:66;
            SDL_Rect box{x+8+col*cell,y+164+row*64,cell-7,58};
            ui::surface(r,box,blendColor(pal.card,pal.accent,intensity),10);
            if(k==selectedDay_||(selectedDay_==0&&k==now.tm_mday&&monthDelta_==0))ui::focusRing(r,box,pal);
            text(r,font,std::to_string(k),box.x+10,box.y+5,pal.fg,box.w-18);
            if(ms)text(r,font,duration(ms,en),box.x+10,box.y+33,pal.fg,box.w-18);
        }
        const int detailDay=selectedDay_?selectedDay_:monthDelta_==0?now.tm_mday:1;
        const int detailY=y+164+6*64+4;
        if(detailY+30<h)text(r,font,std::to_string(detailDay)+"  ·  "+
             duration(dayMs(cal,d.tm_year+1900,d.tm_mon+1,detailDay),en),x+20,detailY,pal.muted,cw-40);
    }else if(tab_==2){
        text(r,font,label(en,"Kitaplara göre","By book"),x,y,pal.fg,cw,true);
        const auto items=journal.books();int n=0;
        for(std::size_t i=std::size_t(offset_);i<items.size();++i){
            int rowY=y+70+n++*84;if(rowY+78>h)break;
            panel(r,pal,{x,rowY,cw,76});
            text(r,font,items[i].title,x+18,rowY+9,pal.fg,cw-210);
            text(r,font,items[i].author,x+18,rowY+44,pal.muted,cw-210);
            text(r,font,duration(items[i].activeMilliseconds,en),x+cw-208,rowY+20,pal.accent,190);
        }
    }else{
        text(r,font,label(en,"Okuma geçmişim","Session history"),x,y,pal.fg,cw,true);
        int n=0;
        for(auto it=journal.sessions().rbegin();it!=journal.sessions().rend();++it){
            if(n++<offset_)continue;
            int rowY=y+70+(n-offset_-1)*84;if(rowY+78>h)break;
            panel(r,pal,{x,rowY,cw,76});
            text(r,font,it->title,x+18,rowY+10,pal.fg,cw-220);
            text(r,font,localDate(it->endedUtc),x+18,rowY+46,pal.muted,cw-220);
            text(r,font,duration(it->activeMilliseconds,en),x+cw-210,rowY+18,pal.accent,200);
        }
    }
    if(goals_){
        SDL_SetRenderDrawBlendMode(r,SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(r,0,0,0,144);SDL_Rect all{0,0,w,h};SDL_RenderFillRect(r,&all);
        const SDL_Rect dialog{(w-520)/2,(h-280)/2,520,280};panel(r,pal,dialog);
        text(r,font,label(en,"Okuma hedeflerim","Reading goals"),dialog.x+24,dialog.y+22,pal.fg,470,true);
        text(r,font,duration(journal.dailyGoalMinutes()*60000ULL,en),dialog.x+24,dialog.y+106,pal.muted,468);
        ui::button(r,font,{dialog.x+16,dialog.y+192,150,56},"−5",pal);
        ui::button(r,font,{dialog.x+180,dialog.y+192,150,56},"+5",pal);
        ui::button(r,font,{dialog.x+344,dialog.y+192,160,56},label(en,"Kapat","Close"),pal);
    }
}
void ReadingJournalUI::renderPreview(SDL_Renderer *r,Typography &font,const ThemePalette &pal,
                                     const ReadingJournal &j,const std::string &bookId,int w,int h,bool en){
    SDL_SetRenderDrawBlendMode(r,SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r,0,0,0,140);SDL_Rect cover{0,0,w,h};SDL_RenderFillRect(r,&cover);
    const int pw=std::min(620,w-24),px=(w-pw)/2,py=(h-320)/2;
    panel(r,pal,{px,py,pw,320});
    text(r,font,label(en,"Bu kitabın istatistikleri","This book's statistics"),px+24,py+22,pal.fg,pw-48,true);
    text(r,font,label(en,"Bugün","Today"),px+24,py+108,pal.muted,pw-48);
    const auto today=nowDate(),days=j.calendar();
    text(r,font,duration(dayMs(days,today.tm_year+1900,today.tm_mon+1,today.tm_mday),en),
         px+24,py+142,pal.fg,pw/2);
    text(r,font,label(en,"Bu kitap","This book"),px+pw/2,py+108,pal.muted,pw/2-24);
    text(r,font,duration(j.bookMilliseconds(bookId),en),px+pw/2,py+142,pal.fg,pw/2-24);
    ui::button(r,font,{px+18,py+244,pw/2-28,56},label(en,"Günlüğe git","Open journal"),pal,true);
    ui::button(r,font,{px+pw/2+10,py+244,pw/2-28,56},label(en,"Kapat","Close"),pal);
}
} // namespace papernx
