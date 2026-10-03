/// @file android_surface.cpp
/// @brief AndroidSurface — EGL window surface backed by ANativeWindow*.
/// The EGL context lives in AndroidPlatformBackend and is shared across
/// surface recreations (e.g. screen rotation). Only the EGL surface
/// is destroyed/recreated when the ANativeWindow changes.

#ifdef __ANDROID__

#include "nisaba/backend_os/android/android_surface.hpp"
#include "nisaba/backend_os/types.hpp"

#include <android/native_window.h>
#include <cmath>

namespace nisaba::backend_os::android {

// ════════════════════════════════════════════════════════════════
// Constructor / Destructor
// ════════════════════════════════════════════════════════════════

AndroidSurface::AndroidSurface(AndroidPlatformBackend& backend)
    : backend_(backend)
{
    // Cache EGL objects from backend (they are stable across surface changes)
    egl_display_ = backend_.getEGLDisplay();
    egl_context_ = backend_.getEGLContext();
    dpi_scale_   = backend_.getDpiScale();
}

AndroidSurface::~AndroidSurface() {
    destroy();
    backend_.unregisterSurface(this);
}

// ════════════════════════════════════════════════════════════════
// init / destroy
// ════════════════════════════════════════════════════════════════

bool AndroidSurface::init() {
    // Register with backend so it can notify us of ANativeWindow events
    backend_.registerSurface(this);

    // The ANativeWindow may not be available yet (surface created later
    // via onNativeWindowCreated). That is expected behaviour.
    Nisaba_ALOG("AndroidSurface::init — waiting for native window");
    return true;
}

void AndroidSurface::destroy() {
    destroyEGLSurface();
}

// ════════════════════════════════════════════════════════════════
// EGL Surface lifecycle
// ════════════════════════════════════════════════════════════════

bool AndroidSurface::createEGLSurface() {
    if (!native_window_) {
        Nisaba_ALOGE("createEGLSurface: no native window");
        return false;
    }
    if (egl_surface_ != EGL_NO_SURFACE) {
        // Destroy existing surface before binding to the new native window
        destroyEGLSurface();
    }

    // Set the native window buffer format to match the EGL config
    EGLint format = 0;
    eglGetConfigAttrib(egl_display_, backend_.getEGLConfig(), EGL_NATIVE_VISUAL_ID, &format);
    ANativeWindow_setBuffersGeometry(native_window_, 0, 0, format);

    egl_surface_ = eglCreateWindowSurface(
        egl_display_, backend_.getEGLConfig(), native_window_, nullptr);

    if (egl_surface_ == EGL_NO_SURFACE) {
        Nisaba_ALOGE("eglCreateWindowSurface failed (error 0x%x)", eglGetError());
        return false;
    }

    Nisaba_ALOG("EGL surface created");
    // Immediately make current so that OpenGL commands and  context are bound
    makeCurrent();
    querySize();
    return true;
}

void AndroidSurface::destroyEGLSurface() {
    if (egl_surface_ == EGL_NO_SURFACE) return;

    // Make sure nothing is current before destroying
    eglMakeCurrent(egl_display_, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroySurface(egl_display_, egl_surface_);
    egl_surface_ = EGL_NO_SURFACE;
    width_  = 0;
    height_ = 0;
    Nisaba_ALOG("EGL surface destroyed");
}

void AndroidSurface::querySize() {
    if (egl_surface_ == EGL_NO_SURFACE) return;

    EGLint w = 0, h = 0;
    eglQuerySurface(egl_display_, egl_surface_, EGL_WIDTH,  &w);
    eglQuerySurface(egl_display_, egl_surface_, EGL_HEIGHT, &h);

    bool changed = (w != width_ || h != height_);
    width_  = w;
    height_ = h;

    if (changed && w > 0 && h > 0) {
        Nisaba_ALOG("Surface size: %dx%d", w, h);
        on_resize_.emit(w, h);
    }
}

// ════════════════════════════════════════════════════════════════
// Android lifecycle hooks (called by AndroidPlatformBackend)
// ════════════════════════════════════════════════════════════════

void AndroidSurface::onNativeWindowCreated(ANativeWindow* window) {
    Nisaba_ALOG("AndroidSurface::onNativeWindowCreated: %p", window);
    native_window_ = window;

    if (!createEGLSurface()) {
        Nisaba_ALOGE("Failed to create EGL surface for new native window");
        return;
    }

    // Update DPI from backend (may have changed after rotation)
    dpi_scale_ = backend_.getDpiScale();

    // Notify state change and surface recreation
    on_state_changed_.emit(getWindowState());
    on_surface_recreated_.emit();
}

void AndroidSurface::onNativeWindowDestroyed() {
    Nisaba_ALOG("AndroidSurface::onNativeWindowDestroyed");
    // Destroy the EGL surface — the EGL context in the backend is preserved
    destroyEGLSurface();
    native_window_ = nullptr;
    on_surface_destroyed_.emit();
}

void AndroidSurface::onWindowFocusChanged(bool focused) {
    activated_ = focused;
    on_focus_.emit(focused);

    WindowState state = getWindowState();
    on_state_changed_.emit(state);
}

// ════════════════════════════════════════════════════════════════
// Rendering API
// ════════════════════════════════════════════════════════════════

void AndroidSurface::makeCurrent() {
    if (egl_surface_ == EGL_NO_SURFACE || egl_context_ == EGL_NO_CONTEXT) return;

    if (!eglMakeCurrent(egl_display_, egl_surface_, egl_surface_, egl_context_)) {
        Nisaba_ALOGE("eglMakeCurrent failed (error 0x%x)", eglGetError());
    }
}

void AndroidSurface::swapBuffers() {
    if (egl_surface_ == EGL_NO_SURFACE) return;

    if (!eglSwapBuffers(egl_display_, egl_surface_)) {
        EGLint err = eglGetError();
        if (err == EGL_BAD_SURFACE || err == EGL_BAD_NATIVE_WINDOW) {
            // Surface was lost (e.g. rotation in progress) — not fatal
            Nisaba_ALOGW("eglSwapBuffers: surface lost (0x%x), will recreate", err);
            destroyEGLSurface();
            on_surface_destroyed_.emit();
        } else {
            Nisaba_ALOGE("eglSwapBuffers failed (error 0x%x)", err);
        }
    }
}

// ════════════════════════════════════════════════════════════════
// Accessors
// ════════════════════════════════════════════════════════════════

Size AndroidSurface::getSize() const {
    float dpi = (dpi_scale_ > 0.0f) ? dpi_scale_ : 1.0f;
    return { static_cast<float>(width_) / dpi, static_cast<float>(height_) / dpi };
}

Size AndroidSurface::getDrawableSize() const {
    // Physical pixel size of the EGL surface buffer
    return { static_cast<float>(width_), static_cast<float>(height_) };
}

float AndroidSurface::getDpiScale() const {
    return dpi_scale_;
}

WindowState AndroidSurface::getWindowState() const {
    // Android is always fullscreen; add Activated flag when focused
    WindowState state = WindowState::Fullscreen;
    if (activated_) {
        state = state | WindowState::Activated;
    }
    return state;
}

} // namespace nisaba::backend_os::android

#endif // __ANDROID__
