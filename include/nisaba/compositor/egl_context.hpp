#pragma once

#include "nisaba/compositor/gbm_device.hpp"

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_set>

namespace nisaba::compositor {

/**
 * @brief Sovereign EGL Context & Display manager for GPU acceleration.
 */
class EglContext {
public:
    static std::shared_ptr<EglContext> create(std::shared_ptr<GbmDevice> device);
    static std::shared_ptr<EglContext> createAuto();
    static std::shared_ptr<EglContext> createForX11(void* x11Display);

    EglContext(std::shared_ptr<GbmDevice> device, EGLDisplay display, EGLContext context, EGLConfig config);
    ~EglContext();

    EglContext(const EglContext&) = delete;
    EglContext& operator=(const EglContext&) = delete;

    [[nodiscard]] EGLDisplay display() const noexcept { return m_display; }
    [[nodiscard]] EGLContext context() const noexcept { return m_context; }
    [[nodiscard]] EGLConfig config() const noexcept { return m_config; }
    [[nodiscard]] const std::shared_ptr<GbmDevice>& device() const noexcept { return m_device; }

    bool makeCurrent(EGLSurface drawSurface = EGL_NO_SURFACE);
    void clearCurrent();
    void swapBuffers(EGLSurface surface);
    void setSwapInterval(int interval);

    EGLSurface createWindowSurface(uintptr_t nativeWindow);
    void destroySurface(EGLSurface surface);

    [[nodiscard]] bool hasExtension(std::string_view extension) const noexcept;
    [[nodiscard]] void* getProcAddress(const char* name) const noexcept;

private:
    std::shared_ptr<GbmDevice> m_device;
    EGLDisplay m_display{EGL_NO_DISPLAY};
    EGLContext m_context{EGL_NO_CONTEXT};
    EGLConfig m_config{nullptr};
    std::unordered_set<std::string> m_extensions;
};

} // namespace nisaba::compositor
