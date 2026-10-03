#pragma once
/// @file android_surface.hpp
/// @brief Android surface backend — wraps ANativeWindow* with an EGL surface.
/// On Android, there is no "window manager"; the application has exactly one
/// ANativeWindow provided by the OS. This class is the Android counterpart of
/// X11Window / WaylandWindow / Win32Window.

#ifdef __ANDROID__

#include "nisaba/backend_os/window.hpp"
#include "nisaba/backend_os/android/android_platform.hpp"

#include <android/native_window.h>
#include <EGL/egl.h>

namespace nisaba::backend_os::android {

/// Represents the single drawable surface on Android.
/// The ANativeWindow handle is obtained from NativeActivity and may be
/// destroyed/recreated on rotation or when the app goes to background.
class AndroidSurface {
public:
    explicit AndroidSurface(AndroidPlatformBackend& backend);
    ~AndroidSurface();

    // Non-copyable
    AndroidSurface(const AndroidSurface&) = delete;
    AndroidSurface& operator=(const AndroidSurface&) = delete;

    /// Called by Window::Impl::init() — attaches to whatever ANativeWindow is
    /// currently available (may be nullptr if surface not yet ready).
    bool init();

    /// Release the EGL surface without destroying the EGL context.
    void destroy();

    // ── Window-like interface (mirrors X11Window / Win32Window) ──

    /// No-op on Android (no title bar).
    void setTitle(std::string_view /*title*/) {}

    /// No-op on Android (OS controls window size).
    void setSize(int /*w*/, int /*h*/) {}

    /// No-op on Android (OS controls window position).
    void setPosition(int /*x*/, int /*y*/) {}

    /// No-op on Android.
    void setBorderless(bool /*b*/) {}

    /// No-op on Android.
    void setAlwaysOnTop(bool /*t*/) {}

    /// No-op on Android.
    void setBlurBehind(bool /*e*/) {}

    [[nodiscard]] Size  getSize() const;
    [[nodiscard]] Size  getDrawableSize() const;
    [[nodiscard]] float getDpiScale() const;

    void makeCurrent();
    void swapBuffers();

    // ── CSD operations — all no-op on Android ────────────────────
    void beginMove(float = 0.f, float = 0.f, int = 1) {}
    void beginResize(WindowEdge, float = 0.f, float = 0.f, int = 1) {}
    void setMaximized(bool /*max*/) {}
    void setMinimized(bool /*min*/) {}
    void setFullscreen(bool /*full*/) {}   // Always full-screen on Android
    void toggleMaximize() {}
    void showWindowMenu(float = 0.f, float = 0.f, int = 3) {}
    void setDecorated(bool /*d*/) {}
    void setWindowGeometry(int, int, int, int) {}

    // ── State queries ─────────────────────────────────────────────
    [[nodiscard]] bool isMaximized()  const { return false; }
    [[nodiscard]] bool isMinimized()  const { return false; }
    [[nodiscard]] bool isFullscreen() const { return true; }  // Always true
    [[nodiscard]] bool isActivated()  const { return activated_; }
    [[nodiscard]] WindowState getWindowState() const;

    // ── Native handle accessors ───────────────────────────────────
    [[nodiscard]] void* getNativeHandle() const { return (void*)native_window_; }
    [[nodiscard]] void* getEGLSurface()   const { return (void*)egl_surface_; }
    [[nodiscard]] void* getEGLContext()   const { return (void*)egl_context_; }

    // ── Signals (same set as X11Window + lifecycle signals) ──────
    Signal<WindowState>& onStateChanged()        { return on_state_changed_; }
    Signal<bool>&        onMaximized()           { return on_maximized_; }
    Signal<bool>&        onFocus()               { return on_focus_; }
    Signal<int, int>&    onResize()              { return on_resize_; }
    Signal<>&            onClose()               { return on_close_; }
    Signal<>&            onSurfaceRecreated()    { return on_surface_recreated_; }
    Signal<>&            onSurfaceDestroyed()    { return on_surface_destroyed_; }

    // ── Android lifecycle hooks (called by AndroidPlatformBackend) ─

    /// Called when a new ANativeWindow is available (app foreground / rotation).
    /// Creates the EGL surface and begins rendering.
    void onNativeWindowCreated(ANativeWindow* window);

    /// Called just before the ANativeWindow is destroyed.
    /// Destroys the EGL surface but keeps the EGL context alive.
    void onNativeWindowDestroyed();

    /// Called when the app window gains or loses focus.
    void onWindowFocusChanged(bool focused);

    /// @return true if the EGL surface is currently valid and renderable.
    [[nodiscard]] bool isReady() const { return egl_surface_ != EGL_NO_SURFACE; }

private:
    AndroidPlatformBackend& backend_;

    ANativeWindow* native_window_ = nullptr;
    EGLSurface     egl_surface_   = EGL_NO_SURFACE;
    EGLContext     egl_context_   = EGL_NO_CONTEXT;  // alias from backend
    EGLDisplay     egl_display_   = EGL_NO_DISPLAY;  // alias from backend

    int   width_     = 0;
    int   height_    = 0;
    float dpi_scale_ = 1.0f;
    bool  activated_ = false;

    Signal<WindowState> on_state_changed_;
    Signal<bool>        on_maximized_;
    Signal<bool>        on_focus_;
    Signal<int, int>    on_resize_;
    Signal<>            on_close_;
    Signal<>            on_surface_recreated_;
    Signal<>            on_surface_destroyed_;

    bool createEGLSurface();
    void destroyEGLSurface();
    void querySize();
};

} // namespace nisaba::backend_os::android

#endif // __ANDROID__
