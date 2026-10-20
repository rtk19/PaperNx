// Included inside PaperNxApp. Integrate held controls once per frame; SDL does
// not generate repeat events for controller buttons or stationary stick axes.
int stickAxes_[4]{};
unsigned heldKeyboardPan_{0},heldPadPan_{0};
Uint32 controllerInputTick_{0},rightPageTick_{0};
float controllerRemainderX_{0},controllerRemainderY_{0};
bool zoomPanContext() const {
    return doc_ && screen_==ScreenState::Reader && !quickMenu_ &&
        catalogPanel_==CatalogPanel::None && choiceKind_<0 && colorEditing_<0 &&
        !continuous() && !px_document_is_epub(doc_) && pdfZoom_>1.001f;
}
static float stickAmount(int value) {
    constexpr int dead=8000;
    if(std::abs(value)<=dead) return 0;
    return (value<0?-1.f:1.f)*std::min(1.f,(std::abs(value)-dead)/float(32767-dead));
}
bool controllerMotionActive() const {
    return heldKeyboardPan_ || heldPadPan_ || std::abs(stickAxes_[2])>8000 || std::abs(stickAxes_[3])>8000;
}
void clearHeldControls() {
    heldKeyboardPan_=heldPadPan_=0;
    for(auto &axis:stickAxes_)axis=0;
    controllerRemainderX_=controllerRemainderY_=0;controllerInputTick_=SDL_GetTicks();
}
void panReaderBy(float dx,float dy) {
    if(!doc_ || px_document_is_epub(doc_) || pageTurnAnimation_.active)return;
    panVelocityX_=panVelocityY_=0;
    controllerRemainderX_+=dx;controllerRemainderY_+=dy;
    const int x=int(controllerRemainderX_),y=int(controllerRemainderY_);
    controllerRemainderX_-=x;controllerRemainderY_-=y;
    const int oldX=pdfPanX_,oldY=pdfPanY_;
    pdfPanX_+=x;pdfPanY_+=y;clampPdfPan();
    if(pdfPanX_!=oldX+x)controllerRemainderX_=0;
    if(pdfPanY_!=oldY+y)controllerRemainderY_=0;
    if(pdfPanX_!=oldX || pdfPanY_!=oldY)dirty_=true;
}
bool trackHeldControl(const SDL_Event &event) {
    if(event.type==SDL_CONTROLLERAXISMOTION && event.caxis.axis<=SDL_CONTROLLER_AXIS_RIGHTY) {
        stickAxes_[event.caxis.axis]=event.caxis.value;
        return event.caxis.axis>=SDL_CONTROLLER_AXIS_RIGHTX || zoomPanContext();
    }
    const bool key=event.type==SDL_KEYDOWN || event.type==SDL_KEYUP;
    const bool pad=event.type==SDL_CONTROLLERBUTTONDOWN || event.type==SDL_CONTROLLERBUTTONUP;
    if(!key && !pad)return false;
    const bool down=event.type==SDL_KEYDOWN || event.type==SDL_CONTROLLERBUTTONDOWN;
    const int value=key?event.key.keysym.sym:event.cbutton.button;
    const int left=key?int(SDLK_LEFT):int(SDL_CONTROLLER_BUTTON_DPAD_LEFT),right=key?int(SDLK_RIGHT):int(SDL_CONTROLLER_BUTTON_DPAD_RIGHT);
    const int up=key?int(SDLK_UP):int(SDL_CONTROLLER_BUTTON_DPAD_UP),bottom=key?int(SDLK_DOWN):int(SDL_CONTROLLER_BUTTON_DPAD_DOWN);
    unsigned bit=value==left?1u:value==right?2u:value==up?4u:value==bottom?8u:0u;
    if(!bit)return false;
    auto &held=key?heldKeyboardPan_:heldPadPan_;
    if(!down) {held&=~bit;return false;}
    if(!zoomPanContext() || !comic())return false;
    if(!(held&bit)) {
        held|=bit;panReaderBy(value==left?-64.f:value==right?64.f:0.f,value==up?-64.f:value==bottom?64.f:0.f);
    }
    return true;
}
void tickControllerInput() {
    const auto now=SDL_GetTicks();
    const float dt=std::min(.05f,(now-controllerInputTick_)/1000.f);controllerInputTick_=now;
    if(appBackground_) {clearHeldControls();return;}
    if(controller_) for(int axis=0;axis<4;++axis)
        stickAxes_[axis]=SDL_GameControllerGetAxis(controller_,static_cast<SDL_GameControllerAxis>(axis));
    const float rx=stickAmount(stickAxes_[2]),ry=stickAmount(stickAxes_[3]);
    if(zoomPanContext()) {
        if(pageTurnAnimation_.active || !activeFingers_.empty())return;
        const unsigned held=heldKeyboardPan_|heldPadPan_;
        float x=stickAmount(stickAxes_[0])+rx+float(bool(held&2))-float(bool(held&1));
        float y=stickAmount(stickAxes_[1])+ry+float(bool(held&8))-float(bool(held&4));
        const float length=std::max(1.f,std::hypot(x,y));
        // A centered stick must not cancel momentum from a touch gesture.
        if(std::abs(x)+std::abs(y)>.001f)panReaderBy(x/length*850.f*dt,y/length*850.f*dt);
        return;
    }
    heldKeyboardPan_=heldPadPan_=0;
    if(std::abs(ry)<.001f)return;
    const float delta=ry*850.f*dt;
    int w,h;getWindowSize(w,h);
    if(colorEditing_>=0)return;
    if(choiceKind_>=0) {
        const auto content=choiceContentRect(w,h);
        choiceScroll_=std::clamp(choiceScroll_+delta,0.f,std::max(0.f,float(choiceOptions().size()*56-content.h)));
    } else if(catalogPanel_!=CatalogPanel::None) {
        const auto panel=catalogPanelRect(w,h);
        const int available=panel.y+panel.h-16-catalogContentTop(panel);
        catalogScroll_=std::clamp(catalogScroll_+delta,0.f,float(std::max(0,int(catalogRows().size())*64-available)));
    } else if(screen_==ScreenState::Reader && !quickMenu_) {
        if(continuous()) {scrollBy(delta*extraStyle_.scrollSensitivity/100.f);saveCurrentPosition();}
        else if(!px_document_is_epub(doc_))panReaderBy(rx*850.f*dt,delta);
        else if(now-rightPageTick_>=350 && !pageTurnAnimation_.active) {turnPage(ry>0?1:-1);rightPageTick_=now;}
    } else {
        uiScroll()=std::clamp(uiScroll()+delta,0.f,maxUiScroll());
        if(screen_==ScreenState::ReadingJournal)journalUI_.setScrollY(int(journalScroll_));
    }
    listVelocity_=0;dirty_=true;
}
