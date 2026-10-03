/// @file x11_window.cpp
/// @brief X11 + EGL window backend implementation.

#include "nisaba/backend_os/x11/x11_window.hpp"

#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <GL/gl.h>
#include <dlfcn.h>
#include <iostream>
#include <cstring>
#include <unistd.h>

namespace nisaba::backend_os::x11 {

X11Window::X11Window(X11PlatformBackend& backend)
    : backend_(backend) {}

X11Window::~X11Window() {
    destroy();
}

bool X11Window::init(const WindowConfig& cfg) {
    config_     = cfg;
    display_    = backend_.getDisplay();
    egl_display_ = backend_.getEGLDisplay();
    EGLConfig egl_cfg = backend_.getEGLConfig();

    if (!display_ || egl_display_ == EGL_NO_DISPLAY || !egl_cfg) {
        std::cerr << "[Nisaba X11Window] Backend display/EGL invalid\n";
        return false;
    }

    int screen     = backend_.getDefaultScreen();
    ::Window root  = RootWindow(display_, screen);

    current_width_  = cfg.width;
    current_height_ = cfg.height;

    int screen_w = DisplayWidth(display_,  screen);
    int screen_h = DisplayHeight(display_, screen);

    ::Window parent_x11 = 0;
    if (cfg.parent_window) {
        auto* parent_x11_win = static_cast<X11Window*>(cfg.parent_window->getBackendWindow());
        if (parent_x11_win) {
            parent_x11 = parent_x11_win->getX11Window();
        }
    }

    int pos_x = (cfg.x >= 0) ? cfg.x : (screen_w - current_width_)  / 2;
    int pos_y = (cfg.y >= 0) ? cfg.y : (screen_h - current_height_) / 2;

    if (cfg.mode == WindowMode::Popup) {
        if (parent_x11 != 0) {
            int root_x = 0, root_y = 0;
            ::Window child = 0;
            XTranslateCoordinates(display_, parent_x11, root,
                                  cfg.x >= 0 ? cfg.x : 0,
                                  cfg.y >= 0 ? cfg.y : 0,
                                  &root_x, &root_y, &child);
            pos_x = root_x;
            pos_y = root_y;
        } else {
            // Fallback: place near pointer if parent window is not supplied
            int root_x = 0, root_y = 0, win_x = 0, win_y = 0;
            unsigned int mask = 0;
            ::Window root_ret = 0, child_ret = 0;
            if (XQueryPointer(display_, root, &root_ret, &child_ret, &root_x, &root_y, &win_x, &win_y, &mask)) {
                pos_x = root_x;
                pos_y = root_y;
            }
        }

        // Clamp to screen bounds so popup doesn't overflow screen
        if (pos_x + current_width_ > screen_w - 5) {
            pos_x = screen_w - current_width_ - 5;
        }
        if (pos_y + current_height_ > screen_h - 5) {
            pos_y = screen_h - current_height_ - 5;
        }
    }

    if (pos_x < 0) pos_x = 0;
    if (pos_y < 0) pos_y = 0;

    // Match X11 visual to EGL native visual id
    EGLint visual_id = 0;
    eglGetConfigAttrib(egl_display_, egl_cfg, EGL_NATIVE_VISUAL_ID, &visual_id);

    Visual* visual = nullptr;
    int depth = 0;
    if (visual_id != 0) {
        XVisualInfo tmpl; tmpl.visualid = visual_id;
        int n = 0;
        XVisualInfo* list = XGetVisualInfo(display_, VisualIDMask, &tmpl, &n);
        if (list && n > 0) { visual = list[0].visual; depth = list[0].depth; }
        if (list) XFree(list);
    }
    if (!visual) {
        visual = DefaultVisual(display_, screen);
        depth  = DefaultDepth(display_, screen);
    }

    colormap_ = XCreateColormap(display_, root, visual, AllocNone);

    XSetWindowAttributes swa{};
    swa.colormap          = colormap_;
    swa.border_pixel      = 0;
    swa.background_pixmap = None;
    swa.bit_gravity       = NorthWestGravity;
    swa.event_mask        = ExposureMask | StructureNotifyMask | ButtonPressMask |
                            ButtonReleaseMask | PointerMotionMask | KeyPressMask |
                            KeyReleaseMask | FocusChangeMask | PropertyChangeMask;
    unsigned long valuemask = CWColormap | CWBorderPixel | CWBackPixmap | CWBitGravity | CWEventMask;

    if (cfg.override_redirect || cfg.mode == WindowMode::Popup) {
        swa.override_redirect = True;
        valuemask |= CWOverrideRedirect;
    }

    x11_window_ = XCreateWindow(display_, root, pos_x, pos_y,
                                 current_width_, current_height_,
                                 0, depth, InputOutput, visual,
                                 valuemask, &swa);
    if (!x11_window_) {
        std::cerr << "[Nisaba X11Window] XCreateWindow failed\n";
        return false;
    }

    // Never paint or clear any background by the X server (pure client rendering)
    XSetWindowBackgroundPixmap(display_, x11_window_, None);

    setTitle(cfg.title);

    // Set WM_CLASS so compositors and window managers can identify the application
    {
        XClassHint class_hint{};
        std::string app_id = cfg.app_id.empty() ? "nisaba.app" : cfg.app_id;
        std::string res_name = app_id;
        auto last_dot = res_name.rfind('.');
        if (last_dot != std::string::npos && last_dot + 1 < res_name.size()) {
            res_name = res_name.substr(last_dot + 1);
        }
        class_hint.res_name  = res_name.data();
        class_hint.res_class = app_id.data();
        XSetClassHint(display_, x11_window_, &class_hint);
    }

    // Set _NET_WM_WINDOW_TYPE
    Atom net_wm_window_type = XInternAtom(display_, "_NET_WM_WINDOW_TYPE", False);
    if (cfg.mode == WindowMode::Popup) {
        Atom net_wm_window_type_popup = XInternAtom(display_, "_NET_WM_WINDOW_TYPE_POPUP_MENU", False);
        XChangeProperty(display_, x11_window_, net_wm_window_type, XA_ATOM, 32,
                        PropModeReplace, (unsigned char*)&net_wm_window_type_popup, 1);
    } else {
        Atom net_wm_window_type_normal = XInternAtom(display_, "_NET_WM_WINDOW_TYPE_NORMAL", False);
        XChangeProperty(display_, x11_window_, net_wm_window_type, XA_ATOM, 32,
                        PropModeReplace, (unsigned char*)&net_wm_window_type_normal, 1);
    }

    if (parent_x11 != 0) {
        XSetTransientForHint(display_, x11_window_, parent_x11);
    }

    // WM_DELETE_WINDOW protocol
    Atom wm_delete = backend_.getAtomWmDeleteWindow();
    XSetWMProtocols(display_, x11_window_, &wm_delete, 1);

    if (cfg.borderless || cfg.csd) {
        setBorderless(true);
    }
    if (cfg.always_on_top) setAlwaysOnTop(true);

    if (cfg.blur) {
        setBlurBehind(true);
    }

    // Set _NET_WM_PID
    Atom net_pid = XInternAtom(display_, "_NET_WM_PID", False);
    long pid     = (long)getpid();
    XChangeProperty(display_, x11_window_, net_pid, XA_CARDINAL, 32,
                    PropModeReplace, (unsigned char*)&pid, 1);

    // Size hints
    XSizeHints sh{};
    sh.flags     = PMinSize;
    sh.min_width  = cfg.min_width;
    sh.min_height = cfg.min_height;
    if (!cfg.resizable) {
        sh.flags |= PMaxSize;
        sh.max_width = sh.max_height = cfg.width;
    }
    XSetWMNormalHints(display_, x11_window_, &sh);

    // EGL surface
    egl_surface_ = eglCreateWindowSurface(egl_display_, egl_cfg,
                                          (EGLNativeWindowType)x11_window_, nullptr);
    if (egl_surface_ == EGL_NO_SURFACE) {
        std::cerr << "[Nisaba X11Window] Failed to create EGL surface\n";
        return false;
    }

    // Use shared master EGL context from backend
    egl_context_ = backend_.getEGLContext();
    if (egl_context_ == EGL_NO_CONTEXT) {
        const EGLint core_attrs[] = {
            EGL_CONTEXT_MAJOR_VERSION, 3, EGL_CONTEXT_MINOR_VERSION, 3,
            EGL_CONTEXT_OPENGL_PROFILE_MASK, EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT,
            EGL_NONE };
        const EGLint gles_attrs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };

        egl_context_ = eglCreateContext(egl_display_, egl_cfg, EGL_NO_CONTEXT, core_attrs);
        if (egl_context_ == EGL_NO_CONTEXT)
            egl_context_ = eglCreateContext(egl_display_, egl_cfg, EGL_NO_CONTEXT, gles_attrs);
        if (egl_context_ == EGL_NO_CONTEXT)
            egl_context_ = eglCreateContext(egl_display_, egl_cfg, EGL_NO_CONTEXT, nullptr);
    }
    if (egl_context_ == EGL_NO_CONTEXT) {
        std::cerr << "[Nisaba X11Window] Failed to create EGL context\n";
        return false;
    }

    eglMakeCurrent(egl_display_, egl_surface_, egl_surface_, egl_context_);
    eglSwapInterval(egl_display_, cfg.vsync ? 1 : 0);

    XMapWindow(display_, x11_window_);
    if (cfg.mode == WindowMode::Popup) {
        XRaiseWindow(display_, x11_window_);
    }
    XFlush(display_);
    return true;
}

void X11Window::destroy() {
    if (egl_display_ != EGL_NO_DISPLAY) {
        if (egl_surface_ != EGL_NO_SURFACE) {
            eglDestroySurface(egl_display_, egl_surface_);
            egl_surface_ = EGL_NO_SURFACE;
        }
    }
    if (display_ && x11_window_) {
        XDestroyWindow(display_, x11_window_);
        x11_window_ = 0;
    }
    if (display_ && colormap_) {
        XFreeColormap(display_, colormap_);
        colormap_ = 0;
    }
}

void X11Window::setTitle(std::string_view title) {
    if (!display_ || !x11_window_) return;
    std::string s(title);
    XStoreName(display_, x11_window_, s.c_str());
    Atom net_name = XInternAtom(display_, "_NET_WM_NAME", False);
    Atom utf8     = backend_.getAtomUtf8String();
    XChangeProperty(display_, x11_window_, net_name, utf8, 8,
                    PropModeReplace, (unsigned char*)s.c_str(), (int)s.size());
    XFlush(display_);
}

void X11Window::setSize(int w, int h) {
    current_width_ = w; current_height_ = h;
    if (display_ && x11_window_) {
        XResizeWindow(display_, x11_window_, w, h);
        if (blur_enabled_) {
            updateBlurBehindRegion();
        }
        XFlush(display_);
    }
}

void X11Window::setPosition(int x, int y) {
    if (!display_ || !x11_window_) return;
    int target_x = x;
    int target_y = y;
    if (config_.mode == WindowMode::Popup && config_.parent_window) {
        auto* parent_x11_win = static_cast<X11Window*>(config_.parent_window->getBackendWindow());
        if (parent_x11_win) {
            int root_x = 0, root_y = 0;
            ::Window child = 0;
            ::Window root = RootWindow(display_, backend_.getDefaultScreen());
            if (XTranslateCoordinates(display_, parent_x11_win->getX11Window(), root, x, y, &root_x, &root_y, &child)) {
                target_x = root_x;
                target_y = root_y;
            }
        }
    }
    XMoveWindow(display_, x11_window_, target_x, target_y);
    XFlush(display_);
}

void X11Window::setBorderless(bool borderless) {
    if (!display_ || !x11_window_) return;
    struct { unsigned long flags, functions, decorations; long inputMode; unsigned long status; } h{};
    h.flags = 2; // MWM_HINTS_DECORATIONS
    h.decorations = borderless ? 0 : 1;
    Atom atom = XInternAtom(display_, "_MOTIF_WM_HINTS", False);
    XChangeProperty(display_, x11_window_, atom, atom, 32, PropModeReplace, (unsigned char*)&h, 5);
    XFlush(display_);
}

void X11Window::setAlwaysOnTop(bool on_top) {
    if (!display_ || !x11_window_) return;
    Atom wm_state = XInternAtom(display_, "_NET_WM_STATE", False);
    Atom wm_above = XInternAtom(display_, "_NET_WM_STATE_ABOVE", False);
    XEvent xev{};
    xev.type = ClientMessage;
    xev.xclient.window       = x11_window_;
    xev.xclient.message_type = wm_state;
    xev.xclient.format       = 32;
    xev.xclient.data.l[0]    = on_top ? 1 : 0;
    xev.xclient.data.l[1]    = wm_above;
    XSendEvent(display_, DefaultRootWindow(display_), False,
               SubstructureRedirectMask | SubstructureNotifyMask, &xev);
    XFlush(display_);
}

// ── Client-Side Decoration (CSD) Operations ─────────────────────

void X11Window::beginMove(float local_x, float local_y, int button) {
    if (!display_ || !x11_window_) return;
    if (state_ == WindowState::Maximized || state_ == WindowState::Fullscreen) return;

    // 1. تحرير الماوس فورا لكي يستلمه مدير النوافذ
    XUngrabPointer(display_, CurrentTime);

    // 2. حساب الإحداثيات المطلقة للمؤشر
    ::Window root = RootWindow(display_, backend_.getDefaultScreen());
    int root_x = 0, root_y = 0;
    ::Window child = 0;
    XTranslateCoordinates(display_, x11_window_, root,
                          static_cast<int>(local_x), static_cast<int>(local_y),
                          &root_x, &root_y, &child);

    // 3. إرسال أمر التحريك القياسي لـ X11
    XClientMessageEvent xclient{};
    xclient.type = ClientMessage;
    xclient.window = x11_window_;
    xclient.message_type = backend_.getAtomNetWmMoveresize();
    xclient.format = 32;
    xclient.data.l[0] = root_x;
    xclient.data.l[1] = root_y;
    xclient.data.l[2] = 8; // 8 تعني _NET_WM_MOVERESIZE_MOVE
    xclient.data.l[3] = button;
    xclient.data.l[4] = 1; // 1 تعني أن الطلب قادم من تطبيق

    XSendEvent(display_, root, False,
               SubstructureRedirectMask | SubstructureNotifyMask,
               reinterpret_cast<XEvent*>(&xclient));
    XFlush(display_);
}

void X11Window::beginResize(WindowEdge edge, float local_x, float local_y, int button) {
    if (!display_ || !x11_window_ || edge == WindowEdge::NoneEdge) return;
    if (state_ == WindowState::Maximized || state_ == WindowState::Fullscreen) return;

    XUngrabPointer(display_, CurrentTime);

    ::Window root = RootWindow(display_, backend_.getDefaultScreen());
    int root_x = 0, root_y = 0;
    ::Window child = 0;
    XTranslateCoordinates(display_, x11_window_, root,
                          static_cast<int>(local_x), static_cast<int>(local_y),
                          &root_x, &root_y, &child);

    // تحويل WindowEdge الخاص بـ Nisaba إلى أرقام بروتوكول X11 EWMH
    int direction = 8; 
    switch (edge) {
        case WindowEdge::TopLeft:     direction = 0; break;
        case WindowEdge::Top:         direction = 1; break;
        case WindowEdge::TopRight:    direction = 2; break;
        case WindowEdge::Right:       direction = 3; break;
        case WindowEdge::BottomRight: direction = 4; break;
        case WindowEdge::Bottom:      direction = 5; break;
        case WindowEdge::BottomLeft:  direction = 6; break;
        case WindowEdge::Left:        direction = 7; break;
        default: return;
    }

    XClientMessageEvent xclient{};
    xclient.type = ClientMessage;
    xclient.window = x11_window_;
    xclient.message_type = backend_.getAtomNetWmMoveresize();
    xclient.format = 32;
    xclient.data.l[0] = root_x;
    xclient.data.l[1] = root_y;
    xclient.data.l[2] = direction;
    xclient.data.l[3] = button;
    xclient.data.l[4] = 1;

    XSendEvent(display_, root, False,
               SubstructureRedirectMask | SubstructureNotifyMask,
               reinterpret_cast<XEvent*>(&xclient));
    XFlush(display_);
}

bool X11Window::handleDragMotion(int /*root_x*/, int /*root_y*/) {
    return false;
}

void X11Window::endDrag() {
}

void X11Window::setMaximized(bool max) {
    if (!display_ || !x11_window_) return;
    Atom wm_state = backend_.getAtomNetWmState();
    Atom max_vert = backend_.getAtomNetWmStateMaxVert();
    Atom max_horz = backend_.getAtomNetWmStateMaxHorz();
    ::Window root = RootWindow(display_, backend_.getDefaultScreen());

    XClientMessageEvent xclient{};
    xclient.type = ClientMessage;
    xclient.window = x11_window_;
    xclient.message_type = wm_state;
    xclient.format = 32;
    xclient.data.l[0] = max ? 1 : 0; // 1 = _NET_WM_STATE_ADD, 0 = _NET_WM_STATE_REMOVE
    xclient.data.l[1] = max_vert;
    xclient.data.l[2] = max_horz;
    xclient.data.l[3] = 1;

    XSendEvent(display_, root, False,
               SubstructureRedirectMask | SubstructureNotifyMask,
               (XEvent*)&xclient);
    XFlush(display_);
}

void X11Window::setMinimized(bool min) {
    if (!display_ || !x11_window_) return;
    if (min) {
        XIconifyWindow(display_, x11_window_, backend_.getDefaultScreen());
        XFlush(display_);
    }
}

void X11Window::setFullscreen(bool full) {
    if (!display_ || !x11_window_) return;
    Atom wm_state = backend_.getAtomNetWmState();
    Atom fullscreen = backend_.getAtomNetWmStateFullscreen();
    ::Window root = RootWindow(display_, backend_.getDefaultScreen());

    XClientMessageEvent xclient{};
    xclient.type = ClientMessage;
    xclient.window = x11_window_;
    xclient.message_type = wm_state;
    xclient.format = 32;
    xclient.data.l[0] = full ? 1 : 0;
    xclient.data.l[1] = fullscreen;
    xclient.data.l[2] = 0;
    xclient.data.l[3] = 1;

    XSendEvent(display_, root, False,
               SubstructureRedirectMask | SubstructureNotifyMask,
               (XEvent*)&xclient);
    XFlush(display_);
}

void X11Window::toggleMaximize() {
    setMaximized(!isMaximized());
}

void X11Window::showWindowMenu(float local_x, float local_y, int /*button*/) {
    if (!display_ || !x11_window_) return;
    int root_x = 0, root_y = 0;
    ::Window child = 0;
    ::Window root = RootWindow(display_, backend_.getDefaultScreen());
    XTranslateCoordinates(display_, x11_window_, root,
                          static_cast<int>(local_x), static_cast<int>(local_y),
                          &root_x, &root_y, &child);

    Atom show_menu = XInternAtom(display_, "_GTK_SHOW_WINDOW_MENU", False);
    XClientMessageEvent xclient{};
    xclient.type = ClientMessage;
    xclient.window = x11_window_;
    xclient.message_type = show_menu;
    xclient.format = 32;
    xclient.data.l[0] = 0; // default device
    xclient.data.l[1] = root_x;
    xclient.data.l[2] = root_y;

    XUngrabPointer(display_, CurrentTime);
    XSendEvent(display_, root, False,
               SubstructureRedirectMask | SubstructureNotifyMask,
               (XEvent*)&xclient);
    XFlush(display_);
}

void X11Window::setDecorated(bool decorated) {
    setBorderless(!decorated);
}

void X11Window::setWindowGeometry(int x, int y, int width, int height) {
    if (!display_ || !x11_window_) return;
    long left = x > 0 ? x : 0;
    long top = y > 0 ? y : 0;
    long right = (current_width_ > (x + width)) ? (current_width_ - (x + width)) : 0;
    long bottom = (current_height_ > (y + height)) ? (current_height_ - (y + height)) : 0;

    Atom frame_extents = backend_.getAtomGtkFrameExtents();
    if (frame_extents) {
        if (left == 0 && right == 0 && top == 0 && bottom == 0) {
            XDeleteProperty(display_, x11_window_, frame_extents);
        } else {
            unsigned long extents[4] = {
                static_cast<unsigned long>(left),
                static_cast<unsigned long>(right),
                static_cast<unsigned long>(top),
                static_cast<unsigned long>(bottom)
            };
            XChangeProperty(display_, x11_window_, frame_extents, XA_CARDINAL, 32,
                            PropModeReplace, reinterpret_cast<unsigned char*>(extents), 4);
        }
        XFlush(display_);
    }
}

void X11Window::updateState() {
    if (!display_ || !x11_window_) return;
    Atom net_wm_state = backend_.getAtomNetWmState();
    Atom actual_type = None;
    int actual_format = 0;
    unsigned long nitems = 0, bytes_after = 0;
    unsigned char* prop_data = nullptr;

    WindowState new_state = state_ & WindowState::Activated;
    int status = XGetWindowProperty(display_, x11_window_, net_wm_state,
                                    0, 64, False, XA_ATOM,
                                    &actual_type, &actual_format, &nitems, &bytes_after,
                                    &prop_data);
    if (status == Success && prop_data && nitems > 0) {
        Atom* atoms = reinterpret_cast<Atom*>(prop_data);
        Atom max_vert = backend_.getAtomNetWmStateMaxVert();
        Atom max_horz = backend_.getAtomNetWmStateMaxHorz();
        Atom fullscreen = backend_.getAtomNetWmStateFullscreen();
        Atom hidden = backend_.getAtomNetWmStateHidden();

        bool has_vert = false, has_horz = false;
        for (unsigned long i = 0; i < nitems; ++i) {
            if (atoms[i] == max_vert) has_vert = true;
            if (atoms[i] == max_horz) has_horz = true;
            if (atoms[i] == fullscreen) new_state |= WindowState::Fullscreen;
            if (atoms[i] == hidden) new_state |= WindowState::Minimized;
        }
        if (has_vert && has_horz) {
            new_state |= WindowState::Maximized;
        }
        XFree(prop_data);
    }

    bool max_changed = (hasWindowState(state_, WindowState::Maximized) != hasWindowState(new_state, WindowState::Maximized));
    bool state_changed = (state_ != new_state);
    state_ = new_state;

    if (max_changed) {
        on_maximized_.emit(hasWindowState(state_, WindowState::Maximized));
    }
    if (state_changed) {
        on_state_changed_.emit(state_);
    }
}

void X11Window::handleFocus(bool focused) {
    bool prev_focused = hasWindowState(state_, WindowState::Activated);
    if (focused) {
        state_ |= WindowState::Activated;
    } else {
        state_ = static_cast<WindowState>(static_cast<uint32_t>(state_) & ~static_cast<uint32_t>(WindowState::Activated));
    }
    if (prev_focused != focused) {
        on_focus_.emit(focused);
        on_state_changed_.emit(state_);
    }
}

void X11Window::handleConfigure(int nw, int nh) {
    if (current_width_ != nw || current_height_ != nh) {
        current_width_  = nw;
        current_height_ = nh;
        if (blur_enabled_) {
            updateBlurBehindRegion();
        }
    }
}

void X11Window::setBlurBehind(bool enable) {
    blur_enabled_ = enable;
    updateBlurBehindRegion();
}

void X11Window::updateBlurBehindRegion() {
    if (!display_ || !x11_window_) return;

    Atom atom_kde_blur = XInternAtom(display_, "_KDE_NET_WM_BLUR_BEHIND_REGION", False);
    Atom atom_net_blur = XInternAtom(display_, "_NET_WM_BLUR_BEHIND_REGION", False);

    if (blur_enabled_) {
        unsigned long rect[4] = {
            0,
            0,
            static_cast<unsigned long>(current_width_ > 0 ? current_width_ : 1),
            static_cast<unsigned long>(current_height_ > 0 ? current_height_ : 1)
        };
        XChangeProperty(display_, x11_window_, atom_kde_blur, XA_CARDINAL, 32,
                        PropModeReplace, reinterpret_cast<const unsigned char*>(rect), 4);
        XChangeProperty(display_, x11_window_, atom_net_blur, XA_CARDINAL, 32,
                        PropModeReplace, reinterpret_cast<const unsigned char*>(rect), 4);
    } else {
        XDeleteProperty(display_, x11_window_, atom_kde_blur);
        XDeleteProperty(display_, x11_window_, atom_net_blur);
    }
    XFlush(display_);
}

Size X11Window::getSize() const {
    return {(float)current_width_, (float)current_height_};
}
Size X11Window::getDrawableSize() const { return getSize(); }
float X11Window::getDpiScale() const { return 1.0f; }

void X11Window::makeCurrent() {
    if (egl_display_ != EGL_NO_DISPLAY && egl_surface_ != EGL_NO_SURFACE)
        eglMakeCurrent(egl_display_, egl_surface_, egl_surface_, egl_context_);
}

void X11Window::swapBuffers() {
    if (egl_display_ != EGL_NO_DISPLAY && egl_surface_ != EGL_NO_SURFACE)
        eglSwapBuffers(egl_display_, egl_surface_);
}

} // namespace nisaba::backend_os::x11
