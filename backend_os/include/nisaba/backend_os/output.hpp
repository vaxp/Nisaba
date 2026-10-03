#pragma once
/// @file output.hpp
/// @brief Unified Output (Monitor / Display / Screen) subsystem for Wayland and X11.
/// Provides monitor identification, geometry, physical & logical sizing, modes, scaling, and hotplugging signals.

#include "nisaba/backend_os/types.hpp"
#include "nisaba/backend_os/signal.hpp"
#include <string>
#include <string_view>
#include <vector>
#include <memory>
#include <cstdint>

namespace nisaba::backend_os {

/// Display output transform / rotation
enum class OutputTransform : uint32_t {
    Normal      = 0,
    Rotated90   = 1,
    Rotated180  = 2,
    Rotated270  = 3,
    Flipped     = 4,
    Flipped90   = 5,
    Flipped180  = 6,
    Flipped270  = 7,
};

/// Subpixel geometry for font rendering
enum class OutputSubpixel : uint32_t {
    Unknown       = 0,
    None          = 1,
    HorizontalRgb = 2,
    HorizontalBgr = 3,
    VerticalRgb   = 4,
    VerticalBgr   = 5,
};

/// Video mode (resolution and refresh rate)
struct OutputMode {
    int32_t width            = 0;     ///< Horizontal resolution in pixels
    int32_t height           = 0;     ///< Vertical resolution in pixels
    int32_t refresh_rate_mHz = 0;     ///< Refresh rate in milliHz (e.g. 60000 = 60.0 Hz, 144000 = 144.0 Hz)
    bool    is_current       = false; ///< Whether this is the active display mode
    bool    is_preferred     = false; ///< Whether this is the native/preferred mode

    [[nodiscard]] double refreshRateHz() const noexcept {
        return refresh_rate_mHz / 1000.0;
    }
};

/// @brief Represents a physical or virtual video display output (monitor / screen).
class Output {
public:
    virtual ~Output() = default;

    /// Output identifier (protocol specific ID or index)
    [[nodiscard]] virtual uint32_t id() const noexcept = 0;

    /// Connector / Output name (e.g. "eDP-1", "HDMI-A-1", "DP-2")
    [[nodiscard]] virtual const std::string& name() const noexcept = 0;

    /// Manufacturer / Make (e.g. "LG Electronics", "Dell Inc.", "Samsung")
    [[nodiscard]] virtual const std::string& make() const noexcept = 0;

    /// Monitor Model (e.g. "DELL U2720Q", "LG UltraFine")
    [[nodiscard]] virtual const std::string& model() const noexcept = 0;

    /// Human-friendly description (e.g. "LG Electronics LG UltraFine (DP-1)")
    [[nodiscard]] virtual const std::string& description() const noexcept = 0;

    /// Physical pixel bounds and offset in compositor global coordinates
    [[nodiscard]] virtual Rect geometry() const noexcept = 0;

    /// Logical bounds and position (compositor layout space, scaling applied)
    [[nodiscard]] virtual Rect logicalGeometry() const noexcept = 0;

    /// Physical width of the panel in millimeters
    [[nodiscard]] virtual int32_t physicalWidthMm() const noexcept = 0;

    /// Physical height of the panel in millimeters
    [[nodiscard]] virtual int32_t physicalHeightMm() const noexcept = 0;

    /// Integer scale factor (1 = 100%, 2 = 200%)
    [[nodiscard]] virtual int32_t scaleFactor() const noexcept = 0;

    /// Fractional scale factor (e.g. 1.0, 1.25, 1.5, 2.0)
    [[nodiscard]] virtual double fractionalScale() const noexcept = 0;

    /// Display orientation / transform
    [[nodiscard]] virtual OutputTransform transform() const noexcept = 0;

    /// Subpixel layout
    [[nodiscard]] virtual OutputSubpixel subpixel() const noexcept = 0;

    /// All available display modes
    [[nodiscard]] virtual const std::vector<OutputMode>& modes() const noexcept = 0;

    /// Currently active display mode
    [[nodiscard]] virtual const OutputMode& currentMode() const noexcept = 0;

    /// Whether this is marked as the primary output (X11 primary or main Wayland output)
    [[nodiscard]] virtual bool isPrimary() const noexcept = 0;

    /// Native handle (struct wl_output* in Wayland, RROutput in X11)
    [[nodiscard]] virtual void* nativeHandle() const noexcept = 0;

    // ── Signals ────────────────────────────────────────────────

    /// Emitted when position, resolution or logical geometry changes
    Signal<>& onGeometryChanged() { return on_geometry_changed_; }

    /// Emitted when active mode or refresh rate changes
    Signal<>& onModeChanged() { return on_mode_changed_; }

    /// Emitted when scale factor changes
    Signal<>& onScaleChanged() { return on_scale_changed_; }

    /// Emitted when this output is disconnected / removed
    Signal<>& onRemoved() { return on_removed_; }

protected:
    Signal<> on_geometry_changed_;
    Signal<> on_mode_changed_;
    Signal<> on_scale_changed_;
    Signal<> on_removed_;
};

} // namespace nisaba::backend_os
