/// @file drm_window.cpp
/// @brief Linux DRM/KMS + GBM + EGL window/surface implementation.

#include "nisaba/backend_os/drm/drm_window.hpp"
#include "nisaba/backend_os/drm/drm_input.hpp"

#include <poll.h>
#include <iostream>
#include <cstring>
#include <algorithm>

namespace nisaba::backend_os::drm {

DRMWindow::DRMWindow(DRMPlatformBackend& backend)
    : backend_(backend) {
}

DRMWindow::~DRMWindow() {
    destroy();
}

bool DRMWindow::init(const WindowConfig& config) {
    config_ = config;

    if (!backend_.findDisplayTarget(connector_id_, crtc_id_, mode_, config.width, config.height)) {
        std::cerr << "[Nisaba DRM Window] Failed to find active connected display connector or CRTC\n";
        return false;
    }

    width_  = mode_.hdisplay;
    height_ = mode_.vdisplay;

    std::cout << "[Nisaba DRM Window] Selected display mode: " << width_ << "x" << height_
              << "@" << mode_.vrefresh << "Hz on connector ID " << connector_id_
              << ", CRTC ID " << crtc_id_ << "\n";

    // Save current CRTC state to restore on exit
    saved_crtc_ = drmModeGetCrtc(backend_.getDrmFd(), crtc_id_);

    // Check hardware capability for asynchronous (uncapped) page flips
    uint64_t cap_val = 0;
    if (drmGetCap(backend_.getDrmFd(), DRM_CAP_ASYNC_PAGE_FLIP, &cap_val) == 0 && cap_val == 1) {
        async_flip_supported_ = true;
    }

    // Create scanout GBM surface
    gbm_surface_ = gbm_surface_create(
        backend_.getGbmDevice(),
        width_,
        height_,
        GBM_FORMAT_XRGB8888,
        GBM_BO_USE_SCANOUT | GBM_BO_USE_RENDERING
    );

    if (!gbm_surface_) {
        std::cerr << "[Nisaba DRM Window] gbm_surface_create failed for " << width_ << "x" << height_ << "\n";
        return false;
    }

    // Create EGL window surface backed by GBM surface
    egl_surface_ = eglCreateWindowSurface(
        backend_.getEGLDisplay(),
        backend_.getEGLConfig(),
        reinterpret_cast<EGLNativeWindowType>(gbm_surface_),
        nullptr
    );

    if (egl_surface_ == EGL_NO_SURFACE) {
        std::cerr << "[Nisaba DRM Window] eglCreateWindowSurface failed (EGL error 0x"
                  << std::hex << eglGetError() << std::dec << ")\n";
        destroy();
        return false;
    }

    // Configure EGL swap interval based on requested VSync mode
    eglSwapInterval(backend_.getEGLDisplay(), config_.vsync ? 1 : 0);

    // Update screen size on input backend
    if (backend_.getInput()) {
        backend_.getInput()->updateScreenSize(width_, height_);
    }

    backend_.setActiveWindow(this);

    // Initial state notifications
    on_resize_.emit(width_, height_);
    on_focus_.emit(true);
    on_state_changed_.emit(getWindowState());

    if (!backend_.isMaster()) {
        std::cerr << "[Nisaba DRM Window] WARNING: Current process does NOT have DRM Master privileges!\n"
                  << "                 Calling drmModeSetCrtc will fail (errno 13: Permission denied).\n"
                  << "                 To display on physical screen, run with 'sudo' or 'sudo setcap cap_sys_admin+ep <exe>'.\n";
    }

    std::cout << "[Nisaba DRM Window] Initialized fullscreen KMS scanout surface ("
              << (config_.vsync ? "VSync Locked @" + std::to_string(mode_.vrefresh) + "Hz" : "VSync OFF - Uncapped")
              << ", Async Flip: " << (async_flip_supported_ ? "Supported" : "Unsupported") << ")\n";
    return true;
}

void DRMWindow::destroy() {
    backend_.setActiveWindow(nullptr);
    backend_.setActiveCrtcId(0);

    // Restore saved CRTC configuration if available
    if (saved_crtc_ && backend_.getDrmFd() >= 0) {
        drmModeSetCrtc(
            backend_.getDrmFd(),
            saved_crtc_->crtc_id,
            saved_crtc_->buffer_id,
            saved_crtc_->x,
            saved_crtc_->y,
            &connector_id_,
            1,
            &saved_crtc_->mode
        );
        drmModeFreeCrtc(saved_crtc_);
        saved_crtc_ = nullptr;
    }

    // Remove DRM framebuffers
    if (backend_.getDrmFd() >= 0) {
        for (const auto& [bo, fb_id] : bo_to_fb_map_) {
            drmModeRmFB(backend_.getDrmFd(), fb_id);
        }
    }
    bo_to_fb_map_.clear();

    if (previous_bo_ && gbm_surface_) {
        gbm_surface_release_buffer(gbm_surface_, previous_bo_);
        previous_bo_ = nullptr;
    }
    if (current_bo_ && gbm_surface_) {
        gbm_surface_release_buffer(gbm_surface_, current_bo_);
        current_bo_ = nullptr;
    }

    if (egl_surface_ != EGL_NO_SURFACE) {
        eglDestroySurface(backend_.getEGLDisplay(), egl_surface_);
        egl_surface_ = EGL_NO_SURFACE;
    }

    if (gbm_surface_) {
        gbm_surface_destroy(gbm_surface_);
        gbm_surface_ = nullptr;
    }
}

void DRMWindow::makeCurrent() {
    if (egl_surface_ != EGL_NO_SURFACE) {
        eglMakeCurrent(backend_.getEGLDisplay(), egl_surface_, egl_surface_, backend_.getEGLContext());
    }
}

uint32_t DRMWindow::getOrCreateFbForBo(struct gbm_bo* bo) {
    auto it = bo_to_fb_map_.find(bo);
    if (it != bo_to_fb_map_.end()) {
        return it->second;
    }

    uint32_t bo_w      = gbm_bo_get_width(bo);
    uint32_t bo_h      = gbm_bo_get_height(bo);
    uint32_t stride    = gbm_bo_get_stride(bo);
    uint32_t handle    = gbm_bo_get_handle(bo).u32;
    uint32_t fb_id     = 0;

    int ret = drmModeAddFB(backend_.getDrmFd(), bo_w, bo_h, 24, 32, stride, handle, &fb_id);
    if (ret != 0) {
        std::cerr << "[Nisaba DRM Window] drmModeAddFB failed: errno " << errno << "\n";
        return 0;
    }

    bo_to_fb_map_[bo] = fb_id;
    return fb_id;
}

void DRMWindow::onPageFlipComplete(unsigned int /*sec*/, unsigned int /*usec*/) {
    waiting_for_flip_ = false;
}

void DRMWindow::swapBuffers() {
    if (egl_surface_ == EGL_NO_SURFACE || !gbm_surface_) {
        return;
    }

    // Drain completed page flip events before next presentation
    if (waiting_for_flip_) {
        struct pollfd pfd{};
        pfd.fd     = backend_.getDrmFd();
        pfd.events = POLLIN;

        int timeout_ms = config_.vsync ? 100 : 0; // Wait on vsync, non-blocking check on uncapped
        while (waiting_for_flip_ && poll(&pfd, 1, timeout_ms) > 0 && (pfd.revents & POLLIN)) {
            drmEventContext evctx{};
            evctx.version = DRM_EVENT_CONTEXT_VERSION;
            evctx.page_flip_handler = [](int, unsigned int, unsigned int sec, unsigned int usec, void* user_data) {
                if (user_data) {
                    static_cast<DRMWindow*>(user_data)->onPageFlipComplete(sec, usec);
                }
            };
            drmHandleEvent(backend_.getDrmFd(), &evctx);
            if (!config_.vsync) break;
        }
    }

    // Flush GPU rendering to GBM buffer
    eglSwapBuffers(backend_.getEGLDisplay(), egl_surface_);

    struct gbm_bo* bo = gbm_surface_lock_front_buffer(gbm_surface_);
    if (!bo) {
        return;
    }

    uint32_t fb_id = getOrCreateFbForBo(bo);
    if (!fb_id) {
        gbm_surface_release_buffer(gbm_surface_, bo);
        return;
    }

    if (!crtc_mode_set_) {
        // Initial modeset
        int ret = drmModeSetCrtc(
            backend_.getDrmFd(),
            crtc_id_,
            fb_id,
            0, 0,
            &connector_id_,
            1,
            &mode_
        );
        if (ret == 0) {
            crtc_mode_set_ = true;
            backend_.setActiveCrtcId(crtc_id_);
            std::cout << "[Nisaba DRM Window] KMS modeset successful: CRTC " << crtc_id_
                      << " scanning out FB " << fb_id << " (" << width_ << "x" << height_ << "@" << mode_.vrefresh << "Hz)\n";
        } else {
            int err = errno;
            std::cerr << "[Nisaba DRM Window] Initial drmModeSetCrtc failed (errno " << err
                      << ": " << std::strerror(err) << ")\n";
            if (err == EACCES || err == EPERM) {
                std::cerr << "[Nisaba DRM Window] CRITICAL: Permission denied when setting CRTC display mode.\n"
                          << "                 Direct DRM modesetting requires root privileges.\n"
                          << "                 Please run with 'sudo' or assign 'sudo setcap cap_sys_admin+ep <exe>'.\n";
            }
        }
        current_bo_ = bo;
    } else {
        uint32_t flip_flags = DRM_MODE_PAGE_FLIP_EVENT;
        if (!config_.vsync && async_flip_supported_) {
            flip_flags |= DRM_MODE_PAGE_FLIP_ASYNC;
        }

        waiting_for_flip_ = true;
        int ret = drmModePageFlip(
            backend_.getDrmFd(),
            crtc_id_,
            fb_id,
            flip_flags,
            this
        );

        if (ret == 0) {
            if (config_.vsync) {
                // Synchronous wait for VBlank interval only when VSync is enabled
                while (waiting_for_flip_) {
                    struct pollfd pfd{};
                    pfd.fd     = backend_.getDrmFd();
                    pfd.events = POLLIN;

                    int r = poll(&pfd, 1, 100);
                    if (r <= 0) {
                        waiting_for_flip_ = false;
                        break;
                    }

                    drmEventContext evctx{};
                    evctx.version = DRM_EVENT_CONTEXT_VERSION;
                    evctx.page_flip_handler = [](int, unsigned int, unsigned int sec, unsigned int usec, void* user_data) {
                        if (user_data) {
                            static_cast<DRMWindow*>(user_data)->onPageFlipComplete(sec, usec);
                        }
                    };
                    drmHandleEvent(backend_.getDrmFd(), &evctx);
                }
            }
        } else if (ret == -EBUSY) {
            // Display controller is currently busy presenting previous flip.
            // Poll for up to 1ms to complete previous flip then retry.
            struct pollfd pfd{};
            pfd.fd     = backend_.getDrmFd();
            pfd.events = POLLIN;
            if (poll(&pfd, 1, 1) > 0 && (pfd.revents & POLLIN)) {
                drmEventContext evctx{};
                evctx.version = DRM_EVENT_CONTEXT_VERSION;
                evctx.page_flip_handler = [](int, unsigned int, unsigned int sec, unsigned int usec, void* user_data) {
                    if (user_data) {
                        static_cast<DRMWindow*>(user_data)->onPageFlipComplete(sec, usec);
                    }
                };
                drmHandleEvent(backend_.getDrmFd(), &evctx);
            }
            ret = drmModePageFlip(backend_.getDrmFd(), crtc_id_, fb_id, flip_flags, this);
            if (ret != 0) {
                waiting_for_flip_ = false;
            }
        } else {
            waiting_for_flip_ = false;
            std::cerr << "[Nisaba DRM Window] drmModePageFlip failed: errno " << errno << "\n";
        }

        // Release previous front buffer back to GBM
        if (previous_bo_) {
            gbm_surface_release_buffer(gbm_surface_, previous_bo_);
        }
        previous_bo_ = current_bo_;
        current_bo_  = bo;
    }
}

// ── Window Properties ────────────────────────────────────────────

void DRMWindow::setTitle(std::string_view /*title*/) {}
void DRMWindow::setSize(int /*width*/, int /*height*/) {}
void DRMWindow::setPosition(int /*x*/, int /*y*/) {}
void DRMWindow::setBorderless(bool /*borderless*/) {}
void DRMWindow::setAlwaysOnTop(bool /*on_top*/) {}
void DRMWindow::setBlurBehind(bool /*enable*/) {}

Size DRMWindow::getSize() const {
    return Size(static_cast<float>(width_), static_cast<float>(height_));
}

Size DRMWindow::getDrawableSize() const {
    return Size(static_cast<float>(width_), static_cast<float>(height_));
}

float DRMWindow::getDpiScale() const {
    return dpi_scale_;
}

void DRMWindow::beginMove(float /*local_x*/, float /*local_y*/, int /*button*/) {}
void DRMWindow::beginResize(WindowEdge /*edge*/, float /*local_x*/, float /*local_y*/, int /*button*/) {}
void DRMWindow::setMaximized(bool /*max*/) {}
void DRMWindow::setMinimized(bool /*min*/) {}
void DRMWindow::setFullscreen(bool /*full*/) {}
void DRMWindow::toggleMaximize() {}
void DRMWindow::showWindowMenu(float /*local_x*/, float /*local_y*/, int /*button*/) {}
void DRMWindow::setDecorated(bool /*decorated*/) {}
void DRMWindow::setWindowGeometry(int /*x*/, int /*y*/, int /*width*/, int /*height*/) {}

} // namespace nisaba::backend_os::drm
