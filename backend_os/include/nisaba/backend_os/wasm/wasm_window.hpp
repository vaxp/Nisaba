#pragma once
/// @file wasm_window.hpp
/// @brief WebAssembly HTML5 Canvas + WebGL 2.0 window backend for Nisaba.

#include "nisaba/backend_os/window.hpp"
#include "nisaba/backend_os/wasm/wasm_platform.hpp"
#include <string>
#include <string_view>

#if defined(__EMSCRIPTEN__)
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

namespace nisaba::backend_os::wasm {

class WasmWindow {
public:
    explicit WasmWindow(WasmPlatformBackend& backend);
    ~WasmWindow();

    bool init(const WindowConfig& config);
    void destroy();

    void setTitle(std::string_view title);
    void setSize(int width, int height);
    void setPosition(int x, int y);
    void setBorderless(bool borderless);
    void setAlwaysOnTop(bool on_top);
    void setBlurBehind(bool enable);

    [[nodiscard]] Size  getSize() const;
    [[nodiscard]] Size  getDrawableSize() const;
    [[nodiscard]] float getDpiScale() const;

    void makeCurrent();
    void swapBuffers();

    // ── Client-Side Decoration / Window Mode Operations ─────────
    void beginMove(float local_x = 0.0f, float local_y = 0.0f, int button = 1);
    void beginResize(WindowEdge edge, float local_x = 0.0f, float local_y = 0.0f, int button = 1);
    void setMaximized(bool max);
    void setMinimized(bool min);
    void setFullscreen(bool full);
    void toggleMaximize();
    void showWindowMenu(float local_x = 0.0f, float local_y = 0.0f, int button = 3);
    void setDecorated(bool decorated);
    void setWindowGeometry(int x, int y, int width, int height);

    [[nodiscard]] bool isMaximized() const { return hasWindowState(state_, WindowState::Maximized); }
    [[nodiscard]] bool isMinimized() const { return hasWindowState(state_, WindowState::Minimized); }
    [[nodiscard]] bool isFullscreen() const { return hasWindowState(state_, WindowState::Fullscreen); }
    [[nodiscard]] bool isActivated() const { return hasWindowState(state_, WindowState::Activated); }
    [[nodiscard]] WindowState getWindowState() const { return state_; }

    [[nodiscard]] void* getNativeHandle() const;
    [[nodiscard]] void* getEGLSurface()   const;
    [[nodiscard]] void* getEGLContext()   const;

    [[nodiscard]] const std::string& getCanvasTarget() const { return canvas_target_; }

    Signal<WindowState>& onStateChanged() { return on_state_changed_; }
    Signal<bool>&        onMaximized()    { return on_maximized_; }
    Signal<bool>&        onFocus()        { return on_focus_; }
    Signal<int, int>&    onResize()       { return on_resize_; }
    Signal<>&            onClose()        { return on_close_; }

    // Internal hook for Canvas / Window DOM events
    void handleCanvasResize(double css_width, double css_height, double pixel_ratio);
    void pollCanvasSize();

private:
    WasmPlatformBackend& backend_;
    WindowConfig config_;
    std::string canvas_target_ = "#canvas";

#if defined(__EMSCRIPTEN__)
    EMSCRIPTEN_WEBGL_CONTEXT_HANDLE gl_context_ = 0;
#else
    void* gl_context_ = nullptr;
#endif

    int current_width_  = 1280;
    int current_height_ = 800;
    float dpi_scale_    = 1.0f;
    WindowState state_  = WindowState::Normal | WindowState::Activated;

    Signal<WindowState> on_state_changed_;
    Signal<bool>        on_maximized_;
    Signal<bool>        on_focus_;
    Signal<int, int>    on_resize_;
    Signal<>            on_close_;
};

} // namespace nisaba::backend_os::wasm
