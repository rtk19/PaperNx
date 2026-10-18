#pragma once
#include <algorithm>
#include <cstdint>
namespace papernx {
// Deterministic non-repeating subtle grain. Only exact paper pixels change; text,
// antialiasing and illustrations retain all their original sample values.
inline void applyPaperTexture(unsigned char *rgb,int width,int height,int stride,int background,int mode,int originY=0) {
    if(!rgb || width<1 || height<1 || width>32768 || height>32768 || stride<width*3 || mode<1 || mode>2)return;
    const int red=(background>>16)&255,green=(background>>8)&255,blue=background&255;
    if(red+green+blue<180)return; // OLED and night canvases remain clear.
    for(int y=0;y<height;++y)for(int x=0;x<width;++x) {
        auto *pixel=rgb+y*stride+x*3;
        if(pixel[0]!=red || pixel[1]!=green || pixel[2]!=blue)continue;
        std::uint32_t hash=std::uint32_t(x)*0x9e3779b9u ^ std::uint32_t(y+originY)*0x85ebca6bu;
        hash^=hash>>16;hash*=0x7feb352du;hash^=hash>>15;
        int shift=int(hash%5)-2;if(mode==1)shift=shift>0?1:shift<0?-1:0;
        for(int channel=0;channel<3;++channel)pixel[channel]=static_cast<unsigned char>(std::clamp(int(pixel[channel])+shift,0,255));
    }
}
}
