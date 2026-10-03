#pragma once
/// @file drm_input.hpp
/// @brief Direct input backend for Linux DRM/KMS using libinput, libudev, and xkbcommon.
/// Handles keyboard, mouse/pointer, and touchscreens with direct event processing.

#include "nisaba/backend_os/types.hpp"
#include "nisaba/backend_os/signal.hpp"

#include <libinput.h>
#include <libudev.h>
#include <xkbcommon/xkbcommon.h>

#include <memory>
#include <string>
#include <unordered_map>

namespace nisaba::backend_os {
class Platform;
}

namespace nisaba::backend_os::drm {

class DRMPlatformBackend;

class DRMInputBackend {
public:
    explicit DRMInputBackend(DRMPlatformBackend* platform);
    ~DRMInputBackend();

    // Non-copyable
    DRMInputBackend(const DRMInputBackend&) = delete;
    DRMInputBackend& operator=(const DRMInputBackend&) = delete;

    bool init();
    void shutdown();

    /// Process all pending libinput events and dispatch to Platform signals.
    void processEvents();

    /// Get the libinput file descriptor for polling with poll() / epoll().
    [[nodiscard]] int getFd() const;

    /// Update display dimensions for pointer clamping and touch normalization.
    void updateScreenSize(int width, int height);

    /// Get current pointer position in screen coordinates.
    [[nodiscard]] Point getPointerPosition() const { return { pointer_x_, pointer_y_ }; }

private:
    void handlePointerMotion(struct libinput_event_pointer* event);
    void handlePointerMotionAbsolute(struct libinput_event_pointer* event);
    void handlePointerButton(struct libinput_event_pointer* event);
    void handlePointerAxis(struct libinput_event_pointer* event);
    void handleKeyboardKey(struct libinput_event_keyboard* event);
    void handleTouchDown(struct libinput_event_touch* event);
    void handleTouchMotion(struct libinput_event_touch* event);
    void handleTouchUp(struct libinput_event_touch* event);

    void scanInputDevices();

    DRMPlatformBackend* platform_ = nullptr;
    struct udev*        udev_     = nullptr;
    struct libinput*    li_       = nullptr;
    struct udev_monitor* udev_mon_ = nullptr;
    int                 udev_mon_fd_ = -1;

    // xkbcommon state for keyboard mapping
    struct xkb_context* xkb_ctx_    = nullptr;
    struct xkb_keymap*  xkb_keymap_ = nullptr;
    struct xkb_state*   xkb_state_  = nullptr;

    // Screen geometry
    int screen_width_  = 1920;
    int screen_height_ = 1080;

    // Pointer state
    float pointer_x_ = 0.0f;
    float pointer_y_ = 0.0f;
    int   active_buttons_ = 0;

    // Touch tracking (slot -> point)
    struct TouchPoint {
        float x = 0.0f;
        float y = 0.0f;
    };
    std::unordered_map<int32_t, TouchPoint> active_touches_;
};

} // namespace nisaba::backend_os::drm
