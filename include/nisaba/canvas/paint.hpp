#pragma once

#include <cstdint>
#include "nisaba/color/color.hpp"
#include "nisaba/color/blend_mode.hpp"
#include "nisaba/shaders/shader.hpp"
#include "nisaba/canvas/mask.hpp"
#include "nisaba/effects/color_matrix.hpp"

namespace nisaba {

struct PixmapPaint {
    float opacity{1.0f};
    BlendMode blend_mode{BlendMode::SourceOver};
    FilterQuality quality{FilterQuality::Nearest};

    constexpr PixmapPaint() noexcept = default;
};

class Paint {
public:
    Shader shader{Shader::from_color(Color::BLACK)};
    BlendMode blend_mode{BlendMode::SourceOver};
    bool anti_alias{true};
    ColorSpace colorspace{ColorSpace::SimpleSRGB};
    bool force_hq_pipeline{false};
    std::optional<effects::ColorMatrix> color_filter{std::nullopt};

    Paint() noexcept = default;

    explicit Paint(Color color) noexcept : shader(Shader::from_color(color)) {}
    explicit Paint(Shader sh) noexcept : shader(std::move(sh)) {}

    void set_color(Color color) noexcept {
        shader = Shader::from_color(color);
    }

    void set_color_rgba8(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) noexcept {
        shader = Shader::from_color_rgba8(r, g, b, a);
    }

    [[nodiscard]] bool is_solid_color() const noexcept {
        return shader.is_solid_color();
    }

    void set_color_filter(const effects::ColorMatrix& filter) noexcept {
        color_filter = filter;
    }

    void clear_color_filter() noexcept {
        color_filter = std::nullopt;
    }

    [[nodiscard]] bool has_color_filter() const noexcept {
        return color_filter.has_value();
    }

    void set_linear_blending(bool enable) noexcept {
        colorspace = enable ? ColorSpace::Linear : ColorSpace::SimpleSRGB;
    }

    [[nodiscard]] bool is_linear_blending() const noexcept {
        return colorspace == ColorSpace::Linear || colorspace == ColorSpace::FullSRGBGamma;
    }
};

} // namespace nisaba
