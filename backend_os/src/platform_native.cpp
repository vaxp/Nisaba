/// @file platform_native.cpp
/// @brief Platform abstraction layer — auto-detects and owns Wayland or X11 backend.
/// Downstream code (Window, LayerSurface, App) is fully backend-agnostic.

#include "nisaba/backend_os/platform.hpp"
#include "nisaba/backend_os/window.hpp"

#if defined(__ANDROID__)
#include "nisaba/backend_os/android/android_platform.hpp"
#elif defined(_WIN32)
#include "nisaba/backend_os/windows/win32_platform.hpp"
#elif defined(__EMSCRIPTEN__)
#include "nisaba/backend_os/wasm/wasm_platform.hpp"
#else
#include "nisaba/backend_os/x11/x11_platform.hpp"
#if defined(NISABA_BACKEND_OS_HAS_WAYLAND)
#include "nisaba/backend_os/wayland/wayland_platform.hpp"
#endif
#if defined(NISABA_BACKEND_OS_HAS_DRM)
#include "nisaba/backend_os/drm/drm_platform.hpp"
#endif
#endif

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <unordered_set>

namespace nisaba::backend_os {

static Platform* g_platform_instance = nullptr;

#if defined(__ANDROID__)
// Static slot for the ANativeActivity — set by android_app_glue before
// Platform::create() is called.
static ::ANativeActivity* s_pending_activity_ = nullptr;

void Platform::setAndroidActivity(::ANativeActivity* activity) {
    s_pending_activity_ = activity;
}
#endif

// ════════════════════════════════════════════════════════════════
// Platform::Impl  — owns active backend (Android / Win32 / Wayland/X11)
// ════════════════════════════════════════════════════════════════
struct Platform::Impl {
    Platform* owner = nullptr;

#if defined(__ANDROID__)
    std::unique_ptr<android::AndroidPlatformBackend> android_backend;
#elif defined(_WIN32)
    std::unique_ptr<win32::Win32PlatformBackend> win32;
#elif defined(__EMSCRIPTEN__)
    std::unique_ptr<wasm::WasmPlatformBackend> wasm_backend;
#else
    std::unique_ptr<wayland::WaylandPlatformBackend> wayland;
    std::unique_ptr<x11::X11PlatformBackend>         x11;
#if defined(NISABA_BACKEND_OS_HAS_DRM)
    std::unique_ptr<drm::DRMPlatformBackend>         drm;
#endif
#endif

    std::chrono::steady_clock::time_point start_time;
    std::unordered_set<Window*> windows;
    std::string clipboard_buffer;

    // ── Backend Selection ────────────────────────────────────────
    bool init() {
        start_time = std::chrono::steady_clock::now();

#if defined(__ANDROID__)
        // Activity pointer is set by android_app_glue before Platform::create()
        // via the static accessor below.
        ANativeActivity* activity = s_pending_activity_;
        if (!activity) {
            std::cerr << "[Nisaba Platform] Android: no ANativeActivity set. "
                         "Call Platform::setAndroidActivity() before Platform::create()\n";
            return false;
        }
        android_backend = std::make_unique<android::AndroidPlatformBackend>(owner, activity);
        if (!android_backend->init()) {
            android_backend.reset();
            return false;
        }
        return true;
#elif defined(_WIN32)
        win32 = std::make_unique<win32::Win32PlatformBackend>(owner);
        if (!win32->init()) {
            win32.reset();
            return false;
        }
        return true;
#elif defined(__EMSCRIPTEN__)
        wasm_backend = std::make_unique<wasm::WasmPlatformBackend>(owner);
        if (!wasm_backend->init()) {
            wasm_backend.reset();
            return false;
        }
        return true;
#else
#if defined(NISABA_BACKEND_OS_HAS_DRM)
        // Explicit DRM preference via environment variable
        const char* plat_env = std::getenv("Nisaba_PLATFORM");
        const char* back_env = std::getenv("Nisaba_BACKEND");
        bool prefer_drm = (plat_env && (std::string_view(plat_env) == "drm" || std::string_view(plat_env) == "kms")) ||
                          (back_env && (std::string_view(back_env) == "drm" || std::string_view(back_env) == "kms"));

        if (prefer_drm) {
            drm = std::make_unique<drm::DRMPlatformBackend>(owner);
            if (drm->init()) {
                return true;
            }
            std::cerr << "[Nisaba Platform] Explicit DRM backend requested but failed to initialize\n";
            drm.reset();
        }
#endif

        // Prefer Wayland if environment indicates it
        const char* wl_disp = std::getenv("WAYLAND_DISPLAY");
        if (wl_disp || std::getenv("WAYLAND_SOCKET")) {
#if defined(NISABA_BACKEND_OS_HAS_WAYLAND)
            wayland = std::make_unique<wayland::WaylandPlatformBackend>(owner);
            if (wayland->init()) {
                return true;
            }
            std::cerr << "[Nisaba Platform] Wayland failed — falling back to X11\n";
            wayland.reset();
#endif
        }

        // X11 check / fallback
        const char* x11_disp = std::getenv("DISPLAY");
        if (x11_disp) {
            x11 = std::make_unique<x11::X11PlatformBackend>(owner);
            if (x11->init()) {
                return true;
            }
            std::cerr << "[Nisaba Platform] X11 failed\n";
            x11.reset();
        }

#if defined(NISABA_BACKEND_OS_HAS_DRM)
        // Fallback to DRM/KMS if neither Wayland nor X11 was present or succeeded
        drm = std::make_unique<drm::DRMPlatformBackend>(owner);
        if (drm->init()) {
            return true;
        }
        drm.reset();
#endif

        // Final X11 attempt if nothing else succeeded
        if (!x11) {
            x11 = std::make_unique<x11::X11PlatformBackend>(owner);
            if (!x11->init()) {
                x11.reset();
                return false;
            }
        }
        return true;
#endif
    }

    void shutdown() {
#if defined(__ANDROID__)
        if (android_backend) { android_backend->shutdown(); android_backend.reset(); }
#elif defined(_WIN32)
        if (win32)   { win32->shutdown();   win32.reset(); }
#elif defined(__EMSCRIPTEN__)
        if (wasm_backend) { wasm_backend->shutdown(); wasm_backend.reset(); }
#else
        if (wayland) { wayland->shutdown(); wayland.reset(); }
        if (x11)     { x11->shutdown();     x11.reset(); }
#if defined(NISABA_BACKEND_OS_HAS_DRM)
        if (drm)     { drm->shutdown();     drm.reset(); }
#endif
#endif
    }

    bool isWayland() const {
#if defined(__ANDROID__) || defined(_WIN32) || defined(__EMSCRIPTEN__)
        return false;
#else
        return wayland != nullptr;
#endif
    }

    bool isAndroid() const {
#if defined(__ANDROID__)
        return android_backend != nullptr;
#else
        return false;
#endif
    }

    bool isDRM() const {
#if defined(__ANDROID__) || defined(_WIN32) || defined(__EMSCRIPTEN__)
        return false;
#elif defined(NISABA_BACKEND_OS_HAS_DRM)
        return drm != nullptr;
#else
        return false;
#endif
    }

    bool isWasm() const {
#if defined(__EMSCRIPTEN__)
        return wasm_backend != nullptr;
#else
        return false;
#endif
    }
};


// ════════════════════════════════════════════════════════════════
// Platform — Public API
// ════════════════════════════════════════════════════════════════

Platform::Platform() : impl_(std::make_unique<Impl>()) {}

Platform::~Platform() {
    if (impl_) impl_->shutdown();
    if (g_platform_instance == this) g_platform_instance = nullptr;
}

Result<std::unique_ptr<Platform>> Platform::create() {
    if (g_platform_instance) {
        return Result<std::unique_ptr<Platform>>::err(
            ErrorCode::PlatformError, "Platform already exists");
    }
    auto p = std::unique_ptr<Platform>(new Platform());
    p->impl_->owner = p.get();
    if (!p->impl_->init()) {
        return Result<std::unique_ptr<Platform>>::err(
            ErrorCode::PlatformError, "Failed to initialize platform");
    }
    g_platform_instance = p.get();
    return Result<std::unique_ptr<Platform>>::ok(std::move(p));
}

Platform* Platform::instance() { return g_platform_instance; }

// ── Event Loop ──────────────────────────────────────────────────
bool Platform::pollEvents() {
#if defined(__ANDROID__)
    if (impl_->android_backend) return impl_->android_backend->pollEvents();
    return false;
#elif defined(_WIN32)
    if (impl_->win32) return impl_->win32->pollEvents();
    return false;
#elif defined(__EMSCRIPTEN__)
    if (impl_->wasm_backend) return impl_->wasm_backend->pollEvents();
    return false;
#else
    if (impl_->wayland) return impl_->wayland->pollEvents();
    if (impl_->x11)     return impl_->x11->pollEvents();
#if defined(NISABA_BACKEND_OS_HAS_DRM)
    if (impl_->drm)     return impl_->drm->pollEvents();
#endif
    return false;
#endif
}

// ── Window Registration ──────────────────────────────────────────
void Platform::registerWindow(Window* w) {
    if (!w) return;
    impl_->windows.insert(w);
#if defined(__ANDROID__)
    // Android has no per-window registration in the backend
#elif defined(_WIN32)
    if (impl_->win32) impl_->win32->registerWindow(w);
#elif defined(__EMSCRIPTEN__)
    if (impl_->wasm_backend) impl_->wasm_backend->registerWindow(w);
#else
    if (impl_->x11) impl_->x11->registerWindow(w);
#if defined(NISABA_BACKEND_OS_HAS_DRM)
    if (impl_->drm) impl_->drm->registerWindow(w);
#endif
#endif
}

void Platform::unregisterWindow(Window* w) {
    if (!w) return;
    impl_->windows.erase(w);
#if defined(__ANDROID__)
    // Android has no per-window registration in the backend
#elif defined(_WIN32)
    if (impl_->win32) impl_->win32->unregisterWindow(w);
#elif defined(__EMSCRIPTEN__)
    if (impl_->wasm_backend) impl_->wasm_backend->unregisterWindow(w);
#else
    if (impl_->x11) impl_->x11->unregisterWindow(w);
#if defined(NISABA_BACKEND_OS_HAS_DRM)
    if (impl_->drm) impl_->drm->unregisterWindow(w);
#endif
#endif
}

// ── Clipboard Subsystem ──────────────────────────────────────
void Platform::setClipboardText(std::string_view text, ClipboardType type) {
    ClipboardData data;
    data.setText(text);
    setClipboardData(data, type);
}

std::string Platform::getClipboardText(ClipboardType type) const {
#if defined(__ANDROID__)
    if (impl_->android_backend) return impl_->android_backend->getClipboardText(type);
#elif defined(_WIN32)
    if (impl_->win32) return impl_->win32->getClipboardText(type);
#elif defined(__EMSCRIPTEN__)
    if (impl_->wasm_backend) return impl_->wasm_backend->getClipboardText(type);
#else
    if (impl_->wayland) return impl_->wayland->getClipboardData(type).getText();
    if (impl_->x11)     return impl_->x11->getClipboardText(type);
#endif
    return impl_->clipboard_buffer;
}

void Platform::setClipboardData(const ClipboardData& data, ClipboardType type) {
    impl_->clipboard_buffer = data.getText();
#if defined(__ANDROID__)
    if (impl_->android_backend) impl_->android_backend->setClipboardData(data, type);
#elif defined(_WIN32)
    if (impl_->win32) impl_->win32->setClipboardData(data, type);
#elif defined(__EMSCRIPTEN__)
    if (impl_->wasm_backend) impl_->wasm_backend->setClipboardData(data, type);
#else
    if (impl_->wayland) impl_->wayland->setClipboardData(data, type);
    if (impl_->x11)     impl_->x11->setClipboardData(data, type);
#endif
}

ClipboardData Platform::getClipboardData(ClipboardType type) const {
#if defined(__ANDROID__)
    if (impl_->android_backend) return impl_->android_backend->getClipboardData(type);
#elif defined(_WIN32)
    if (impl_->win32) return impl_->win32->getClipboardData(type);
#elif defined(__EMSCRIPTEN__)
    if (impl_->wasm_backend) return impl_->wasm_backend->getClipboardData(type);
#else
    if (impl_->wayland) return impl_->wayland->getClipboardData(type);
    if (impl_->x11)     return impl_->x11->getClipboardData(type);
#endif
    ClipboardData cd;
    cd.setText(impl_->clipboard_buffer);
    return cd;
}

std::vector<uint8_t> Platform::getClipboardDataForMime(std::string_view mime_type, ClipboardType type) const {
#if defined(__ANDROID__)
    if (impl_->android_backend) return impl_->android_backend->getClipboardDataForMime(mime_type, type);
#elif defined(_WIN32)
    if (impl_->win32) return impl_->win32->getClipboardDataForMime(mime_type, type);
#elif defined(__EMSCRIPTEN__)
    if (impl_->wasm_backend) return impl_->wasm_backend->getClipboardDataForMime(mime_type, type);
#else
    if (impl_->wayland) return impl_->wayland->getClipboardDataForMime(mime_type, type);
    if (impl_->x11)     return impl_->x11->getClipboardDataForMime(mime_type, type);
#endif
    return {};
}

std::vector<std::string> Platform::getClipboardFormats(ClipboardType type) const {
#if defined(__ANDROID__)
    if (impl_->android_backend) return impl_->android_backend->getClipboardFormats(type);
#elif defined(_WIN32)
    if (impl_->win32) return impl_->win32->getClipboardFormats(type);
#elif defined(__EMSCRIPTEN__)
    if (impl_->wasm_backend) return impl_->wasm_backend->getClipboardFormats(type);
#else
    if (impl_->wayland) return impl_->wayland->getClipboardFormats(type);
    if (impl_->x11)     return impl_->x11->getClipboardFormats(type);
#endif
    return { std::string(mime::TextPlainUtf8) };
}

bool Platform::hasClipboardFormat(std::string_view mime_type, ClipboardType type) const {
#if defined(__ANDROID__)
    if (impl_->android_backend) return impl_->android_backend->hasClipboardFormat(mime_type, type);
#elif defined(_WIN32)
    if (impl_->win32) return impl_->win32->hasClipboardFormat(mime_type, type);
#elif defined(__EMSCRIPTEN__)
    if (impl_->wasm_backend) return impl_->wasm_backend->hasClipboardFormat(mime_type, type);
#else
    if (impl_->wayland) return impl_->wayland->hasClipboardFormat(mime_type, type);
    if (impl_->x11)     return impl_->x11->hasClipboardFormat(mime_type, type);
#endif
    return mime_type == mime::TextPlainUtf8 || mime_type == mime::TextPlain;
}

// ── Drag & Drop Subsystem ────────────────────────────────────
bool Platform::startDrag(const DragData& data, DragAction actions) {
#if defined(__ANDROID__) || defined(__EMSCRIPTEN__)
    return false;  // DnD not supported on Android/Wasm
#elif defined(_WIN32)
    if (impl_->win32) return impl_->win32->startDrag(data, actions);
#else
    if (impl_->wayland) return impl_->wayland->startDrag(data, actions);
    if (impl_->x11)     return impl_->x11->startDrag(data, actions);
#endif
    return false;
}

// ── Foreign Toplevel Subsystem ───────────────────────────────
std::vector<std::shared_ptr<ToplevelWindow>> Platform::getToplevels() const {
#if defined(__ANDROID__) || defined(__EMSCRIPTEN__)
    return {};  // No foreign window manager on Android/Wasm
#elif defined(_WIN32)
    if (impl_->win32) return impl_->win32->getToplevels();
#else
    if (impl_->wayland) return impl_->wayland->getToplevels();
    if (impl_->x11)     return impl_->x11->getToplevels();
#endif
    return {};
}

std::shared_ptr<ToplevelWindow> Platform::getActiveToplevel() const {
#if defined(__ANDROID__) || defined(__EMSCRIPTEN__)
    return nullptr;  // No foreign window manager on Android/Wasm
#elif defined(_WIN32)
    if (impl_->win32) return impl_->win32->getActiveToplevel();
#else
    if (impl_->wayland) return impl_->wayland->getActiveToplevel();
    if (impl_->x11)     return impl_->x11->getActiveToplevel();
#endif
    return nullptr;
}

// ── Output / Monitor Subsystem ──────────────────────────────
std::vector<std::shared_ptr<Output>> Platform::getOutputs() const {
#if defined(__ANDROID__)
    if (impl_->android_backend) return impl_->android_backend->getOutputs();
#elif defined(_WIN32)
    if (impl_->win32) return impl_->win32->getOutputs();
#elif defined(__EMSCRIPTEN__)
    if (impl_->wasm_backend) return impl_->wasm_backend->getOutputs();
#else
    if (impl_->wayland) return impl_->wayland->getOutputs();
    if (impl_->x11)     return impl_->x11->getOutputs();
#if defined(NISABA_BACKEND_OS_HAS_DRM)
    if (impl_->drm)     return impl_->drm->getOutputs();
#endif
#endif
    return {};
}

std::shared_ptr<Output> Platform::getOutputByName(std::string_view name) const {
#if defined(__ANDROID__)
    if (impl_->android_backend) return impl_->android_backend->getOutputByName(name);
#elif defined(_WIN32)
    if (impl_->win32) return impl_->win32->getOutputByName(name);
#elif defined(__EMSCRIPTEN__)
    if (impl_->wasm_backend) return impl_->wasm_backend->getOutputByName(name);
#else
    if (impl_->wayland) return impl_->wayland->getOutputByName(name);
    if (impl_->x11)     return impl_->x11->getOutputByName(name);
#if defined(NISABA_BACKEND_OS_HAS_DRM)
    if (impl_->drm)     return impl_->drm->getOutputByName(name);
#endif
#endif
    return nullptr;
}

std::shared_ptr<Output> Platform::getPrimaryOutput() const {
#if defined(__ANDROID__)
    if (impl_->android_backend) return impl_->android_backend->getPrimaryOutput();
#elif defined(_WIN32)
    if (impl_->win32) return impl_->win32->getPrimaryOutput();
#elif defined(__EMSCRIPTEN__)
    if (impl_->wasm_backend) return impl_->wasm_backend->getPrimaryOutput();
#else
    if (impl_->wayland) return impl_->wayland->getPrimaryOutput();
    if (impl_->x11)     return impl_->x11->getPrimaryOutput();
#if defined(NISABA_BACKEND_OS_HAS_DRM)
    if (impl_->drm)     return impl_->drm->getPrimaryOutput();
#endif
#endif
    return nullptr;
}

// ── Cursor ───────────────────────────────────────────────────────
void Platform::setCursor(SystemCursor cursor) {
#if defined(__ANDROID__)
    // No cursor on Android — silently ignore
#elif defined(_WIN32)
    if (impl_->win32) impl_->win32->setCursor(cursor);
#elif defined(__EMSCRIPTEN__)
    if (impl_->wasm_backend) impl_->wasm_backend->setCursor(cursor);
#else
    if (impl_->wayland) impl_->wayland->setCursor(cursor);
    if (impl_->x11)     impl_->x11->setCursor(cursor);
#if defined(NISABA_BACKEND_OS_HAS_DRM)
    if (impl_->drm)     impl_->drm->setCursor(cursor);
#endif
#endif
}

// ── Timing ───────────────────────────────────────────────────────
double Platform::getTime() const {
    return std::chrono::duration<double>(
        std::chrono::steady_clock::now() - impl_->start_time).count();
}

// ── Backend accessors ─────────────────────────────────────────────
void* Platform::getNativeDisplay() const {
#if defined(__ANDROID__) || defined(_WIN32) || defined(__EMSCRIPTEN__)
    return nullptr;
#else
    if (impl_->wayland) return (void*)impl_->wayland->getDisplay();
    if (impl_->x11)     return (void*)impl_->x11->getDisplay();
    return nullptr;
#endif
}
void* Platform::getEGLDisplay() const {
#if defined(__ANDROID__)
    if (impl_->android_backend) return (void*)impl_->android_backend->getEGLDisplay();
    return nullptr;
#elif defined(_WIN32) || defined(__EMSCRIPTEN__)
    return nullptr;
#else
    if (impl_->wayland) return (void*)impl_->wayland->getEGLDisplay();
    if (impl_->x11)     return (void*)impl_->x11->getEGLDisplay();
#if defined(NISABA_BACKEND_OS_HAS_DRM)
    if (impl_->drm)     return (void*)impl_->drm->getEGLDisplay();
#endif
    return nullptr;
#endif
}
void* Platform::getEGLConfig() const {
#if defined(__ANDROID__)
    if (impl_->android_backend) return (void*)impl_->android_backend->getEGLConfig();
    return nullptr;
#elif defined(_WIN32) || defined(__EMSCRIPTEN__)
    return nullptr;
#else
    if (impl_->wayland) return (void*)impl_->wayland->getEGLConfig();
    if (impl_->x11)     return (void*)impl_->x11->getEGLConfig();
#if defined(NISABA_BACKEND_OS_HAS_DRM)
    if (impl_->drm)     return (void*)impl_->drm->getEGLConfig();
#endif
    return nullptr;
#endif
}
void* Platform::getEGLContext() const {
#if defined(__ANDROID__)
    if (impl_->android_backend) return (void*)impl_->android_backend->getEGLContext();
    return nullptr;
#elif defined(_WIN32) || defined(__EMSCRIPTEN__)
    return nullptr;
#else
    if (impl_->wayland) return (void*)impl_->wayland->getEGLContext();
    if (impl_->x11)     return (void*)impl_->x11->getEGLContext();
#if defined(NISABA_BACKEND_OS_HAS_DRM)
    if (impl_->drm)     return (void*)impl_->drm->getEGLContext();
#endif
    return nullptr;
#endif
}

EdgeInsets Platform::getSafeAreaInsets() const {
#if defined(__ANDROID__)
    if (impl_->android_backend) return impl_->android_backend->getSafeAreaInsets();
#endif
    return {};  // Desktop: no safe area insets
}

bool  Platform::isWayland()        const { return impl_->isWayland(); }
bool  Platform::isAndroid()        const { return impl_->isAndroid(); }
bool  Platform::isDRM()            const { return impl_->isDRM(); }
bool  Platform::isWasm()           const { return impl_->isWasm(); }

void* Platform::getDRMBackend() const {
#if defined(NISABA_BACKEND_OS_HAS_DRM)
    return (void*)impl_->drm.get();
#else
    return nullptr;
#endif
}

void* Platform::getAndroidBackend() const {
#if defined(__ANDROID__)
    return (void*)impl_->android_backend.get();
#else
    return nullptr;
#endif
}

void* Platform::getWasmBackend() const {
#if defined(__EMSCRIPTEN__)
    return (void*)impl_->wasm_backend.get();
#else
    return nullptr;
#endif
}
void* Platform::getWaylandBackend() const {
#if defined(__ANDROID__) || defined(_WIN32) || defined(__EMSCRIPTEN__)
    return nullptr;
#else
    return (void*)impl_->wayland.get();
#endif
}
void* Platform::getX11Backend()    const {
#if defined(__ANDROID__) || defined(_WIN32) || defined(__EMSCRIPTEN__)
    return nullptr;
#else
    return (void*)impl_->x11.get();
#endif
}

void* Platform::getWin32Backend() const {
#if defined(_WIN32)
    return (void*)impl_->win32.get();
#else
    return nullptr;
#endif
}

}  // namespace nisaba::backend_os
