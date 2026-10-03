#pragma once
/// @file wayland_window.hpp
/// @brief Standard Desktop Window backend for Wayland using xdg_shell protocol and EGL.

#include "nisaba/backend_os/window.hpp"
#include "nisaba/backend_os/wayland/wayland_platform.hpp"

#include <wayland-client.h>
#include <wayland-egl.h>
#include <EGL/egl.h>

namespace nisaba::backend_os::wayland {

class WaylandWindow {
public:
    explicit WaylandWindow(WaylandPlatformBackend& backend);
    ~WaylandWindow();

    bool init(const WindowConfig& config);
    void destroy();

    void setTitle(std::string_view title);
    void setSize(int width, int height);
    void setPosition(int x, int y);

    [[nodiscard]] Size  getSize() const;
    [[nodiscard]] Size  getDrawableSize() const;
    [[nodiscard]] float getDpiScale() const;

    void makeCurrent();
    void swapBuffers();
    void setDamage(const std::vector<Rect>& damage_rects);

    [[nodiscard]] void* getNativeHandle() const { return (void*)wl_surface_; }
    [[nodiscard]] void* getEGLSurface()   const { return (void*)egl_surface_; }
    [[nodiscard]] void* getEGLContext()   const { return (void*)egl_context_; }
    [[nodiscard]] wl_surface* getWlSurface() const { return wl_surface_; }
    [[nodiscard]] xdg_surface* getXdgSurface() const { return xdg_surface_; }
    [[nodiscard]] xdg_toplevel* getXdgToplevel() const { return xdg_toplevel_; }

    // ── Client-Side Decoration (CSD) Operations ─────────────────
    void beginMove(float local_x = 0.0f, float local_y = 0.0f, int button = 1);
    void beginResize(WindowEdge edge, float local_x = 0.0f, float local_y = 0.0f, int button = 1);
    void setMaximized(bool max);
    void setMinimized(bool min);
    void setFullscreen(bool full);
    void toggleMaximize();
    void showWindowMenu(float local_x = 0.0f, float local_y = 0.0f, int button = 3);
    void setDecorated(bool decorated);
    void setWindowGeometry(int x, int y, int width, int height);

    [[nodiscard]] bool isMaximized() const { return hasWindowState(state_, WindowState::Maximized); }
    [[nodiscard]] bool isMinimized() const { return hasWindowState(state_, WindowState::Minimized); }
    [[nodiscard]] bool isFullscreen() const { return hasWindowState(state_, WindowState::Fullscreen); }
    [[nodiscard]] bool isActivated() const { return hasWindowState(state_, WindowState::Activated); }
    [[nodiscard]] WindowState getWindowState() const { return state_; }

    // XDG Shell Protocol Callbacks
    void handleSurfaceConfigure(uint32_t serial);
    void handleToplevelConfigure(int32_t width, int32_t height, wl_array* states);
    void handlePopupConfigure(int32_t x, int32_t y, int32_t width, int32_t height);
    void handleClose();

    // Signals
    Signal<int, int>&       onResize()       { return on_resize_; }
    Signal<>&               onClose()        { return on_close_; }
    Signal<bool>&           onFocus()        { return on_focus_; }
    Signal<WindowState>&     onStateChanged() { return on_state_changed_; }
    Signal<bool>&           onMaximized()    { return on_maximized_; }

private:
    WaylandPlatformBackend& backend_;
    WindowConfig            config_;

    wl_surface*                 wl_surface_    = nullptr;
    xdg_surface*                xdg_surface_   = nullptr;
    xdg_toplevel*               xdg_toplevel_  = nullptr;
    xdg_popup*                  xdg_popup_     = nullptr;
    zxdg_toplevel_decoration_v1* decoration_   = nullptr;
    wl_egl_window*              egl_window_    = nullptr;

    EGLDisplay              egl_display_   = EGL_NO_DISPLAY;
    EGLSurface              egl_surface_   = EGL_NO_SURFACE;
    EGLContext              egl_context_   = EGL_NO_CONTEXT;

    int32_t                 current_width_  = 0;
    int32_t                 current_height_ = 0;
    bool                    configured_     = false;
    float                   scale_factor_   = 1.0f;
    WindowState             state_          = WindowState::Normal;

    Signal<int, int>        on_resize_;
    Signal<>                on_close_;
    Signal<bool>            on_focus_;
    Signal<WindowState>     on_state_changed_;
    Signal<bool>            on_maximized_;

    std::vector<Rect> pending_damage_;
};

} // namespace nisaba::backend_os::wayland
