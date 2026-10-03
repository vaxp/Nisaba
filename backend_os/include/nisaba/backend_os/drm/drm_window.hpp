#pragma once
/// @file drm_window.hpp
/// @brief Linux DRM/KMS + GBM + EGL scanout surface / window implementation.
/// Manages KMS CRTC mode setting, GBM scanout buffer allocation, and VSync page flipping.

#include "nisaba/backend_os/window.hpp"
#include "nisaba/backend_os/drm/drm_platform.hpp"

#include <xf86drm.h>
#include <xf86drmMode.h>
#include <gbm.h>

#include <EGL/egl.h>
#include <unordered_map>

namespace nisaba::backend_os::drm {

class DRMWindow {
public:
    explicit DRMWindow(DRMPlatformBackend& backend);
    ~DRMWindow();

    // Non-copyable
    DRMWindow(const DRMWindow&) = delete;
    DRMWindow& operator=(const DRMWindow&) = delete;

    bool init(const WindowConfig& config);
    void destroy();

    // ── Window Properties ────────────────────────────────────────
    void setTitle(std::string_view title);
    void setSize(int width, int height);
    void setPosition(int x, int y);
    void setBorderless(bool borderless);
    void setAlwaysOnTop(bool on_top);
    void setBlurBehind(bool enable);

    [[nodiscard]] Size  getSize() const;
    [[nodiscard]] Size  getDrawableSize() const;
    [[nodiscard]] float getDpiScale() const;

    // ── GPU Context & Presentation ───────────────────────────────
    void makeCurrent();
    void swapBuffers();

    // ── Client-Side Decoration / Window Operations (Full Screen KMS)
    void beginMove(float local_x = 0.0f, float local_y = 0.0f, int button = 1);
    void beginResize(WindowEdge edge, float local_x = 0.0f, float local_y = 0.0f, int button = 1);
    void setMaximized(bool max);
    void setMinimized(bool min);
    void setFullscreen(bool full);
    void toggleMaximize();
    void showWindowMenu(float local_x = 0.0f, float local_y = 0.0f, int button = 3);
    void setDecorated(bool decorated);
    void setWindowGeometry(int x, int y, int width, int height);

    [[nodiscard]] bool isMaximized() const { return true; }  // Fullscreen KMS
    [[nodiscard]] bool isMinimized() const { return false; }
    [[nodiscard]] bool isFullscreen() const { return true; }
    [[nodiscard]] bool isActivated() const { return true; }
    [[nodiscard]] WindowState getWindowState() const { return WindowState::Fullscreen | WindowState::Activated; }

    [[nodiscard]] void* getNativeHandle() const { return (void*)gbm_surface_; }
    [[nodiscard]] void* getEGLSurface()   const { return (void*)egl_surface_; }
    [[nodiscard]] void* getEGLContext()   const { return (void*)backend_.getEGLContext(); }

    // Signals
    Signal<WindowState>& onStateChanged() { return on_state_changed_; }
    Signal<bool>&        onMaximized()    { return on_maximized_; }
    Signal<bool>&        onFocus()        { return on_focus_; }
    Signal<int, int>&    onResize()       { return on_resize_; }
    Signal<>&            onClose()        { return on_close_; }

    /// Callback invoked when DRM page flip completes (VBlank)
    void onPageFlipComplete(unsigned int sec, unsigned int usec);

private:
    uint32_t getOrCreateFbForBo(struct gbm_bo* bo);

    DRMPlatformBackend& backend_;
    WindowConfig        config_;

    uint32_t            connector_id_ = 0;
    uint32_t            crtc_id_      = 0;
    drmModeModeInfo     mode_{};
    drmModeCrtcPtr      saved_crtc_   = nullptr;

    struct gbm_surface* gbm_surface_  = nullptr;
    EGLSurface          egl_surface_  = EGL_NO_SURFACE;

    // Double buffering / Page flip tracking
    struct gbm_bo*      current_bo_   = nullptr;
    struct gbm_bo*      previous_bo_  = nullptr;
    bool                waiting_for_flip_ = false;
    bool                crtc_mode_set_ = false;
    bool                async_flip_supported_ = false;

    // Buffer Cache: gbm_bo -> DRM Framebuffer ID
    std::unordered_map<struct gbm_bo*, uint32_t> bo_to_fb_map_;

    int   width_     = 0;
    int   height_    = 0;
    float dpi_scale_ = 1.0f;

    Signal<WindowState> on_state_changed_;
    Signal<bool>        on_maximized_;
    Signal<bool>        on_focus_;
    Signal<int, int>    on_resize_;
    Signal<>            on_close_;
};

} // namespace nisaba::backend_os::drm
