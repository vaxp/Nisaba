/// @file native_popup.cpp
/// @brief Native Compositor-level Popup surface implementation.

#include "nisaba/backend_os/native_popup.hpp"
#include <iostream>

namespace nisaba::backend_os {

NativePopup::NativePopup(const PopupOptions& options)
    : options_(options) {}

NativePopup::~NativePopup() {
    close();
}

std::shared_ptr<NativePopup> NativePopup::show(
    Window& parent,
    const PopupOptions& options,
    RenderCallback render_cb) {

    App* app = App::instance();
    if (!app) {
        std::cerr << "[Nisaba NativePopup] No active App instance found\n";
        return nullptr;
    }

    auto popup = std::shared_ptr<NativePopup>(new NativePopup(options));

    int32_t popup_x = static_cast<int32_t>(options.position.x);
    int32_t popup_y = static_cast<int32_t>(options.position.y);
    if (popup_x < 0) popup_x = 0;
    if (popup_y < 0) popup_y = 0;

    WindowConfig win_cfg;
    win_cfg.title             = "nisaba-popup";
    win_cfg.x                 = popup_x;
    win_cfg.y                 = popup_y;
    win_cfg.width             = options.width;
    win_cfg.height            = options.height;
    win_cfg.borderless        = true;
    win_cfg.always_on_top     = true;
    win_cfg.override_redirect = true;
    win_cfg.resizable         = false;
    win_cfg.transparent       = true;
    win_cfg.mode              = WindowMode::Popup;
    win_cfg.parent_window     = &parent;

    HostedSurface* host = app->addPopup(&parent, win_cfg, std::move(render_cb), options.auto_dismiss);
    if (!host) {
        std::cerr << "[Nisaba NativePopup] Failed to create native popup surface\n";
        return nullptr;
    }

    popup->host_ = host;

    std::weak_ptr<NativePopup> weak_popup = popup;
    host->onClose().connect([weak_popup]() {
        if (auto p = weak_popup.lock()) {
            p->host_ = nullptr;
            if (p->options_.on_close) {
                auto cb = std::move(p->options_.on_close);
                cb();
            }
        }
    });

    return popup;
}

void NativePopup::close() {
    if (!host_) return;

    App* app = App::instance();
    if (app) {
        app->removeSurface(host_);
    }
    host_ = nullptr;

    if (options_.on_close) {
        auto cb = std::move(options_.on_close);
        options_.on_close = nullptr;
        cb();
    }
}

Window* NativePopup::window() const {
    return host_ ? host_->getWindow() : nullptr;
}

} // namespace nisaba::backend_os
