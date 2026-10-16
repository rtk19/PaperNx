#include "papernx/core.hpp"
#include "papernx/mupdf_bridge.h"
#include <SDL.h>
#include <SDL_ttf.h>
#include <algorithm>
#include <chrono>
#include <filesystem>
#include <string>
#include <vector>
using namespace papernx;
namespace {
struct Palette {SDL_Color bg,fg,muted,accent;};
const Palette palettes[]={
 {{247,244,236,255},{36,36,36,255},{120,115,105,255},{110,90,66,255}},
 {{255,253,245,255},{48,43,37,255},{130,120,108,255},{114,83,49,255}},
 {{232,217,186,255},{68,54,41,255},{121,98,79,255},{114,80,51,255}},
 {{217,222,225,255},{39,49,55,255},{109,119,127,255},{55,91,120,255}},
 {{23,25,27,255},{232,231,226,255},{165,164,158,255},{193,166,132,255}},
 {{0,0,0,255},{231,231,231,255},{148,148,148,255},{203,183,146,255}}
};
void fill(SDL_Renderer *r,SDL_Rect rect,SDL_Color c){SDL_SetRenderDrawColor(r,c.r,c.g,c.b,c.a);SDL_RenderFillRect(r,&rect);}
TTF_Font *loadFont(const fs::path&root,int size){
 std::vector<std::string> names={(root/"fonts"/"NotoSans-Regular.ttf").string(),(root/"fonts"/"DejaVuSans.ttf").string()};
#ifndef __SWITCH__
 names.push_back("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf");
 names.push_back("/usr/share/fonts/truetype/liberation2/LiberationSans-Regular.ttf");
#endif
 for(const auto&f:names)if(auto *font=TTF_OpenFont(f.c_str(),size))return font;
 return nullptr;
}
void text(SDL_Renderer*r,TTF_Font*f,std::string value,int x,int y,SDL_Color color,int limit=0){
 if(!f||value.empty())return;
 if(limit>0){int w=0;while(!value.empty()&&TTF_SizeUTF8(f,value.c_str(),&w,nullptr)==0&&w>limit)value.pop_back();}
 if(value.empty())return;
 SDL_Surface*s=TTF_RenderUTF8_Blended(f,value.c_str(),color);if(!s)return;
 SDL_Texture*t=SDL_CreateTextureFromSurface(r,s);
 if(t){SDL_Rect rect{x,y,s->w,s->h};SDL_RenderCopy(r,t,nullptr,&rect);SDL_DestroyTexture(t);}
 SDL_FreeSurface(s);
}
struct App {
 fs::path root;std::vector<Book> books;ProgressStore progress;
 SDL_Window*window{};SDL_Renderer*renderer{};TTF_Font*font{};TTF_Font*small{};
 PxDocument*doc{};SDL_Texture*pageTexture{};
 std::string currentId,query,error;
 int selected{},offset{},page{},pages{},fontSize{22},theme{},imageW{},imageH{};
 bool reading{},toolbar{},searching{},dirty{};
 std::chrono::steady_clock::time_point changed=std::chrono::steady_clock::now();
 explicit App(fs::path rootPath):root(std::move(rootPath)),progress(root/"data"/"progress.txt"){}
 ~App(){closeBook();if(font)TTF_CloseFont(font);if(small)TTF_CloseFont(small);if(renderer)SDL_DestroyRenderer(renderer);if(window)SDL_DestroyWindow(window);}
 void dimensions(int&w,int&h){SDL_GetRendererOutputSize(renderer,&w,&h);}
 bool init(){
  std::error_code ec;fs::create_directories(root/"books",ec);progress.load();
  books=scan(root/"books");
  window=SDL_CreateWindow("PaperNX",SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,1280,720,SDL_WINDOW_SHOWN|SDL_WINDOW_RESIZABLE);
  if(!window)return false;
  renderer=SDL_CreateRenderer(window,-1,SDL_RENDERER_ACCELERATED|SDL_RENDERER_PRESENTVSYNC);
  if(!renderer)return false;
  font=loadFont(root,24);small=loadFont(root,17);
  return true;
 }
 void savePosition(){
  if(!reading)return;auto p=progress.get(currentId);
  p.page=page;p.pageCount=pages;p.fraction=pages>1?(double)page/(pages-1):0;
  p.fontSize=fontSize;p.theme=theme;progress.put(currentId,p);
  dirty=true;changed=std::chrono::steady_clock::now();
 }
 void closeBook(){
  if(reading)savePosition();
  if(dirty){progress.save();dirty=false;}
  if(pageTexture){SDL_DestroyTexture(pageTexture);pageTexture=nullptr;}
  if(doc){px_document_close(doc);doc=nullptr;}
  reading=false;toolbar=false;
 }
 void renderPage(){
  if(pageTexture){SDL_DestroyTexture(pageTexture);pageTexture=nullptr;}
  if(!doc)return;
  int w=1280,h=720;dimensions(w,h);
  PxImage image{};char err[256]={};
  if(!px_document_render(doc,page,std::min(2048,w-100),std::min(2048,h-80),&image,err,sizeof(err))){error=err;return;}
  pageTexture=SDL_CreateTexture(renderer,SDL_PIXELFORMAT_RGB24,SDL_TEXTUREACCESS_STATIC,image.width,image.height);
  if(pageTexture)SDL_UpdateTexture(pageTexture,nullptr,image.rgb,image.stride);
  imageW=image.width;imageH=image.height;px_image_free(&image);error.clear();
 }
 void openBook(int idx){
  if(idx<0||idx>=(int)books.size())return;
  closeBook();int w=1280,h=720;dimensions(w,h);
  const auto&p=books[idx];auto state=progress.get(p.id);
  char err[256]={};theme=state.theme;fontSize=state.fontSize;
  doc=px_document_open(p.path.c_str(),w-100,h-120,(float)fontSize,"body {line-height:1.4;}",err,sizeof(err));
  if(!doc){error=err;return;}
  currentId=p.id;selected=idx;pages=px_document_pages(doc);page=restorePage(state,pages);
  reading=true;toolbar=false;renderPage();
 }
 void turn(int delta){
  if(!reading)return;const int target=std::clamp(page+delta,0,std::max(0,pages-1));
  if(page!=target){page=target;renderPage();savePosition();}
 }
 void resizeFont(int delta){
  if(!reading||!px_document_is_epub(doc))return;
  const int target=std::clamp(fontSize+delta,12,42);if(fontSize==target)return;
  Progress previous;previous.page=page;previous.pageCount=pages;
  previous.fraction=pages>1?(double)page/(pages-1):0.0;
  int w=1280,h=720;dimensions(w,h);char err[256]={};
  if(px_document_relayout(doc,w-100,h-120,(float)target,"body {line-height:1.4;}",err,sizeof(err))){
   fontSize=target;pages=px_document_pages(doc);page=restorePage(previous,pages);renderPage();savePosition();
  }else error=err;
 }
 void navigate(int delta){
  if(reading){turn(delta);return;}
  if(books.empty())return;
  selected=std::clamp(selected+delta,0,(int)books.size()-1);
  int w=1280,h=720;dimensions(w,h);const int cols=std::max(1,(w-80)/190);
  if(selected/cols<offset)offset=selected/cols;
  if(selected/cols>=offset+3)offset=selected/cols-2;
 }
 void draw(){
  int w=1280,h=720;dimensions(w,h);const auto &pal=palettes[theme];
  fill(renderer,{0,0,w,h},pal.bg);
  if(reading){
   if(pageTexture){SDL_Rect box{(w-imageW)/2,(h-imageH)/2,imageW,imageH};SDL_RenderCopy(renderer,pageTexture,nullptr,&box);}
   if(toolbar){
    fill(renderer,{0,0,w,68},pal.bg);
    text(renderer,font,"PaperNX",45,18,pal.fg);
    text(renderer,small,books[selected].title,240,24,pal.fg,w-520);
    fill(renderer,{0,h-54,w,54},pal.bg);
    text(renderer,small,"Page "+std::to_string(page+1)+" / "+std::to_string(pages),45,h-41,pal.fg);
    fill(renderer,{45,h-10,(w-90)*page/std::max(1,pages-1),4},pal.accent);
   }
  }else{
   text(renderer,font,"PaperNX",48,24,pal.fg);
   text(renderer,small,"My Library",48,70,pal.muted);
   text(renderer,small,"A Open  B Back  X Search  T Theme  R Rescan",w-480,42,pal.muted);
   if(searching)text(renderer,small,"Search: "+query,48,104,pal.accent);
   const auto filtered=search(books,query);
   const int cols=std::max(1,(w-80)/190),top=searching?155:127;
   for(int i=0;i<(int)filtered.size();++i){
    int row=i/cols-offset;if(row<0||row>2)continue;
    int x=48+(i%cols)*190,y=top+row*195;
    auto found=std::find_if(books.begin(),books.end(),[&](const Book&b){return b.id==filtered[i].id;});
    bool active=(int)(found-books.begin())==selected;
    fill(renderer,{x,y,145,170},active?pal.accent:pal.muted);
    fill(renderer,{x+4,y+4,137,162},pal.bg);
    text(renderer,small,filtered[i].format==Format::Pdf?"PDF":"EPUB",x+12,y+15,pal.muted);
    text(renderer,small,filtered[i].title,x+12,y+62,pal.fg,125);
    auto state=progress.get(filtered[i].id);
    fill(renderer,{x,y+166,(int)(145*state.fraction),4},pal.accent);
    text(renderer,small,filtered[i].title,x,y+175,pal.fg,145);
   }
   if(books.empty()){
    text(renderer,font,"Copy EPUB/PDF books into",65,h/2-30,pal.fg);
    text(renderer,small,(root/"books").string(),65,h/2+12,pal.muted,w-100);
   }
  }
  if(!error.empty())text(renderer,small,error,40,h-25,pal.accent,w-80);
  SDL_RenderPresent(renderer);
 }
 void touch(float x,float y){
  if(reading){if(x<.22f)turn(-1);else if(x>.78f)turn(1);else toolbar=!toolbar;return;}
  int w=1280,h=720;dimensions(w,h);const int cols=std::max(1,(w-80)/190);
  if(y<.15f)return;
  const int col=std::clamp((int)((x*w-48)/190),0,cols-1);
  const int row=std::max(0,(int)((y*h-(searching?155:127))/195));
  const auto filtered=search(books,query);const int idx=(row+offset)*cols+col;
  if(idx<0||idx>=(int)filtered.size())return;
  auto it=std::find_if(books.begin(),books.end(),[&](const Book&b){return b.id==filtered[idx].id;});
  if(it!=books.end())openBook((int)(it-books.begin()));
 }
 void event(const SDL_Event&e,bool &running){
  if(e.type==SDL_QUIT){running=false;return;}
  if(e.type==SDL_TEXTINPUT&&searching){query+=e.text.text;offset=0;selected=0;return;}
  if(e.type==SDL_FINGERDOWN){touch(e.tfinger.x,e.tfinger.y);return;}
  if(e.type==SDL_MOUSEBUTTONDOWN){int w=1280,h=720;dimensions(w,h);touch((float)e.button.x/w,(float)e.button.y/h);return;}
  if(e.type!=SDL_KEYDOWN&&e.type!=SDL_CONTROLLERBUTTONDOWN)return;
  bool pad=e.type==SDL_CONTROLLERBUTTONDOWN;int key=pad?e.cbutton.button:e.key.keysym.sym;
  auto hit=[&](int k,int p){return pad?key==p:key==k;};
  if(hit(SDLK_ESCAPE,SDL_CONTROLLER_BUTTON_B)){
   if(searching){searching=false;SDL_StopTextInput();query.clear();}
   else if(reading)closeBook();else running=false;
  }else if(hit(SDLK_RETURN,SDL_CONTROLLER_BUTTON_A)){if(reading)toolbar=!toolbar;else openBook(selected);}
  else if(hit(SDLK_RIGHT,SDL_CONTROLLER_BUTTON_DPAD_RIGHT)||hit(SDLK_PAGEDOWN,SDL_CONTROLLER_BUTTON_RIGHTSHOULDER))navigate(1);
  else if(hit(SDLK_LEFT,SDL_CONTROLLER_BUTTON_DPAD_LEFT)||hit(SDLK_PAGEUP,SDL_CONTROLLER_BUTTON_LEFTSHOULDER))navigate(-1);
  else if(hit(SDLK_DOWN,SDL_CONTROLLER_BUTTON_DPAD_DOWN)){int w=1280,h=720;dimensions(w,h);navigate(std::max(1,(w-80)/190));}
  else if(hit(SDLK_UP,SDL_CONTROLLER_BUTTON_DPAD_UP)){int w=1280,h=720;dimensions(w,h);navigate(-std::max(1,(w-80)/190));}
  else if(!pad&&key==SDLK_t){theme=(theme+1)%6;if(reading)savePosition();}
  else if(reading&&hit(SDLK_EQUALS,SDL_CONTROLLER_BUTTON_X))resizeFont(2);
  else if(reading&&hit(SDLK_MINUS,SDL_CONTROLLER_BUTTON_Y))resizeFont(-2);
  else if(!reading&&hit(SDLK_x,SDL_CONTROLLER_BUTTON_X)){searching=true;SDL_StartTextInput();}
  else if(!pad&&key==SDLK_BACKSPACE&&searching&&!query.empty())query.pop_back();
  else if(!pad&&key==SDLK_r&&!reading){books=scan(root/"books");selected=0;offset=0;}
 }
 void run(){
  bool running=true;
  while(running){
   SDL_Event e;
   if(SDL_WaitEventTimeout(&e,150)){event(e,running);while(SDL_PollEvent(&e))event(e,running);}
   if(dirty&&std::chrono::steady_clock::now()-changed>std::chrono::seconds(5)){progress.save();dirty=false;}
   if(running)draw();
  }
  closeBook();
 }
};
}
int main(int argc,char **argv){
 if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_GAMECONTROLLER|SDL_INIT_EVENTS)!=0)return 1;
 if(TTF_Init()!=0){SDL_Quit();return 1;}
#ifdef __SWITCH__
 fs::path root="sdmc:/switch/PaperNX";
#else
 fs::path root=argc>1?argv[1]:"./PaperNX-data";
#endif
 {App app(root);if(app.init())app.run();}
 TTF_Quit();SDL_Quit();return 0;
}
