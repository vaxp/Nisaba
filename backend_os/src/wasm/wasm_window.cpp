/// @file wasm_window.cpp
/// @brief WebAssembly HTML5 Canvas + WebGL 2.0 window backend implementation.

#include "nisaba/backend_os/wasm/wasm_window.hpp"
#include <iostream>
#include <cmath>

namespace nisaba::backend_os::wasm {

WasmWindow::WasmWindow(WasmPlatformBackend& backend)
    : backend_(backend) {}

WasmWindow::~WasmWindow() {
    destroy();
}

bool WasmWindow::init(const WindowConfig& config) {
    config_ = config;
    current_width_ = config.width > 0 ? config.width : 1280;
    current_height_ = config.height > 0 ? config.height : 800;

#if defined(__EMSCRIPTEN__)
    canvas_target_ = "#canvas";

    // Detect browser canvas CSS dimensions and High-DPI ratio
    double css_w = 0.0, css_h = 0.0;
    emscripten_get_element_css_size(canvas_target_.c_str(), &css_w, &css_h);
    if (css_w <= 10.0 || css_h <= 10.0) {
        css_w = EM_ASM_DOUBLE({ return window.innerWidth; });
        css_h = EM_ASM_DOUBLE({ return window.innerHeight; });
    }
    if (css_w > 10.0 && css_h > 10.0) {
        current_width_ = static_cast<int>(css_w);
        current_height_ = static_cast<int>(css_h);
    }

    double dpr = emscripten_get_device_pixel_ratio();
    dpi_scale_ = (dpr > 0.1) ? static_cast<float>(dpr) : 1.0f;

    // Initialize WebGL 2.0 context
    EmscriptenWebGLContextAttributes attrs;
    emscripten_webgl_init_context_attributes(&attrs);
    attrs.majorVersion = 2; // WebGL 2.0
    attrs.minorVersion = 0;
    attrs.alpha = config.transparent;
    attrs.depth = true;
    attrs.stencil = true;
    attrs.antialias = true;
    attrs.premultipliedAlpha = false;
    attrs.preserveDrawingBuffer = false;
    attrs.powerPreference = EM_WEBGL_POWER_PREFERENCE_HIGH_PERFORMANCE;

    gl_context_ = emscripten_webgl_create_context(canvas_target_.c_str(), &attrs);
    if (gl_context_ <= 0) {
        std::cerr << "[Nisaba Wasm] WebGL 2.0 context creation failed on '" << canvas_target_
                  << "', falling back to default WebGL context\n";
        attrs.majorVersion = 1;
        gl_context_ = emscripten_webgl_create_context(canvas_target_.c_str(), &attrs);
    }

    if (gl_context_ <= 0) {
        std::cerr << "[Nisaba Wasm] Fatal: Could not create any WebGL context!\n";
        return false;
    }

    emscripten_webgl_make_context_current(gl_context_);

    // Set canvas internal render buffer size matching physical device pixels
    int phys_w = static_cast<int>(std::round(current_width_ * dpi_scale_));
    int phys_h = static_cast<int>(std::round(current_height_ * dpi_scale_));
    emscripten_set_canvas_element_size(canvas_target_.c_str(), phys_w, phys_h);
#endif

    return true;
}

void WasmWindow::destroy() {
#if defined(__EMSCRIPTEN__)
    if (gl_context_ > 0) {
        emscripten_webgl_destroy_context(gl_context_);
        gl_context_ = 0;
    }
#endif
    on_close_.emit();
}

void WasmWindow::setTitle(std::string_view title) {
    config_.title = std::string(title);
#if defined(__EMSCRIPTEN__)
    std::string t(title);
    EM_ASM({
        document.title = UTF8ToString($0);
    }, t.c_str());
#endif
}

void WasmWindow::setSize(int width, int height) {
    if (width <= 0 || height <= 0) return;
    current_width_ = width;
    current_height_ = height;
#if defined(__EMSCRIPTEN__)
    double dpr = emscripten_get_device_pixel_ratio();
    if (dpr > 0.1) dpi_scale_ = static_cast<float>(dpr);
    int phys_w = static_cast<int>(std::round(current_width_ * dpi_scale_));
    int phys_h = static_cast<int>(std::round(current_height_ * dpi_scale_));
    emscripten_set_canvas_element_size(canvas_target_.c_str(), phys_w, phys_h);
#endif
    on_resize_.emit(current_width_, current_height_);
}

void WasmWindow::setPosition(int, int) {
    // In web browser, canvas positioning is managed by CSS / flex layout
}

void WasmWindow::setBorderless(bool) {}
void WasmWindow::setAlwaysOnTop(bool) {}
void WasmWindow::setBlurBehind(bool) {}

Size WasmWindow::getSize() const {
    const_cast<WasmWindow*>(this)->pollCanvasSize();
    return Size{static_cast<float>(current_width_), static_cast<float>(current_height_)};
}

Size WasmWindow::getDrawableSize() const {
    const_cast<WasmWindow*>(this)->pollCanvasSize();
    return Size{
        static_cast<float>(std::round(current_width_ * dpi_scale_)),
        static_cast<float>(std::round(current_height_ * dpi_scale_))
    };
}

float WasmWindow::getDpiScale() const {
    const_cast<WasmWindow*>(this)->pollCanvasSize();
    return dpi_scale_;
}

void WasmWindow::makeCurrent() {
#if defined(__EMSCRIPTEN__)
    if (gl_context_ > 0) {
        emscripten_webgl_make_context_current(gl_context_);
    }
#endif
}

void WasmWindow::swapBuffers() {
    // In WebGL, presentation is automatically synchronized with requestAnimationFrame
#if defined(__EMSCRIPTEN__)
    emscripten_webgl_commit_frame();
#endif
}

void WasmWindow::beginMove(float, float, int) {}
void WasmWindow::beginResize(WindowEdge, float, float, int) {}

void WasmWindow::setMaximized(bool max) {
    if (max) state_ = state_ | WindowState::Maximized;
    else     state_ = state_ & ~WindowState::Maximized;
    on_maximized_.emit(max);
    on_state_changed_.emit(state_);
}

void WasmWindow::setMinimized(bool min) {
    if (min) state_ = state_ | WindowState::Minimized;
    else     state_ = state_ & ~WindowState::Minimized;
    on_state_changed_.emit(state_);
}

void WasmWindow::setFullscreen(bool full) {
#if defined(__EMSCRIPTEN__)
    if (full) {
        EmscriptenFullscreenStrategy strategy;
        strategy.scaleMode = EMSCRIPTEN_FULLSCREEN_SCALE_DEFAULT;
        strategy.canvasResolutionScaleMode = EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_STDDEF;
        strategy.filteringMode = EMSCRIPTEN_FULLSCREEN_FILTERING_DEFAULT;
        strategy.canvasResizedCallback = nullptr;
        emscripten_request_fullscreen_strategy(canvas_target_.c_str(), EM_TRUE, &strategy);
        state_ = state_ | WindowState::Fullscreen;
    } else {
        emscripten_exit_fullscreen();
        state_ = state_ & ~WindowState::Fullscreen;
    }
    on_state_changed_.emit(state_);
#endif
}

void WasmWindow::toggleMaximize() {
    setMaximized(!isMaximized());
}

void WasmWindow::showWindowMenu(float, float, int) {}
void WasmWindow::setDecorated(bool) {}
void WasmWindow::setWindowGeometry(int, int, int width, int height) {
    setSize(width, height);
}

void* WasmWindow::getNativeHandle() const {
    return (void*)canvas_target_.c_str();
}

void* WasmWindow::getEGLSurface() const {
    return nullptr;
}

void* WasmWindow::getEGLContext() const {
#if defined(__EMSCRIPTEN__)
    return reinterpret_cast<void*>(static_cast<uintptr_t>(gl_context_));
#else
    return gl_context_;
#endif
}

void WasmWindow::handleCanvasResize(double css_width, double css_height, double pixel_ratio) {
    if (css_width <= 0.0 || css_height <= 0.0) return;
    int new_w = static_cast<int>(css_width);
    int new_h = static_cast<int>(css_height);
    float new_dpr = (pixel_ratio > 0.1) ? static_cast<float>(pixel_ratio) : 1.0f;

    if (new_w == current_width_ && new_h == current_height_ && std::abs(new_dpr - dpi_scale_) < 0.001f) {
        return;
    }

    current_width_  = new_w;
    current_height_ = new_h;
    dpi_scale_      = new_dpr;

#if defined(__EMSCRIPTEN__)
    int phys_w = static_cast<int>(std::round(current_width_ * dpi_scale_));
    int phys_h = static_cast<int>(std::round(current_height_ * dpi_scale_));
    emscripten_set_canvas_element_size(canvas_target_.c_str(), phys_w, phys_h);
#endif
    on_resize_.emit(current_width_, current_height_);
}

void WasmWindow::pollCanvasSize() {
#if defined(__EMSCRIPTEN__)
    double css_w = 0.0, css_h = 0.0;
    emscripten_get_element_css_size(canvas_target_.c_str(), &css_w, &css_h);
    if (css_w <= 10.0 || css_h <= 10.0) {
        css_w = EM_ASM_DOUBLE({ return window.innerWidth; });
        css_h = EM_ASM_DOUBLE({ return window.innerHeight; });
    }
    double dpr = emscripten_get_device_pixel_ratio();
    if (dpr <= 0.0) dpr = 1.0;

    int new_w = static_cast<int>(css_w);
    int new_h = static_cast<int>(css_h);
    float new_dpr = static_cast<float>(dpr);

    if (new_w > 0 && new_h > 0 &&
        (new_w != current_width_ || new_h != current_height_ || std::abs(new_dpr - dpi_scale_) > 0.001f)) {
        handleCanvasResize(css_w, css_h, dpr);
    }
#endif
}

} // namespace nisaba::backend_os::wasm
