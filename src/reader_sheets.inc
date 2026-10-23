// Book folios belong to the paper. Comic counters are independent fixed chrome
// and must never enter any of the animated image snapshots.
void drawReaderLeaf(int page,SDL_Rect leaf) {
    if(page<0 || page>=pageCount_)return; // Blank facing leaf at the end.
    auto texture=comic()?nullptr:getOrRenderPageTexture(page);
    if(comic())drawComicLeaf(page,leaf);
    if(texture) {
        int tw=0,th=0;SDL_QueryTexture(texture,nullptr,nullptr,&tw,&th);
        const auto layout=readerLayout();
        const int inset=layout.contentInset();
        SDL_Rect content{leaf.x+inset,leaf.y+ReaderLayout::top,
            std::max(1,leaf.w-inset*2),layout.contentHeight()};
        SDL_Rect destination{content.x+(content.w-tw)/2,content.y+(content.h-th)/2,tw,th};
        SDL_Rect oldClip{};const bool clipped=SDL_RenderIsClipEnabled(renderer_);
        SDL_RenderGetClipRect(renderer_,&oldClip);
        SDL_RenderSetClipRect(renderer_,&content);
        SDL_RenderCopy(renderer_,texture,nullptr,&destination);
        SDL_RenderSetClipRect(renderer_,clipped?&oldClip:nullptr);
    }
    if(!comic())drawReaderCaption(std::to_string(page+1),
        {leaf.x+16,leaf.y+leaf.h-readerLayout().footerHeight(),std::max(1,leaf.w-32),readerLayout().footerHeight()},palette());
}

int readerSidePage(int base,int side) const {
    return base+(readingDirection()<0?1-side:side);
}
void drawReaderSheets(int page) {
    auto layout=readerLayout();
    if(!layout.spread){drawReaderLeaf(page,layout.leaf(0));return;}
    const int base=page/2*2;
    for(int side=0;side<2;++side)drawReaderLeaf(readerSidePage(base,side),layout.leaf(side));
}

template<class Draw>
PageTurnTexture captureReaderSurface(int width,int height,Draw draw,bool transparent=false) {
    PageTurnTexture result{nullptr,SDL_DestroyTexture};
    if(!SDL_RenderTargetSupported(renderer_))return result;
    result.reset(SDL_CreateTexture(renderer_,SDL_PIXELFORMAT_RGBA8888,SDL_TEXTUREACCESS_TARGET,width,height));
    if(!result)return result;
    auto *oldTarget=SDL_GetRenderTarget(renderer_);
    SDL_Rect viewport{},clip{};SDL_RenderGetViewport(renderer_,&viewport);SDL_RenderGetClipRect(renderer_,&clip);
    const bool clipped=SDL_RenderIsClipEnabled(renderer_);
    float sx=1,sy=1;SDL_RenderGetScale(renderer_,&sx,&sy);
    Uint8 r,g,b,a;SDL_GetRenderDrawColor(renderer_,&r,&g,&b,&a);
    if(SDL_SetRenderTarget(renderer_,result.get())!=0){result.reset();return result;}
    SDL_RenderSetScale(renderer_,1,1);SDL_RenderSetViewport(renderer_,nullptr);SDL_RenderSetClipRect(renderer_,nullptr);
    const auto paper=palette().bg;SDL_SetRenderDrawColor(renderer_,paper.r,paper.g,paper.b,transparent?0:255);
    SDL_RenderClear(renderer_);draw();
    SDL_SetRenderTarget(renderer_,oldTarget);SDL_RenderSetScale(renderer_,sx,sy);
    SDL_RenderSetViewport(renderer_,&viewport);SDL_RenderSetClipRect(renderer_,clipped?&clip:nullptr);
    SDL_SetRenderDrawColor(renderer_,r,g,b,a);
    if(transparent)SDL_SetTextureBlendMode(result.get(),SDL_BLENDMODE_BLEND);
    return result;
}

bool prepareComicTurn(int from,int target) {
    auto &turn=pageTurnAnimation_;
    turn.sourcePage=from;
    turn.comicSheet=true;turn.spread=false;
    turn.viewport=comicTurnBounds(from);turn.nextViewport=comicTurnBounds(target,true);
    // Use the book curl's front/reverse/next model. An explicit paper reverse
    // avoids bending a second full-strength mirrored copy of the comic image.
    turn.previous=captureTurnPage(from,false,turn.viewport);
    turn.back=turn.previous?captureTurnPage(from,true,turn.viewport):PageTurnTexture{nullptr,SDL_DestroyTexture};
    turn.next=captureTurnPage(target,false,turn.nextViewport);
    return turn.previous && turn.back && turn.next;
}

bool prepareReaderTurn(int from,int target,int direction) {
    if(comic())return prepareComicTurn(from,target);
    auto &turn=pageTurnAnimation_;
    const auto layout=readerLayout();
    turn.spread=layout.spread;
    if(layout.spread) {
        const int side=direction>0?1:0;
        const int oldBase=from/2*2;
        const int newBase=target/2*2;
        const auto leaf=layout.leaf(side),opposite=layout.leaf(1-side);
        turn.previous=captureReaderSurface(leaf.w,leaf.h,[&]{drawReaderLeaf(readerSidePage(oldBase,side),{0,0,leaf.w,leaf.h});});
        turn.back=captureReaderSurface(opposite.w,opposite.h,[&]{drawReaderLeaf(readerSidePage(newBase,1-side),{0,0,opposite.w,opposite.h});});
        turn.under=captureReaderSurface(layout.width,layout.height,[&]{
            drawReaderLeaf(readerSidePage(oldBase,1-side),opposite);
            drawReaderLeaf(readerSidePage(newBase,side),leaf);
        });
        turn.viewport=leaf;
        return turn.previous && turn.back && turn.under;
    }
    turn.viewport=turnCanvas(target);
    turn.comicSheet=comic() && std::abs(pdfZoom_-1.f)>.01f;
    turn.nextViewport=turn.comicSheet?comicPageRect(target,true):turn.viewport;
    turn.previous=captureTurnPage(from,false,turn.viewport);
    turn.back=turn.previous?captureTurnPage(from,true,turn.viewport):PageTurnTexture{nullptr,SDL_DestroyTexture};
    turn.next=turn.previous?captureTurnPage(target,false,turn.viewport):PageTurnTexture{nullptr,SDL_DestroyTexture};
    return turn.previous && turn.next;
}
