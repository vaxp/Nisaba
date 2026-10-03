#pragma once

#include <cstdint>
#include <cmath>
#include <optional>
#include <algorithm>
#include "nisaba/types.hpp"

namespace nisaba {

class Size;

/// An integer size with width > 0 and height > 0.
class IntSize {
public:
    constexpr IntSize() : width_(LengthU32::create_unchecked(1)), height_(LengthU32::create_unchecked(1)) {}

    static constexpr std::optional<IntSize> from_wh(uint32_t width, uint32_t height) noexcept {
        auto w = LengthU32::create(width);
        auto h = LengthU32::create(height);
        if (w && h) {
            return IntSize(*w, *h);
        }
        return std::nullopt;
    }

    static constexpr IntSize from_wh_safe(LengthU32 width, LengthU32 height) noexcept {
        return IntSize(width, height);
    }

    constexpr uint32_t width() const noexcept { return width_.get(); }
    constexpr uint32_t height() const noexcept { return height_.get(); }

    std::optional<IntSize> scale_by(float factor) const noexcept {
        return from_wh(
            static_cast<uint32_t>(std::round(static_cast<float>(width()) * factor)),
            static_cast<uint32_t>(std::round(static_cast<float>(height()) * factor))
        );
    }

    IntSize scale_to(IntSize to) const noexcept;

    std::optional<IntSize> scale_to_width(uint32_t new_width) const noexcept {
        float new_height = std::ceil(static_cast<float>(new_width) * static_cast<float>(height()) / static_cast<float>(width()));
        return from_wh(new_width, static_cast<uint32_t>(new_height));
    }

    std::optional<IntSize> scale_to_height(uint32_t new_height) const noexcept {
        float new_width = std::ceil(static_cast<float>(new_height) * static_cast<float>(width()) / static_cast<float>(height()));
        return from_wh(static_cast<uint32_t>(new_width), new_height);
    }

    Size to_size() const noexcept;

    constexpr bool operator==(const IntSize& o) const noexcept {
        return width_ == o.width_ && height_ == o.height_;
    }

    constexpr bool operator!=(const IntSize& o) const noexcept {
        return !(*this == o);
    }

private:
    constexpr IntSize(LengthU32 w, LengthU32 h) noexcept : width_(w), height_(h) {}
    LengthU32 width_;
    LengthU32 height_;
};

/// A floating-point size with positive, non-zero and finite width and height.
class Size {
public:
    static std::optional<Size> from_wh(float width, float height) noexcept {
        auto w = NonZeroPositiveF32::create(width);
        auto h = NonZeroPositiveF32::create(height);
        if (w && h) {
            return Size(*w, *h);
        }
        return std::nullopt;
    }

    constexpr float width() const noexcept { return width_.get(); }
    constexpr float height() const noexcept { return height_.get(); }

    Size scale_to(Size to) const noexcept;
    Size expand_to(Size to) const noexcept;

    std::optional<Size> scale_by(float factor) const noexcept {
        return from_wh(width() * factor, height() * factor);
    }

    std::optional<Size> scale_to_width(float new_width) const noexcept {
        float new_height = new_width * height() / width();
        return from_wh(new_width, new_height);
    }

    std::optional<Size> scale_to_height(float new_height) const noexcept {
        float new_width = new_height * width() / height();
        return from_wh(new_width, new_height);
    }

    IntSize to_int_size() const noexcept {
        uint32_t w = std::max(1u, static_cast<uint32_t>(std::round(width())));
        uint32_t h = std::max(1u, static_cast<uint32_t>(std::round(height())));
        return *IntSize::from_wh(w, h);
    }

    constexpr bool operator==(const Size& o) const noexcept {
        return width_ == o.width_ && height_ == o.height_;
    }

    constexpr bool operator!=(const Size& o) const noexcept {
        return !(*this == o);
    }

private:
    constexpr Size(NonZeroPositiveF32 w, NonZeroPositiveF32 h) noexcept : width_(w), height_(h) {}
    NonZeroPositiveF32 width_;
    NonZeroPositiveF32 height_;

    friend Size size_scale_f64(Size s1, Size s2, bool expand) noexcept;
};

inline IntSize size_scale(IntSize s1, IntSize s2, bool expand) noexcept {
    auto rw = static_cast<uint32_t>(std::ceil(static_cast<float>(s2.height()) * static_cast<float>(s1.width()) / static_cast<float>(s1.height())));
    bool with_h = expand ? (rw <= s2.width()) : (rw >= s2.width());
    if (!with_h) {
        return *IntSize::from_wh(rw, s2.height());
    } else {
        auto h = static_cast<uint32_t>(std::ceil(static_cast<float>(s2.width()) * static_cast<float>(s1.height()) / static_cast<float>(s1.width())));
        return *IntSize::from_wh(s2.width(), h);
    }
}

inline IntSize IntSize::scale_to(IntSize to) const noexcept {
    return size_scale(*this, to, false);
}

inline Size size_scale_f64(Size s1, Size s2, bool expand) noexcept {
    float rw = s2.height() * s1.width() / s1.height();
    bool with_h = expand ? (rw <= s2.width()) : (rw >= s2.width());
    if (!with_h) {
        return *Size::from_wh(rw, s2.height());
    } else {
        float h = s2.width() * s1.height() / s1.width();
        return *Size::from_wh(s2.width(), h);
    }
}

inline Size Size::scale_to(Size to) const noexcept {
    return size_scale_f64(*this, to, false);
}

inline Size Size::expand_to(Size to) const noexcept {
    return size_scale_f64(*this, to, true);
}

inline Size IntSize::to_size() const noexcept {
    return *Size::from_wh(static_cast<float>(width()), static_cast<float>(height()));
}

} // namespace nisaba
