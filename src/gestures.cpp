#include "papernx/gestures.hpp"
#include <cmath>

#ifndef SDL_CONTROLLER_AXIS_LEFTX
#define SDL_CONTROLLER_AXIS_LEFTX 0
#endif
#ifndef SDL_CONTROLLER_AXIS_LEFTY
#define SDL_CONTROLLER_AXIS_LEFTY 1
#endif

namespace papernx {

TouchGestureTracker::TouchGestureTracker() {
    reset();
}

void TouchGestureTracker::reset() {
    active_ = false;
    trackingFinger_ = -1;
    startX_ = 0.0f;
    startY_ = 0.0f;
    currentX_ = 0.0f;
    currentY_ = 0.0f;
}

void TouchGestureTracker::onFingerDown(float x, float y, std::int64_t fingerId) {
    if (!active_) {
        active_ = true;
        trackingFinger_ = fingerId;
        startX_ = x;
        startY_ = y;
        currentX_ = x;
        currentY_ = y;
        startTime_ = std::chrono::steady_clock::now();
    }
}

void TouchGestureTracker::onFingerMotion(float x, float y, std::int64_t fingerId) {
    if (active_ && trackingFinger_ == fingerId) {
        currentX_ = x;
        currentY_ = y;
    }
}

GestureResult TouchGestureTracker::onFingerUp(float x, float y, std::int64_t fingerId) {
    GestureResult result;
    if (!active_ || trackingFinger_ != fingerId) {
        return result;
    }

    currentX_ = x;
    currentY_ = y;
    auto now = std::chrono::steady_clock::now();
    auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime_).count();

    float dx = currentX_ - startX_;
    float dy = currentY_ - startY_;
    float dist = std::hypotf(dx, dy);

    result.tapX = x;
    result.tapY = y;
    result.deltaX = dx;
    result.deltaY = dy;

    // A swipe must have sufficient distance, horizontal dominance, and reasonably swift duration
    if (dist > 0.08f && elapsedMs < 800) {
        if (std::abs(dx) > std::abs(dy) * 1.3f) {
            result.type = (dx < 0.0f) ? GestureType::SwipeLeft : GestureType::SwipeRight;
        } else if (std::abs(dy) > std::abs(dx) * 1.3f) {
            result.type = (dy < 0.0f) ? GestureType::SwipeUp : GestureType::SwipeDown;
        }
    } else if (dist < 0.04f && elapsedMs < 500) {
        // Tap gesture: classify by reading touch zones
        if (x < 0.22f) {
            result.type = GestureType::TapLeft;
        } else if (x > 0.78f) {
            result.type = GestureType::TapRight;
        } else {
            result.type = GestureType::TapCenter;
        }
    }

    reset();
    return result;
}

ControllerNavigator::ControllerNavigator() {
    auto now = std::chrono::steady_clock::now();
    lastTickX_ = now;
    lastTickY_ = now;
}

bool ControllerNavigator::processAxis(int axis, int value, int &outDeltaX, int &outDeltaY) {
    outDeltaX = 0;
    outDeltaY = 0;
    const int DEADZONE = 12000;
    const int REPEAT_MS = 220;

    auto now = std::chrono::steady_clock::now();

    if (axis == SDL_CONTROLLER_AXIS_LEFTX) {
        if (std::abs(value) > DEADZONE) {
            auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastTickX_).count();
            if (diff >= REPEAT_MS) {
                outDeltaX = (value > 0) ? 1 : -1;
                lastTickX_ = now;
                return true;
            }
        }
    } else if (axis == SDL_CONTROLLER_AXIS_LEFTY) {
        if (std::abs(value) > DEADZONE) {
            auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastTickY_).count();
            if (diff >= REPEAT_MS) {
                outDeltaY = (value > 0) ? 1 : -1;
                lastTickY_ = now;
                return true;
            }
        }
    }
    return false;
}

} // namespace papernx
