#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>
namespace papernx {
enum class PdfGesture { Tap, Pan, PagePrevious, PageNext, Pinch, Ignore };
struct PdfGestureSample {
    float dx{0},dy{0},viewportWidth{1280};
    std::uint32_t durationMs{0};
    bool multiTouch{false},uiInteraction{false};
};
struct PdfGestureThresholds {
    float distanceFraction{.12f},minimumDistance{90},dominance{2.4f},velocity{.65f},tapDistance{12};
    std::uint32_t maximumDurationMs{440};
};
// Conservative release classifier. Slow drags keep their pan; only a deliberate
// horizontal flick turns a page. UI and pinch sequences can never turn pages.
inline PdfGesture classifyPdfGesture(const PdfGestureSample &s,const PdfGestureThresholds &t={}) {
    if(s.uiInteraction) return PdfGesture::Ignore;
    if(s.multiTouch) return PdfGesture::Pinch;
    const float x=std::abs(s.dx),y=std::abs(s.dy);
    if(std::hypot(x,y)<=t.tapDistance) return PdfGesture::Tap;
    if(s.durationMs>0 && s.durationMs<=t.maximumDurationMs &&
       x>=std::max(t.minimumDistance,s.viewportWidth*t.distanceFraction) &&
       x>=y*t.dominance && x/s.durationMs>=t.velocity)
        return s.dx<0?PdfGesture::PageNext:PdfGesture::PagePrevious;
    return PdfGesture::Pan;
}
}
