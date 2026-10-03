/// @file drm_input.cpp
/// @brief Direct input implementation for Linux DRM/KMS via libinput + udev + xkbcommon.

#include "nisaba/backend_os/drm/drm_input.hpp"
#include "nisaba/backend_os/drm/drm_platform.hpp"
#include "nisaba/backend_os/drm/drm_window.hpp"
#include "nisaba/backend_os/platform.hpp"

#include <fcntl.h>
#include <unistd.h>
#include <poll.h>
#include <errno.h>
#include <cstring>
#include <iostream>
#include <vector>
#include <algorithm>
#include <filesystem>
#include <linux/input.h>

namespace nisaba::backend_os::drm {

namespace {

static int open_restricted(const char* path, int flags, void* /*user_data*/) {
    int fd = ::open(path, flags);
    if (fd < 0) {
        return -errno;
    }
    return fd;
}

static void close_restricted(int fd, void* /*user_data*/) {
    ::close(fd);
}

static const struct libinput_interface s_libinput_interface = {
    .open_restricted  = open_restricted,
    .close_restricted = close_restricted,
};

} // anonymous namespace

DRMInputBackend::DRMInputBackend(DRMPlatformBackend* platform)
    : platform_(platform) {
}

DRMInputBackend::~DRMInputBackend() {
    shutdown();
}

bool DRMInputBackend::init() {
    udev_ = udev_new();

    // Prefer path-based context: directly scans evdev nodes without relying on systemd seat tagging
    li_ = libinput_path_create_context(&s_libinput_interface, this);
    if (li_) {
        scanInputDevices();

        // Setup netlink udev monitor for hotplugged input devices
        if (udev_) {
            udev_mon_ = udev_monitor_new_from_netlink(udev_, "udev");
            if (udev_mon_) {
                udev_monitor_filter_add_match_subsystem_devtype(udev_mon_, "input", nullptr);
                udev_monitor_enable_receiving(udev_mon_);
                udev_mon_fd_ = udev_monitor_get_fd(udev_mon_);
            }
        }
    } else if (udev_) {
        // Fallback to udev seat assignment
        li_ = libinput_udev_create_context(&s_libinput_interface, this, udev_);
        if (li_) {
            libinput_udev_assign_seat(li_, "seat0");
        }
    }

    if (!li_) {
        std::cerr << "[Nisaba DRM Input] Failed to create libinput context\n";
        return false;
    }

    // Initialize xkbcommon for keyboard translation
    xkb_ctx_ = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    if (xkb_ctx_) {
        xkb_keymap_ = xkb_keymap_new_from_names(xkb_ctx_, nullptr, XKB_KEYMAP_COMPILE_NO_FLAGS);
        if (xkb_keymap_) {
            xkb_state_ = xkb_state_new(xkb_keymap_);
        }
    }

    std::cout << "[Nisaba DRM Input] Initialized direct libinput input subsystem\n";
    return true;
}

void DRMInputBackend::scanInputDevices() {
    if (!li_) return;

    int count = 0;
    try {
        if (std::filesystem::exists("/dev/input")) {
            std::vector<std::string> paths;
            for (const auto& entry : std::filesystem::directory_iterator("/dev/input")) {
                std::string fname = entry.path().filename().string();
                if (fname.rfind("event", 0) == 0) { // starts with "event"
                    paths.push_back(entry.path().string());
                }
            }
            std::sort(paths.begin(), paths.end());

            for (const auto& path : paths) {
                struct libinput_device* dev = libinput_path_add_device(li_, path.c_str());
                if (dev) {
                    const char* name = libinput_device_get_name(dev);
                    bool is_pointer = libinput_device_has_capability(dev, LIBINPUT_DEVICE_CAP_POINTER);
                    bool is_kbd     = libinput_device_has_capability(dev, LIBINPUT_DEVICE_CAP_KEYBOARD);
                    bool is_touch   = libinput_device_has_capability(dev, LIBINPUT_DEVICE_CAP_TOUCH);

                    // Enable tap-to-click on touchpads automatically
                    if (is_pointer && libinput_device_config_tap_get_finger_count(dev) > 0) {
                        libinput_device_config_tap_set_enabled(dev, LIBINPUT_CONFIG_TAP_ENABLED);
                    }

                    std::string caps;
                    if (is_pointer) caps += "Pointer ";
                    if (is_kbd)     caps += "Keyboard ";
                    if (is_touch)   caps += "Touch ";

                    std::cout << "[Nisaba DRM Input] Registered device: " << path
                              << " [" << (caps.empty() ? "Other" : caps) << "] - "
                              << (name ? name : "Unknown") << "\n";
                    count++;
                }
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "[Nisaba DRM Input] Exception scanning /dev/input: " << e.what() << "\n";
    }

    if (count == 0) {
        std::cerr << "[Nisaba DRM Input] WARNING: No input devices could be opened!\n"
                  << "                 Make sure the application is run with 'sudo' or user is in 'input' group:\n"
                  << "                 sudo usermod -aG input $USER\n";
    }
}

void DRMInputBackend::shutdown() {
    if (udev_mon_) {
        udev_monitor_unref(udev_mon_);
        udev_mon_ = nullptr;
        udev_mon_fd_ = -1;
    }
    if (xkb_state_) {
        xkb_state_unref(xkb_state_);
        xkb_state_ = nullptr;
    }
    if (xkb_keymap_) {
        xkb_keymap_unref(xkb_keymap_);
        xkb_keymap_ = nullptr;
    }
    if (xkb_ctx_) {
        xkb_context_unref(xkb_ctx_);
        xkb_ctx_ = nullptr;
    }
    if (li_) {
        libinput_unref(li_);
        li_ = nullptr;
    }
    if (udev_) {
        udev_unref(udev_);
        udev_ = nullptr;
    }
}

int DRMInputBackend::getFd() const {
    return li_ ? libinput_get_fd(li_) : -1;
}

void DRMInputBackend::updateScreenSize(int width, int height) {
    bool was_uninit = (pointer_x_ == 0.0f && pointer_y_ == 0.0f);
    screen_width_  = std::max(1, width);
    screen_height_ = std::max(1, height);

    if (was_uninit) {
        pointer_x_ = screen_width_ * 0.5f;
        pointer_y_ = screen_height_ * 0.5f;
        if (platform_) {
            platform_->moveCursor(static_cast<int>(pointer_x_), static_cast<int>(pointer_y_));
        }
    } else {
        pointer_x_ = std::clamp(pointer_x_, 0.0f, static_cast<float>(screen_width_ - 1));
        pointer_y_ = std::clamp(pointer_y_, 0.0f, static_cast<float>(screen_height_ - 1));
    }
}

void DRMInputBackend::processEvents() {
    if (!li_ || !platform_ || !platform_->getOwner()) {
        return;
    }

    if (udev_mon_fd_ >= 0) {
        struct pollfd pfd{};
        pfd.fd = udev_mon_fd_;
        pfd.events = POLLIN;
        if (poll(&pfd, 1, 0) > 0 && (pfd.revents & POLLIN)) {
            struct udev_device* udev_dev = udev_monitor_receive_device(udev_mon_);
            if (udev_dev) {
                const char* action = udev_device_get_action(udev_dev);
                const char* devnode = udev_device_get_devnode(udev_dev);
                if (action && devnode && std::string_view(devnode).find("event") != std::string_view::npos) {
                    if (std::string_view(action) == "add") {
                        struct libinput_device* dev = libinput_path_add_device(li_, devnode);
                        if (dev) {
                            if (libinput_device_has_capability(dev, LIBINPUT_DEVICE_CAP_POINTER) &&
                                libinput_device_config_tap_get_finger_count(dev) > 0) {
                                libinput_device_config_tap_set_enabled(dev, LIBINPUT_CONFIG_TAP_ENABLED);
                            }
                            const char* name = libinput_device_get_name(dev);
                            std::cout << "[Nisaba DRM Input] Hotplugged device: " << devnode
                                      << " - " << (name ? name : "Unknown") << "\n";
                        }
                    }
                }
                udev_device_unref(udev_dev);
            }
        }
    }

    libinput_dispatch(li_);

    struct libinput_event* event = nullptr;
    while ((event = libinput_get_event(li_)) != nullptr) {
        enum libinput_event_type type = libinput_event_get_type(event);

        switch (type) {
            case LIBINPUT_EVENT_POINTER_MOTION:
                handlePointerMotion(libinput_event_get_pointer_event(event));
                break;
            case LIBINPUT_EVENT_POINTER_MOTION_ABSOLUTE:
                handlePointerMotionAbsolute(libinput_event_get_pointer_event(event));
                break;
            case LIBINPUT_EVENT_POINTER_BUTTON:
                handlePointerButton(libinput_event_get_pointer_event(event));
                break;
            case LIBINPUT_EVENT_POINTER_AXIS:
                handlePointerAxis(libinput_event_get_pointer_event(event));
                break;
            case LIBINPUT_EVENT_KEYBOARD_KEY:
                handleKeyboardKey(libinput_event_get_keyboard_event(event));
                break;
            case LIBINPUT_EVENT_TOUCH_DOWN:
                handleTouchDown(libinput_event_get_touch_event(event));
                break;
            case LIBINPUT_EVENT_TOUCH_MOTION:
                handleTouchMotion(libinput_event_get_touch_event(event));
                break;
            case LIBINPUT_EVENT_TOUCH_UP:
            case LIBINPUT_EVENT_TOUCH_CANCEL:
                handleTouchUp(libinput_event_get_touch_event(event));
                break;
            default:
                break;
        }

        libinput_event_destroy(event);
    }
}

void DRMInputBackend::handlePointerMotion(struct libinput_event_pointer* event) {
    double dx = libinput_event_pointer_get_dx(event);
    double dy = libinput_event_pointer_get_dy(event);

    pointer_x_ = std::clamp(pointer_x_ + static_cast<float>(dx), 0.0f, static_cast<float>(screen_width_ - 1));
    pointer_y_ = std::clamp(pointer_y_ + static_cast<float>(dy), 0.0f, static_cast<float>(screen_height_ - 1));

    if (platform_) {
        platform_->moveCursor(static_cast<int>(pointer_x_), static_cast<int>(pointer_y_));
    }

    void* win_handle = (platform_ && platform_->getActiveWindow())
        ? platform_->getActiveWindow()->getNativeHandle() : nullptr;

    Platform* p = platform_->getOwner();
    p->onMouseMove().emit(pointer_x_, pointer_y_);
    p->onTargetedMouseMove().emit(win_handle, pointer_x_, pointer_y_);
}

void DRMInputBackend::handlePointerMotionAbsolute(struct libinput_event_pointer* event) {
    double ax = libinput_event_pointer_get_absolute_x_transformed(event, screen_width_);
    double ay = libinput_event_pointer_get_absolute_y_transformed(event, screen_height_);

    pointer_x_ = std::clamp(static_cast<float>(ax), 0.0f, static_cast<float>(screen_width_ - 1));
    pointer_y_ = std::clamp(static_cast<float>(ay), 0.0f, static_cast<float>(screen_height_ - 1));

    if (platform_) {
        platform_->moveCursor(static_cast<int>(pointer_x_), static_cast<int>(pointer_y_));
    }

    void* win_handle = (platform_ && platform_->getActiveWindow())
        ? platform_->getActiveWindow()->getNativeHandle() : nullptr;

    Platform* p = platform_->getOwner();
    p->onMouseMove().emit(pointer_x_, pointer_y_);
    p->onTargetedMouseMove().emit(win_handle, pointer_x_, pointer_y_);
}

void DRMInputBackend::handlePointerButton(struct libinput_event_pointer* event) {
    uint32_t button = libinput_event_pointer_get_button(event);
    enum libinput_button_state state = libinput_event_pointer_get_button_state(event);

    int button_idx = 1;
    switch (button) {
        case BTN_LEFT:   button_idx = 1; break;
        case BTN_MIDDLE: button_idx = 2; break;
        case BTN_RIGHT:  button_idx = 3; break;
        default:         button_idx = 1; break;
    }

    void* win_handle = (platform_ && platform_->getActiveWindow())
        ? platform_->getActiveWindow()->getNativeHandle() : nullptr;

    Platform* p = platform_->getOwner();
    if (state == LIBINPUT_BUTTON_STATE_PRESSED) {
        active_buttons_ |= (1 << (button_idx - 1));
        p->onMouseDown().emit(pointer_x_, pointer_y_, button_idx);
        p->onTargetedMouseDown().emit(win_handle, pointer_x_, pointer_y_, button_idx);
    } else {
        active_buttons_ &= ~(1 << (button_idx - 1));
        p->onMouseUp().emit(pointer_x_, pointer_y_, button_idx);
        p->onTargetedMouseUp().emit(win_handle, pointer_x_, pointer_y_, button_idx);
    }
}

void DRMInputBackend::handlePointerAxis(struct libinput_event_pointer* event) {
    double dx = 0.0;
    double dy = 0.0;

    if (libinput_event_pointer_has_axis(event, LIBINPUT_POINTER_AXIS_SCROLL_VERTICAL)) {
        dy = libinput_event_pointer_get_axis_value(event, LIBINPUT_POINTER_AXIS_SCROLL_VERTICAL);
    }
    if (libinput_event_pointer_has_axis(event, LIBINPUT_POINTER_AXIS_SCROLL_HORIZONTAL)) {
        dx = libinput_event_pointer_get_axis_value(event, LIBINPUT_POINTER_AXIS_SCROLL_HORIZONTAL);
    }

    void* win_handle = (platform_ && platform_->getActiveWindow())
        ? platform_->getActiveWindow()->getNativeHandle() : nullptr;

    Platform* p = platform_->getOwner();
    p->onScroll().emit(static_cast<float>(dx), static_cast<float>(dy));
    p->onTargetedScroll().emit(win_handle, static_cast<float>(dx), static_cast<float>(dy));
}

void DRMInputBackend::handleKeyboardKey(struct libinput_event_keyboard* event) {
    uint32_t key = libinput_event_keyboard_get_key(event);
    enum libinput_key_state state = libinput_event_keyboard_get_key_state(event);

    if (!xkb_state_) {
        return;
    }

    // evdev keycode to XKB keycode has an offset of 8
    xkb_keycode_t xkb_key = key + 8;
    xkb_state_update_key(xkb_state_, xkb_key,
                         (state == LIBINPUT_KEY_STATE_PRESSED) ? XKB_KEY_DOWN : XKB_KEY_UP);

    xkb_keysym_t sym = xkb_state_key_get_one_sym(xkb_state_, xkb_key);

    // Calculate modifier mask
    int mods = 0;
    if (xkb_state_mod_name_is_active(xkb_state_, XKB_MOD_NAME_SHIFT, XKB_STATE_MODS_EFFECTIVE)) {
        mods |= (1 << 0);
    }
    if (xkb_state_mod_name_is_active(xkb_state_, XKB_MOD_NAME_CTRL, XKB_STATE_MODS_EFFECTIVE)) {
        mods |= (1 << 1);
    }
    if (xkb_state_mod_name_is_active(xkb_state_, XKB_MOD_NAME_ALT, XKB_STATE_MODS_EFFECTIVE)) {
        mods |= (1 << 2);
    }

    Platform* p = platform_->getOwner();
    if (state == LIBINPUT_KEY_STATE_PRESSED) {
        p->onKeyDown().emit(static_cast<int>(sym), mods);

        char utf8[64] = {0};
        int len = xkb_state_key_get_utf8(xkb_state_, xkb_key, utf8, sizeof(utf8));
        if (len > 0) {
            p->onTextInput().emit(std::string_view(utf8, len));
        }
    } else {
        p->onKeyUp().emit(static_cast<int>(sym), mods);
    }
}

void DRMInputBackend::handleTouchDown(struct libinput_event_touch* event) {
    int32_t slot = libinput_event_touch_get_slot(event);
    double x = libinput_event_touch_get_x_transformed(event, screen_width_);
    double y = libinput_event_touch_get_y_transformed(event, screen_height_);

    pointer_x_ = std::clamp(static_cast<float>(x), 0.0f, static_cast<float>(screen_width_ - 1));
    pointer_y_ = std::clamp(static_cast<float>(y), 0.0f, static_cast<float>(screen_height_ - 1));

    active_touches_[slot] = { pointer_x_, pointer_y_ };

    Platform* p = platform_->getOwner();
    p->onMouseMove().emit(pointer_x_, pointer_y_);
    p->onTargetedMouseMove().emit(nullptr, pointer_x_, pointer_y_);
    p->onMouseDown().emit(pointer_x_, pointer_y_, 1);
    p->onTargetedMouseDown().emit(nullptr, pointer_x_, pointer_y_, 1);
}

void DRMInputBackend::handleTouchMotion(struct libinput_event_touch* event) {
    int32_t slot = libinput_event_touch_get_slot(event);
    double x = libinput_event_touch_get_x_transformed(event, screen_width_);
    double y = libinput_event_touch_get_y_transformed(event, screen_height_);

    pointer_x_ = std::clamp(static_cast<float>(x), 0.0f, static_cast<float>(screen_width_ - 1));
    pointer_y_ = std::clamp(static_cast<float>(y), 0.0f, static_cast<float>(screen_height_ - 1));

    active_touches_[slot] = { pointer_x_, pointer_y_ };

    Platform* p = platform_->getOwner();
    p->onMouseMove().emit(pointer_x_, pointer_y_);
    p->onTargetedMouseMove().emit(nullptr, pointer_x_, pointer_y_);
}

void DRMInputBackend::handleTouchUp(struct libinput_event_touch* event) {
    int32_t slot = libinput_event_touch_get_slot(event);
    active_touches_.erase(slot);

    Platform* p = platform_->getOwner();
    p->onMouseUp().emit(pointer_x_, pointer_y_, 1);
    p->onTargetedMouseUp().emit(nullptr, pointer_x_, pointer_y_, 1);
}

} // namespace nisaba::backend_os::drm
