#pragma once

#include <optional>
#include "nisaba/color/color.hpp"
#include "nisaba/effects/drop_shadow.hpp"

namespace nisaba::effects {

/// Parameters configuring a modern frosted glassmorphic UI card or panel.
struct GlassParams {
    float blur_sigma{12.0f};                                     ///< Gaussian blur standard deviation for backdrop
    Color tint_color{Color::from_rgba8(255, 255, 255, 35)};      ///< Translucent frost overlay tint
    float border_width{1.2f};                                    ///< Subtle edge reflection border width
    Color border_color{Color::from_rgba8(255, 255, 255, 75)};    ///< Subtle edge reflection border color
    std::optional<DropShadow> shadow{DropShadow::medium(8.0f, 12.0f)}; ///< Elevation drop shadow

    constexpr GlassParams() noexcept = default;

    static constexpr GlassParams dark() noexcept {
        GlassParams p;
        p.blur_sigma = 14.0f;
        p.tint_color = Color::from_rgba8(20, 24, 33, 140);
        p.border_width = 1.0f;
        p.border_color = Color::from_rgba8(255, 255, 255, 40);
        p.shadow = DropShadow(0.0f, 10.0f, 16.0f, Color::from_rgba8(0, 0, 0, 160));
        return p;
    }

    static constexpr GlassParams light() noexcept {
        GlassParams p;
        p.blur_sigma = 12.0f;
        p.tint_color = Color::from_rgba8(255, 255, 255, 50);
        p.border_width = 1.5f;
        p.border_color = Color::from_rgba8(255, 255, 255, 95);
        p.shadow = DropShadow(0.0f, 8.0f, 12.0f, Color::from_rgba8(0, 0, 0, 80));
        return p;
    }

    static constexpr GlassParams neon(Color accent, Color bg) noexcept {
        GlassParams p;
        p.blur_sigma = 16.0f;
        p.tint_color = bg;
        p.border_width = 1.5f;
        p.border_color = accent;
        p.shadow = DropShadow::glow(accent, 14.0f);
        return p;
    }
};

/// Renders a modern frosted glass panel (Glassmorphism) with backdrop blur,
/// translucent tint, edge reflection border, and elevation shadow.
void draw_glass_panel(
    PixmapMut& dst,
    const Rect& rect,
    float rx,
    float ry,
    const GlassParams& params,
    Transform transform = Transform(),
    const Mask* clip_mask = nullptr
);

} // namespace nisaba::effects
