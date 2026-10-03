#pragma once
/// @file drm_platform.hpp
/// @brief Linux Direct Rendering Manager (DRM/KMS) + GBM + EGL Native Platform Backend.
/// Standalone embedded platform with zero dependency on X11 or Wayland.

#include "nisaba/backend_os/platform.hpp"
#include "nisaba/backend_os/output.hpp"
#include "nisaba/backend_os/types.hpp"
#include "nisaba/backend_os/signal.hpp"

#include <xf86drm.h>
#include <xf86drmMode.h>
#include <gbm.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include <unordered_set>

namespace nisaba::backend_os {
class Window;
}

namespace nisaba::backend_os::drm {

class DRMInputBackend;
class DRMWindow;

class DRMPlatformBackend {
public:
    explicit DRMPlatformBackend(Platform* owner);
    ~DRMPlatformBackend();

    // Non-copyable
    DRMPlatformBackend(const DRMPlatformBackend&) = delete;
    DRMPlatformBackend& operator=(const DRMPlatformBackend&) = delete;

    /// Initialize DRM device, GBM, EGL display/context, and input backend.
    bool init();

    /// Cleanly shutdown and restore original console/display mode.
    void shutdown();

    /// Poll and dispatch DRM page flip and libinput events.
    bool pollEvents();

    // ── Direct Hardware / Backend Accessors ──────────────────────
    [[nodiscard]] int               getDrmFd()      const { return drm_fd_; }
    [[nodiscard]] struct gbm_device* getGbmDevice()  const { return gbm_dev_; }
    [[nodiscard]] EGLDisplay        getEGLDisplay() const { return egl_display_; }
    [[nodiscard]] EGLConfig         getEGLConfig()  const { return egl_config_; }
    [[nodiscard]] EGLContext        getEGLContext() const { return egl_context_; }
    [[nodiscard]] Platform*         getOwner()      const { return owner_; }
    [[nodiscard]] DRMInputBackend*  getInput()      const { return input_.get(); }

    // ── Session / VT Switch Management ───────────────────────────
    bool dropMaster();
    bool acquireMaster();
    [[nodiscard]] bool isMaster() const { return is_master_; }

    // ── Output / Monitor Subsystem ───────────────────────────────
    [[nodiscard]] std::vector<std::shared_ptr<Output>> getOutputs() const;
    [[nodiscard]] std::shared_ptr<Output> getOutputByName(std::string_view name) const;
    [[nodiscard]] std::shared_ptr<Output> getPrimaryOutput() const;
    void refreshOutputs();

    // ── Window Management ────────────────────────────────────────
    void registerWindow(Window* w);
    void unregisterWindow(Window* w);
    [[nodiscard]] DRMWindow* getActiveWindow() const { return active_window_; }
    void setActiveWindow(DRMWindow* w) { active_window_ = w; }

    /// Find preferred DRM connector, encoder, and CRTC for display.
    bool findDisplayTarget(uint32_t& out_connector_id,
                           uint32_t& out_crtc_id,
                           drmModeModeInfo& out_mode,
                           int target_w = -1, int target_h = -1);

    // ── Hardware Cursor Plane Subsystem ─────────────────────────
    void setCursor(SystemCursor cursor);
    void moveCursor(int x, int y);
    void setActiveCrtcId(uint32_t crtc_id);

private:
    bool openDrmDevice();
    bool initGbm();
    bool initEgl();
    void enumerateOutputs();

    Platform* owner_ = nullptr;

    int               drm_fd_   = -1;
    std::string       drm_card_path_;
    bool              is_master_ = false;

    struct gbm_device* gbm_dev_   = nullptr;
    EGLDisplay        egl_display_ = EGL_NO_DISPLAY;
    EGLConfig         egl_config_  = nullptr;
    EGLContext        egl_context_ = EGL_NO_CONTEXT;

    std::unique_ptr<DRMInputBackend> input_;
    DRMWindow*                       active_window_ = nullptr;
    std::unordered_set<Window*>      windows_;

    std::vector<std::shared_ptr<Output>> outputs_;
    std::shared_ptr<Output>              primary_output_;

    void setupTty();
    void restoreTty();

    bool initCursor();
    void updateCursorImage(SystemCursor cursor);
    void cleanupCursor();

    struct gbm_bo* cursor_bo_        = nullptr;
    uint32_t       cursor_bo_handle_ = 0;
    uint32_t       active_crtc_id_   = 0;
    SystemCursor   current_cursor_   = SystemCursor::Default;
    int            cursor_x_         = 0;
    int            cursor_y_         = 0;
    int            current_hot_x_    = 0;
    int            current_hot_y_    = 0;
    bool           cursor_visible_   = false;

    int  tty_fd_ = -1;
    long orig_kd_mode_ = 0;
    bool tty_mode_changed_ = false;
    void* orig_termios_ = nullptr;

    bool quit_requested_ = false;
};

} // namespace nisaba::backend_os::drm
