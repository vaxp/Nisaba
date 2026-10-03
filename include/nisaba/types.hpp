#pragma once

#include <cstdint>
#include <cmath>
#include <optional>
#include <cassert>

namespace nisaba {

/// An integer length that is guaranteed to be > 0.
class LengthU32 {
public:
    constexpr LengthU32() : value_(1) {}

    static constexpr std::optional<LengthU32> create(uint32_t val) {
        if (val > 0) {
            return LengthU32(val);
        }
        return std::nullopt;
    }

    static constexpr LengthU32 create_unchecked(uint32_t val) {
        return LengthU32(val);
    }

    constexpr uint32_t get() const noexcept { return value_; }
    constexpr operator uint32_t() const noexcept { return value_; }

    constexpr bool operator==(const LengthU32& o) const = default;
    constexpr bool operator!=(const LengthU32& o) const = default;

private:
    constexpr explicit LengthU32(uint32_t val) : value_(val) {}
    uint32_t value_;
};

/// A float that is guaranteed to be finite (not NaN, not +/-Infinity).
class FiniteF32 {
public:
    constexpr FiniteF32() : value_(0.0f) {}

    static std::optional<FiniteF32> create(float val) {
        if (std::isfinite(val)) {
            return FiniteF32(val);
        }
        return std::nullopt;
    }

    static constexpr FiniteF32 create_unchecked(float val) {
        return FiniteF32(val);
    }

    constexpr float get() const noexcept { return value_; }
    constexpr operator float() const noexcept { return value_; }

    constexpr bool operator==(const FiniteF32& o) const = default;
    constexpr auto operator<=>(const FiniteF32& o) const = default;

private:
    constexpr explicit FiniteF32(float val) : value_(val) {}
    float value_;
};

/// A float that is guaranteed to be positive and non-zero (> 0.0f and finite).
class NonZeroPositiveF32 {
public:
    static std::optional<NonZeroPositiveF32> create(float val) {
        if (val > 0.0f && std::isfinite(val)) {
            return NonZeroPositiveF32(val);
        }
        return std::nullopt;
    }

    static constexpr NonZeroPositiveF32 create_unchecked(float val) {
        return NonZeroPositiveF32(val);
    }

    constexpr float get() const noexcept { return value_; }
    constexpr operator float() const noexcept { return value_; }

    constexpr bool operator==(const NonZeroPositiveF32& o) const = default;
    constexpr auto operator<=>(const NonZeroPositiveF32& o) const = default;

private:
    constexpr explicit NonZeroPositiveF32(float val) : value_(val) {}
    float value_{1.0f};
};

/// An immutable f32 that is >= 0.0f and <= 1.0f.
class NormalizedF32 {
public:
    constexpr NormalizedF32() : value_(0.0f) {}

    static std::optional<NormalizedF32> create(float val) {
        if (val >= 0.0f && val <= 1.0f && std::isfinite(val)) {
            return NormalizedF32(val);
        }
        return std::nullopt;
    }

    static constexpr NormalizedF32 create_unchecked(float val) {
        return NormalizedF32(val);
    }

    static constexpr NormalizedF32 create_clamped(float val) noexcept {
        if (std::isnan(val) || val <= 0.0f) {
            return NormalizedF32(0.0f);
        }
        if (val >= 1.0f) {
            return NormalizedF32(1.0f);
        }
        return NormalizedF32(val);
    }

    static constexpr NormalizedF32 zero() noexcept { return NormalizedF32(0.0f); }
    static constexpr NormalizedF32 one() noexcept { return NormalizedF32(1.0f); }

    static const NormalizedF32 ZERO;
    static const NormalizedF32 ONE;

    static constexpr NormalizedF32 from_u8(uint8_t val) noexcept {
        return NormalizedF32(static_cast<float>(val) / 255.0f);
    }

    constexpr float get() const noexcept { return value_; }
    constexpr operator float() const noexcept { return value_; }

    constexpr NormalizedF32 operator*(NormalizedF32 o) const noexcept {
        return NormalizedF32(value_ * o.value_);
    }

    constexpr bool operator==(const NormalizedF32& o) const = default;
    constexpr auto operator<=>(const NormalizedF32& o) const = default;

private:
    constexpr explicit NormalizedF32(float val) : value_(val) {}
    float value_;
    friend class NormalizedF32Exclusive;
};

inline constexpr NormalizedF32 NormalizedF32::ZERO{0.0f};
inline constexpr NormalizedF32 NormalizedF32::ONE{1.0f};

/// An immutable f32 that is > 0.0f and < 1.0f.
class NormalizedF32Exclusive {
public:
    constexpr NormalizedF32Exclusive() : value_(0.5f) {}

    static constexpr NormalizedF32Exclusive half() {
        return NormalizedF32Exclusive(0.5f);
    }

    static const NormalizedF32Exclusive HALF;
    static const NormalizedF32Exclusive ANY;

    static std::optional<NormalizedF32Exclusive> create(float val) {
        if (val > 0.0f && val < 1.0f && std::isfinite(val)) {
            return NormalizedF32Exclusive(val);
        }
        return std::nullopt;
    }

    static constexpr NormalizedF32Exclusive create_unchecked(float val) noexcept {
        return NormalizedF32Exclusive(val);
    }

    static NormalizedF32Exclusive create_bounded(float val);

    constexpr float get() const noexcept { return value_; }
    constexpr operator float() const noexcept { return value_; }

    constexpr NormalizedF32 to_normalized() const {
        return NormalizedF32(value_);
    }

    constexpr bool operator==(const NormalizedF32Exclusive& o) const = default;
    constexpr auto operator<=>(const NormalizedF32Exclusive& o) const = default;

private:
    constexpr explicit NormalizedF32Exclusive(float val) : value_(val) {}
    float value_;
};

inline constexpr NormalizedF32Exclusive NormalizedF32Exclusive::HALF{0.5f};
inline constexpr NormalizedF32Exclusive NormalizedF32Exclusive::ANY{0.5f};

} // namespace nisaba
