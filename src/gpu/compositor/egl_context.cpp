#include "nisaba/compositor/egl_context.hpp"
#include "nisaba/compositor/log.hpp"

#include <sstream>

namespace nisaba::compositor {

namespace {

#ifndef EGL_PLATFORM_GBM_KHR
#define EGL_PLATFORM_GBM_KHR 0x31D7
#endif

typedef EGLDisplay (*PFNEGLGETPLATFORMDISPLAYEXTPROC)(EGLenum platform, void *native_display, const EGLint *attrib_list);

} // namespace

std::shared_ptr<EglContext> EglContext::create(std::shared_ptr<GbmDevice> device) {
    if (!device || !device->raw()) {
        logError("EglContext::create: Invalid GbmDevice");
        return nullptr;
    }

    EGLDisplay display = EGL_NO_DISPLAY;
    auto getPlatformDisplay = reinterpret_cast<PFNEGLGETPLATFORMDISPLAYEXTPROC>(
        eglGetProcAddress("eglGetPlatformDisplayEXT"));

    if (getPlatformDisplay) {
        display = getPlatformDisplay(EGL_PLATFORM_GBM_KHR, device->raw(), nullptr);
    }
    if (display == EGL_NO_DISPLAY) {
        display = eglGetDisplay(reinterpret_cast<EGLNativeDisplayType>(device->raw()));
    }

    if (display == EGL_NO_DISPLAY) {
        logError("EglContext::create: Failed to get EGLDisplay from GBM device");
        return nullptr;
    }

    EGLint major = 0, minor = 0;
    if (!eglInitialize(display, &major, &minor)) {
        logError("EglContext::create: Failed to initialize EGLDisplay (error 0x{:04x})", eglGetError());
        return nullptr;
    }

    logInfo("EGL initialized: version {}.{}", major, minor);

    if (!eglBindAPI(EGL_OPENGL_ES_API)) {
        logError("EglContext::create: Failed to bind OpenGL ES API");
        eglTerminate(display);
        return nullptr;
    }

    const EGLint configAttribs[] = {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_NONE,
    };

    EGLConfig config = nullptr;
    EGLint numConfigs = 0;
    if (!eglChooseConfig(display, configAttribs, &config, 1, &numConfigs) || numConfigs <= 0) {
        logError("EglContext::create: Failed to choose EGLConfig");
        eglTerminate(display);
        return nullptr;
    }

    const EGLint contextAttribs[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE,
    };

    EGLContext context = eglCreateContext(display, config, EGL_NO_CONTEXT, contextAttribs);
    if (context == EGL_NO_CONTEXT) {
        logError("EglContext::create: Failed to create EGLContext (error 0x{:04x})", eglGetError());
        eglTerminate(display);
        return nullptr;
    }

    auto eglCtx = std::make_shared<EglContext>(std::move(device), display, context, config);
    logInfo("Sovereign EGLContext created successfully!");
    return eglCtx;
}

std::shared_ptr<EglContext> EglContext::createAuto() {
    auto device = GbmDevice::createAuto();
    if (!device) return nullptr;
    return create(std::move(device));
}

std::shared_ptr<EglContext> EglContext::createForX11(void* x11Display) {
    if (!x11Display) return nullptr;

    EGLDisplay display = EGL_NO_DISPLAY;
    auto getPlatformDisplay = reinterpret_cast<PFNEGLGETPLATFORMDISPLAYEXTPROC>(
        eglGetProcAddress("eglGetPlatformDisplayEXT"));

#ifndef EGL_PLATFORM_X11_KHR
#define EGL_PLATFORM_X11_KHR 0x31D5
#endif

    if (getPlatformDisplay) {
        display = getPlatformDisplay(EGL_PLATFORM_X11_KHR, x11Display, nullptr);
    }
    if (display == EGL_NO_DISPLAY) {
        display = eglGetDisplay(reinterpret_cast<EGLNativeDisplayType>(x11Display));
    }

    if (display == EGL_NO_DISPLAY) {
        logError("EglContext::createForX11: Failed to get EGLDisplay from X11 display");
        return nullptr;
    }

    EGLint major = 0, minor = 0;
    if (!eglInitialize(display, &major, &minor)) {
        logError("EglContext::createForX11: Failed to initialize EGLDisplay (error 0x{:04x})", eglGetError());
        return nullptr;
    }

    logInfo("EGL (X11) initialized: version {}.{}", major, minor);

    if (!eglBindAPI(EGL_OPENGL_ES_API)) {
        logError("EglContext::createForX11: Failed to bind OpenGL ES API");
        eglTerminate(display);
        return nullptr;
    }

    const EGLint configAttribs[] = {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_NONE,
    };

    EGLConfig config = nullptr;
    EGLint numConfigs = 0;
    if (!eglChooseConfig(display, configAttribs, &config, 1, &numConfigs) || numConfigs <= 0) {
        logError("EglContext::createForX11: Failed to choose EGLConfig");
        eglTerminate(display);
        return nullptr;
    }

    const EGLint contextAttribs[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE,
    };

    EGLContext context = eglCreateContext(display, config, EGL_NO_CONTEXT, contextAttribs);
    if (context == EGL_NO_CONTEXT) {
        logError("EglContext::createForX11: Failed to create EGLContext (error 0x{:04x})", eglGetError());
        eglTerminate(display);
        return nullptr;
    }

    auto eglCtx = std::make_shared<EglContext>(nullptr, display, context, config);
    logInfo("Sovereign X11 EGLContext created successfully!");
    return eglCtx;
}

EglContext::EglContext(std::shared_ptr<GbmDevice> device, EGLDisplay display, EGLContext context, EGLConfig config)
    : m_device(std::move(device)), m_display(display), m_context(context), m_config(config) {
    const char* extStr = eglQueryString(m_display, EGL_EXTENSIONS);
    if (extStr) {
        std::istringstream stream(extStr);
        std::string ext;
        while (stream >> ext) {
            m_extensions.insert(ext);
        }
    }
}

EglContext::~EglContext() {
    if (m_display != EGL_NO_DISPLAY) {
        eglMakeCurrent(m_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (m_context != EGL_NO_CONTEXT) {
            eglDestroyContext(m_display, m_context);
            m_context = EGL_NO_CONTEXT;
        }
        eglTerminate(m_display);
        m_display = EGL_NO_DISPLAY;
    }
}

bool EglContext::makeCurrent(EGLSurface drawSurface) {
    if (m_display == EGL_NO_DISPLAY || m_context == EGL_NO_CONTEXT) return false;
    EGLSurface surf = (drawSurface != EGL_NO_SURFACE) ? drawSurface : EGL_NO_SURFACE;
    return eglMakeCurrent(m_display, surf, surf, m_context) == EGL_TRUE;
}

void EglContext::clearCurrent() {
    if (m_display != EGL_NO_DISPLAY) {
        eglMakeCurrent(m_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    }
}

void EglContext::swapBuffers(EGLSurface surface) {
    if (m_display != EGL_NO_DISPLAY && surface != EGL_NO_SURFACE) {
        if (!eglSwapBuffers(m_display, surface)) {
            logError("eglSwapBuffers failed: 0x{:04x}", eglGetError());
        }
    }
}

void EglContext::setSwapInterval(int interval) {
    if (m_display != EGL_NO_DISPLAY) {
        eglSwapInterval(m_display, interval);
    }
}

EGLSurface EglContext::createWindowSurface(uintptr_t nativeWindow) {
    if (m_display == EGL_NO_DISPLAY || !m_config) return EGL_NO_SURFACE;
    return eglCreateWindowSurface(m_display, m_config, reinterpret_cast<EGLNativeWindowType>(nativeWindow), nullptr);
}

void EglContext::destroySurface(EGLSurface surface) {
    if (m_display != EGL_NO_DISPLAY && surface != EGL_NO_SURFACE) {
        eglDestroySurface(m_display, surface);
    }
}

bool EglContext::hasExtension(std::string_view extension) const noexcept {
    return m_extensions.contains(std::string(extension));
}

void* EglContext::getProcAddress(const char* name) const noexcept {
    return reinterpret_cast<void*>(eglGetProcAddress(name));
}

} // namespace nisaba::compositor
