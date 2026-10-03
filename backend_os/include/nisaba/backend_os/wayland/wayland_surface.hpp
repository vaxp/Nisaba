#pragma once
/// @file wayland_surface.hpp
/// @brief Wayland Layer Surface implementation with zwlr_layer_shell_v1 and EGL.

#include "nisaba/backend_os/layer_surface.hpp"
#include "nisaba/backend_os/wayland/wayland_platform.hpp"

#include <wayland-client.h>
#include <wayland-egl.h>
#include <EGL/egl.h>

namespace nisaba::backend_os::wayland {

class WaylandLayerSurface : public LayerSurface {
public:
    WaylandLayerSurface(WaylandPlatformBackend& backend, LayerSurfaceConfig config);
    ~WaylandLayerSurface() override;

    bool init();

    void setSize(int32_t width, int32_t height) override;
    void setLayer(ShellLayer layer) override;
    void setAnchor(ShellAnchor anchor) override;
    void setExclusiveZone(int32_t zone) override;
    void setMargin(const SurfaceMargin& margin) override;
    void setKeyboardMode(KeyboardMode mode) override;

    [[nodiscard]] Size getSize() const override;
    [[nodiscard]] Size getDrawableSize() const override;
    [[nodiscard]] float getDpiScale() const override;

    void makeCurrent() override;
    void swapBuffers() override;
    void* getNativeHandle() const override { return (void*)wl_surface_; }
    void* getEGLSurface() const override { return (void*)egl_surface_; }
    void* getEGLContext() const override { return (void*)egl_context_; }
    void* getBackendLayer() const override { return (void*)this; }

    [[nodiscard]] wl_surface* getWlSurface() const { return wl_surface_; }
    [[nodiscard]] zwlr_layer_surface_v1* getLayerSurface() const { return layer_surface_; }

    // Layer-shell protocol callbacks
    void handleConfigure(uint32_t serial, uint32_t width, uint32_t height);
    void handleClosed();

private:
    WaylandPlatformBackend& backend_;
    LayerSurfaceConfig      config_;

    wl_surface*             wl_surface_    = nullptr;
    zwlr_layer_surface_v1*  layer_surface_ = nullptr;
    wl_egl_window*          egl_window_    = nullptr;

    EGLDisplay              egl_display_   = EGL_NO_DISPLAY;
    EGLSurface              egl_surface_   = EGL_NO_SURFACE;
    EGLContext              egl_context_   = EGL_NO_CONTEXT;

    int32_t current_width_  = 0;
    int32_t current_height_ = 0;
    bool    configured_     = false;
    float   scale_factor_   = 1.0f;
};

} // namespace nisaba::backend_os::wayland
