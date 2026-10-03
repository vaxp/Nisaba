/// @file window_native.cpp
/// @brief Window abstraction layer — delegates to X11 or Wayland backend.
/// The Window class is backend-agnostic; it holds an X11Window, WaylandWindow, or WaylandLayerSurface
/// depending on which Platform backend and WindowMode is active.

#include "nisaba/backend_os/window.hpp"
#include "nisaba/backend_os/platform.hpp"

#if defined(__ANDROID__)
#include "nisaba/backend_os/android/android_platform.hpp"
#include "nisaba/backend_os/android/android_surface.hpp"
#elif defined(_WIN32)
#include "nisaba/backend_os/windows/win32_window.hpp"
#include "nisaba/backend_os/windows/win32_platform.hpp"
#elif defined(__EMSCRIPTEN__)
#include "nisaba/backend_os/wasm/wasm_window.hpp"
#include "nisaba/backend_os/wasm/wasm_platform.hpp"
#else
#include "nisaba/backend_os/x11/x11_platform.hpp"
#include "nisaba/backend_os/x11/x11_window.hpp"

#if defined(NISABA_BACKEND_OS_HAS_WAYLAND)
#include "nisaba/backend_os/wayland/wayland_platform.hpp"
#include "nisaba/backend_os/wayland/wayland_surface.hpp"
#include "nisaba/backend_os/wayland/wayland_window.hpp"
#endif
#if defined(NISABA_BACKEND_OS_HAS_DRM)
#include "nisaba/backend_os/drm/drm_platform.hpp"
#include "nisaba/backend_os/drm/drm_window.hpp"
#endif
#endif

#include <iostream>

namespace nisaba::backend_os {

// ════════════════════════════════════════════════════════════════
// Window::Impl  — owned backend handle (Android / Win32 / X11 / Wayland)
// ════════════════════════════════════════════════════════════════
struct Window::Impl {
    Platform* platform = nullptr;
    Window*   window   = nullptr;

    // Active backend
#if defined(__ANDROID__)
    std::unique_ptr<android::AndroidSurface> android_surface;
#elif defined(_WIN32)
    std::unique_ptr<win32::Win32Window>           win32_window;
#elif defined(__EMSCRIPTEN__)
    std::unique_ptr<wasm::WasmWindow>             wasm_window;
#else
    std::unique_ptr<x11::X11Window>               x11;
#if defined(NISABA_BACKEND_OS_HAS_WAYLAND)
    std::unique_ptr<wayland::WaylandWindow>       wayland_window;
    std::unique_ptr<wayland::WaylandLayerSurface> wayland_layer;
#endif
#if defined(NISABA_BACKEND_OS_HAS_DRM)
    std::unique_ptr<drm::DRMWindow>               drm_window;
#endif
#endif

    int current_width  = 0;
    int current_height = 0;

    // ── Factory ─────────────────────────────────────────────────
    bool init(Window* win, Platform& plat, const WindowConfig& cfg) {
        platform = &plat;
        window   = win;

#if defined(__ANDROID__)
        auto* ab = static_cast<android::AndroidPlatformBackend*>(plat.getAndroidBackend());
        if (!ab) {
            std::cerr << "[Nisaba Window] Android backend unavailable\n";
            return false;
        }

        android_surface = std::make_unique<android::AndroidSurface>(*ab);
        if (!android_surface->init()) {
            std::cerr << "[Nisaba Window] Failed to create AndroidSurface\n";
            android_surface.reset();
            return false;
        }
        android_surface->onResize().connect([this](int w, int h) {
            current_width  = w;
            current_height = h;
            if (window) window->onResize().emit(w, h);
        });
        android_surface->onFocus().connect([this](bool f) {
            if (window) window->onFocus().emit(f);
        });
        android_surface->onStateChanged().connect([this](WindowState s) {
            if (window) window->onStateChanged().emit(s);
        });
        android_surface->onClose().connect([this]() {
            if (window) window->onClose().emit();
        });
        android_surface->onSurfaceRecreated().connect([this]() {
            if (window) window->onSurfaceRecreated().emit();
        });
        android_surface->onSurfaceDestroyed().connect([this]() {
            if (window) window->onSurfaceDestroyed().emit();
        });
        current_width  = cfg.width;
        current_height = cfg.height;
        return true;
#elif defined(_WIN32)
        auto* wb = static_cast<win32::Win32PlatformBackend*>(plat.getWin32Backend());
        if (!wb) {
            std::cerr << "[Nisaba Window] Win32 backend unavailable\n";
            return false;
        }

        win32_window = std::make_unique<win32::Win32Window>(*wb);
        if (!win32_window->init(cfg)) {
            win32_window.reset();
            return false;
        }
        win32_window->onClose().connect([this]() {
            if (window) window->onClose().emit();
        });
        win32_window->onFocus().connect([this](bool f) {
            if (window) window->onFocus().emit(f);
        });
        win32_window->onMaximized().connect([this](bool m) {
            if (window) window->onMaximized().emit(m);
        });
        win32_window->onStateChanged().connect([this](WindowState s) {
            if (window) window->onStateChanged().emit(s);
        });
        win32_window->onResize().connect([this](int w, int h) {
            current_width  = w;
            current_height = h;
            if (window) window->onResize().emit(w, h);
        });
        current_width  = cfg.width;
        current_height = cfg.height;
        return true;
#elif defined(__EMSCRIPTEN__)
        auto* wb = static_cast<wasm::WasmPlatformBackend*>(plat.getWasmBackend());
        if (!wb) {
            std::cerr << "[Nisaba Window] Wasm backend unavailable\n";
            return false;
        }

        wasm_window = std::make_unique<wasm::WasmWindow>(*wb);
        if (!wasm_window->init(cfg)) {
            wasm_window.reset();
            return false;
        }
        wasm_window->onClose().connect([this]() {
            if (window) window->onClose().emit();
        });
        wasm_window->onFocus().connect([this](bool f) {
            if (window) window->onFocus().emit(f);
        });
        wasm_window->onMaximized().connect([this](bool m) {
            if (window) window->onMaximized().emit(m);
        });
        wasm_window->onStateChanged().connect([this](WindowState s) {
            if (window) window->onStateChanged().emit(s);
        });
        wasm_window->onResize().connect([this](int w, int h) {
            current_width  = w;
            current_height = h;
            if (window) window->onResize().emit(w, h);
        });
        current_width  = static_cast<int>(wasm_window->getSize().width);
        current_height = static_cast<int>(wasm_window->getSize().height);
        return true;
#else
#if defined(NISABA_BACKEND_OS_HAS_WAYLAND)
        if (plat.isWayland()) {
            auto* wb = static_cast<wayland::WaylandPlatformBackend*>(plat.getWaylandBackend());
            if (!wb) {
                std::cerr << "[Nisaba Window] Wayland backend unavailable\n";
                return false;
            }

            if (cfg.mode == WindowMode::LayerShell) {
                LayerSurfaceConfig lsc;
                lsc.namespace_id   = "nisaba-window";
                lsc.layer          = ShellLayer::Top;
                lsc.anchor         = ShellAnchor::None;     // Floating window
                lsc.width          = cfg.width;
                lsc.height         = cfg.height;
                lsc.exclusive_zone = 0;
                lsc.keyboard_mode  = KeyboardMode::OnDemand;
                lsc.transparent    = cfg.transparent;
                lsc.vsync          = cfg.vsync;

                wayland_layer = std::make_unique<wayland::WaylandLayerSurface>(*wb, lsc);
                if (!wayland_layer->init()) {
                    std::cerr << "[Nisaba Window] Failed to create Wayland layer surface\n";
                    wayland_layer.reset();
                    return false;
                }
                wayland_layer->onClose().connect([this]() {
                    if (window) window->onClose().emit();
                });
                wayland_layer->onResize().connect([this](int w, int h) {
                    if (window) window->onResize().emit(w, h);
                });
            } else {
                wayland_window = std::make_unique<wayland::WaylandWindow>(*wb);
                if (!wayland_window->init(cfg)) {
                    std::cerr << "[Nisaba Window] Failed to create Wayland XDG window\n";
                    wayland_window.reset();
                    return false;
                }
                wayland_window->onClose().connect([this]() {
                    if (window) window->onClose().emit();
                });
                wayland_window->onResize().connect([this](int w, int h) {
                    if (window) window->onResize().emit(w, h);
                });
                wayland_window->onFocus().connect([this](bool f) {
                    if (window) window->onFocus().emit(f);
                });
                wayland_window->onMaximized().connect([this](bool m) {
                    if (window) window->onMaximized().emit(m);
                });
                wayland_window->onStateChanged().connect([this](WindowState s) {
                    if (window) window->onStateChanged().emit(s);
                });
            }
            current_width  = cfg.width;
            current_height = cfg.height;
            return true;
        }
#endif

#if defined(NISABA_BACKEND_OS_HAS_DRM)
        if (plat.isDRM()) {
            auto* db = static_cast<drm::DRMPlatformBackend*>(plat.getDRMBackend());
            if (!db) {
                std::cerr << "[Nisaba Window] DRM backend unavailable\n";
                return false;
            }

            drm_window = std::make_unique<drm::DRMWindow>(*db);
            if (!drm_window->init(cfg)) {
                std::cerr << "[Nisaba Window] Failed to create DRM KMS scanout window\n";
                drm_window.reset();
                return false;
            }

            drm_window->onClose().connect([this]() {
                if (window) window->onClose().emit();
            });
            drm_window->onResize().connect([this](int w, int h) {
                current_width  = w;
                current_height = h;
                if (window) window->onResize().emit(w, h);
            });
            drm_window->onFocus().connect([this](bool f) {
                if (window) window->onFocus().emit(f);
            });
            drm_window->onStateChanged().connect([this](WindowState s) {
                if (window) window->onStateChanged().emit(s);
            });

            current_width  = static_cast<int>(drm_window->getSize().width);
            current_height = static_cast<int>(drm_window->getSize().height);
            return true;
        }
#endif

        // X11 path
        auto* xb = static_cast<x11::X11PlatformBackend*>(plat.getX11Backend());
        if (!xb) {
            std::cerr << "[Nisaba Window] X11 backend unavailable\n";
            return false;
        }

        x11 = std::make_unique<x11::X11Window>(*xb);
        if (!x11->init(cfg)) {
            x11.reset();
            return false;
        }
        x11->onFocus().connect([this](bool f) {
            if (window) window->onFocus().emit(f);
        });
        x11->onMaximized().connect([this](bool m) {
            if (window) window->onMaximized().emit(m);
        });
        x11->onStateChanged().connect([this](WindowState s) {
            if (window) window->onStateChanged().emit(s);
        });
        x11->onResize().connect([this](int w, int h) {
            current_width  = w;
            current_height = h;
            if (window) window->onResize().emit(w, h);
        });
        current_width  = cfg.width;
        current_height = cfg.height;
        return true;
#endif
    }

    void destroy() {
#if defined(__ANDROID__)
        if (android_surface) { android_surface.reset(); }
#elif defined(_WIN32)
        if (win32_window) { win32_window.reset(); }
#elif defined(__EMSCRIPTEN__)
        if (wasm_window) { wasm_window.reset(); }
#else
        if (x11) { x11.reset(); }
#if defined(NISABA_BACKEND_OS_HAS_WAYLAND)
        if (wayland_window) { wayland_window.reset(); }
        if (wayland_layer)  { wayland_layer.reset(); }
#endif
#if defined(NISABA_BACKEND_OS_HAS_DRM)
        if (drm_window) { drm_window.reset(); }
#endif
#endif
    }
};

// ════════════════════════════════════════════════════════════════
// Window — Public API
// ════════════════════════════════════════════════════════════════

Window::Window() : impl_(std::make_unique<Impl>()) {}

Window::~Window() {
    if (impl_) {
        if (impl_->platform) impl_->platform->unregisterWindow(this);
        impl_->destroy();
    }
}

Result<std::unique_ptr<Window>> Window::create(Platform& platform, WindowConfig config) {
    auto window = std::unique_ptr<Window>(new Window());
    if (!window->impl_->init(window.get(), platform, config)) {
        return Result<std::unique_ptr<Window>>::err(
            ErrorCode::WindowError, "Failed to initialize Window");
    }
    platform.registerWindow(window.get());

    window->onResize().connect([w = window.get()](int nw, int nh) {
        w->impl_->current_width  = nw;
        w->impl_->current_height = nh;
    });

    return Result<std::unique_ptr<Window>>::ok(std::move(window));
}

// ── Mutators ────────────────────────────────────────────────────
void Window::setTitle(std::string_view title) {
#if defined(__ANDROID__)
    if (impl_->android_surface) impl_->android_surface->setTitle(title);
#elif defined(_WIN32)
    if (impl_->win32_window) impl_->win32_window->setTitle(title);
#elif defined(__EMSCRIPTEN__)
    if (impl_->wasm_window) impl_->wasm_window->setTitle(title);
#else
    if (impl_->x11) impl_->x11->setTitle(title);
#if defined(NISABA_BACKEND_OS_HAS_WAYLAND)
    if (impl_->wayland_window) impl_->wayland_window->setTitle(title);
#endif
#if defined(NISABA_BACKEND_OS_HAS_DRM)
    if (impl_->drm_window) impl_->drm_window->setTitle(title);
#endif
#endif
}

void Window::setSize(int w, int h) {
#if defined(__ANDROID__)
    if (impl_->android_surface) impl_->android_surface->setSize(w, h);
#elif defined(_WIN32)
    if (impl_->win32_window) impl_->win32_window->setSize(w, h);
#elif defined(__EMSCRIPTEN__)
    if (impl_->wasm_window) impl_->wasm_window->setSize(w, h);
#else
    if (impl_->x11) impl_->x11->setSize(w, h);
#if defined(NISABA_BACKEND_OS_HAS_WAYLAND)
    if (impl_->wayland_window) impl_->wayland_window->setSize(w, h);
    if (impl_->wayland_layer)  impl_->wayland_layer->setSize(w, h);
#endif
#if defined(NISABA_BACKEND_OS_HAS_DRM)
    if (impl_->drm_window) impl_->drm_window->setSize(w, h);
#endif
#endif
    impl_->current_width  = w;
    impl_->current_height = h;
}

void Window::setPosition(int x, int y) {
#if defined(__ANDROID__)
    if (impl_->android_surface) impl_->android_surface->setPosition(x, y);
#elif defined(_WIN32)
    if (impl_->win32_window) impl_->win32_window->setPosition(x, y);
#elif defined(__EMSCRIPTEN__)
    if (impl_->wasm_window) impl_->wasm_window->setPosition(x, y);
#else
    if (impl_->x11) impl_->x11->setPosition(x, y);
#if defined(NISABA_BACKEND_OS_HAS_WAYLAND)
    if (impl_->wayland_window) impl_->wayland_window->setPosition(x, y);
#endif
#if defined(NISABA_BACKEND_OS_HAS_DRM)
    if (impl_->drm_window) impl_->drm_window->setPosition(x, y);
#endif
#endif
}

void Window::setBorderless(bool b) {
#if defined(__ANDROID__)
    if (impl_->android_surface) impl_->android_surface->setBorderless(b);
#elif defined(_WIN32)
    if (impl_->win32_window) impl_->win32_window->setBorderless(b);
#elif defined(__EMSCRIPTEN__)
    if (impl_->wasm_window) impl_->wasm_window->setBorderless(b);
#else
    if (impl_->x11) impl_->x11->setBorderless(b);
#endif
}

void Window::setAlwaysOnTop(bool t) {
#if defined(__ANDROID__)
    if (impl_->android_surface) impl_->android_surface->setAlwaysOnTop(t);
#elif defined(_WIN32)
    if (impl_->win32_window) impl_->win32_window->setAlwaysOnTop(t);
#elif defined(__EMSCRIPTEN__)
    // Not applicable in web browser
#else
    if (impl_->x11) impl_->x11->setAlwaysOnTop(t);
#endif
}

void Window::setBlurBehind(bool enable) {
#if defined(__ANDROID__)
    if (impl_->android_surface) impl_->android_surface->setBlurBehind(enable);
#elif defined(_WIN32)
    if (impl_->win32_window) impl_->win32_window->setBlurBehind(enable);
#elif defined(__EMSCRIPTEN__)
    // Not applicable in web browser
#else
    if (impl_->x11) impl_->x11->setBlurBehind(enable);
#endif
}

// ── Accessors ───────────────────────────────────────────────────
Size Window::getSize() const {
#if defined(__ANDROID__)
    if (impl_->android_surface) return impl_->android_surface->getSize();
#elif defined(_WIN32)
    if (impl_->win32_window) return impl_->win32_window->getSize();
#elif defined(__EMSCRIPTEN__)
    if (impl_->wasm_window) return impl_->wasm_window->getSize();
#else
    if (impl_->x11) return impl_->x11->getSize();
#if defined(NISABA_BACKEND_OS_HAS_WAYLAND)
    if (impl_->wayland_window) return impl_->wayland_window->getSize();
    if (impl_->wayland_layer)  return impl_->wayland_layer->getSize();
#endif
#if defined(NISABA_BACKEND_OS_HAS_DRM)
    if (impl_->drm_window) return impl_->drm_window->getSize();
#endif
#endif
    return {(float)impl_->current_width, (float)impl_->current_height};
}

Size Window::getDrawableSize() const {
#if defined(__ANDROID__)
    if (impl_->android_surface) return impl_->android_surface->getDrawableSize();
#elif defined(_WIN32)
    if (impl_->win32_window) return impl_->win32_window->getDrawableSize();
#elif defined(__EMSCRIPTEN__)
    if (impl_->wasm_window) return impl_->wasm_window->getDrawableSize();
#else
    if (impl_->x11) return impl_->x11->getDrawableSize();
#if defined(NISABA_BACKEND_OS_HAS_WAYLAND)
    if (impl_->wayland_window) return impl_->wayland_window->getDrawableSize();
    if (impl_->wayland_layer)  return impl_->wayland_layer->getDrawableSize();
#endif
#if defined(NISABA_BACKEND_OS_HAS_DRM)
    if (impl_->drm_window) return impl_->drm_window->getDrawableSize();
#endif
#endif
    return getSize();
}

float Window::getDpiScale() const {
#if defined(__ANDROID__)
    if (impl_->android_surface) return impl_->android_surface->getDpiScale();
#elif defined(_WIN32)
    if (impl_->win32_window) return impl_->win32_window->getDpiScale();
#elif defined(__EMSCRIPTEN__)
    if (impl_->wasm_window) return impl_->wasm_window->getDpiScale();
#else
    if (impl_->x11) return impl_->x11->getDpiScale();
#if defined(NISABA_BACKEND_OS_HAS_WAYLAND)
    if (impl_->wayland_window) return impl_->wayland_window->getDpiScale();
    if (impl_->wayland_layer)  return impl_->wayland_layer->getDpiScale();
#endif
#if defined(NISABA_BACKEND_OS_HAS_DRM)
    if (impl_->drm_window) return impl_->drm_window->getDpiScale();
#endif
#endif
    return 1.0f;
}

void Window::makeCurrent() {
#if defined(__ANDROID__)
    if (impl_->android_surface) impl_->android_surface->makeCurrent();
#elif defined(_WIN32)
    if (impl_->win32_window) impl_->win32_window->makeCurrent();
#elif defined(__EMSCRIPTEN__)
    if (impl_->wasm_window) impl_->wasm_window->makeCurrent();
#else
    if (impl_->x11) impl_->x11->makeCurrent();
#if defined(NISABA_BACKEND_OS_HAS_WAYLAND)
    if (impl_->wayland_window) impl_->wayland_window->makeCurrent();
    if (impl_->wayland_layer)  impl_->wayland_layer->makeCurrent();
#endif
#if defined(NISABA_BACKEND_OS_HAS_DRM)
    if (impl_->drm_window) impl_->drm_window->makeCurrent();
#endif
#endif
}

void Window::swapBuffers() {
#if defined(__ANDROID__)
    if (impl_->android_surface) impl_->android_surface->swapBuffers();
#elif defined(_WIN32)
    if (impl_->win32_window) impl_->win32_window->swapBuffers();
#elif defined(__EMSCRIPTEN__)
    if (impl_->wasm_window) impl_->wasm_window->swapBuffers();
#else
    if (impl_->x11) impl_->x11->swapBuffers();
#if defined(NISABA_BACKEND_OS_HAS_WAYLAND)
    if (impl_->wayland_window) impl_->wayland_window->swapBuffers();
    if (impl_->wayland_layer)  impl_->wayland_layer->swapBuffers();
#endif
#if defined(NISABA_BACKEND_OS_HAS_DRM)
    if (impl_->drm_window) impl_->drm_window->swapBuffers();
#endif
#endif
}

void Window::setDamage(const std::vector<Rect>& damage_rects) {
#if defined(NISABA_BACKEND_OS_HAS_WAYLAND)
    if (impl_->wayland_window) impl_->wayland_window->setDamage(damage_rects);
#else
    (void)damage_rects;
#endif
}

void Window::setDamage(const Rect& damage_rect) {
    setDamage(std::vector<Rect>{damage_rect});
}


void* Window::getNativeHandle() const {

#if defined(__ANDROID__)
    if (impl_->android_surface) return impl_->android_surface->getNativeHandle();
#elif defined(_WIN32)
    if (impl_->win32_window) return impl_->win32_window->getNativeHandle();
#elif defined(__EMSCRIPTEN__)
    if (impl_->wasm_window) return impl_->wasm_window->getNativeHandle();
#else
    if (impl_->x11) return impl_->x11->getNativeHandle();
#if defined(NISABA_BACKEND_OS_HAS_WAYLAND)
    if (impl_->wayland_window) return impl_->wayland_window->getNativeHandle();
    if (impl_->wayland_layer)  return impl_->wayland_layer->getWlSurface();
#endif
#if defined(NISABA_BACKEND_OS_HAS_DRM)
    if (impl_->drm_window) return impl_->drm_window->getNativeHandle();
#endif
#endif
    return nullptr;
}

void* Window::getEGLSurface() const {
#if defined(__ANDROID__)
    if (impl_->android_surface) return impl_->android_surface->getEGLSurface();
    return nullptr;
#elif defined(_WIN32) || defined(__EMSCRIPTEN__)
    return nullptr;
#else
    if (impl_->x11) return impl_->x11->getEGLSurface();
#if defined(NISABA_BACKEND_OS_HAS_WAYLAND)
    if (impl_->wayland_window) return impl_->wayland_window->getEGLSurface();
    if (impl_->wayland_layer)  return impl_->wayland_layer->getEGLSurface();
#endif
#if defined(NISABA_BACKEND_OS_HAS_DRM)
    if (impl_->drm_window) return impl_->drm_window->getEGLSurface();
#endif
    return nullptr;
#endif
}

void* Window::getEGLContext() const {
#if defined(__ANDROID__)
    if (impl_->android_surface) return impl_->android_surface->getEGLContext();
    return nullptr;
#elif defined(_WIN32)
    if (impl_->win32_window) return impl_->win32_window->getEGLContext();
    return nullptr;
#elif defined(__EMSCRIPTEN__)
    if (impl_->wasm_window) return impl_->wasm_window->getEGLContext();
    return nullptr;
#else
    if (impl_->x11) return impl_->x11->getEGLContext();
#if defined(NISABA_BACKEND_OS_HAS_WAYLAND)
    if (impl_->wayland_window) return impl_->wayland_window->getEGLContext();
    if (impl_->wayland_layer)  return impl_->wayland_layer->getEGLContext();
#endif
#if defined(NISABA_BACKEND_OS_HAS_DRM)
    if (impl_->drm_window) return impl_->drm_window->getEGLContext();
#endif
    return nullptr;
#endif
}

void* Window::getBackendWindow() const {
#if defined(__ANDROID__)
    if (impl_->android_surface) return impl_->android_surface.get();
#elif defined(_WIN32)
    if (impl_->win32_window) return impl_->win32_window.get();
#elif defined(__EMSCRIPTEN__)
    if (impl_->wasm_window) return impl_->wasm_window.get();
#else
    if (impl_->x11) return impl_->x11.get();
#if defined(NISABA_BACKEND_OS_HAS_WAYLAND)
    if (impl_->wayland_window) return impl_->wayland_window.get();
#endif
#if defined(NISABA_BACKEND_OS_HAS_DRM)
    if (impl_->drm_window) return impl_->drm_window.get();
#endif
#endif
    return nullptr;
}

void* Window::getBackendLayer() const {
#if defined(NISABA_BACKEND_OS_HAS_WAYLAND) && !defined(_WIN32) && !defined(__ANDROID__)
    if (impl_->wayland_layer) return impl_->wayland_layer.get();
#endif
    return nullptr;
}

// ── Client-Side Decoration (CSD) Operations ─────────────────────

void Window::beginMove(float local_x, float local_y, int button) {
#if defined(__ANDROID__)
    if (impl_->android_surface) impl_->android_surface->beginMove(local_x, local_y, button);
#elif defined(_WIN32)
    if (impl_->win32_window) impl_->win32_window->beginMove(local_x, local_y, button);
#elif defined(__EMSCRIPTEN__)
    if (impl_->wasm_window) impl_->wasm_window->beginMove(local_x, local_y, button);
#else
    if (impl_->x11) impl_->x11->beginMove(local_x, local_y, button);
#if defined(NISABA_BACKEND_OS_HAS_WAYLAND)
    if (impl_->wayland_window) impl_->wayland_window->beginMove(local_x, local_y, button);
#endif
#endif
}

void Window::beginResize(WindowEdge edge, float local_x, float local_y, int button) {
#if defined(__ANDROID__)
    if (impl_->android_surface) impl_->android_surface->beginResize(edge, local_x, local_y, button);
#elif defined(_WIN32)
    if (impl_->win32_window) impl_->win32_window->beginResize(edge, local_x, local_y, button);
#elif defined(__EMSCRIPTEN__)
    if (impl_->wasm_window) impl_->wasm_window->beginResize(edge, local_x, local_y, button);
#else
    if (impl_->x11) impl_->x11->beginResize(edge, local_x, local_y, button);
#if defined(NISABA_BACKEND_OS_HAS_WAYLAND)
    if (impl_->wayland_window) impl_->wayland_window->beginResize(edge, local_x, local_y, button);
#endif
#endif
}

void Window::setMaximized(bool max) {
#if defined(__ANDROID__)
    if (impl_->android_surface) impl_->android_surface->setMaximized(max);
#elif defined(_WIN32)
    if (impl_->win32_window) impl_->win32_window->setMaximized(max);
#elif defined(__EMSCRIPTEN__)
    if (impl_->wasm_window) impl_->wasm_window->setMaximized(max);
#else
    if (impl_->x11) impl_->x11->setMaximized(max);
#if defined(NISABA_BACKEND_OS_HAS_WAYLAND)
    if (impl_->wayland_window) impl_->wayland_window->setMaximized(max);
#endif
#endif
}

void Window::setMinimized(bool min) {
#if defined(__ANDROID__)
    if (impl_->android_surface) impl_->android_surface->setMinimized(min);
#elif defined(_WIN32)
    if (impl_->win32_window) impl_->win32_window->setMinimized(min);
#elif defined(__EMSCRIPTEN__)
    if (impl_->wasm_window) impl_->wasm_window->setMinimized(min);
#else
    if (impl_->x11) impl_->x11->setMinimized(min);
#if defined(NISABA_BACKEND_OS_HAS_WAYLAND)
    if (impl_->wayland_window) impl_->wayland_window->setMinimized(min);
#endif
#endif
}

void Window::setFullscreen(bool full) {
#if defined(__ANDROID__)
    if (impl_->android_surface) impl_->android_surface->setFullscreen(full);
#elif defined(_WIN32)
    if (impl_->win32_window) impl_->win32_window->setFullscreen(full);
#elif defined(__EMSCRIPTEN__)
    if (impl_->wasm_window) impl_->wasm_window->setFullscreen(full);
#else
    if (impl_->x11) impl_->x11->setFullscreen(full);
#if defined(NISABA_BACKEND_OS_HAS_WAYLAND)
    if (impl_->wayland_window) impl_->wayland_window->setFullscreen(full);
#endif
#endif
}

void Window::toggleMaximize() {
#if defined(__ANDROID__)
    if (impl_->android_surface) impl_->android_surface->toggleMaximize();
#elif defined(_WIN32)
    if (impl_->win32_window) impl_->win32_window->toggleMaximize();
#elif defined(__EMSCRIPTEN__)
    if (impl_->wasm_window) impl_->wasm_window->toggleMaximize();
#else
    if (impl_->x11) impl_->x11->toggleMaximize();
#if defined(NISABA_BACKEND_OS_HAS_WAYLAND)
    if (impl_->wayland_window) impl_->wayland_window->toggleMaximize();
#endif
#endif
}

void Window::showWindowMenu(float local_x, float local_y, int button) {
#if defined(__ANDROID__)
    if (impl_->android_surface) impl_->android_surface->showWindowMenu(local_x, local_y, button);
#elif defined(_WIN32)
    if (impl_->win32_window) impl_->win32_window->showWindowMenu(local_x, local_y, button);
#elif defined(__EMSCRIPTEN__)
    // Not applicable
#else
    if (impl_->x11) impl_->x11->showWindowMenu(local_x, local_y, button);
#if defined(NISABA_BACKEND_OS_HAS_WAYLAND)
    if (impl_->wayland_window) impl_->wayland_window->showWindowMenu(local_x, local_y, button);
#endif
#endif
}

void Window::setDecorated(bool decorated) {
#if defined(__ANDROID__)
    if (impl_->android_surface) impl_->android_surface->setDecorated(decorated);
#elif defined(_WIN32)
    if (impl_->win32_window) impl_->win32_window->setDecorated(decorated);
#elif defined(__EMSCRIPTEN__)
    // Not applicable in canvas
#else
    if (impl_->x11) impl_->x11->setDecorated(decorated);
#if defined(NISABA_BACKEND_OS_HAS_WAYLAND)
    if (impl_->wayland_window) impl_->wayland_window->setDecorated(decorated);
#endif
#endif
}

void Window::setWindowGeometry(int x, int y, int width, int height) {
#if defined(__ANDROID__)
    if (impl_->android_surface) impl_->android_surface->setWindowGeometry(x, y, width, height);
#elif defined(_WIN32)
    if (impl_->win32_window) impl_->win32_window->setWindowGeometry(x, y, width, height);
#elif defined(__EMSCRIPTEN__)
    if (impl_->wasm_window) impl_->wasm_window->setSize(width, height);
#else
    if (impl_->x11) impl_->x11->setWindowGeometry(x, y, width, height);
#if defined(NISABA_BACKEND_OS_HAS_WAYLAND)
    if (impl_->wayland_window) impl_->wayland_window->setWindowGeometry(x, y, width, height);
#endif
#endif
}

bool Window::isMaximized() const {
#if defined(__ANDROID__)
    if (impl_->android_surface) return impl_->android_surface->isMaximized();
#elif defined(_WIN32)
    if (impl_->win32_window) return impl_->win32_window->isMaximized();
#elif defined(__EMSCRIPTEN__)
    if (impl_->wasm_window) return impl_->wasm_window->isMaximized();
#else
    if (impl_->x11) return impl_->x11->isMaximized();
#if defined(NISABA_BACKEND_OS_HAS_WAYLAND)
    if (impl_->wayland_window) return impl_->wayland_window->isMaximized();
#endif
#if defined(NISABA_BACKEND_OS_HAS_DRM)
    if (impl_->drm_window) return impl_->drm_window->isMaximized();
#endif
#endif
    return false;
}

bool Window::isMinimized() const {
#if defined(__ANDROID__)
    if (impl_->android_surface) return impl_->android_surface->isMinimized();
#elif defined(_WIN32)
    if (impl_->win32_window) return impl_->win32_window->isMinimized();
#elif defined(__EMSCRIPTEN__)
    if (impl_->wasm_window) return impl_->wasm_window->isMinimized();
#else
    if (impl_->x11) return impl_->x11->isMinimized();
#if defined(NISABA_BACKEND_OS_HAS_WAYLAND)
    if (impl_->wayland_window) return impl_->wayland_window->isMinimized();
#endif
#if defined(NISABA_BACKEND_OS_HAS_DRM)
    if (impl_->drm_window) return impl_->drm_window->isMinimized();
#endif
#endif
    return false;
}

bool Window::isFullscreen() const {
#if defined(__ANDROID__)
    if (impl_->android_surface) return impl_->android_surface->isFullscreen();
#elif defined(_WIN32)
    if (impl_->win32_window) return impl_->win32_window->isFullscreen();
#elif defined(__EMSCRIPTEN__)
    if (impl_->wasm_window) return impl_->wasm_window->isFullscreen();
#else
    if (impl_->x11) return impl_->x11->isFullscreen();
#if defined(NISABA_BACKEND_OS_HAS_WAYLAND)
    if (impl_->wayland_window) return impl_->wayland_window->isFullscreen();
#endif
#if defined(NISABA_BACKEND_OS_HAS_DRM)
    if (impl_->drm_window) return impl_->drm_window->isFullscreen();
#endif
#endif
    return false;
}

bool Window::isActivated() const {
#if defined(__ANDROID__)
    if (impl_->android_surface) return impl_->android_surface->isActivated();
#elif defined(_WIN32)
    if (impl_->win32_window) return impl_->win32_window->isActivated();
#elif defined(__EMSCRIPTEN__)
    if (impl_->wasm_window) return impl_->wasm_window->isActivated();
#else
    if (impl_->x11) return impl_->x11->isActivated();
#if defined(NISABA_BACKEND_OS_HAS_WAYLAND)
    if (impl_->wayland_window) return impl_->wayland_window->isActivated();
#endif
#if defined(NISABA_BACKEND_OS_HAS_DRM)
    if (impl_->drm_window) return impl_->drm_window->isActivated();
#endif
#endif
    return true;
}

WindowState Window::getWindowState() const {
#if defined(__ANDROID__)
    if (impl_->android_surface) return impl_->android_surface->getWindowState();
#elif defined(_WIN32)
    if (impl_->win32_window) return impl_->win32_window->getWindowState();
#elif defined(__EMSCRIPTEN__)
    if (impl_->wasm_window) return impl_->wasm_window->getWindowState();
#else
    if (impl_->x11) return impl_->x11->getWindowState();
#if defined(NISABA_BACKEND_OS_HAS_WAYLAND)
    if (impl_->wayland_window) return impl_->wayland_window->getWindowState();
#endif
#if defined(NISABA_BACKEND_OS_HAS_DRM)
    if (impl_->drm_window) return impl_->drm_window->getWindowState();
#endif
#endif
    return WindowState::Normal;
}

}  // namespace nisaba::backend_os

