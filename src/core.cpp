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
std::string asciiLower(std::string s) {
 for(char &c:s) if(c>='A'&&c<='Z') c=static_cast<char>(c-'A'+'a');
 return s;
}
std::uint64_t fnv1a(const std::string &s) {
 std::uint64_t h=14695981039346656037ull;
 for(unsigned char c:s){h^=c;h*=1099511628211ull;}
 return h;
}
std::string hex(std::uint64_t h) {
 std::ostringstream out;out<<std::hex<<std::setw(16)<<std::setfill('0')<<h;return out.str();
}
std::string payload(const std::map<std::string,Progress>& records) {
 std::ostringstream out;out<<"PAPERNX-1\n";
 for(const auto &[id,p]:records){
  if(id.size()>128 || id.find_first_of("\t\r\n")!=std::string::npos)continue;
  out<<id<<'\t'<<p.page<<'\t'<<p.pageCount<<'\t'<<std::setprecision(17)<<p.fraction
   <<'\t'<<p.fontSize<<'\t'<<p.theme<<'\t'<<(p.favorite?1:0)<<'\n';
 }
 return out.str();
}
}
Format detectFormat(const fs::path &path) {
 const auto ext=asciiLower(path.extension().string());
 if(ext==".epub")return Format::Epub;
 if(ext==".pdf")return Format::Pdf;
 return Format::Unsupported;
}
std::string defaultTitle(const fs::path &path) {
 std::string title=path.stem().string();
 for(char &c:title)if(c=='_')c=' ';
 return title.empty()?"Untitled":title;
}
std::string documentId(const std::string &path,std::uintmax_t size) {
 return hex(fnv1a(path+"\n"+std::to_string(size)));
}
std::vector<Book> scan(const fs::path& root,std::size_t maxBooks) {
 std::vector<Book> books;std::error_code ec;
 if(!fs::is_directory(root,ec))return books;
 fs::recursive_directory_iterator it(root,fs::directory_options::skip_permission_denied,ec),end;
 for(;it!=end&&books.size()<maxBooks;it.increment(ec)){
  if(ec){ec.clear();continue;}
  if(it.depth()>16){it.disable_recursion_pending();continue;}
  if(it->is_symlink(ec)){it.disable_recursion_pending();continue;}
  if(!it->is_regular_file(ec))continue;
  const auto path=it->path(), type=detectFormat(path);
  if(type==Format::Unsupported)continue;
  auto size=it->file_size(ec);if(ec){ec.clear();continue;}
  Book b;b.path=path.string();b.bytes=size;b.id=documentId(b.path,size);
  b.title=defaultTitle(path);b.format=type;
  const auto mtime=it->last_write_time(ec);
  if(!ec)b.modified=std::chrono::duration_cast<std::chrono::seconds>(mtime.time_since_epoch()).count();
  ec.clear();books.push_back(std::move(b));
 }
 std::sort(books.begin(),books.end(),[](const Book&a,const Book&b){return a.title<b.title;});
 return books;
}
std::vector<Book> search(const std::vector<Book>&books,const std::string&needle,Sort sort){
 std::vector<Book> result;const auto q=asciiLower(needle);
 for(const auto&b:books){
  if(asciiLower(b.title).find(q)!=std::string::npos||asciiLower(b.author).find(q)!=std::string::npos)result.push_back(b);
 }
 std::sort(result.begin(),result.end(),[sort](const Book&a,const Book&b){
  if(sort==Sort::Modified)return a.modified==b.modified?a.id<b.id:a.modified>b.modified;
  auto l=asciiLower(sort==Sort::Filename?fs::path(a.path).filename().string():a.title);
  auto r=asciiLower(sort==Sort::Filename?fs::path(b.path).filename().string():b.title);
  return l==r?a.id<b.id:l<r;
 });
 return result;
}
int restorePage(const Progress&p,int n){
 if(n<=0)return 0;
 if(p.pageCount==n)return std::clamp(p.page,0,n-1);
 const double f=std::isfinite(p.fraction)?std::clamp(p.fraction,0.0,1.0):0.0;
 return std::clamp(static_cast<int>(std::round(f*(n-1))),0,n-1);
}
ProgressStore::ProgressStore(fs::path p):path_(std::move(p)){}
Progress ProgressStore::get(const std::string&id)const{auto it=records_.find(id);return it==records_.end()?Progress{}:it->second;}
void ProgressStore::put(const std::string&id,Progress p){
 p.page=std::max(0,p.page);p.pageCount=std::max(0,p.pageCount);
 p.fraction=std::isfinite(p.fraction)?std::clamp(p.fraction,0.0,1.0):0.0;
 p.fontSize=std::clamp(p.fontSize,12,42);p.theme=std::clamp(p.theme,0,5);
 records_[id]=p;
}
std::size_t ProgressStore::count()const{return records_.size();}
bool ProgressStore::load(){
 auto readFile=[&](const fs::path&candidate)->bool{
  std::ifstream in(candidate,std::ios::binary);if(!in)return false;
  std::string encoded((std::istreambuf_iterator<char>(in)),{});
  if(encoded.size()<35||encoded.size()>2*1024*1024)return false;
  const auto split=encoded.find('\n');
  if(split==std::string::npos||encoded.substr(0,split)!="CHECKSUM="+hex(fnv1a(encoded.substr(split+1))))return false;
  std::istringstream rows(encoded.substr(split+1));std::string line;
  if(!std::getline(rows,line)||line!="PAPERNX-1")return false;
  std::map<std::string,Progress> restored;
  while(std::getline(rows,line)){
   if(line.empty())continue;
   std::replace(line.begin(),line.end(),'\t',' ');
   std::istringstream row(line);std::string id;Progress p;int fav=0;
   if(!(row>>id>>p.page>>p.pageCount>>p.fraction>>p.fontSize>>p.theme>>fav))return false;
   if(id.size()!=16||p.page<0||p.pageCount<0||!std::isfinite(p.fraction))return false;
   p.favorite=fav!=0;restored[id]=p;
   if(restored.size()>10000)return false;
  }
  records_.swap(restored);return true;
 };
 if(readFile(path_))return true;
 fs::path backup=path_;backup+=".bak";return readFile(backup);
}
bool ProgressStore::save()const{
 std::error_code ec;fs::create_directories(path_.parent_path(),ec);if(ec)return false;
 const auto data=payload(records_);fs::path temp=path_;temp+=".tmp";
 {
  std::ofstream out(temp,std::ios::binary|std::ios::trunc);if(!out)return false;
  out<<"CHECKSUM="<<hex(fnv1a(data))<<'\n'<<data;out.flush();if(!out.good())return false;
 }
 fs::path backup=path_;backup+=".bak";
 if(fs::exists(path_,ec)){fs::remove(backup,ec);ec.clear();fs::rename(path_,backup,ec);if(ec)return false;}
 fs::rename(temp,path_,ec);
 if(ec){ec.clear();if(fs::exists(backup,ec)){ec.clear();fs::rename(backup,path_,ec);}return false;}
 return true;
}
}
