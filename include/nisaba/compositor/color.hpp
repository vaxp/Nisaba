#pragma once

#include <cstdint>
#include "nisaba/color/color.hpp"

namespace nisaba::compositor {

/**
 * @brief Sovereign RGBA Color representation for compositor rendering.
 * Floating point components are normalized in range [0.0f, 1.0f].
 */
struct Color {
    float r{0.0f};
    float g{0.0f};
    float b{0.0f};
    float a{1.0f};

    constexpr Color() noexcept = default;
    constexpr Color(float red, float green, float blue, float alpha = 1.0f) noexcept
        : r(red), g(green), b(blue), a(alpha) {}

    constexpr Color(const nisaba::ColorU8& c) noexcept
        : r(static_cast<float>(c.r) / 255.0f),
          g(static_cast<float>(c.g) / 255.0f),
          b(static_cast<float>(c.b) / 255.0f),
          a(static_cast<float>(c.a) / 255.0f) {}

    static constexpr Color fromRgba(float r, float g, float b, float a = 1.0f) noexcept {
        return Color{r, g, b, a};
    }

    static constexpr Color fromRgb255(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) noexcept {
        return Color{
            static_cast<float>(r) / 255.0f,
            static_cast<float>(g) / 255.0f,
            static_cast<float>(b) / 255.0f,
            static_cast<float>(a) / 255.0f
        };
    }

    [[nodiscard]] constexpr Color premultiplied() const noexcept {
        return Color{r * a, g * a, b * a, a};
    }

    [[nodiscard]] constexpr nisaba::ColorU8 toColorU8() const noexcept {
        return nisaba::ColorU8(
            static_cast<uint8_t>(r * 255.0f + 0.5f),
            static_cast<uint8_t>(g * 255.0f + 0.5f),
            static_cast<uint8_t>(b * 255.0f + 0.5f),
            static_cast<uint8_t>(a * 255.0f + 0.5f)
        );
    }

    static constexpr Color transparent() noexcept { return Color{0.0f, 0.0f, 0.0f, 0.0f}; }
    static constexpr Color black() noexcept { return Color{0.0f, 0.0f, 0.0f, 1.0f}; }
    static constexpr Color white() noexcept { return Color{1.0f, 1.0f, 1.0f, 1.0f}; }
    static constexpr Color red() noexcept { return Color{1.0f, 0.0f, 0.0f, 1.0f}; }
    static constexpr Color green() noexcept { return Color{0.0f, 1.0f, 0.0f, 1.0f}; }
    static constexpr Color blue() noexcept { return Color{0.0f, 0.0f, 1.0f, 1.0f}; }
};

} // namespace nisaba::compositor
