/// @file win32_window.cpp
/// @brief Win32 + OpenGL WGL native window backend implementation.

#include "nisaba/backend_os/windows/win32_window.hpp"
#include "nisaba/backend_os/windows/win32_platform.hpp"
#include "nisaba/backend_os/layer_surface.hpp"
#include <windowsx.h>
#include <dwmapi.h>
#include <GL/gl.h>
#include <iostream>
#include <string>

#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif

namespace nisaba::backend_os::win32 {

static const wchar_t* kWindowClass      = L"NisabaWin32WindowClass";
static const wchar_t* kPopupWindowClass = L"NisabaWin32PopupWindowClass";
static bool g_class_registered       = false;
static bool g_popup_class_registered = false;
static HGLRC g_shared_hglrc = nullptr;
static int   g_hglrc_ref_count = 0;

Win32Window::Win32Window(Win32PlatformBackend& backend)
    : backend_(backend) {}

Win32Window::~Win32Window() {
    destroy();
}

LRESULT CALLBACK Win32Window::staticWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    Win32Window* self = nullptr;
    if (msg == WM_NCCREATE) {
        auto* cs = reinterpret_cast<CREATESTRUCTW*>(lParam);
        self = reinterpret_cast<Win32Window*>(cs->lpCreateParams);
        if (self) {
            self->hwnd_ = hwnd;
            SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
        }
        return DefWindowProcW(hwnd, msg, wParam, lParam);
    } else {
        self = reinterpret_cast<Win32Window*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }

    if (self) {
        return self->handleMessage(msg, wParam, lParam);
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

bool Win32Window::init(const WindowConfig& config) {
    config_ = config;
    HINSTANCE hInst = GetModuleHandleW(nullptr);

    // Register standard window class without CS_DROPSHADOW (DWM handles shadows natively)
    if (!g_class_registered) {
        WNDCLASSEXW wc{};
        wc.cbSize        = sizeof(WNDCLASSEXW);
        wc.style         = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc   = staticWndProc;
        wc.hInstance     = hInst;
        wc.hCursor       = LoadCursorA(nullptr, IDC_ARROW);
        wc.lpszClassName = kWindowClass;

        if (!RegisterClassExW(&wc)) {
            std::cerr << "[Nisaba Win32Window] Failed to register window class\n";
            return false;
        }
        g_class_registered = true;
    }

    // Register dedicated popup class for borderless transient menus/popups
    if (config.mode == WindowMode::Popup && !g_popup_class_registered) {
        WNDCLASSEXW wc{};
        wc.cbSize        = sizeof(WNDCLASSEXW);
        wc.style         = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc   = staticWndProc;
        wc.hInstance     = hInst;
        wc.hCursor       = LoadCursorA(nullptr, IDC_ARROW);
        wc.lpszClassName = kPopupWindowClass;

        if (!RegisterClassExW(&wc)) {
            std::cerr << "[Nisaba Win32Window] Failed to register popup window class\n";
            return false;
        }
        g_popup_class_registered = true;
    }

    current_width_  = config.width > 0 ? config.width : 1280;
    current_height_ = config.height > 0 ? config.height : 800;

    DWORD style = WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
    DWORD exStyle = WS_EX_APPWINDOW;

    if (config.mode == WindowMode::Popup) {
        // Native frameless popup window: tool window (no taskbar item), topmost, owned by parent
        style = WS_POPUP | WS_VISIBLE | WS_CLIPSIBLINGS;
        exStyle = WS_EX_TOOLWINDOW | WS_EX_TOPMOST;
    } else if (config.borderless || config.csd) {
        // Modern frameless window with resize border, minimize/maximize and clipping
        style = WS_POPUP | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
    } else {
        // Standard native decorated window: respect resizable flag
        if (!config.resizable) {
            style &= ~(WS_THICKFRAME | WS_MAXIMIZEBOX);
        }
    }
    if (config.always_on_top && config.mode != WindowMode::Popup) {
        exStyle |= WS_EX_TOPMOST;
    }

    RECT wr = { 0, 0, current_width_, current_height_ };
    if (!(config.borderless || config.csd || config.mode == WindowMode::Popup)) {
        HMODULE user32 = GetModuleHandleW(L"user32.dll");
        typedef BOOL (WINAPI *AdjustWindowRectExForDpiProc)(LPRECT, DWORD, BOOL, DWORD, UINT);
        auto adjustDpi = (AdjustWindowRectExForDpiProc)GetProcAddress(user32, "AdjustWindowRectExForDpi");
        if (adjustDpi) {
            typedef UINT (WINAPI *GetDpiForSystemProc)();
            auto getDpiSys = (GetDpiForSystemProc)GetProcAddress(user32, "GetDpiForSystem");
            UINT dpiSys = getDpiSys ? getDpiSys() : 96;
            adjustDpi(&wr, style, FALSE, exStyle, dpiSys);
        } else {
            AdjustWindowRectEx(&wr, style, FALSE, exStyle);
        }
    }

    int win_w = wr.right - wr.left;
    int win_h = wr.bottom - wr.top;

    int pos_x = (config.x >= 0) ? config.x : CW_USEDEFAULT;
    int pos_y = (config.y >= 0) ? config.y : CW_USEDEFAULT;

    HWND parent_hwnd = nullptr;
    if (config.parent_window) {
        parent_hwnd = static_cast<HWND>(config.parent_window->getNativeHandle());
    }
    if (!parent_hwnd && config.mode == WindowMode::Popup) {
        parent_hwnd = GetActiveWindow();
    }

    if (config.mode == WindowMode::Popup) {
        // Convert client/window coordinates from parent to absolute desktop screen coordinates
        if (parent_hwnd) {
            POINT pt = { pos_x, pos_y };
            ClientToScreen(parent_hwnd, &pt);
            pos_x = pt.x;
            pos_y = pt.y;
        }
    } else if (config.x < 0) {
        int screen_w = GetSystemMetrics(SM_CXSCREEN);
        int screen_h = GetSystemMetrics(SM_CYSCREEN);
        pos_x = (screen_w - win_w) / 2;
        pos_y = (screen_h - win_h) / 2;
    }

    std::wstring wideTitle = utf8ToWide(config.title);
    const wchar_t* targetClass = (config.mode == WindowMode::Popup) ? kPopupWindowClass : kWindowClass;

    hwnd_ = CreateWindowExW(
        exStyle,
        targetClass,
        wideTitle.c_str(),
        style,
        pos_x, pos_y,
        win_w, win_h,
        parent_hwnd, nullptr,
        hInst,
        this
    );

    if (!hwnd_) {
        std::cerr << "[Nisaba Win32Window] Failed to create window, error: " << GetLastError() << "\n";
        return false;
    }

    setupDpi();

    // ── Setup OpenGL / WGL ──────────────────────────────────────────
    hdc_ = GetDC(hwnd_);
    if (!hdc_) {
        std::cerr << "[Nisaba Win32Window] Failed to get device context (DC)\n";
        return false;
    }

    PIXELFORMATDESCRIPTOR pfd = {
        sizeof(PIXELFORMATDESCRIPTOR),
        1,
        PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
        PFD_TYPE_RGBA,
        32,
        0, 0, 0, 0, 0, 0,
        8, 0,
        0,
        0, 0, 0, 0,
        24, // depth
        8,  // stencil
        0,
        PFD_MAIN_PLANE,
        0,
        0, 0, 0
    };

    int format = ChoosePixelFormat(hdc_, &pfd);
    if (!format || !SetPixelFormat(hdc_, format, &pfd)) {
        std::cerr << "[Nisaba Win32Window] Failed to set pixel format\n";
        return false;
    }

    // Share a single WGL context across all windows so 's GrDirectContext remains valid
    if (!g_shared_hglrc) {
        g_shared_hglrc = wglCreateContext(hdc_);
        if (!g_shared_hglrc) {
            std::cerr << "[Nisaba Win32Window] Failed to create OpenGL WGL context\n";
            return false;
        }
    }

    hglrc_ = g_shared_hglrc;
    g_hglrc_ref_count++;

    wglMakeCurrent(hdc_, hglrc_);

    // VSync configuration
    typedef BOOL (WINAPI *wglSwapIntervalEXTProc)(int);
    auto wglSwapIntervalEXT = (wglSwapIntervalEXTProc)wglGetProcAddress("wglSwapIntervalEXT");
    if (wglSwapIntervalEXT) {
        wglSwapIntervalEXT(config.vsync ? 1 : 0);
    }

    // Windows Dark Mode & DWM styling
    BOOL darkMode = TRUE;
    DwmSetWindowAttribute(hwnd_, DWMWA_USE_IMMERSIVE_DARK_MODE, &darkMode, sizeof(darkMode));
    DwmSetWindowAttribute(hwnd_, 19 /* DWMWA_USE_IMMERSIVE_DARK_MODE_BEFORE_20H1 */, &darkMode, sizeof(darkMode));

    // Windows 11 rounded window corners (Build 22000+) - for top-level windows only
    #ifndef DWMWA_WINDOW_CORNER_PREFERENCE
    #define DWMWA_WINDOW_CORNER_PREFERENCE 33
    #endif
    #ifndef DWMWCP_ROUND
    #define DWMWCP_ROUND 2
    #endif
    if (config.mode != WindowMode::Popup) {
        DWORD cornerPref = DWMWCP_ROUND;
        DwmSetWindowAttribute(hwnd_, DWMWA_WINDOW_CORNER_PREFERENCE, &cornerPref, sizeof(cornerPref));
    }

    if (config.borderless || config.csd) {
        SetWindowPos(hwnd_, nullptr, 0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
    }

    if (config.blur && (config.csd || config.borderless) && config.mode != WindowMode::Popup) {
        setBlurBehind(true);
    }

    ShowWindow(hwnd_, (config.mode == WindowMode::Popup) ? SW_SHOWNA : SW_SHOW);

    if (config.borderless || config.csd) {
        SetWindowPos(hwnd_, nullptr, 0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
    }

    UpdateWindow(hwnd_);

    // Sync actual client dimensions right from launch
    RECT cr{};
    if (GetClientRect(hwnd_, &cr)) {
        current_width_  = cr.right - cr.left;
        current_height_ = cr.bottom - cr.top;
    }

    return true;
}

void Win32Window::setupDpi() {
    UINT dpi = 96;
    HMODULE user32 = GetModuleHandleW(L"user32.dll");
    if (user32) {
        typedef UINT (WINAPI *GetDpiForWindowProc)(HWND);
        auto getDpi = (GetDpiForWindowProc)GetProcAddress(user32, "GetDpiForWindow");
        if (getDpi && hwnd_) {
            dpi = getDpi(hwnd_);
        }
    }
    dpi_scale_ = dpi / 96.0f;
}

void Win32Window::destroy() {
    is_destroying_ = true;

    // Disconnect all signals before window teardown to prevent re-entrant callbacks
    on_close_.disconnectAll();
    on_resize_.disconnectAll();
    on_focus_.disconnectAll();
    on_maximized_.disconnectAll();
    on_state_changed_.disconnectAll();

    if (hglrc_) {
        if (wglGetCurrentContext() == hglrc_ && wglGetCurrentDC() == hdc_) {
            wglMakeCurrent(nullptr, nullptr);
        }
        g_hglrc_ref_count--;
        if (g_hglrc_ref_count <= 0) {
            wglDeleteContext(g_shared_hglrc);
            g_shared_hglrc = nullptr;
            g_hglrc_ref_count = 0;
        }
        hglrc_ = nullptr;
    }
    if (hdc_ && hwnd_) {
        ReleaseDC(hwnd_, hdc_);
        hdc_ = nullptr;
    }
    if (hwnd_) {
        HWND h = hwnd_;
        hwnd_ = nullptr;
        DestroyWindow(h);
    }
}

void Win32Window::setTitle(std::string_view title) {
    if (!hwnd_) return;
    std::wstring wtitle = utf8ToWide(title);
    SetWindowTextW(hwnd_, wtitle.c_str());
}

void Win32Window::setSize(int width, int height) {
    if (!hwnd_) return;
    current_width_  = width;
    current_height_ = height;
    SetWindowPos(hwnd_, nullptr, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
}

void Win32Window::setPosition(int x, int y) {
    if (!hwnd_) return;
    if (config_.mode == WindowMode::Popup) {
        HWND parent_hwnd = nullptr;
        if (config_.parent_window) {
            parent_hwnd = static_cast<HWND>(config_.parent_window->getNativeHandle());
        }
        if (!parent_hwnd) parent_hwnd = GetActiveWindow();
        if (parent_hwnd) {
            POINT pt = { x, y };
            ClientToScreen(parent_hwnd, &pt);
            x = pt.x;
            y = pt.y;
        }
    }
    SetWindowPos(hwnd_, nullptr, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
}

void Win32Window::setBorderless(bool borderless) {
    if (!hwnd_) return;
    config_.borderless = borderless;
    DWORD style = borderless ? (WS_POPUP | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX) : WS_OVERLAPPEDWINDOW;
    if (!borderless && !config_.resizable) {
        style &= ~(WS_THICKFRAME | WS_MAXIMIZEBOX);
    }
    SetWindowLongW(hwnd_, GWL_STYLE, style);
    SetWindowPos(hwnd_, nullptr, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
}

void Win32Window::setAlwaysOnTop(bool on_top) {
    if (!hwnd_) return;
    SetWindowPos(hwnd_, on_top ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0,
                 SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
}

void Win32Window::applyBlurBehind(bool enable) {
    if (!hwnd_) return;

    // 1. Try Windows 11 System Backdrop (Acrylic / Mica, Build 22621+)
    DWORD backdropType = enable ? 3 /* DWMSBT_TRANSIENTWINDOW (Acrylic) */ : 1 /* DWMSBT_NONE */;
    HRESULT hr = DwmSetWindowAttribute(hwnd_, 38 /* DWMWA_SYSTEMBACKDROP_TYPE */, &backdropType, sizeof(backdropType));
    if (SUCCEEDED(hr)) {
        return;
    }

    // 2. Try Windows 10 SetWindowCompositionAttribute (Lightweight blur without red tint)
    HMODULE hUser = GetModuleHandleW(L"user32.dll");
    if (hUser) {
        struct ACCENT_POLICY {
            int   AccentState;
            DWORD AccentFlags;
            DWORD GradientColor;
            DWORD AnimationId;
        };

        struct WINDOWCOMPOSITIONATTRIBDATA {
            DWORD  Attrib;
            void*  pvData;
            SIZE_T cbData;
        };

        typedef BOOL (WINAPI *pfnSetWindowCompositionAttribute)(HWND, WINDOWCOMPOSITIONATTRIBDATA*);
        auto setWindowCompositionAttribute = (pfnSetWindowCompositionAttribute)GetProcAddress(hUser, "SetWindowCompositionAttribute");
        if (setWindowCompositionAttribute) {
            ACCENT_POLICY policy{};
            if (enable) {
                policy.AccentState = 3; // ACCENT_ENABLE_BLURBEHIND (Lightweight Aero blur, avoids Windows 10 Acrylic lag)
                policy.AccentFlags = 2;
                policy.GradientColor = 0x01000000; // Neutral transparent tint, never system accent red
            } else {
                policy.AccentState = 0; // ACCENT_DISABLED
            }

            WINDOWCOMPOSITIONATTRIBDATA data{};
            data.Attrib = 19; // WCA_ACCENT_POLICY
            data.pvData = &policy;
            data.cbData = sizeof(policy);
            setWindowCompositionAttribute(hwnd_, &data);
            return;
        }
    }
}

void Win32Window::setBlurBehind(bool enable) {
    if (enable && !(config_.csd || config_.borderless)) {
        enable = false;
    }
    blur_enabled_ = enable;
    if (!in_size_move_) {
        applyBlurBehind(enable);
    }
}

Size Win32Window::getSize() const {
    return { static_cast<float>(current_width_), static_cast<float>(current_height_) };
}

Size Win32Window::getDrawableSize() const {
    return getSize();
}

float Win32Window::getDpiScale() const {
    return dpi_scale_;
}

void Win32Window::makeCurrent() {
    if (hdc_ && hglrc_) {
        wglMakeCurrent(hdc_, hglrc_);
    }
}

void Win32Window::swapBuffers() {
    if (hdc_) {
        SwapBuffers(hdc_);
    }
}

// ── Client-Side Decoration (CSD) Operations ─────────────────────────

void Win32Window::beginMove(float /*local_x*/, float /*local_y*/, int /*button*/) {
    if (!hwnd_) return;
    ReleaseCapture();
    PostMessageW(hwnd_, WM_SYSCOMMAND, 0xF010 /*SC_MOVE*/ + 2, 0);
}

void Win32Window::beginResize(WindowEdge edge, float /*local_x*/, float /*local_y*/, int /*button*/) {
    if (!hwnd_ || edge == WindowEdge::NoneEdge) return;
    ReleaseCapture();

    int dir = 0;
    switch (edge) {
        case WindowEdge::Left:        dir = 1; break; // WMSZ_LEFT
        case WindowEdge::Right:       dir = 2; break; // WMSZ_RIGHT
        case WindowEdge::Top:         dir = 3; break; // WMSZ_TOP
        case WindowEdge::TopLeft:     dir = 4; break; // WMSZ_TOPLEFT
        case WindowEdge::TopRight:    dir = 5; break; // WMSZ_TOPRIGHT
        case WindowEdge::Bottom:      dir = 6; break; // WMSZ_BOTTOM
        case WindowEdge::BottomLeft:  dir = 7; break; // WMSZ_BOTTOMLEFT
        case WindowEdge::BottomRight: dir = 8; break; // WMSZ_BOTTOMRIGHT
        default: return;
    }
    PostMessageW(hwnd_, WM_SYSCOMMAND, 0xF000 /*SC_SIZE*/ + dir, 0);
}

void Win32Window::setMaximized(bool max) {
    if (!hwnd_) return;
    ShowWindow(hwnd_, max ? SW_MAXIMIZE : SW_RESTORE);
}

void Win32Window::setMinimized(bool min) {
    if (!hwnd_) return;
    ShowWindow(hwnd_, min ? SW_MINIMIZE : SW_RESTORE);
}

void Win32Window::setFullscreen(bool full) {
    if (!hwnd_ || is_fullscreen_ == full) return;
    is_fullscreen_ = full;

    DWORD style = GetWindowLongW(hwnd_, GWL_STYLE);
    if (full) {
        GetWindowPlacement(hwnd_, &prev_placement_);
        HMONITOR hMon = MonitorFromWindow(hwnd_, MONITOR_DEFAULTTONEAREST);
        MONITORINFO mi{ sizeof(MONITORINFO) };
        GetMonitorInfoW(hMon, &mi);

        SetWindowLongW(hwnd_, GWL_STYLE, style & ~WS_OVERLAPPEDWINDOW);
        SetWindowPos(hwnd_, HWND_TOP,
                     mi.rcMonitor.left, mi.rcMonitor.top,
                     mi.rcMonitor.right - mi.rcMonitor.left,
                     mi.rcMonitor.bottom - mi.rcMonitor.top,
                     SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
        state_ |= WindowState::Fullscreen;
    } else {
        SetWindowLongW(hwnd_, GWL_STYLE, style | WS_OVERLAPPEDWINDOW);
        SetWindowPlacement(hwnd_, &prev_placement_);
        SetWindowPos(hwnd_, nullptr, 0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
        state_ = static_cast<WindowState>(static_cast<uint32_t>(state_) & ~static_cast<uint32_t>(WindowState::Fullscreen));
    }
    on_state_changed_.emit(state_);
}

void Win32Window::toggleMaximize() {
    setMaximized(!isMaximized());
}

void Win32Window::showWindowMenu(float local_x, float local_y, int /*button*/) {
    if (!hwnd_) return;
    POINT pt = { static_cast<LONG>(local_x), static_cast<LONG>(local_y) };
    ClientToScreen(hwnd_, &pt);
    HMENU hMenu = GetSystemMenu(hwnd_, FALSE);
    if (hMenu) {
        TrackPopupMenu(hMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd_, nullptr);
    }
}

void Win32Window::setDecorated(bool decorated) {
    setBorderless(!decorated);
}

void Win32Window::setWindowGeometry(int x, int y, int width, int height) {
    if (!hwnd_) return;
    SetWindowPos(hwnd_, nullptr, x, y, width, height, SWP_NOZORDER | SWP_NOACTIVATE);
}

// ── Window Message Loop Handler ────────────────────────────────────

LRESULT Win32Window::handleMessage(UINT msg, WPARAM wParam, LPARAM lParam) {
    if (is_destroying_) {
        return DefWindowProcW(hwnd_, msg, wParam, lParam);
    }

    auto* plat = backend_.getOwner();

    switch (msg) {
        case WM_GETMINMAXINFO: {
            if ((config_.borderless || config_.csd) && config_.mode != WindowMode::Popup && !is_fullscreen_) {
                auto* mmi = reinterpret_cast<MINMAXINFO*>(lParam);
                HMONITOR hMon = MonitorFromWindow(hwnd_, MONITOR_DEFAULTTONEAREST);
                MONITORINFO mi{ sizeof(MONITORINFO) };
                if (GetMonitorInfoW(hMon, &mi)) {
                    mmi->ptMaxPosition.x = std::abs(mi.rcWork.left - mi.rcMonitor.left);
                    mmi->ptMaxPosition.y = std::abs(mi.rcWork.top - mi.rcMonitor.top);
                    mmi->ptMaxSize.x     = mi.rcWork.right - mi.rcWork.left;
                    mmi->ptMaxSize.y     = mi.rcWork.bottom - mi.rcWork.top;
                    mmi->ptMaxTrackSize.x = mmi->ptMaxSize.x;
                    mmi->ptMaxTrackSize.y = mmi->ptMaxSize.y;
                }
                return 0;
            }

            if (config_.min_width > 0 || config_.min_height > 0 || !config_.resizable) {
                auto* mmi = reinterpret_cast<MINMAXINFO*>(lParam);
                if (!config_.resizable) {
                    mmi->ptMinTrackSize.x = current_width_;
                    mmi->ptMinTrackSize.y = current_height_;
                    mmi->ptMaxTrackSize.x = current_width_;
                    mmi->ptMaxTrackSize.y = current_height_;
                } else {
                    if (config_.min_width > 0)  mmi->ptMinTrackSize.x = config_.min_width;
                    if (config_.min_height > 0) mmi->ptMinTrackSize.y = config_.min_height;
                }
                return 0;
            }
            break;
        }

        case WM_NCCALCSIZE: {
            if (config_.borderless || config_.csd) {
                if (wParam == TRUE) {
                    if (hwnd_ && IsZoomed(hwnd_)) {
                        auto* params = reinterpret_cast<NCCALCSIZE_PARAMS*>(lParam);
                        HMONITOR hMon = MonitorFromWindow(hwnd_, MONITOR_DEFAULTTONEAREST);
                        MONITORINFO mi{ sizeof(MONITORINFO) };
                        if (GetMonitorInfoW(hMon, &mi)) {
                            params->rgrc[0] = mi.rcWork;
                        }
                    }
                    return 0;
                }
                return 0;
            }
            break;
        }

        case WM_SIZE: {
            int nw = LOWORD(lParam);
            int nh = HIWORD(lParam);
            current_width_  = nw;
            current_height_ = nh;

            if (wParam == SIZE_MAXIMIZED) {
                state_ |= WindowState::Maximized;
                state_ = static_cast<WindowState>(static_cast<uint32_t>(state_) & ~static_cast<uint32_t>(WindowState::Minimized));
                on_maximized_.emit(true);
            } else if (wParam == SIZE_MINIMIZED) {
                state_ |= WindowState::Minimized;
                state_ = static_cast<WindowState>(static_cast<uint32_t>(state_) & ~static_cast<uint32_t>(WindowState::Maximized));
            } else if (wParam == SIZE_RESTORED) {
                state_ = static_cast<WindowState>(static_cast<uint32_t>(state_) & ~(static_cast<uint32_t>(WindowState::Maximized) | static_cast<uint32_t>(WindowState::Minimized)));
                on_maximized_.emit(false);
            }

            on_resize_.emit(nw, nh);
            on_state_changed_.emit(state_);
            return 0;
        }

        case WM_SETFOCUS: {
            state_ |= WindowState::Activated;
            on_focus_.emit(true);
            on_state_changed_.emit(state_);
            return 0;
        }

        case WM_KILLFOCUS: {
            state_ = static_cast<WindowState>(static_cast<uint32_t>(state_) & ~static_cast<uint32_t>(WindowState::Activated));
            on_focus_.emit(false);
            on_state_changed_.emit(state_);
            return 0;
        }

        case WM_CLOSE: {
            on_close_.emit();
            return 0;
        }

        case WM_DESTROY: {
            return 0;
        }

        case WM_DPICHANGED: {
            setupDpi();
            auto* rect = reinterpret_cast<RECT*>(lParam);
            SetWindowPos(hwnd_, nullptr, rect->left, rect->top,
                         rect->right - rect->left, rect->bottom - rect->top,
                         SWP_NOZORDER | SWP_NOACTIVATE);
            return 0;
        }

        case WM_MOUSEMOVE: {
            float x = static_cast<float>(GET_X_LPARAM(lParam));
            float y = static_cast<float>(GET_Y_LPARAM(lParam));
            if (plat) {
                plat->onTargetedMouseMove().emit((void*)hwnd_, x, y);
                plat->onMouseMove().emit(x, y);
            }
            return 0;
        }

        case WM_LBUTTONDOWN: {
            SetCapture(hwnd_);
            float x = static_cast<float>(GET_X_LPARAM(lParam));
            float y = static_cast<float>(GET_Y_LPARAM(lParam));
            if (plat) {
                plat->onTargetedMouseDown().emit((void*)hwnd_, x, y, 1);
                plat->onMouseDown().emit(x, y, 1);
            }
            return 0;
        }

        case WM_LBUTTONUP: {
            ReleaseCapture();
            float x = static_cast<float>(GET_X_LPARAM(lParam));
            float y = static_cast<float>(GET_Y_LPARAM(lParam));
            if (plat) {
                plat->onTargetedMouseUp().emit((void*)hwnd_, x, y, 1);
                plat->onMouseUp().emit(x, y, 1);
            }
            return 0;
        }

        case WM_RBUTTONDOWN: {
            float x = static_cast<float>(GET_X_LPARAM(lParam));
            float y = static_cast<float>(GET_Y_LPARAM(lParam));
            if (plat) {
                plat->onTargetedMouseDown().emit((void*)hwnd_, x, y, 3);
                plat->onMouseDown().emit(x, y, 3);
            }
            return 0;
        }

        case WM_RBUTTONUP: {
            float x = static_cast<float>(GET_X_LPARAM(lParam));
            float y = static_cast<float>(GET_Y_LPARAM(lParam));
            if (plat) {
                plat->onTargetedMouseUp().emit((void*)hwnd_, x, y, 3);
                plat->onMouseUp().emit(x, y, 3);
            }
            return 0;
        }

        case WM_MBUTTONDOWN: {
            float x = static_cast<float>(GET_X_LPARAM(lParam));
            float y = static_cast<float>(GET_Y_LPARAM(lParam));
            if (plat) {
                plat->onTargetedMouseDown().emit((void*)hwnd_, x, y, 2);
                plat->onMouseDown().emit(x, y, 2);
            }
            return 0;
        }

        case WM_MBUTTONUP: {
            float x = static_cast<float>(GET_X_LPARAM(lParam));
            float y = static_cast<float>(GET_Y_LPARAM(lParam));
            if (plat) {
                plat->onTargetedMouseUp().emit((void*)hwnd_, x, y, 2);
                plat->onMouseUp().emit(x, y, 2);
            }
            return 0;
        }

        case WM_MOUSEWHEEL: {
            float delta = static_cast<float>(GET_WHEEL_DELTA_WPARAM(wParam)) / static_cast<float>(WHEEL_DELTA);
            if (plat) {
                plat->onTargetedScroll().emit((void*)hwnd_, 0.0f, delta);
                plat->onScroll().emit(0.0f, delta);
            }
            return 0;
        }

        case WM_CHAR: {
            wchar_t ch = static_cast<wchar_t>(wParam);
            if (ch >= 32 || ch == '\t' || ch == '\n' || ch == '\r') {
                std::wstring ws(1, ch);
                std::string utf8 = wideToUtf8(ws);
                if (plat && !utf8.empty()) {
                    plat->onTextInput().emit(utf8);
                }
            }
            return 0;
        }

        case WM_KEYDOWN:
        case WM_SYSKEYDOWN: {
            int mods = 0;
            if (GetKeyState(VK_SHIFT) & 0x8000)   mods |= 1;
            if (GetKeyState(VK_CONTROL) & 0x8000) mods |= 2;
            if (GetKeyState(VK_MENU) & 0x8000)    mods |= 4;

            if (plat) {
                plat->onKeyDown().emit(static_cast<int>(wParam), mods);
            }
            break;
        }

        case WM_KEYUP:
        case WM_SYSKEYUP: {
            int mods = 0;
            if (GetKeyState(VK_SHIFT) & 0x8000)   mods |= 1;
            if (GetKeyState(VK_CONTROL) & 0x8000) mods |= 2;
            if (GetKeyState(VK_MENU) & 0x8000)    mods |= 4;

            if (plat) {
                plat->onKeyUp().emit(static_cast<int>(wParam), mods);
            }
            break;
        }

        case WM_ENTERSIZEMOVE: {
            in_size_move_ = true;
            if (blur_enabled_) {
                applyBlurBehind(false);
            }
            return 0;
        }

        case WM_EXITSIZEMOVE: {
            in_size_move_ = false;
            if (blur_enabled_) {
                applyBlurBehind(true);
            }
            return 0;
        }

        case WM_PAINT: {
            PAINTSTRUCT ps;
            BeginPaint(hwnd_, &ps);
            EndPaint(hwnd_, &ps);
            return 0;
        }

        case WM_ERASEBKGND:
            return 1; // Prevent flickering

        default:
            break;
    }

    return DefWindowProcW(hwnd_, msg, wParam, lParam);
}

} // namespace nisaba::backend_os::win32

namespace nisaba::backend_os {

Result<std::unique_ptr<LayerSurface>> LayerSurface::create(Platform&, LayerSurfaceConfig) {
    return Result<std::unique_ptr<LayerSurface>>::err(
        ErrorCode::NotSupported,
        "LayerSurface is not supported on Windows (Wayland/X11 desktop shell only)"
    );
}

} // namespace nisaba::backend_os

