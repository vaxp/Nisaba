#pragma once

#include <cstdint>
#include <vector>
#include <optional>
#include "nisaba/math/screen_int_rect.hpp"
#include "nisaba/math/rect.hpp"

namespace nisaba::damage {

/// @brief Represents a single horizontal pixel span [x0, x1) on scanline y.
struct Span {
    uint16_t y{0};
    uint16_t x0{0};
    uint16_t x1{0};

    constexpr Span() noexcept = default;
    constexpr Span(uint16_t scanline_y, uint16_t left, uint16_t right) noexcept
        : y(scanline_y), x0(left), x1(right) {}

    [[nodiscard]] constexpr uint16_t width() const noexcept {
        return (x1 > x0) ? (x1 - x0) : 0;
    }

    [[nodiscard]] constexpr bool is_valid() const noexcept {
        return x1 > x0;
    }

    constexpr bool operator==(const Span& other) const noexcept = default;
};

/// @brief Represents an invalidated damage rectangle with bounding geometry.
struct DamageRegion {
    ScreenIntRect bounds;

    constexpr explicit DamageRegion(ScreenIntRect b) noexcept : bounds(b) {}
};

} // namespace nisaba::damage
