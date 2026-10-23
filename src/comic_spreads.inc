// Comics always use the original one-image viewport, in every orientation.
// In particular, do not inspect adjacent headers, pair images or build a spread
// just because a stored book/PDF landscape preference is enabled.
bool readyComicTurn(int direction,int &target) {
    target=std::clamp(currentPage_+direction,0,std::max(0,pageCount_-1));
    if(target==currentPage_)return true;
    configureComicPages();
    if(px_document_comic_size_known(doc_,target) && pageCache_.get(comicTextureKey(target)))return true;
    comicPages_.request(target,false,true);return false;
}
int comicViewportHeight(int h) const {return std::max(1,h-72-(appSettings_.get().expandedViewport?44:72));}
void drawComicLeaf(int page,SDL_Rect leaf) {
    (void)leaf; // A comic leaf always occupies the complete reader viewport.
    if(auto texture=comicZoomTexture(page)) {
        const auto rect=comicPageRect(page);
        SDL_RenderCopy(renderer_,texture,nullptr,&rect);
    }
}
SDL_Rect zoomTopRect(int w,int h) const {return {w-80,h-136,56,56};}
void returnComicToTop() {
    cancelPageTurn();pdfPanY_=0;panVelocityX_=panVelocityY_=0;panInertiaY_=0;
    zoomFractionY_=0;zoomLastPage_=-1;
    gestureTracker_.reset();dirty_=true;
}
void drawZoomTop(int w,int h,const ThemePalette &pal) {
    const auto r=zoomTopRect(w,h);ui::button(renderer_,*typography_,r,"",pal);
    SDL_SetRenderDrawColor(renderer_,pal.fg.r,pal.fg.g,pal.fg.b,255);
    const int x=r.x+r.w/2,y=r.y+19;
    SDL_RenderDrawLine(renderer_,x-9,y+8,x,y);SDL_RenderDrawLine(renderer_,x,y,x+9,y+8);
    SDL_RenderDrawLine(renderer_,x,y,x,y+22);
}

// Input can start anywhere, but the physical paper is only this image's
// rectangle. The touch path intersects it before beginning the fold.
SDL_Rect comicTurnBounds(int page,bool head=false) {return comicPageRect(page,head);}
bool comicCanGrip(float x,float y) {
    int w,h;getWindowSize(w,h);const auto r=comicTurnBounds(currentPage_);
    const int maxPanX=std::max(0,r.w-fixedViewportWidth(w));
    return x*w<r.x || x*w>=r.x+r.w || y*h<r.y || y*h>=r.y+r.h ||
        x<.12f || x>.88f || (pdfPanX_==0 && x>.5f) || (pdfPanX_>=maxPanX && x<.5f);
}
