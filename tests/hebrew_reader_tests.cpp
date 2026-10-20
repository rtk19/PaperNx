#include "papernx/mupdf_bridge.h"
#include "papernx/theme.hpp"
#include <cassert>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <vector>

// Observe the app's actual rendered pixels, including its CSS and bridge.
static void check(const char *path,int align,int height,int indent,bool publisher=false) {
    papernx::ReaderPreferences style;
    style.fontSize=24; style.indentation=indent;style.paragraphSpacing=100;style.textAlign=align;
    style.backgroundColor=0xffffff;style.textColor=0;
    auto css=publisher?std::string{}:papernx::generateEpubCss(style); char error[256]{},value[1024]{};
    auto *doc=px_document_open(path,600,height,24,css.c_str(),error,sizeof error);
    if(!doc) std::cerr<<error<<'\n';
    assert(doc);
    px_document_metadata(doc,"info:Title",value,sizeof value); assert(std::strstr(value,u8"ספר"));
    px_document_metadata(doc,"info:Author",value,sizeof value); assert(std::strstr(value,u8"ישראלי"));
    PxImage image{}; assert(px_document_render(doc,0,600,1000,&image,error,sizeof error));
    struct Band {int minx,maxx;}; std::vector<Band> bands; bool active=false; int emptyRows=0;
    for(int y=0;y<image.height;++y) {
        int minx=image.width,maxx=-1;
        for(int x=0;x<image.width;++x) {
            auto *pixel=image.rgb+y*image.stride+x*3;
            if(pixel[0]<100 && pixel[1]<100 && pixel[2]<100) {minx=std::min(minx,x);maxx=x;}
        }
        if(maxx<0) {if(++emptyRows>4) active=false;continue;}
        emptyRows=0;
        if(!active) bands.push_back({minx,maxx});
        else {bands.back().minx=std::min(bands.back().minx,minx);bands.back().maxx=std::max(bands.back().maxx,maxx);}
        active=true;
    }
    assert(bands.size()>=4);
    for(std::size_t i=0;i<4;++i) std::cout<<i<<": "<<bands[i].minx<<","<<bands[i].maxx<<'\n';
    assert(bands[0].minx>image.width/2 && bands[0].maxx>image.width-110); // Automatic Hebrew.
    assert(bands[1].minx<110 && bands[1].maxx<image.width/2); // Automatic English.
    assert(bands[2].minx<110 && bands[2].maxx<image.width/2); // Explicit dir=ltr.
    assert(bands[3].minx>image.width/2 && bands[3].maxx>image.width-110); // Explicit dir=rtl.
    if(indent) {
        float scale=image.width/600.f;
        assert((image.width-bands[0].maxx)/scale>68);
        assert(bands[1].minx/scale>68);
    }
    if(align==0 && height>0 && !indent && !publisher) {
        std::filesystem::create_directories("verification/hebrew");
        auto *out=std::fopen("verification/hebrew/reader.ppm","wb"); assert(out);
        std::fprintf(out,"P6\n%d %d\n255\n",image.width,image.height);
        for(int y=0;y<image.height;++y) std::fwrite(image.rgb+y*image.stride,1,image.width*3,out);
        std::fclose(out);
    }
    auto bookmark=px_document_make_position_bookmark(doc,0,100);
    style.fontSize=28; css=papernx::generateEpubCss(style);
    assert(px_document_relayout(doc,520,height,28,css.c_str(),error,sizeof error));
    int page=0;float y=0;assert(px_document_lookup_position_bookmark(doc,bookmark,&page,&y));
    px_document_metadata(doc,"info:ReadingDirection",value,sizeof value); assert(std::strcmp(value,"rtl")==0);
    px_image_free(&image);px_document_close(doc);
}
int main() {
    for(int align:{0,1}) for(int height:{720,0}) for(int indent:{0,120}) check("tests/fixtures/hebrew_auto.epub",align,height,indent);
    check("tests/fixtures/hebrew_publisher.epub",0,720,0,true);
    char error[256],value[64];
    for(auto entry:{std::pair<const char *,const char *>{"hebrew_spine_ltr.epub","ltr"},{"hebrew_spine_rtl.epub","rtl"},{"hebrew_legacy.epub","rtl"},{"sample_book.epub","ltr"}}) {
        std::string path="tests/fixtures/"+std::string(entry.first);
        auto *doc=px_document_open(path.c_str(),600,720,24,"",error,sizeof error);assert(doc);
        px_document_metadata(doc,"info:ReadingDirection",value,sizeof value);assert(std::strcmp(value,entry.second)==0);
        px_document_close(doc);
    }
    std::cout<<"PASS: automatic per-paragraph Hebrew/English, explicit direction, justified and start alignment, reflow anchors, continuous layout, EPUB language/spine metadata\n";
}
