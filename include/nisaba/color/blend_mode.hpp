#pragma once

namespace nisaba {

/// Supported Porter-Duff and advanced blend modes.
enum class BlendMode {
    /// Replaces destination with zero: fully transparent.
    Clear,
    /// Replaces destination with source.
    Source,
    /// Preserves destination.
    Destination,
    /// Source over destination (default).
    SourceOver,
    /// Destination over source.
    DestinationOver,
    /// Source trimmed inside destination.
    SourceIn,
    /// Destination trimmed by source.
    DestinationIn,
    /// Source trimmed outside destination.
    SourceOut,
    /// Destination trimmed outside source.
    DestinationOut,
    /// Source inside destination blended with destination.
    SourceAtop,
    /// Destination inside source blended with source.
    DestinationAtop,
    /// Each of source and destination trimmed outside the other.
    Xor,
    /// Sum of colors.
    Plus,
    /// Product of premultiplied colors; darkens destination.
    Modulate,
    /// Multiply inverse of pixels, inverting result; brightens destination.
    Screen,
    /// Multiply or screen, depending on destination.
    Overlay,
    /// Darker of source and destination.
    Darken,
    /// Lighter of source and destination.
    Lighten,
    /// Brighten destination to reflect source.
    ColorDodge,
    /// Darken destination to reflect source.
    ColorBurn,
    /// Multiply or screen, depending on source.
    HardLight,
    /// Lighten or darken, depending on source.
    SoftLight,
    /// Subtract darker from lighter with higher contrast.
    Difference,
    /// Subtract darker from lighter with lower contrast.
    Exclusion,
    /// Multiply source with destination, darkening image.
    Multiply,
    /// Hue of source with saturation and luminosity of destination.
    Hue,
    /// Saturation of source with hue and luminosity of destination.
    Saturation,
    /// Hue and saturation of source with luminosity of destination.
    Color,
    /// Luminosity of source with hue and saturation of destination.
    Luminosity,
};

/// Check if coverage can be pre-scaled into alpha for this blend mode.
inline constexpr bool should_pre_scale_coverage(BlendMode mode) noexcept {
    switch (mode) {
        case BlendMode::Destination:
        case BlendMode::DestinationOver:
        case BlendMode::Plus:
        case BlendMode::DestinationOut:
        case BlendMode::SourceAtop:
        case BlendMode::SourceOver:
        case BlendMode::Xor:
            return true;
        default:
            return false;
    }
}

} // namespace nisaba
