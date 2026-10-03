#pragma once
/// @file app.hpp
/// @brief Nisaba Native Application lifecycle manager and cross-platform event loop.

#include "nisaba/backend_os/types.hpp"
#include "nisaba/backend_os/result.hpp"
#include "nisaba/backend_os/signal.hpp"
#include "nisaba/backend_os/platform.hpp"
#include "nisaba/backend_os/window.hpp"

#include <memory>
#include <string>
#include <string_view>
#include <functional>
#include <vector>

namespace nisaba::backend_os {

/// @brief Real-time performance and frame statistics.
struct FrameStats {
    double   fps               = 0.0;   ///< Instantaneous smoothed frames per second.
    double   frame_time_ms     = 0.0;   ///< Total frame duration in milliseconds.
    double   p95_frame_time_ms = 0.0;   ///< 95th-percentile frame time (last 120 frames).
    uint64_t total_frames      = 0;     ///< Total frames processed since start.
};

/// @brief Configuration for the native application.
struct AppConfig {
    std::string title                    = "Nisaba App";
    int         width                    = 1280;
    int         height                   = 800;
    bool        resizable                = true;
    bool        vsync                    = true;
    int         target_fps               = 60;
    WindowMode  window_mode              = WindowMode::Normal;
    bool        enable_csd               = false;
    bool        enable_blur              = true;
    std::string app_id                   = "nisaba.app";
};

/// @brief Represents a secondary or popup surface hosted by App.
struct HostedSurface {
    std::unique_ptr<Window> window;
    std::function<void(Window& win, double dt)> render_cb;
    bool auto_dismiss = false;
    Signal<> on_close;

    [[nodiscard]] void* getNativeHandle() const { return window ? window->getNativeHandle() : nullptr; }
    [[nodiscard]] bool isAutoDismiss() const { return auto_dismiss; }
    Signal<>& onClose() { return on_close; }
    Window* getWindow() const { return window.get(); }
    void swapBuffers() { if (window) window->swapBuffers(); }
    void makeCurrent() { if (window) window->makeCurrent(); }
};

/// @brief Manages the complete lifecycle and event loops of a Nisaba application.
class App {
public:
    using FrameCallback = std::function<void(Window& window, double dt)>;

    /// Create and initialize the application.
    static Result<std::unique_ptr<App>> create(AppConfig config = {});

    ~App();

    App(const App&) = delete;
    App& operator=(const App&) = delete;

    /// Run the application event loop. Blocks until the window is closed or quit() is requested.
    int run();

    /// Execute a single frame step.
    bool stepFrame();

    /// Request application termination.
    void quit();

    /// Attach the main window frame render callback.
    void onFrame(FrameCallback callback);

    /// Access performance metrics.
    [[nodiscard]] FrameStats frameStats() const;
    [[nodiscard]] double currentFps() const;
    [[nodiscard]] double currentFrameTimeMs() const;

    /// Window properties
    [[nodiscard]] const std::string& title() const;
    void setTitle(std::string_view title);
    [[nodiscard]] Size windowSize() const;
    [[nodiscard]] float dpiScale() const;

    /// Direct access to underlying subsystems
    [[nodiscard]] Platform& platform();
    [[nodiscard]] Window& window();

    /// Singleton instance
    static App* instance();

    /// Multi-surface & popup management
    HostedSurface* addPopup(Window* parent, WindowConfig config, FrameCallback render_cb, bool auto_dismiss = true);
    HostedSurface* addWindow(WindowConfig config, FrameCallback render_cb);
    void removeSurface(HostedSurface* surface);
    [[nodiscard]] HostedSurface* findSurfaceForHandle(void* handle) const;

    struct Impl;
    Impl* impl() { return impl_.get(); }

private:
    App();
    std::unique_ptr<Impl> impl_;
};

} // namespace nisaba::backend_os
