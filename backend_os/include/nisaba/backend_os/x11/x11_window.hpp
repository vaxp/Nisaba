#pragma once
/// @file x11_window.hpp
/// @brief X11 + EGL native window backend.
/// Implements the Window abstraction for X11 sessions.

#include "nisaba/backend_os/window.hpp"
#include "nisaba/backend_os/x11/x11_platform.hpp"

#include <X11/Xlib.h>
#include <EGL/egl.h>

namespace nisaba::backend_os::x11 {

class X11Window {
public:
    explicit X11Window(X11PlatformBackend& backend);
    ~X11Window();

    bool init(const WindowConfig& config);
    void destroy();

    void setTitle(std::string_view title);
    void setSize(int width, int height);
    void setPosition(int x, int y);
    void setBorderless(bool borderless);
    void setAlwaysOnTop(bool on_top);
    void setBlurBehind(bool enable);

    [[nodiscard]] Size  getSize() const;
    [[nodiscard]] Size  getDrawableSize() const;
    [[nodiscard]] float getDpiScale() const;

    void makeCurrent();
    void swapBuffers();

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

    void updateState();
    void handleFocus(bool focused);
    void handleConfigure(int nw, int nh);
    bool handleDragMotion(int root_x, int root_y);
    void endDrag();

    [[nodiscard]] void* getNativeHandle() const { return (void*)(uintptr_t)x11_window_; }
    [[nodiscard]] void* getEGLSurface()   const { return (void*)egl_surface_; }
    [[nodiscard]] void* getEGLContext()   const { return (void*)egl_context_; }
    [[nodiscard]] ::Window getX11Window() const { return x11_window_; }

    Signal<WindowState>& onStateChanged() { return on_state_changed_; }
    Signal<bool>&        onMaximized()    { return on_maximized_; }
    Signal<bool>&        onFocus()        { return on_focus_; }
    Signal<int, int>&    onResize()       { return on_resize_; }

private:
    X11PlatformBackend& backend_;
    WindowConfig config_;

    ::Display* display_     = nullptr;
    ::Window   x11_window_  = 0;
    Colormap   colormap_    = 0;
    EGLDisplay egl_display_ = EGL_NO_DISPLAY;
    EGLSurface egl_surface_ = EGL_NO_SURFACE;
    EGLContext egl_context_ = EGL_NO_CONTEXT;

    int current_width_  = 0;
    int current_height_ = 0;
    WindowState state_  = WindowState::Normal;
    bool blur_enabled_  = false;

    void updateBlurBehindRegion();

    Signal<WindowState> on_state_changed_;
    Signal<bool>        on_maximized_;
    Signal<bool>        on_focus_;
    Signal<int, int>    on_resize_;
};

} // namespace nisaba::backend_os::x11
