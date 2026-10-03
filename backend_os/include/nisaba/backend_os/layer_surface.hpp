#pragma once
/// @file layer_surface.hpp
/// @brief Wayland wlr-layer-shell surface specification & interface.
/// Provides layer levels (Background, Bottom, Top, Overlay), screen anchors, exclusive zone margins.

// Deliberately included before X11 headers to avoid macro pollution (None, Top, Bottom etc.)
#include "nisaba/backend_os/types.hpp"
#include "nisaba/backend_os/result.hpp"
#include "nisaba/backend_os/signal.hpp"
#include "nisaba/backend_os/platform.hpp"
#include "nisaba/backend_os/output.hpp"
#include <string>
#include <memory>
#include <cstdint>

// Undefine X11 macros that pollute global namespace and clash with our enums
#ifdef None
#  undef None
#endif
#ifdef Above
#  undef Above
#endif
#ifdef Below
#  undef Below
#endif

namespace nisaba::backend_os {

/// Desktop Layer levels corresponding to zwlr_layer_shell_v1_layer
enum class ShellLayer : uint32_t {
    Background = 0, ///< Behind normal windows (e.g. dynamic wallpaper)
    Bottom     = 1, ///< Below normal windows (e.g. desktop widgets)
    Top        = 2, ///< Above normal windows (e.g. panels, taskbars, docks)
    Overlay    = 3, ///< Above all windows & lockscreen (e.g. launchers, OSDs, notifications)
};

/// Anchor bitmask corresponding to zwlr_layer_surface_v1_anchor
enum class ShellAnchor : uint32_t {
    None   = 0,
    Top    = 1 << 0,
    Bottom = 1 << 1,
    Left   = 1 << 2,
    Right  = 1 << 3,
    TopLeft     = Top | Left,
    TopRight    = Top | Right,
    BottomLeft  = Bottom | Left,
    BottomRight = Bottom | Right,
    TopAll      = Top | Left | Right,
    BottomAll   = Bottom | Left | Right,
    LeftAll     = Left | Top | Bottom,
    RightAll    = Right | Top | Bottom,
    AllEdges    = Top | Bottom | Left | Right
};

inline ShellAnchor operator|(ShellAnchor a, ShellAnchor b) {
    return static_cast<ShellAnchor>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}

inline ShellAnchor operator&(ShellAnchor a, ShellAnchor b) {
    return static_cast<ShellAnchor>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
}

inline bool hasAnchor(ShellAnchor flags, ShellAnchor flag) {
    return (static_cast<uint32_t>(flags) & static_cast<uint32_t>(flag)) != 0;
}

/// Keyboard focus behavior for the layer surface
enum class KeyboardMode : uint32_t {
    None      = 0, ///< Never accepts keyboard focus (bars, wallpapers)
    Exclusive = 1, ///< Grabs all keyboard input (lock screens)
    OnDemand  = 2  ///< Accepts keyboard focus when clicked/requested (app launcher search)
};

/// Edge margins around the layer surface anchor
struct SurfaceMargin {
    int32_t top    = 0;
    int32_t right  = 0;
    int32_t bottom = 0;
    int32_t left   = 0;
};

/// Configuration for creating a desktop Layer Surface
struct LayerSurfaceConfig {
    std::string             namespace_id   = "nisaba-shell";
    ShellLayer              layer          = ShellLayer::Top;
    ShellAnchor             anchor         = ShellAnchor::TopAll;
    int32_t                 width          = 0;   ///< 0 means span entire anchor width
    int32_t                 height         = 34;  ///< Desired height in pixels
    int32_t                 exclusive_zone = 0;   ///< Pixels reserved to prevent normal windows from overlapping (-1 ignore, 0 none, >0 reserved)
    SurfaceMargin           margin;
    KeyboardMode            keyboard_mode  = KeyboardMode::None;
    bool                    transparent    = true;
    bool                    vsync          = true;
    std::shared_ptr<Output> target_output  = nullptr; ///< Specific monitor to display on (nullptr = default/focused)
};

/// Represents a Wayland Layer Shell surface (or fallback X11 Dock/Strut Window)
class LayerSurface {
public:
    static Result<std::unique_ptr<LayerSurface>> create(Platform& platform, LayerSurfaceConfig config);

    virtual ~LayerSurface() = default;

    virtual void setSize(int32_t width, int32_t height) = 0;
    virtual void setLayer(ShellLayer layer) = 0;
    virtual void setAnchor(ShellAnchor anchor) = 0;
    virtual void setExclusiveZone(int32_t zone) = 0;
    virtual void setMargin(const SurfaceMargin& margin) = 0;
    virtual void setKeyboardMode(KeyboardMode mode) = 0;

    [[nodiscard]] virtual Size getSize() const = 0;
    [[nodiscard]] virtual Size getDrawableSize() const = 0;
    [[nodiscard]] virtual float getDpiScale() const = 0;

    virtual void makeCurrent() = 0;
    virtual void swapBuffers() = 0;
    virtual void* getNativeHandle() const = 0;
    virtual void* getEGLSurface() const = 0;
    virtual void* getEGLContext() const = 0;
    virtual void* getBackendLayer() const { return nullptr; }

    Signal<int, int>& onResize() { return on_resize_; }
    Signal<>&         onClose()  { return on_close_; }

protected:
    Signal<int, int> on_resize_;
    Signal<>         on_close_;
};

} // namespace nisaba::backend_os
