/// @file wayland_window.cpp
/// @brief Standard Desktop Window implementation using xdg_shell protocol and EGL.

#include "nisaba/backend_os/wayland/wayland_window.hpp"
#include "nisaba/backend_os/wayland/wayland_platform.hpp"
#include "nisaba/backend_os/wayland/wayland_surface.hpp"
#include "nisaba/backend_os/window.hpp"
#include <iostream>

namespace nisaba::backend_os::wayland {

// ── XDG Protocol Listeners ───────────────────────────────────────

static void xdg_surface_configure_handler(void* data, xdg_surface* xdg_surf, uint32_t serial) {
    xdg_surface_ack_configure(xdg_surf, serial);
    auto* self = static_cast<WaylandWindow*>(data);
    self->handleSurfaceConfigure(serial);
}

static const struct xdg_surface_listener xdg_surface_listener_impl = {
    .configure = xdg_surface_configure_handler,
};

static void xdg_toplevel_configure_handler(void* data, xdg_toplevel* /*toplevel*/,
                                           int32_t width, int32_t height, wl_array* states) {
    auto* self = static_cast<WaylandWindow*>(data);
    self->handleToplevelConfigure(width, height, states);
}

static void xdg_toplevel_close_handler(void* data, xdg_toplevel* /*toplevel*/) {
    auto* self = static_cast<WaylandWindow*>(data);
    self->handleClose();
}

static void xdg_toplevel_configure_bounds_handler(void* /*data*/, xdg_toplevel* /*toplevel*/,
                                                  int32_t /*width*/, int32_t /*height*/) {
    // Optional bounds recommendation from compositor
}

static void xdg_toplevel_wm_capabilities_handler(void* /*data*/, xdg_toplevel* /*toplevel*/,
                                                 wl_array* /*capabilities*/) {
    // Optional WM capabilities advertised by compositor
}

static const struct xdg_toplevel_listener xdg_toplevel_listener_impl = {
    .configure        = xdg_toplevel_configure_handler,
    .close            = xdg_toplevel_close_handler,
    .configure_bounds = xdg_toplevel_configure_bounds_handler,
    .wm_capabilities  = xdg_toplevel_wm_capabilities_handler,
};

static void xdg_popup_configure_handler(void* data, xdg_popup* /*popup*/,
                                        int32_t x, int32_t y, int32_t width, int32_t height) {
    auto* self = static_cast<WaylandWindow*>(data);
    self->handlePopupConfigure(x, y, width, height);
}

static void xdg_popup_done_handler(void* data, xdg_popup* /*popup*/) {
    auto* self = static_cast<WaylandWindow*>(data);
    self->handleClose();
}

static const struct xdg_popup_listener xdg_popup_listener_impl = {
    .configure = xdg_popup_configure_handler,
    .popup_done = xdg_popup_done_handler,
    .repositioned = nullptr,
};

// ════════════════════════════════════════════════════════════════
// WaylandWindow Implementation
// ════════════════════════════════════════════════════════════════

WaylandWindow::WaylandWindow(WaylandPlatformBackend& backend)
    : backend_(backend) {}

WaylandWindow::~WaylandWindow() {
    destroy();
}

bool WaylandWindow::init(const WindowConfig& config) {
    config_         = config;
    current_width_  = config_.width > 0 ? config_.width : 1280;
    current_height_ = config_.height > 0 ? config_.height : 800;

    auto* compositor = backend_.getCompositor();
    auto* xdg_wm_base = backend_.getXdgWmBase();
    egl_display_ = backend_.getEGLDisplay();
    auto egl_cfg = backend_.getEGLConfig();

    if (!compositor) {
        std::cerr << "[Nisaba WaylandWindow] wl_compositor unavailable\n";
        return false;
    }
    if (!xdg_wm_base) {
        std::cerr << "[Nisaba WaylandWindow] xdg_wm_base unavailable on compositor\n";
        return false;
    }
    if (egl_display_ == EGL_NO_DISPLAY || !egl_cfg) {
        std::cerr << "[Nisaba WaylandWindow] EGL display/config invalid\n";
        return false;
    }

    // 1. Create base Wayland surface
    wl_surface_ = wl_compositor_create_surface(compositor);
    if (!wl_surface_) {
        std::cerr << "[Nisaba WaylandWindow] Failed to create wl_surface\n";
        return false;
    }

    // 2. Create XDG Surface
    xdg_surface_ = xdg_wm_base_get_xdg_surface(xdg_wm_base, wl_surface_);
    if (!xdg_surface_) {
        std::cerr << "[Nisaba WaylandWindow] Failed to get xdg_surface\n";
        return false;
    }
    xdg_surface_add_listener(xdg_surface_, &xdg_surface_listener_impl, this);

    if (config_.mode == WindowMode::Popup) {
        xdg_positioner* positioner = xdg_wm_base_create_positioner(xdg_wm_base);
        xdg_positioner_set_size(positioner, current_width_, current_height_);
        xdg_positioner_set_anchor_rect(positioner, config_.x, config_.y, 1, 1);
        xdg_positioner_set_anchor(positioner, XDG_POSITIONER_ANCHOR_TOP_LEFT);
        xdg_positioner_set_gravity(positioner, XDG_POSITIONER_GRAVITY_BOTTOM_RIGHT);
        xdg_positioner_set_constraint_adjustment(positioner, 
            XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_SLIDE_X | XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_SLIDE_Y);

        xdg_surface* parent_xdg = nullptr;
        if (config_.parent_window) {
            auto* parent_wl_win = static_cast<WaylandWindow*>(config_.parent_window->getBackendWindow());
            if (parent_wl_win) parent_xdg = parent_wl_win->getXdgSurface();
        }

        xdg_popup_ = xdg_surface_get_popup(xdg_surface_, parent_xdg, positioner);
        xdg_popup_add_listener(xdg_popup_, &xdg_popup_listener_impl, this);
        xdg_positioner_destroy(positioner);

        if (config_.parent_layer) {
            auto* layer_shell = backend_.getLayerShell();
            auto* parent_wl_layer = static_cast<WaylandLayerSurface*>(config_.parent_layer->getBackendLayer());
            if (layer_shell && parent_wl_layer && parent_wl_layer->getLayerSurface()) {
                zwlr_layer_surface_v1_get_popup(parent_wl_layer->getLayerSurface(), xdg_popup_);
            }
        }
    } else {
        xdg_toplevel_ = xdg_surface_get_toplevel(xdg_surface_);
        if (!xdg_toplevel_) {
            std::cerr << "[Nisaba WaylandWindow] Failed to get xdg_toplevel\n";
            return false;
        }
        xdg_toplevel_add_listener(xdg_toplevel_, &xdg_toplevel_listener_impl, this);

        // Apply configuration metadata
        if (!config_.title.empty()) {
            xdg_toplevel_set_title(xdg_toplevel_, config_.title.c_str());
        }
        if (!config_.app_id.empty()) {
            xdg_toplevel_set_app_id(xdg_toplevel_, config_.app_id.c_str());
        } else {
            xdg_toplevel_set_app_id(xdg_toplevel_, "nisaba.app");
        }

        if (config_.min_width > 0 && config_.min_height > 0) {
            xdg_toplevel_set_min_size(xdg_toplevel_, config_.min_width, config_.min_height);
        }

        // CSD negotiation via zxdg_decoration_manager_v1 if available
        if (backend_.getDecorationManager()) {
            decoration_ = zxdg_decoration_manager_v1_get_toplevel_decoration(
                backend_.getDecorationManager(), xdg_toplevel_);
            if (decoration_) {
                zxdg_toplevel_decoration_v1_set_mode(decoration_,
                    config_.csd ? ZXDG_TOPLEVEL_DECORATION_V1_MODE_CLIENT_SIDE
                                : ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE);
            }
        }
    }

    // Commit initial state and wait for compositor's first configure sequence
    wl_surface_commit(wl_surface_);
    wl_display_roundtrip(backend_.getDisplay());

    // 3. Create Wayland EGL Window & Surface
    egl_window_ = wl_egl_window_create(wl_surface_, current_width_, current_height_);
    if (!egl_window_) {
        std::cerr << "[Nisaba WaylandWindow] Failed to create wl_egl_window\n";
        return false;
    }

    egl_surface_ = eglCreateWindowSurface(egl_display_, egl_cfg, (EGLNativeWindowType)egl_window_, nullptr);
    if (egl_surface_ == EGL_NO_SURFACE) {
        std::cerr << "[Nisaba WaylandWindow] Failed to create EGL surface\n";
        return false;
    }

    // 4. Use shared EGL Context from backend
    egl_context_ = backend_.getEGLContext();
    if (egl_context_ == EGL_NO_CONTEXT) {
        const EGLint ctx_attribs[] = {
            EGL_CONTEXT_MAJOR_VERSION, 3,
            EGL_CONTEXT_MINOR_VERSION, 3,
            EGL_CONTEXT_OPENGL_PROFILE_MASK, EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT,
            EGL_NONE
        };
        egl_context_ = eglCreateContext(egl_display_, egl_cfg, EGL_NO_CONTEXT, ctx_attribs);
        if (egl_context_ == EGL_NO_CONTEXT) {
            const EGLint gles_attribs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };
            egl_context_ = eglCreateContext(egl_display_, egl_cfg, EGL_NO_CONTEXT, gles_attribs);
        }
        if (egl_context_ == EGL_NO_CONTEXT) {
            egl_context_ = eglCreateContext(egl_display_, egl_cfg, EGL_NO_CONTEXT, nullptr);
        }
    }

    if (egl_context_ == EGL_NO_CONTEXT) {
        std::cerr << "[Nisaba WaylandWindow] Failed to create EGL context\n";
        return false;
    }

    // Make current & configure VSync
    makeCurrent();
    eglSwapInterval(egl_display_, config_.vsync ? 1 : 0);

    return true;
}

void WaylandWindow::destroy() {
    if (egl_display_ != EGL_NO_DISPLAY) {
        if (egl_surface_ != EGL_NO_SURFACE) {
            eglDestroySurface(egl_display_, egl_surface_);
            egl_surface_ = EGL_NO_SURFACE;
        }
    }

    if (egl_window_) {
        wl_egl_window_destroy(egl_window_);
        egl_window_ = nullptr;
    }
    if (decoration_) {
        zxdg_toplevel_decoration_v1_destroy(decoration_);
        decoration_ = nullptr;
    }
    if (xdg_popup_) {
        xdg_popup_destroy(xdg_popup_);
        xdg_popup_ = nullptr;
    }
    if (xdg_toplevel_) {
        xdg_toplevel_destroy(xdg_toplevel_);
        xdg_toplevel_ = nullptr;
    }
    if (xdg_surface_) {
        xdg_surface_destroy(xdg_surface_);
        xdg_surface_ = nullptr;
    }
    if (wl_surface_) {
        wl_surface_destroy(wl_surface_);
        wl_surface_ = nullptr;
    }
}

void WaylandWindow::handleSurfaceConfigure(uint32_t /*serial*/) {
    configured_ = true;
}

void WaylandWindow::handleToplevelConfigure(int32_t width, int32_t height, wl_array* states) {
    WindowState new_state = WindowState::Normal;
    if (states && states->data) {
        auto* p = static_cast<const uint32_t*>(states->data);
        size_t count = states->size / sizeof(uint32_t);
        for (size_t i = 0; i < count; ++i) {
            uint32_t s = p[i];
            if (s == XDG_TOPLEVEL_STATE_MAXIMIZED)  new_state |= WindowState::Maximized;
            if (s == XDG_TOPLEVEL_STATE_FULLSCREEN) new_state |= WindowState::Fullscreen;
            if (s == XDG_TOPLEVEL_STATE_ACTIVATED)  new_state |= WindowState::Activated;
        }
    }

    bool max_changed = (hasWindowState(state_, WindowState::Maximized) != hasWindowState(new_state, WindowState::Maximized));
    bool focus_changed = (hasWindowState(state_, WindowState::Activated) != hasWindowState(new_state, WindowState::Activated));
    bool state_changed = (state_ != new_state);
    state_ = new_state;

    if (focus_changed) {
        on_focus_.emit(hasWindowState(state_, WindowState::Activated));
    }
    if (max_changed) {
        on_maximized_.emit(hasWindowState(state_, WindowState::Maximized));
    }
    if (state_changed) {
        on_state_changed_.emit(state_);
    }

    if (width > 0 && height > 0) {
        if (current_width_ != width || current_height_ != height) {
            current_width_  = width;
            current_height_ = height;
            if (egl_window_) {
                wl_egl_window_resize(egl_window_, current_width_, current_height_, 0, 0);
            }
            on_resize_.emit(current_width_, current_height_);
        }
    }
}

void WaylandWindow::handlePopupConfigure(int32_t /*x*/, int32_t /*y*/, int32_t width, int32_t height) {
    if (width > 0 && height > 0) {
        if (current_width_ != width || current_height_ != height) {
            current_width_  = width;
            current_height_ = height;
            if (egl_window_) {
                wl_egl_window_resize(egl_window_, current_width_, current_height_, 0, 0);
            }
            on_resize_.emit(current_width_, current_height_);
        }
    }
}

void WaylandWindow::handleClose() {
    on_close_.emit();
}

void WaylandWindow::setTitle(std::string_view title) {
    config_.title = std::string(title);
    if (xdg_toplevel_) {
        xdg_toplevel_set_title(xdg_toplevel_, config_.title.c_str());
    }
}

void WaylandWindow::setSize(int width, int height) {
    current_width_  = width;
    current_height_ = height;
    if (egl_window_) {
        wl_egl_window_resize(egl_window_, current_width_, current_height_, 0, 0);
    }
    if (wl_surface_) {
        wl_surface_commit(wl_surface_);
    }
}

void WaylandWindow::setPosition(int /*x*/, int /*y*/) {
    // Window position on Wayland is managed by the compositor
}

Size WaylandWindow::getSize() const {
    return Size{static_cast<float>(current_width_), static_cast<float>(current_height_)};
}

Size WaylandWindow::getDrawableSize() const {
    return Size{static_cast<float>(current_width_), static_cast<float>(current_height_)};
}

float WaylandWindow::getDpiScale() const {
    return scale_factor_;
}

void WaylandWindow::makeCurrent() {
    if (egl_display_ != EGL_NO_DISPLAY && egl_surface_ != EGL_NO_SURFACE && egl_context_ != EGL_NO_CONTEXT) {
        eglMakeCurrent(egl_display_, egl_surface_, egl_surface_, egl_context_);
    }
}

void WaylandWindow::setDamage(const std::vector<Rect>& damage_rects) {
    pending_damage_ = damage_rects;
}

void WaylandWindow::swapBuffers() {
    if (!configured_) {
        return;
    }
    if (wl_surface_) {
        for (const auto& r : pending_damage_) {
            wl_surface_damage_buffer(wl_surface_, static_cast<int32_t>(r.x), static_cast<int32_t>(r.y),
                                     static_cast<int32_t>(r.width), static_cast<int32_t>(r.height));
        }
        pending_damage_.clear();
    }
    if (egl_display_ != EGL_NO_DISPLAY && egl_surface_ != EGL_NO_SURFACE) {
        eglSwapBuffers(egl_display_, egl_surface_);
    }
}



// ── Client-Side Decoration (CSD) Operations ─────────────────────

void WaylandWindow::beginMove(float /*local_x*/, float /*local_y*/, int /*button*/) {
    if (!xdg_toplevel_ || !backend_.getSeat()) return;
    xdg_toplevel_move(xdg_toplevel_, backend_.getSeat(), backend_.getLastPointerSerial());
}

void WaylandWindow::beginResize(WindowEdge edge, float /*local_x*/, float /*local_y*/, int /*button*/) {
    if (!xdg_toplevel_ || !backend_.getSeat() || edge == WindowEdge::NoneEdge) return;
    uint32_t wl_edge = XDG_TOPLEVEL_RESIZE_EDGE_NONE;
    switch (edge) {
        case WindowEdge::Top:         wl_edge = XDG_TOPLEVEL_RESIZE_EDGE_TOP; break;
        case WindowEdge::Bottom:      wl_edge = XDG_TOPLEVEL_RESIZE_EDGE_BOTTOM; break;
        case WindowEdge::Left:        wl_edge = XDG_TOPLEVEL_RESIZE_EDGE_LEFT; break;
        case WindowEdge::Right:       wl_edge = XDG_TOPLEVEL_RESIZE_EDGE_RIGHT; break;
        case WindowEdge::TopLeft:     wl_edge = XDG_TOPLEVEL_RESIZE_EDGE_TOP_LEFT; break;
        case WindowEdge::TopRight:    wl_edge = XDG_TOPLEVEL_RESIZE_EDGE_TOP_RIGHT; break;
        case WindowEdge::BottomLeft:  wl_edge = XDG_TOPLEVEL_RESIZE_EDGE_BOTTOM_LEFT; break;
        case WindowEdge::BottomRight: wl_edge = XDG_TOPLEVEL_RESIZE_EDGE_BOTTOM_RIGHT; break;
        default: break;
    }
    xdg_toplevel_resize(xdg_toplevel_, backend_.getSeat(), backend_.getLastPointerSerial(), wl_edge);
}

void WaylandWindow::setMaximized(bool max) {
    if (!xdg_toplevel_) return;
    if (max) {
        xdg_toplevel_set_maximized(xdg_toplevel_);
    } else {
        xdg_toplevel_unset_maximized(xdg_toplevel_);
    }
}

void WaylandWindow::setMinimized(bool min) {
    if (!xdg_toplevel_) return;
    if (min) {
        xdg_toplevel_set_minimized(xdg_toplevel_);
    }
}

void WaylandWindow::setFullscreen(bool full) {
    if (!xdg_toplevel_) return;
    if (full) {
        xdg_toplevel_set_fullscreen(xdg_toplevel_, nullptr);
    } else {
        xdg_toplevel_unset_fullscreen(xdg_toplevel_);
    }
}

void WaylandWindow::toggleMaximize() {
    setMaximized(!isMaximized());
}

void WaylandWindow::showWindowMenu(float local_x, float local_y, int /*button*/) {
    if (!xdg_toplevel_ || !backend_.getSeat()) return;
    xdg_toplevel_show_window_menu(xdg_toplevel_, backend_.getSeat(), backend_.getLastPointerSerial(),
                                  static_cast<int32_t>(local_x), static_cast<int32_t>(local_y));
}

void WaylandWindow::setDecorated(bool decorated) {
    if (!decoration_) return;
    zxdg_toplevel_decoration_v1_set_mode(decoration_,
        decorated ? ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE
                  : ZXDG_TOPLEVEL_DECORATION_V1_MODE_CLIENT_SIDE);
}

void WaylandWindow::setWindowGeometry(int x, int y, int width, int height) {
    if (!xdg_surface_) return;
    xdg_surface_set_window_geometry(xdg_surface_, x, y, width, height);
}

} // namespace nisaba::backend_os::wayland
