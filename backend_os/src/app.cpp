/// @file app.cpp
/// @brief Nisaba Native App lifecycle and cross-platform event loop implementation.

#include "nisaba/backend_os/app.hpp"
#include <iostream>
#include <chrono>
#include <thread>
#include <deque>
#include <algorithm>

#if defined(__EMSCRIPTEN__)
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

namespace nisaba::backend_os {

static App* s_app_instance = nullptr;

struct App::Impl {
    AppConfig config;
    std::string title;

    std::unique_ptr<Platform> platform;
    std::unique_ptr<Window>   window;

    FrameCallback main_render_cb;

    // Loop control
    bool quit_requested = false;

    // Multi-surface / Popups
    std::vector<std::unique_ptr<HostedSurface>> surfaces;
    std::vector<std::unique_ptr<HostedSurface>> pending_remove_surfaces;
    HostedSurface* active_popup_host = nullptr;

    // Timing & Real-time performance metrics
    using Clock = std::chrono::steady_clock;
    Clock::time_point last_frame_time;
    Clock::time_point last_fps_sample_time;
    uint32_t          frames_in_sample = 0;
    FrameStats        stats;

    static constexpr size_t kFrameWindow = 120;
    std::deque<double>      frame_times_window;

    void drainPendingSurfaces() {
        if (!pending_remove_surfaces.empty()) {
            pending_remove_surfaces.clear();
            if (window) {
                window->makeCurrent();
            }
        }
    }

    HostedSurface* findSurfaceForHandle(void* handle) {
        if (!handle) return nullptr;
        for (auto& s : surfaces) {
            if (s && s->getNativeHandle() == handle) {
                return s.get();
            }
        }
        return nullptr;
    }

    bool initPlatform() {
        auto result = Platform::create();
        if (!result.isOk()) return false;
        platform = std::move(result.value());
        return true;
    }

    bool initWindow() {
        WindowConfig win_cfg;
        win_cfg.title     = config.title;
        win_cfg.width     = config.width;
        win_cfg.height    = config.height;
        win_cfg.resizable = config.resizable;
        win_cfg.vsync     = config.vsync;
        win_cfg.mode      = config.window_mode;
        win_cfg.app_id    = config.app_id;
        win_cfg.blur      = config.enable_blur;
        win_cfg.csd       = config.enable_csd;

        auto result = Window::create(*platform, win_cfg);
        if (!result.isOk()) return false;
        window = std::move(result.value());
        title = config.title;

        // Hook window close
        window->onClose().connect([this]() {
            quit_requested = true;
        });

        // Targeted event routing for multi-surface / popups
        platform->onTargetedMouseDown().connect([this](void* handle, [[maybe_unused]] float x, [[maybe_unused]] float y, [[maybe_unused]] int btn) {
            HostedSurface* popup_target = findSurfaceForHandle(handle);

            // Auto-dismiss: if click is outside active popups, dismiss auto-dismiss popups in LIFO order
            if (!popup_target && !surfaces.empty()) {
                for (int i = static_cast<int>(surfaces.size()) - 1; i >= 0; --i) {
                    if (surfaces[i]->isAutoDismiss()) {
                        if (active_popup_host == surfaces[i].get()) {
                            active_popup_host = nullptr;
                        }
                        surfaces[i]->onClose().emit();
                        pending_remove_surfaces.push_back(std::move(surfaces[i]));
                        surfaces.erase(surfaces.begin() + i);
                    }
                }
                if (window) {
                    window->makeCurrent();
                }
            }

            if (popup_target) {
                active_popup_host = popup_target;
            }
        });

        platform->onTargetedMouseUp().connect([this](void* handle, [[maybe_unused]] float x, [[maybe_unused]] float y, [[maybe_unused]] int btn) {
            HostedSurface* popup_target = active_popup_host ? active_popup_host : findSurfaceForHandle(handle);
            if (popup_target) {
                active_popup_host = nullptr;
            }
        });

        return true;
    }

    void capFrameRate() {
        if (config.target_fps <= 0) return;

        using namespace std::chrono;
        const auto frame_duration = duration_cast<Clock::duration>(
            duration<double>(1.0 / config.target_fps));

        auto now = Clock::now();
        auto elapsed = now - last_frame_time;
        if (elapsed < frame_duration) {
            std::this_thread::sleep_for(frame_duration - elapsed);
        }
        last_frame_time = Clock::now();
    }

    bool stepFrame() {
        // 1. Poll platform events
        if (!platform->pollEvents() || quit_requested) {
            quit_requested = true;
            return false;
        }

        drainPendingSurfaces();

        // 2. Measure delta time
        auto now = Clock::now();
        std::chrono::duration<double> dt_sec = now - last_frame_time;
        double dt = dt_sec.count();
        if (dt <= 0.0 || dt > 1.0) dt = 1.0 / 60.0;
        last_frame_time = now;

        // 3. Render main window
        if (window) {
            window->makeCurrent();
            if (main_render_cb) {
                main_render_cb(*window, dt);
            }
            window->swapBuffers();
        }

        // 4. Render secondary surfaces / popups
        for (size_t i = 0; i < surfaces.size(); ++i) {
            auto& s = surfaces[i];
            if (!s || !s->window) continue;

            s->makeCurrent();
            if (s->render_cb) {
                s->render_cb(*s->window, dt);
            }
            s->swapBuffers();
        }

        // Restore main window context
        if (!surfaces.empty() && window) {
            window->makeCurrent();
        }

        drainPendingSurfaces();

        // 5. Update frame stats
        double frame_ms = dt * 1000.0;
        stats.frame_time_ms = frame_ms;
        stats.total_frames++;

        frame_times_window.push_back(frame_ms);
        if (frame_times_window.size() > kFrameWindow) {
            frame_times_window.pop_front();
        }

        frames_in_sample++;
        std::chrono::duration<double> sample_elapsed = now - last_fps_sample_time;
        if (sample_elapsed.count() >= 0.5) {
            stats.fps = frames_in_sample / sample_elapsed.count();
            frames_in_sample = 0;
            last_fps_sample_time = now;

            if (!frame_times_window.empty()) {
                std::vector<double> sorted(frame_times_window.begin(), frame_times_window.end());
                std::sort(sorted.begin(), sorted.end());
                size_t p95_idx = static_cast<size_t>(sorted.size() * 0.95);
                if (p95_idx >= sorted.size()) p95_idx = sorted.size() - 1;
                stats.p95_frame_time_ms = sorted[p95_idx];
            }
        }

        // 6. Frame rate pacing
#if !defined(__EMSCRIPTEN__)
        capFrameRate();
#endif
        return true;
    }
};

App::App() : impl_(std::make_unique<Impl>()) {
    s_app_instance = this;
}

App::~App() {
    if (s_app_instance == this) {
        s_app_instance = nullptr;
    }
}

App* App::instance() {
    return s_app_instance;
}

Result<std::unique_ptr<App>> App::create(AppConfig config) {
    auto app = std::unique_ptr<App>(new App());
    app->impl_->config = std::move(config);

    if (!app->impl_->initPlatform()) {
        return Result<std::unique_ptr<App>>::err(
            ErrorCode::PlatformError, "Failed to initialize platform subsystem");
    }

    if (!app->impl_->initWindow()) {
        return Result<std::unique_ptr<App>>::err(
            ErrorCode::WindowError, "Failed to create application window");
    }

    app->impl_->last_frame_time      = Impl::Clock::now();
    app->impl_->last_fps_sample_time = app->impl_->last_frame_time;

    return Result<std::unique_ptr<App>>::ok(std::move(app));
}

int App::run() {
#if defined(__EMSCRIPTEN__)
    emscripten_set_main_loop_arg([](void* arg) {
        auto* app = static_cast<App*>(arg);
        if (!app->impl_->quit_requested) {
            app->impl_->stepFrame();
        } else {
            emscripten_cancel_main_loop();
        }
    }, this, 0, false);
    return 0;
#else
    while (!impl_->quit_requested) {
        if (!impl_->stepFrame()) {
            break;
        }
    }
    return 0;
#endif
}

bool App::stepFrame() {
    return impl_->stepFrame();
}

void App::quit() {
    impl_->quit_requested = true;
}

void App::onFrame(FrameCallback callback) {
    impl_->main_render_cb = std::move(callback);
}

FrameStats App::frameStats() const { return impl_->stats; }
double App::currentFps() const { return impl_->stats.fps; }
double App::currentFrameTimeMs() const { return impl_->stats.frame_time_ms; }

const std::string& App::title() const { return impl_->title; }

void App::setTitle(std::string_view title) {
    impl_->title = title;
    impl_->window->setTitle(title);
}

Size App::windowSize() const { return impl_->window->getSize(); }
float App::dpiScale()   const { return impl_->window->getDpiScale(); }
Platform& App::platform()    { return *impl_->platform; }
Window&   App::window()      { return *impl_->window; }

HostedSurface* App::addPopup(Window* parent, WindowConfig config, FrameCallback render_cb, bool auto_dismiss) {
    config.mode = WindowMode::Popup;
    config.parent_window = parent ? parent : impl_->window.get();

    auto win_res = Window::create(*impl_->platform, config);
    if (!win_res.isOk()) {
        std::cerr << "[Nisaba App] Failed to create Popup Window: " << win_res.error().message << "\n";
        return nullptr;
    }

    auto host = std::make_unique<HostedSurface>();
    host->window = std::move(win_res.value());
    host->render_cb = std::move(render_cb);
    host->auto_dismiss = auto_dismiss;

    HostedSurface* ptr = host.get();
    impl_->surfaces.push_back(std::move(host));
    return ptr;
}

HostedSurface* App::addWindow(WindowConfig config, FrameCallback render_cb) {
    auto win_res = Window::create(*impl_->platform, config);
    if (!win_res.isOk()) {
        std::cerr << "[Nisaba App] Failed to create Window: " << win_res.error().message << "\n";
        return nullptr;
    }

    auto host = std::make_unique<HostedSurface>();
    host->window = std::move(win_res.value());
    host->render_cb = std::move(render_cb);
    host->auto_dismiss = false;

    HostedSurface* ptr = host.get();
    impl_->surfaces.push_back(std::move(host));
    return ptr;
}

void App::removeSurface(HostedSurface* surface) {
    if (!surface) return;
    for (auto it = impl_->surfaces.begin(); it != impl_->surfaces.end(); ++it) {
        if (it->get() == surface) {
            surface->onClose().emit();
            impl_->pending_remove_surfaces.push_back(std::move(*it));
            impl_->surfaces.erase(it);
            break;
        }
    }
}

HostedSurface* App::findSurfaceForHandle(void* handle) const {
    return impl_->findSurfaceForHandle(handle);
}

} // namespace nisaba::backend_os
