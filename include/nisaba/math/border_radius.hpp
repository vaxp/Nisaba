#pragma once

#include <algorithm>
#include <cmath>

namespace nisaba {

/// Represents the radii of the four corners of a rounded rectangle.
struct BorderRadius {
    float top_left{0.0f};
    float top_right{0.0f};
    float bottom_right{0.0f};
    float bottom_left{0.0f};

    constexpr BorderRadius() noexcept = default;
    constexpr BorderRadius(float tl, float tr, float br, float bl) noexcept
        : top_left(tl), top_right(tr), bottom_right(br), bottom_left(bl) {}

    /// Creates a BorderRadius with all radii set to 0.
    static constexpr BorderRadius zero() noexcept {
        return {0.0f, 0.0f, 0.0f, 0.0f};
    }

    /// Creates a BorderRadius with all four corners set to r.
    static constexpr BorderRadius all(float r) noexcept {
        return {r, r, r, r};
    }

    /// Alias for all(r).
    static constexpr BorderRadius circular(float r) noexcept {
        return {r, r, r, r};
    }

    /// Creates a BorderRadius with vertical symmetry (top corners and bottom corners).
    static constexpr BorderRadius vertical(float top, float bottom) noexcept {
        return {top, top, bottom, bottom};
    }

    /// Creates a BorderRadius with horizontal symmetry (left corners and right corners).
    static constexpr BorderRadius horizontal(float left, float right) noexcept {
        return {left, right, right, left};
    }

    /// Creates a BorderRadius with custom values for individual corners.
    static constexpr BorderRadius only(float tl = 0.0f, float tr = 0.0f, float br = 0.0f, float bl = 0.0f) noexcept {
        return {tl, tr, br, bl};
    }

    /// Returns true if all radii are zero (or negative).
    constexpr bool is_zero() const noexcept {
        return top_left <= 0.0f && top_right <= 0.0f && bottom_right <= 0.0f && bottom_left <= 0.0f;
    }

    /// Returns true if all four corners have the exact same radius.
    constexpr bool is_uniform() const noexcept {
        return top_left == top_right && top_right == bottom_right && bottom_right == bottom_left;
    }

    /// Returns a new BorderRadius scaled by factor.
    constexpr BorderRadius scaled_by(float factor) const noexcept {
        return {top_left * factor, top_right * factor, bottom_right * factor, bottom_left * factor};
    }

    /// Returns clamped non-negative radii.
    constexpr BorderRadius non_negative() const noexcept {
        return {
            (top_left > 0.0f) ? top_left : 0.0f,
            (top_right > 0.0f) ? top_right : 0.0f,
            (bottom_right > 0.0f) ? bottom_right : 0.0f,
            (bottom_left > 0.0f) ? bottom_left : 0.0f
        };
    }

    /// Scales corner radii so adjacent corners never overlap, conforming to W3C CSS Backgrounds & Borders Level 3.
    BorderRadius constrained_for_size(float width, float height) const noexcept {
        if (width <= 0.0f || height <= 0.0f || is_zero()) {
            return {0.0f, 0.0f, 0.0f, 0.0f};
        }

        BorderRadius nn = non_negative();
        float top_sum = nn.top_left + nn.top_right;
        float bottom_sum = nn.bottom_left + nn.bottom_right;
        float left_sum = nn.top_left + nn.bottom_left;
        float right_sum = nn.top_right + nn.bottom_right;

        float factor = 1.0f;
        if (top_sum > width) factor = std::min(factor, width / top_sum);
        if (bottom_sum > width) factor = std::min(factor, width / bottom_sum);
        if (left_sum > height) factor = std::min(factor, height / left_sum);
        if (right_sum > height) factor = std::min(factor, height / right_sum);

        if (factor < 1.0f) {
            return nn.scaled_by(factor);
        }
        return nn;
    }

    constexpr bool operator==(const BorderRadius& o) const noexcept {
        return top_left == o.top_left && top_right == o.top_right &&
               bottom_right == o.bottom_right && bottom_left == o.bottom_left;
    }

    constexpr bool operator!=(const BorderRadius& o) const noexcept {
        return !(*this == o);
    }
};

} // namespace nisaba
