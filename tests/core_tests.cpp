#include "papernx/core.hpp"
#include <cassert>
#include <fstream>
#include <iostream>
using namespace papernx;
int main(){
 const auto root=fs::temp_directory_path()/"papernx-test-fixture";
 fs::remove_all(root);fs::create_directories(root/"nested");
 {std::ofstream(root/"İstanbul.epub")<<"fixture";std::ofstream(root/"nested"/"scan.PDF")<<"pdf";std::ofstream(root/"ignored.txt")<<"ignore";}
 assert(detectFormat("book.EPUB")==Format::Epub);
 assert(detectFormat("x.PDF")==Format::Pdf);
 assert(detectFormat("x.txt")==Format::Unsupported);
 const auto found=scan(root);assert(found.size()==2);
 assert(scan(root/"missing").empty());
 assert(search(found,"scan").size()==1);
 assert(documentId("x",100)!=documentId("x",101));
 Progress p;p.page=49;p.pageCount=100;p.fraction=49.0/99.0;p.fontSize=27;p.theme=2;
 assert(restorePage(p,100)==49);assert(restorePage(p,200)==98);
 ProgressStore st(root/"data"/"progress.txt");st.put("0123456789abcdef",p);
 assert(st.save());ProgressStore loaded(root/"data"/"progress.txt");
 assert(loaded.load());assert(loaded.get("0123456789abcdef").page==49);
 assert(loaded.get("0123456789abcdef").theme==2);assert(loaded.save());
 {std::ofstream(root/"data"/"progress.txt",std::ios::trunc)<<"bad";}
 ProgressStore backup(root/"data"/"progress.txt");
 assert(backup.load());assert(backup.get("0123456789abcdef").page==49);
 fs::remove_all(root);std::cout<<"PASS core tests\n";
}
