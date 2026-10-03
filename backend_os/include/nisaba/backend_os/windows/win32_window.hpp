#pragma once
/// @file win32_window.hpp
/// @brief Win32 + OpenGL WGL native window backend.

#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include "nisaba/backend_os/window.hpp"
#include "nisaba/backend_os/windows/win32_platform.hpp"

namespace nisaba::backend_os::win32 {

// Helper: Convert UTF-8 to UTF-16
inline std::wstring utf8ToWide(std::string_view utf8) {
    if (utf8.empty()) return L"";
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, utf8.data(), static_cast<int>(utf8.size()), nullptr, 0);
    std::wstring result(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, utf8.data(), static_cast<int>(utf8.size()), &result[0], size_needed);
    return result;
}

// Helper: Convert UTF-16 to UTF-8
inline std::string wideToUtf8(std::wstring_view wide) {
    if (wide.empty()) return "";
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, wide.data(), static_cast<int>(wide.size()), nullptr, 0, nullptr, nullptr);
    std::string result(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, wide.data(), static_cast<int>(wide.size()), &result[0], size_needed, nullptr, nullptr);
    return result;
}

class Win32Window {
public:
    explicit Win32Window(Win32PlatformBackend& backend);
    ~Win32Window();

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

    // ── Client-Side Decoration (CSD) Operations ─────────────────
    void beginMove(float local_x = 0.0f, float local_y = 0.0f, int button = 1);
    void beginResize(WindowEdge edge, float local_x = 0.0f, float local_y = 0.0f, int button = 1);
    void setMaximized(bool max);
    void setMinimized(bool min);
    void setFullscreen(bool full);
    void toggleMaximize();
    void showWindowMenu(float local_x = 0.0f, float local_y = 0.0f, int button = 3);
    void setDecorated(bool decorated);
    void setWindowGeometry(int x, int y, int width, int height);

    [[nodiscard]] bool isMaximized() const { return (hwnd_ && IsZoomed(hwnd_)) || hasWindowState(state_, WindowState::Maximized); }
    [[nodiscard]] bool isMinimized() const { return (hwnd_ && IsIconic(hwnd_)) || hasWindowState(state_, WindowState::Minimized); }
    [[nodiscard]] bool isFullscreen() const { return hasWindowState(state_, WindowState::Fullscreen); }
    [[nodiscard]] bool isActivated() const { return hasWindowState(state_, WindowState::Activated); }
    [[nodiscard]] WindowState getWindowState() const { return state_; }

    [[nodiscard]] void* getNativeHandle() const { return (void*)hwnd_; }
    [[nodiscard]] void* getEGLSurface()   const { return nullptr; }
    [[nodiscard]] void* getEGLContext()   const { return (void*)hglrc_; }
    [[nodiscard]] HWND  getHWND()         const { return hwnd_; }
    [[nodiscard]] HDC   getHDC()          const { return hdc_; }

    Signal<WindowState>& onStateChanged() { return on_state_changed_; }
    Signal<bool>&        onMaximized()    { return on_maximized_; }
    Signal<bool>&        onFocus()        { return on_focus_; }
    Signal<int, int>&    onResize()       { return on_resize_; }
    Signal<>&            onClose()        { return on_close_; }

    // Message handler
    LRESULT handleMessage(UINT msg, WPARAM wParam, LPARAM lParam);

private:
    Win32PlatformBackend& backend_;
    WindowConfig config_;

    HWND  hwnd_  = nullptr;
    HDC   hdc_   = nullptr;
    HGLRC hglrc_ = nullptr;

    int current_width_  = 0;
    int current_height_ = 0;
    float dpi_scale_    = 1.0f;
    WindowState state_  = WindowState::Normal;

    WINDOWPLACEMENT prev_placement_{ sizeof(WINDOWPLACEMENT) };
    bool is_fullscreen_ = false;
    bool blur_enabled_  = false;
    bool in_size_move_  = false;
    bool is_destroying_ = false;

    Signal<WindowState> on_state_changed_;
    Signal<bool>        on_maximized_;
    Signal<bool>        on_focus_;
    Signal<int, int>    on_resize_;
    Signal<>            on_close_;

    void setupDpi();
    void applyWindowStyle();
    void applyBlurBehind(bool enable);
    static LRESULT CALLBACK staticWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
};

} // namespace nisaba::backend_os::win32
