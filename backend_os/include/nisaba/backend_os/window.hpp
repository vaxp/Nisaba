#pragma once
/// @file window.hpp
/// @brief Native OS Window management with direct EGL + OpenGL +  context.
/// Zero SDL dependency — pure native platform window.

#include "nisaba/backend_os/types.hpp"
#include "nisaba/backend_os/result.hpp"
#include "nisaba/backend_os/signal.hpp"
#include "nisaba/backend_os/platform.hpp"
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace nisaba::backend_os {

/// Window display and role mode.
enum class WindowMode {
    Normal,      ///< Standard application window (XDG Toplevel on Wayland, Normal window on X11).
    LayerShell,  ///< Desktop overlay surface (zwlr_layer_shell_v1).
    Popup,       ///< Child popup window (xdg_popup on Wayland, transient override-redirect on X11).
};

/// Window creation configuration.
struct WindowConfig {
    std::string title        = "Nisaba Window";
    int         width        = 1280;
    int         height       = 800;
    int         x            = -1; // -1 for centered
    int         y            = -1;
    bool        resizable    = true;
    bool        fullscreen   = false;
    bool        borderless   = false;
    bool        transparent  = false; // 32-bit ARGB with compositing
    bool        blur         = true;  ///< Request desktop compositor to blur behind transparent areas
    bool        always_on_top = false;
    bool        override_redirect = false; ///< Bypass window manager (for popups, tooltips, panels)
    int         min_width    = 2;
    int         min_height   = 2;
    bool        vsync        = true;
    bool        csd          = false; ///< Client-Side Decoration mode (frameless + CSD geometry/hints)
    std::string app_id       = "nisaba.app";
    WindowMode  mode         = WindowMode::Normal; ///< Standard window or layer surface overlay.
    class Window* parent_window = nullptr; ///< Parent window if mode == Popup
    class LayerSurface* parent_layer = nullptr; ///< Parent layer surface if mode == Popup
};

/// Represents a native window backed by EGL and OpenGL for  GPU rendering.
class Window {
public:
    /// Create a new native window.
    static Result<std::unique_ptr<Window>> create(Platform& platform, WindowConfig config);

    ~Window();

    // Non-copyable
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    /// Set the window title.
    void setTitle(std::string_view title);

    /// Set window size.
    void setSize(int width, int height);

    /// Set window position on screen.
    void setPosition(int x, int y);

    /// Set whether window is borderless / frameless.
    void setBorderless(bool borderless);

    /// Set whether window stays on top of other windows.
    void setAlwaysOnTop(bool on_top);

    /// Request desktop compositor to blur background behind transparent window regions.
    void setBlurBehind(bool enable);

    // ── Client-Side Decoration (CSD) Actions ────────────────────

    /// Start interactive window move/drag driven by the Wayland compositor or X11 Window Manager.
    void beginMove(float local_x = 0.0f, float local_y = 0.0f, int button = 1);

    /// Start interactive window resize driven by the Wayland compositor or X11 Window Manager.
    void beginResize(WindowEdge edge, float local_x = 0.0f, float local_y = 0.0f, int button = 1);

    /// Request the window to be maximized or restored.
    void setMaximized(bool max);

    /// Request the window to be minimized.
    void setMinimized(bool min);

    /// Request the window to be fullscreen or restored.
    void setFullscreen(bool full);

    /// Toggle maximized state.
    void toggleMaximize();

    /// Request the window manager or compositor to display the window menu.
    void showWindowMenu(float local_x = 0.0f, float local_y = 0.0f, int button = 3);

    /// Set server-side vs client-side window decorations negotiation.
    void setDecorated(bool decorated);

    /// Set the inner window geometry excluding shadows/margins (Wayland window geometry & X11 frame extents).
    void setWindowGeometry(int x, int y, int width, int height);

    /// Check if the window is currently maximized.
    [[nodiscard]] bool isMaximized() const;

    /// Check if the window is currently minimized.
    [[nodiscard]] bool isMinimized() const;

    /// Check if the window is currently fullscreen.
    [[nodiscard]] bool isFullscreen() const;

    /// Check if the window currently has keyboard/input focus.
    [[nodiscard]] bool isActivated() const;

    /// Get the current window state flags.
    [[nodiscard]] WindowState getWindowState() const;

    /// Get current window size in logical/pixel coordinates.
    [[nodiscard]] Size getSize() const;

    /// Get drawable framebuffer size in pixels.
    [[nodiscard]] Size getDrawableSize() const;

    /// Get the DPI scale factor (default 1.0f on standard displays).
    [[nodiscard]] float getDpiScale() const;

    /// Make the OpenGL context of this window current on the active thread.
    void makeCurrent();

    /// Swap front/back buffers to present frame on screen.
    void swapBuffers();

    /// Set damage regions to be committed to the compositor / display server on swapBuffers.
    void setDamage(const std::vector<Rect>& damage_rects);
    void setDamage(const Rect& damage_rect);

    /// Get native window handle (e.g. X11 Window uint64_t or wl_surface*).
    void* getNativeHandle() const;

    /// Get the EGL Surface handle.
    void* getEGLSurface() const;

    /// Get the EGL Context handle.
    void* getEGLContext() const;

    /// Get the internal backend window object (e.g. WaylandWindow*).
    void* getBackendWindow() const;

    /// Get the internal backend layer object (e.g. WaylandLayerSurface*).
    void* getBackendLayer() const;

    // --- Signals ---
    Signal<int, int>&       onResize()           { return on_resize_; }
    Signal<>&               onClose()            { return on_close_; }
    Signal<bool>&           onFocus()            { return on_focus_; }
    Signal<WindowState>&     onStateChanged()     { return on_state_changed_; }
    Signal<bool>&           onMaximized()        { return on_maximized_; }
    Signal<>&               onSurfaceRecreated() { return on_surface_recreated_; }
    Signal<>&               onSurfaceDestroyed() { return on_surface_destroyed_; }

    struct Impl;
    Impl* impl() { return impl_.get(); }

private:
    Window();
    std::unique_ptr<Impl> impl_;
    Signal<int, int>       on_resize_;
    Signal<>               on_close_;
    Signal<bool>           on_focus_;
    Signal<WindowState>     on_state_changed_;
    Signal<bool>           on_maximized_;
    Signal<>               on_surface_recreated_;
    Signal<>               on_surface_destroyed_;
};

}  // namespace nisaba::backend_os
