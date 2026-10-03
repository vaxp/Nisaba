/// @file drm_platform.cpp
/// @brief Linux Direct Rendering Manager (DRM/KMS) + GBM + EGL platform backend implementation.

#include "nisaba/backend_os/drm/drm_platform.hpp"
#include "nisaba/backend_os/drm/drm_window.hpp"
#include "nisaba/backend_os/drm/drm_input.hpp"
#include "nisaba/backend_os/platform.hpp"

#include <fcntl.h>
#include <unistd.h>
#include <poll.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <linux/vt.h>
#include <linux/kd.h>
#include <cstring>
#include <iostream>
#include <algorithm>
#include <sstream>

namespace nisaba::backend_os::drm {

namespace {

static const char* const s_arrow_pattern[] = {
    "#...................",
    "##..................",
    "#*#.................",
    "#**#................",
    "#***#...............",
    "#****#..............",
    "#*****#.............",
    "#******#............",
    "#*******#...........",
    "#********#..........",
    "#*********#.........",
    "#******#####........",
    "#***#**#..s.........",
    "#**#.#**#...........",
    "#*#..#**#...........",
    "##....#**#..........",
    "#.....#**#..........",
    ".......#**#.........",
    ".......#**#.........",
    "........##..........",
    nullptr
};

static const char* const s_pointer_pattern[] = {
    "....##..............",
    "...#**#.............",
    "...#**#.............",
    "...#**#.............",
    "...#**#.............",
    "...#**#..##.........",
    "...#**#.#**#..##....",
    "...#**#.#**#.#**#...",
    ".###**#.#**#.#**#...",
    "#**#**#.#**#.#**#...",
    "#**#**#.#**#.#**#...",
    "#**#************#...",
    ".#**************#...",
    "..#*************#...",
    "..#*************#...",
    "...#***********#....",
    "...#***********#....",
    "....#*********#.....",
    "....###########.....",
    nullptr
};

static const char* const s_text_pattern[] = {
    "#####.#####.........",
    "#***#.#***#.........",
    "##*#####*##.........",
    "..#*****#...........",
    "...#***#............",
    "...#***#............",
    "...#***#............",
    "...#***#............",
    "...#***#............",
    "...#***#............",
    "...#***#............",
    "...#***#............",
    "...#***#............",
    "..#*****#...........",
    "##*#####*##.........",
    "#***#.#***#.........",
    "#####.#####.........",
    nullptr
};

static const char* const s_crosshair_pattern[] = {
    ".........#..........",
    ".........#..........",
    ".........#..........",
    ".........#..........",
    ".........#..........",
    ".....####.####......",
    "....#.........#.....",
    "...#...........#....",
    "..#.............#...",
    "#####.........#####.",
    "..#.............#...",
    "...#...........#....",
    "....#.........#.....",
    ".....####.####......",
    ".........#..........",
    ".........#..........",
    ".........#..........",
    ".........#..........",
    ".........#..........",
    nullptr
};

static void fillCursorPixels(uint32_t* dst, const char* const* pattern, int& out_hot_x, int& out_hot_y, int hot_x, int hot_y) {
    std::fill_n(dst, 64 * 64, 0x00000000);
    out_hot_x = hot_x;
    out_hot_y = hot_y;

    if (!pattern) return;

    for (int y = 0; y < 64 && pattern[y] != nullptr; ++y) {
        const char* row = pattern[y];
        for (int x = 0; x < 64 && row[x] != '\0'; ++x) {
            char c = row[x];
            uint32_t color = 0;
            if (c == '#')      color = 0xFF000000;
            else if (c == '*') color = 0xFFFFFFFF;
            else if (c == 's') color = 0x50000000;
            else               color = 0x00000000;

            dst[y * 64 + x] = color;
        }
    }
}

const char* connectorTypeName(uint32_t type) {
    switch (type) {
        case DRM_MODE_CONNECTOR_VGA:         return "VGA";
        case DRM_MODE_CONNECTOR_DVII:        return "DVI-I";
        case DRM_MODE_CONNECTOR_DVID:        return "DVI-D";
        case DRM_MODE_CONNECTOR_DVIA:        return "DVI-A";
        case DRM_MODE_CONNECTOR_Composite:   return "Composite";
        case DRM_MODE_CONNECTOR_SVIDEO:      return "SVIDEO";
        case DRM_MODE_CONNECTOR_LVDS:        return "LVDS";
        case DRM_MODE_CONNECTOR_Component:   return "Component";
        case DRM_MODE_CONNECTOR_9PinDIN:     return "DIN";
        case DRM_MODE_CONNECTOR_DisplayPort: return "DP";
        case DRM_MODE_CONNECTOR_HDMIA:       return "HDMI-A";
        case DRM_MODE_CONNECTOR_HDMIB:       return "HDMI-B";
        case DRM_MODE_CONNECTOR_TV:          return "TV";
        case DRM_MODE_CONNECTOR_eDP:         return "eDP";
        case DRM_MODE_CONNECTOR_VIRTUAL:     return "Virtual";
        case DRM_MODE_CONNECTOR_DSI:         return "DSI";
        case DRM_MODE_CONNECTOR_DPI:         return "DPI";
        case DRM_MODE_CONNECTOR_WRITEBACK:   return "Writeback";
        case DRM_MODE_CONNECTOR_SPI:         return "SPI";
        case DRM_MODE_CONNECTOR_USB:         return "USB";
        default:                             return "Unknown";
    }
}

class DRMOutput : public Output {
public:
    uint32_t    id_          = 0;
    std::string name_        = "DRM-Output";
    std::string make_        = "Embedded";
    std::string model_       = "KMS Display";
    std::string description_ = "Direct Rendering Display";
    Rect        geometry_;
    Rect        logical_geometry_;
    int32_t     physical_width_mm_  = 0;
    int32_t     physical_height_mm_ = 0;
    int32_t     scale_factor_       = 1;
    double      fractional_scale_   = 1.0;
    OutputTransform transform_      = OutputTransform::Normal;
    OutputSubpixel  subpixel_       = OutputSubpixel::HorizontalRgb;
    std::vector<OutputMode> modes_;
    OutputMode  current_mode_;
    bool        is_primary_         = true;

    [[nodiscard]] uint32_t     id()               const noexcept override { return id_; }
    [[nodiscard]] const std::string& name()       const noexcept override { return name_; }
    [[nodiscard]] const std::string& make()       const noexcept override { return make_; }
    [[nodiscard]] const std::string& model()      const noexcept override { return model_; }
    [[nodiscard]] const std::string& description()const noexcept override { return description_; }
    [[nodiscard]] Rect         geometry()         const noexcept override { return geometry_; }
    [[nodiscard]] Rect         logicalGeometry()  const noexcept override { return logical_geometry_; }
    [[nodiscard]] int32_t      physicalWidthMm()  const noexcept override { return physical_width_mm_; }
    [[nodiscard]] int32_t      physicalHeightMm() const noexcept override { return physical_height_mm_; }
    [[nodiscard]] int32_t      scaleFactor()      const noexcept override { return scale_factor_; }
    [[nodiscard]] double       fractionalScale()  const noexcept override { return fractional_scale_; }
    [[nodiscard]] OutputTransform transform()     const noexcept override { return transform_; }
    [[nodiscard]] OutputSubpixel  subpixel()      const noexcept override { return subpixel_; }
    [[nodiscard]] const std::vector<OutputMode>& modes() const noexcept override { return modes_; }
    [[nodiscard]] const OutputMode& currentMode() const noexcept override { return current_mode_; }
    [[nodiscard]] bool         isPrimary()        const noexcept override { return is_primary_; }
    [[nodiscard]] void*        nativeHandle()     const noexcept override { return nullptr; }
};

static void pageFlipHandler(int /*fd*/, unsigned int /*sequence*/, unsigned int tv_sec, unsigned int tv_usec, void* user_data) {
    if (user_data) {
        auto* window = static_cast<DRMWindow*>(user_data);
        window->onPageFlipComplete(tv_sec, tv_usec);
    }
}

} // anonymous namespace

DRMPlatformBackend::DRMPlatformBackend(Platform* owner)
    : owner_(owner) {
}

DRMPlatformBackend::~DRMPlatformBackend() {
    shutdown();
}

bool DRMPlatformBackend::init() {
    std::cout << "[Nisaba DRM Platform] Initializing Linux DRM/KMS + GBM Backend...\n";

    if (!openDrmDevice()) {
        std::cerr << "[Nisaba DRM Platform] Failed to open suitable DRM/KMS device node\n";
        return false;
    }

    if (!initGbm()) {
        std::cerr << "[Nisaba DRM Platform] Failed to initialize GBM device\n";
        shutdown();
        return false;
    }

    if (!initEgl()) {
        std::cerr << "[Nisaba DRM Platform] Failed to initialize EGL with GBM\n";
        shutdown();
        return false;
    }

    setupTty();
    initCursor();

    input_ = std::make_unique<DRMInputBackend>(this);
    if (!input_->init()) {
        std::cerr << "[Nisaba DRM Platform] Warning: Failed to initialize direct input backend\n";
    }

    enumerateOutputs();

    std::cout << "[Nisaba DRM Platform] Initialization successful on " << drm_card_path_ << "\n";
    return true;
}

void DRMPlatformBackend::setupTty() {
    tty_fd_ = ::open("/dev/tty", O_RDWR | O_CLOEXEC);
    if (tty_fd_ < 0) {
        tty_fd_ = ::open("/dev/tty0", O_RDWR | O_CLOEXEC);
    }

    if (tty_fd_ >= 0) {
        if (ioctl(tty_fd_, KDGETMODE, &orig_kd_mode_) == 0) {
            if (ioctl(tty_fd_, KDSETMODE, KD_GRAPHICS) == 0) {
                tty_mode_changed_ = true;
            }
        }

        struct termios tios{};
        if (tcgetattr(tty_fd_, &tios) == 0) {
            orig_termios_ = new termios(tios);
            tios.c_lflag &= ~(ECHO | ECHONL | ICANON);
            tcsetattr(tty_fd_, TCSANOW, &tios);
        }

        [[maybe_unused]] auto w = ::write(tty_fd_, "\033[?25l", 6);
    }
}

void DRMPlatformBackend::restoreTty() {
    if (tty_fd_ >= 0) {
        [[maybe_unused]] auto w = ::write(tty_fd_, "\033[?25h", 6);

        if (orig_termios_) {
            auto* tios = static_cast<termios*>(orig_termios_);
            tcsetattr(tty_fd_, TCSANOW, tios);
            delete tios;
            orig_termios_ = nullptr;
        }

        if (tty_mode_changed_) {
            ioctl(tty_fd_, KDSETMODE, orig_kd_mode_);
            tty_mode_changed_ = false;
        }

        ::close(tty_fd_);
        tty_fd_ = -1;
    }
}

bool DRMPlatformBackend::initCursor() {
    if (!gbm_dev_) return false;

    // Allocate 64x64 cursor scanout BO
    cursor_bo_ = gbm_bo_create(gbm_dev_, 64, 64, GBM_FORMAT_ARGB8888, GBM_BO_USE_CURSOR | GBM_BO_USE_WRITE);
    if (!cursor_bo_) {
        std::cerr << "[Nisaba DRM Platform] Notice: gbm_bo_create failed for hardware cursor plane\n";
        return false;
    }

    cursor_bo_handle_ = gbm_bo_get_handle(cursor_bo_).u32;
    updateCursorImage(SystemCursor::Arrow);

    std::cout << "[Nisaba DRM Platform] Initialized hardware cursor plane (BO handle: " << cursor_bo_handle_ << ")\n";
    return true;
}

void DRMPlatformBackend::updateCursorImage(SystemCursor cursor) {
    if (!cursor_bo_) return;

    uint32_t pixels[64 * 64];
    int hot_x = 0, hot_y = 0;

    switch (cursor) {
        case SystemCursor::Pointer:
            fillCursorPixels(pixels, s_pointer_pattern, hot_x, hot_y, 4, 0);
            break;
        case SystemCursor::Text:
            fillCursorPixels(pixels, s_text_pattern, hot_x, hot_y, 5, 8);
            break;
        case SystemCursor::Crosshair:
        case SystemCursor::Move:
            fillCursorPixels(pixels, s_crosshair_pattern, hot_x, hot_y, 9, 9);
            break;
        case SystemCursor::Arrow:
        case SystemCursor::Default:
        default:
            fillCursorPixels(pixels, s_arrow_pattern, hot_x, hot_y, 0, 0);
            break;
    }

    current_hot_x_ = hot_x;
    current_hot_y_ = hot_y;

    if (gbm_bo_write(cursor_bo_, pixels, sizeof(pixels)) != 0) {
        std::cerr << "[Nisaba DRM Platform] gbm_bo_write failed for cursor image\n";
        return;
    }

    if (active_crtc_id_ && drm_fd_ >= 0) {
        if (drmModeSetCursor2(drm_fd_, active_crtc_id_, cursor_bo_handle_, 64, 64, hot_x, hot_y) != 0) {
            drmModeSetCursor(drm_fd_, active_crtc_id_, cursor_bo_handle_, 64, 64);
        }
    }
}

void DRMPlatformBackend::setActiveCrtcId(uint32_t crtc_id) {
    if (active_crtc_id_ == crtc_id && cursor_visible_) return;
    active_crtc_id_ = crtc_id;

    if (cursor_bo_handle_ && active_crtc_id_ && drm_fd_ >= 0) {
        if (drmModeSetCursor2(drm_fd_, active_crtc_id_, cursor_bo_handle_, 64, 64, current_hot_x_, current_hot_y_) != 0) {
            drmModeSetCursor(drm_fd_, active_crtc_id_, cursor_bo_handle_, 64, 64);
        }
        drmModeMoveCursor(drm_fd_, active_crtc_id_, cursor_x_, cursor_y_);
        cursor_visible_ = true;
    }
}

void DRMPlatformBackend::setCursor(SystemCursor cursor) {
    if (current_cursor_ == cursor) return;
    current_cursor_ = cursor;
    updateCursorImage(cursor);
}

void DRMPlatformBackend::moveCursor(int x, int y) {
    cursor_x_ = x;
    cursor_y_ = y;

    if (active_crtc_id_ && drm_fd_ >= 0) {
        if (!cursor_visible_ && cursor_bo_handle_) {
            if (drmModeSetCursor2(drm_fd_, active_crtc_id_, cursor_bo_handle_, 64, 64, current_hot_x_, current_hot_y_) != 0) {
                drmModeSetCursor(drm_fd_, active_crtc_id_, cursor_bo_handle_, 64, 64);
            }
            cursor_visible_ = true;
        }
        drmModeMoveCursor(drm_fd_, active_crtc_id_, x, y);
    }
}

void DRMPlatformBackend::cleanupCursor() {
    if (active_crtc_id_ && drm_fd_ >= 0) {
        drmModeSetCursor(drm_fd_, active_crtc_id_, 0, 0, 0);
    }
    cursor_visible_ = false;
    active_crtc_id_ = 0;

    if (cursor_bo_) {
        gbm_bo_destroy(cursor_bo_);
        cursor_bo_ = nullptr;
        cursor_bo_handle_ = 0;
    }
}

void DRMPlatformBackend::shutdown() {
    cleanupCursor();
    restoreTty();

    if (input_) {
        input_->shutdown();
        input_.reset();
    }

    if (egl_display_ != EGL_NO_DISPLAY) {
        eglMakeCurrent(egl_display_, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (egl_context_ != EGL_NO_CONTEXT) {
            eglDestroyContext(egl_display_, egl_context_);
            egl_context_ = EGL_NO_CONTEXT;
        }
        eglTerminate(egl_display_);
        egl_display_ = EGL_NO_DISPLAY;
    }

    if (gbm_dev_) {
        gbm_device_destroy(gbm_dev_);
        gbm_dev_ = nullptr;
    }

    if (drm_fd_ >= 0) {
        if (is_master_) {
            drmDropMaster(drm_fd_);
            is_master_ = false;
        }
        ::close(drm_fd_);
        drm_fd_ = -1;
    }

    outputs_.clear();
    primary_output_.reset();
}

bool DRMPlatformBackend::openDrmDevice() {
    // 1. Check for explicit card specified by environment variable
    const char* explicit_card = std::getenv("Nisaba_DRM_CARD");
    if (!explicit_card) explicit_card = std::getenv("DRM_CARD");

    if (explicit_card && explicit_card[0] != '\0') {
        int fd = ::open(explicit_card, O_RDWR | O_CLOEXEC);
        if (fd >= 0) {
            drmModeRes* res = drmModeGetResources(fd);
            if (res && res->count_connectors > 0) {
                drmModeFreeResources(res);
                drm_fd_ = fd;
                drm_card_path_ = explicit_card;

                drmVersionPtr ver = drmGetVersion(drm_fd_);
                std::string driver_name = (ver && ver->name) ? ver->name : "unknown";
                if (ver) drmFreeVersion(ver);

                std::cout << "[Nisaba DRM Platform] Using explicitly specified DRM device: "
                          << drm_card_path_ << " (driver: " << driver_name << ")\n";

                if (drmSetMaster(drm_fd_) == 0) {
                    is_master_ = true;
                } else {
                    int err = errno;
                    std::cerr << "[Nisaba DRM Platform] Notice: drmSetMaster() returned errno " << err
                              << " (" << std::strerror(err) << ").\n";
                    if (err == EACCES || err == EPERM) {
                        std::cerr << "[Nisaba DRM Platform] NOTE: Direct DRM modesetting requires root or CAP_SYS_ADMIN privileges.\n"
                                  << "       If modeset fails, run with 'sudo' or use 'sudo setcap cap_sys_admin+ep <binary>'.\n";
                    }
                }
                drmSetClientCap(drm_fd_, DRM_CLIENT_CAP_UNIVERSAL_PLANES, 1);
                return true;
            }
            if (res) drmModeFreeResources(res);
            ::close(fd);
        }
        std::cerr << "[Nisaba DRM Platform] Specified DRM device " << explicit_card << " is not usable\n";
    }

    // 2. Scan card0 through card8
    const char* prefer_vkms_env = std::getenv("Nisaba_DRM_PREFER_VKMS");
    bool prefer_vkms = prefer_vkms_env && (std::string_view(prefer_vkms_env) == "1" || std::string_view(prefer_vkms_env) == "true");

    int candidate_fd = -1;
    std::string candidate_path;

    int vkms_fd = -1;
    std::string vkms_path;

    for (int i = 0; i <= 8; ++i) {
        std::string path = "/dev/dri/card" + std::to_string(i);
        int fd = ::open(path.c_str(), O_RDWR | O_CLOEXEC);
        if (fd < 0) {
            continue;
        }

        // Verify KMS support
        drmModeRes* res = drmModeGetResources(fd);
        if (!res) {
            ::close(fd);
            continue;
        }

        bool has_connectors = (res->count_connectors > 0);
        if (!has_connectors) {
            drmModeFreeResources(res);
            ::close(fd);
            continue;
        }

        // Check driver name
        drmVersionPtr ver = drmGetVersion(fd);
        std::string driver_name = (ver && ver->name) ? ver->name : "";
        if (ver) drmFreeVersion(ver);

        if (driver_name == "vkms") {
            if (prefer_vkms) {
                std::cout << "[Nisaba DRM Platform] Using requested VKMS virtual device at " << path << "\n";
                drmModeFreeResources(res);
                if (candidate_fd >= 0) ::close(candidate_fd);
                candidate_fd = fd;
                candidate_path = path;
                break;
            } else {
                // Keep VKMS as fallback only if no physical device is found
                if (vkms_fd < 0) {
                    vkms_fd = fd;
                    vkms_path = path;
                } else {
                    ::close(fd);
                }
                drmModeFreeResources(res);
                continue;
            }
        }

        // Check if this card has any actively connected physical monitors
        bool has_connected_connector = false;
        for (int c = 0; c < res->count_connectors; ++c) {
            drmModeConnector* conn = drmModeGetConnector(fd, res->connectors[c]);
            if (conn) {
                if (conn->connection == DRM_MODE_CONNECTED && conn->count_modes > 0) {
                    has_connected_connector = true;
                    drmModeFreeConnector(conn);
                    break;
                }
                drmModeFreeConnector(conn);
            }
        }
        drmModeFreeResources(res);

        // Found physical card with an active monitor connected
        if (has_connected_connector) {
            if (candidate_fd >= 0) ::close(candidate_fd);
            candidate_fd = fd;
            candidate_path = path;
            std::cout << "[Nisaba DRM Platform] Found active display on " << path << " (" << driver_name << ")\n";
            break;
        }

        // Fallback candidate if no connected display is found yet
        if (candidate_fd < 0) {
            candidate_fd = fd;
            candidate_path = path;
        } else {
            ::close(fd);
        }
    }

    if (candidate_fd < 0 && vkms_fd >= 0) {
        candidate_fd = vkms_fd;
        candidate_path = vkms_path;
        vkms_fd = -1;
    } else if (vkms_fd >= 0) {
        ::close(vkms_fd);
    }

    if (candidate_fd >= 0) {
        drm_fd_ = candidate_fd;
        drm_card_path_ = candidate_path;

        if (drmSetMaster(drm_fd_) == 0) {
            is_master_ = true;
        } else {
            int err = errno;
            std::cerr << "[Nisaba DRM Platform] Notice: drmSetMaster() returned errno " << err
                      << " (" << std::strerror(err) << ").\n";
            if (err == EACCES || err == EPERM) {
                std::cerr << "[Nisaba DRM Platform] CRITICAL: Direct DRM modesetting requires root or CAP_SYS_ADMIN privileges.\n"
                          << "       Please run with 'sudo' or assign capabilities to the executable:\n"
                          << "       sudo setcap cap_sys_admin+ep <path-to-executable>\n";
            }
        }

        drmSetClientCap(drm_fd_, DRM_CLIENT_CAP_UNIVERSAL_PLANES, 1);
        drmSetClientCap(drm_fd_, DRM_CLIENT_CAP_STEREO_3D, 0);

        std::cout << "[Nisaba DRM Platform] Using DRM node: " << drm_card_path_ << "\n";
        return true;
    }

    return false;
}


bool DRMPlatformBackend::initGbm() {
    gbm_dev_ = gbm_create_device(drm_fd_);
    if (!gbm_dev_) {
        std::cerr << "[Nisaba DRM Platform] gbm_create_device failed\n";
        return false;
    }
    return true;
}

bool DRMPlatformBackend::initEgl() {
    typedef EGLDisplay (*PFNEGLGETPLATFORMDISPLAYEXTPROC)(EGLenum platform, void *native_display, const EGLint *attrib_list);
    PFNEGLGETPLATFORMDISPLAYEXTPROC eglGetPlatformDisplayEXT =
        (PFNEGLGETPLATFORMDISPLAYEXTPROC)eglGetProcAddress("eglGetPlatformDisplayEXT");

    if (eglGetPlatformDisplayEXT) {
        egl_display_ = eglGetPlatformDisplayEXT(EGL_PLATFORM_GBM_KHR, gbm_dev_, nullptr);
    }
    if (egl_display_ == EGL_NO_DISPLAY) {
        egl_display_ = eglGetDisplay(reinterpret_cast<EGLNativeDisplayType>(gbm_dev_));
    }
    if (egl_display_ == EGL_NO_DISPLAY) {
        std::cerr << "[Nisaba DRM Platform] Failed to get EGLDisplay for GBM device\n";
        return false;
    }

    EGLint major = 0, minor = 0;
    if (!eglInitialize(egl_display_, &major, &minor)) {
        std::cerr << "[Nisaba DRM Platform] eglInitialize failed\n";
        return false;
    }
    std::cout << "[Nisaba DRM Platform] EGL initialized: version " << major << "." << minor << "\n";

    // Bind OpenGL ES or Desktop GL API
    if (!eglBindAPI(EGL_OPENGL_ES_API)) {
        eglBindAPI(EGL_OPENGL_API);
    }

    const EGLint config_attribs[] = {
        EGL_SURFACE_TYPE,    EGL_WINDOW_BIT,
        EGL_RED_SIZE,        8,
        EGL_GREEN_SIZE,      8,
        EGL_BLUE_SIZE,       8,
        EGL_ALPHA_SIZE,      8,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_NONE
    };

    EGLint num_configs = 0;
    if (!eglChooseConfig(egl_display_, config_attribs, &egl_config_, 1, &num_configs) || num_configs < 1) {
        // Fallback without alpha requirement
        const EGLint fallback_attribs[] = {
            EGL_SURFACE_TYPE,    EGL_WINDOW_BIT,
            EGL_RED_SIZE,        8,
            EGL_GREEN_SIZE,      8,
            EGL_BLUE_SIZE,       8,
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
            EGL_NONE
        };
        if (!eglChooseConfig(egl_display_, fallback_attribs, &egl_config_, 1, &num_configs) || num_configs < 1) {
            std::cerr << "[Nisaba DRM Platform] Failed to choose suitable EGLConfig\n";
            return false;
        }
    }

    const EGLint context_attribs[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };
    egl_context_ = eglCreateContext(egl_display_, egl_config_, EGL_NO_CONTEXT, context_attribs);
    if (egl_context_ == EGL_NO_CONTEXT) {
        std::cerr << "[Nisaba DRM Platform] eglCreateContext failed\n";
        return false;
    }

    return true;
}

void DRMPlatformBackend::enumerateOutputs() {
    outputs_.clear();
    primary_output_.reset();

    if (drm_fd_ < 0) return;

    drmModeRes* res = drmModeGetResources(drm_fd_);
    if (!res) return;

    for (int i = 0; i < res->count_connectors; ++i) {
        drmModeConnector* conn = drmModeGetConnector(drm_fd_, res->connectors[i]);
        if (!conn) continue;

        if (conn->connection == DRM_MODE_CONNECTED && conn->count_modes > 0) {
            auto out = std::make_shared<DRMOutput>();
            out->id_ = conn->connector_id;

            std::ostringstream ss;
            ss << connectorTypeName(conn->connector_type) << "-" << conn->connector_type_id;
            out->name_ = ss.str();
            out->description_ = out->name_ + " (DRM Display)";

            out->physical_width_mm_  = conn->mmWidth;
            out->physical_height_mm_ = conn->mmHeight;

            // Enumerate modes
            for (int m = 0; m < conn->count_modes; ++m) {
                const auto& drm_mode = conn->modes[m];
                OutputMode mode;
                mode.width            = drm_mode.hdisplay;
                mode.height           = drm_mode.vdisplay;
                mode.refresh_rate_mHz = drm_mode.vrefresh * 1000;
                mode.is_preferred     = (drm_mode.type & DRM_MODE_TYPE_PREFERRED) != 0;
                mode.is_current       = (m == 0);

                out->modes_.push_back(mode);
            }

            if (!out->modes_.empty()) {
                out->current_mode_ = out->modes_.front();
                out->geometry_ = Rect(0, 0, static_cast<float>(out->current_mode_.width), static_cast<float>(out->current_mode_.height));
                out->logical_geometry_ = out->geometry_;
            }

            out->is_primary_ = outputs_.empty();
            if (out->is_primary_) {
                primary_output_ = out;
            }

            outputs_.push_back(out);

            if (owner_) {
                owner_->onOutputAdded().emit(out);
            }
        }
        drmModeFreeConnector(conn);
    }

    drmModeFreeResources(res);
}

void DRMPlatformBackend::refreshOutputs() {
    enumerateOutputs();
}

std::vector<std::shared_ptr<Output>> DRMPlatformBackend::getOutputs() const {
    return outputs_;
}

std::shared_ptr<Output> DRMPlatformBackend::getPrimaryOutput() const {
    return primary_output_;
}

std::shared_ptr<Output> DRMPlatformBackend::getOutputByName(std::string_view name) const {
    for (auto& o : outputs_) {
        if (o && o->name() == name) return o;
    }
    return nullptr;
}

bool DRMPlatformBackend::findDisplayTarget(uint32_t& out_connector_id,
                                          uint32_t& out_crtc_id,
                                          drmModeModeInfo& out_mode,
                                          int target_w, int target_h) {
    if (drm_fd_ < 0) return false;

    drmModeRes* res = drmModeGetResources(drm_fd_);
    if (!res) return false;

    bool found = false;
    for (int i = 0; i < res->count_connectors && !found; ++i) {
        drmModeConnector* conn = drmModeGetConnector(drm_fd_, res->connectors[i]);
        if (!conn) continue;

        if (conn->connection == DRM_MODE_CONNECTED && conn->count_modes > 0) {
            out_connector_id = conn->connector_id;

            // Pick preferred mode or matching resolution
            int chosen_idx = 0;
            for (int m = 0; m < conn->count_modes; ++m) {
                if (target_w > 0 && target_h > 0 &&
                    conn->modes[m].hdisplay == target_w &&
                    conn->modes[m].vdisplay == target_h) {
                    chosen_idx = m;
                    break;
                }
                if (conn->modes[m].type & DRM_MODE_TYPE_PREFERRED) {
                    chosen_idx = m;
                }
            }
            out_mode = conn->modes[chosen_idx];

            // Resolve CRTC: check existing encoder or find compatible encoder
            drmModeEncoder* enc = nullptr;
            if (conn->encoder_id) {
                enc = drmModeGetEncoder(drm_fd_, conn->encoder_id);
            }
            if (!enc && conn->count_encoders > 0) {
                enc = drmModeGetEncoder(drm_fd_, conn->encoders[0]);
            }

            if (enc) {
                if (enc->crtc_id) {
                    out_crtc_id = enc->crtc_id;
                } else {
                    // Pick first compatible CRTC
                    for (int c = 0; c < res->count_crtcs; ++c) {
                        if (enc->possible_crtcs & (1 << c)) {
                            out_crtc_id = res->crtcs[c];
                            break;
                        }
                    }
                }
                drmModeFreeEncoder(enc);
            }

            if (!out_crtc_id && res->count_crtcs > 0) {
                out_crtc_id = res->crtcs[0];
            }

            found = (out_crtc_id != 0);
        }
        drmModeFreeConnector(conn);
    }

    drmModeFreeResources(res);
    return found;
}

bool DRMPlatformBackend::dropMaster() {
    if (drm_fd_ >= 0 && is_master_) {
        if (drmDropMaster(drm_fd_) == 0) {
            is_master_ = false;
            return true;
        }
    }
    return false;
}

bool DRMPlatformBackend::acquireMaster() {
    if (drm_fd_ >= 0 && !is_master_) {
        if (drmSetMaster(drm_fd_) == 0) {
            is_master_ = true;
            return true;
        }
    }
    return false;
}

void DRMPlatformBackend::registerWindow(Window* w) {
    if (w) windows_.insert(w);
}

void DRMPlatformBackend::unregisterWindow(Window* w) {
    if (w) windows_.erase(w);
}

bool DRMPlatformBackend::pollEvents() {
    if (quit_requested_) return false;

    // Process libinput events
    if (input_) {
        input_->processEvents();
    }

    // Process DRM page flip events
    if (drm_fd_ >= 0) {
        struct pollfd pfd{};
        pfd.fd     = drm_fd_;
        pfd.events = POLLIN;

        int ret = poll(&pfd, 1, 0); // Non-blocking check
        if (ret > 0 && (pfd.revents & POLLIN)) {
            drmEventContext evctx{};
            evctx.version = DRM_EVENT_CONTEXT_VERSION;
            evctx.page_flip_handler = pageFlipHandler;
            drmHandleEvent(drm_fd_, &evctx);
        }
    }

    return true;
}

} // namespace nisaba::backend_os::drm
