#pragma once

#include <cstdint>
#include <cmath>
#include <optional>
#include <array>
#include "nisaba/types.hpp"
#include "nisaba/math/scalar.hpp"

namespace nisaba {

using AlphaU8 = uint8_t;

inline constexpr AlphaU8 ALPHA_U8_TRANSPARENT = 0x00;
inline constexpr AlphaU8 ALPHA_U8_OPAQUE = 0xFF;

inline const NormalizedF32 ALPHA_TRANSPARENT = NormalizedF32::ZERO;
inline const NormalizedF32 ALPHA_OPAQUE = NormalizedF32::ONE;

/// Return a * b / 255, rounding any fractional bits.
inline constexpr uint8_t premultiply_u8(uint8_t c, uint8_t a) noexcept {
    uint32_t prod = static_cast<uint32_t>(c) * static_cast<uint32_t>(a) + 128;
    return static_cast<uint8_t>((prod + (prod >> 8)) >> 8);
}

struct PremultipliedColorU8;

inline constexpr uint16_t color_to_rgb565(uint8_t r, uint8_t g, uint8_t b) noexcept {
    return static_cast<uint16_t>(((static_cast<uint16_t>(r & 0xF8) << 8)) |
                                 ((static_cast<uint16_t>(g & 0xFC) << 3)) |
                                 (static_cast<uint16_t>(b >> 3)));
}

inline constexpr uint8_t rgb_to_gray8(uint8_t r, uint8_t g, uint8_t b) noexcept {
    return static_cast<uint8_t>((static_cast<uint32_t>(r) * 54 +
                                 static_cast<uint32_t>(g) * 183 +
                                 static_cast<uint32_t>(b) * 19) >> 8);
}

/// A 32-bit RGBA color value.
///
/// Byte order: RGBA
struct ColorU8 {
    uint8_t r{0};
    uint8_t g{0};
    uint8_t b{0};
    uint8_t a{0};

    constexpr ColorU8() noexcept = default;
    constexpr ColorU8(uint8_t r, uint8_t g, uint8_t b, uint8_t a) noexcept
        : r(r), g(g), b(b), a(a) {}

    static constexpr ColorU8 from_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a) noexcept {
        return ColorU8(r, g, b, a);
    }

    constexpr uint8_t red() const noexcept { return r; }
    constexpr uint8_t green() const noexcept { return g; }
    constexpr uint8_t blue() const noexcept { return b; }
    constexpr uint8_t alpha() const noexcept { return a; }

    constexpr bool is_opaque() const noexcept { return a == ALPHA_U8_OPAQUE; }

    constexpr bool operator==(const ColorU8& other) const noexcept = default;

    constexpr PremultipliedColorU8 premultiply() const noexcept;

    constexpr uint16_t to_rgb565() const noexcept {
        return color_to_rgb565(r, g, b);
    }

    static constexpr ColorU8 from_rgb565(uint16_t c) noexcept {
        uint8_t r5 = static_cast<uint8_t>((c >> 11) & 0x1F);
        uint8_t g6 = static_cast<uint8_t>((c >> 5) & 0x3F);
        uint8_t b5 = static_cast<uint8_t>(c & 0x1F);
        uint8_t r8 = static_cast<uint8_t>((r5 << 3) | (r5 >> 2));
        uint8_t g8 = static_cast<uint8_t>((g6 << 2) | (g6 >> 4));
        uint8_t b8 = static_cast<uint8_t>((b5 << 3) | (b5 >> 2));
        return ColorU8(r8, g8, b8, ALPHA_U8_OPAQUE);
    }

    constexpr uint8_t to_gray8() const noexcept {
        return rgb_to_gray8(r, g, b);
    }
};

inline constexpr ColorU8 rgb565_to_color(uint16_t c) noexcept {
    return ColorU8::from_rgb565(c);
}

/// A 32-bit premultiplied RGBA color value.
///
/// Byte order: RGBA
struct PremultipliedColorU8 {
    uint8_t r{0};
    uint8_t g{0};
    uint8_t b{0};
    uint8_t a{0};

    constexpr PremultipliedColorU8() noexcept = default;
    constexpr PremultipliedColorU8(uint8_t r, uint8_t g, uint8_t b, uint8_t a) noexcept
        : r(r), g(g), b(b), a(a) {}

    static const PremultipliedColorU8 TRANSPARENT;

    static constexpr std::optional<PremultipliedColorU8> from_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a) noexcept {
        if (r <= a && g <= a && b <= a) {
            return PremultipliedColorU8(r, g, b, a);
        }
        return std::nullopt;
    }

    static constexpr PremultipliedColorU8 from_rgba_unchecked(uint8_t r, uint8_t g, uint8_t b, uint8_t a) noexcept {
        return PremultipliedColorU8(r, g, b, a);
    }

    constexpr uint8_t red() const noexcept { return r; }
    constexpr uint8_t green() const noexcept { return g; }
    constexpr uint8_t blue() const noexcept { return b; }
    constexpr uint8_t alpha() const noexcept { return a; }

    constexpr bool is_opaque() const noexcept { return a == ALPHA_U8_OPAQUE; }

    constexpr bool operator==(const PremultipliedColorU8& other) const noexcept = default;

    constexpr uint16_t to_rgb565() const noexcept {
        return color_to_rgb565(r, g, b);
    }

    static constexpr PremultipliedColorU8 from_rgb565(uint16_t c) noexcept {
        ColorU8 col = rgb565_to_color(c);
        return PremultipliedColorU8::from_rgba_unchecked(col.red(), col.green(), col.blue(), 255);
    }

    constexpr uint8_t to_gray8() const noexcept {
        return rgb_to_gray8(r, g, b);
    }

    ColorU8 demultiply() const noexcept {
        if (a == ALPHA_U8_OPAQUE) {
            return ColorU8(r, g, b, a);
        } else {
            double af = static_cast<double>(a) / 255.0;
            return ColorU8(
                static_cast<uint8_t>(static_cast<double>(r) / af + 0.5),
                static_cast<uint8_t>(static_cast<double>(g) / af + 0.5),
                static_cast<uint8_t>(static_cast<double>(b) / af + 0.5),
                a
            );
        }
    }
};

inline constexpr PremultipliedColorU8 PremultipliedColorU8::TRANSPARENT{0, 0, 0, 0};

inline constexpr PremultipliedColorU8 ColorU8::premultiply() const noexcept {
    if (a != ALPHA_U8_OPAQUE) {
        return PremultipliedColorU8::from_rgba_unchecked(
            premultiply_u8(r, a),
            premultiply_u8(g, a),
            premultiply_u8(b, a),
            a
        );
    } else {
        return PremultipliedColorU8::from_rgba_unchecked(r, g, b, a);
    }
}

class PremultipliedColor;

/// An RGBA color value holding four floating-point components in 0..=1 range.
class Color {
public:
    static const Color TRANSPARENT;
    static const Color BLACK;
    static const Color WHITE;
    static const Color RED;
    static const Color GREEN;
    static const Color BLUE;

    constexpr Color() noexcept = default;
    constexpr Color(NormalizedF32 r, NormalizedF32 g, NormalizedF32 b, NormalizedF32 a) noexcept
        : r_(r), g_(g), b_(b), a_(a) {}

    static constexpr Color from_rgba_unchecked(float r, float g, float b, float a) noexcept {
        return Color(
            NormalizedF32::create_unchecked(r),
            NormalizedF32::create_unchecked(g),
            NormalizedF32::create_unchecked(b),
            NormalizedF32::create_unchecked(a)
        );
    }

    static std::optional<Color> from_rgba(float r, float g, float b, float a) noexcept {
        auto nr = NormalizedF32::create(r);
        auto ng = NormalizedF32::create(g);
        auto nb = NormalizedF32::create(b);
        auto na = NormalizedF32::create(a);
        if (nr && ng && nb && na) {
            return Color(*nr, *ng, *nb, *na);
        }
        return std::nullopt;
    }

    static constexpr Color from_rgba8(uint8_t r, uint8_t g, uint8_t b, uint8_t a) noexcept {
        return Color(
            NormalizedF32::from_u8(r),
            NormalizedF32::from_u8(g),
            NormalizedF32::from_u8(b),
            NormalizedF32::from_u8(a)
        );
    }

    constexpr float red() const noexcept { return r_.get(); }
    constexpr float green() const noexcept { return g_.get(); }
    constexpr float blue() const noexcept { return b_.get(); }
    constexpr float alpha() const noexcept { return a_.get(); }

    constexpr NormalizedF32 red_norm() const noexcept { return r_; }
    constexpr NormalizedF32 green_norm() const noexcept { return g_; }
    constexpr NormalizedF32 blue_norm() const noexcept { return b_; }
    constexpr NormalizedF32 alpha_norm() const noexcept { return a_; }

    void set_red(float c) noexcept { r_ = NormalizedF32::create_clamped(c); }
    void set_green(float c) noexcept { g_ = NormalizedF32::create_clamped(c); }
    void set_blue(float c) noexcept { b_ = NormalizedF32::create_clamped(c); }
    void set_alpha(float c) noexcept { a_ = NormalizedF32::create_clamped(c); }

    void apply_opacity(float opacity) noexcept {
        a_ = NormalizedF32::create_clamped(a_.get() * scalar::bound(opacity, 0.0f, 1.0f));
    }

    constexpr bool is_opaque() const noexcept {
        return a_ == NormalizedF32::ONE;
    }

    constexpr bool operator==(const Color& other) const noexcept = default;

    constexpr PremultipliedColor premultiply() const noexcept;

    ColorU8 to_color_u8() const noexcept {
        return ColorU8::from_rgba(
            static_cast<uint8_t>(r_.get() * 255.0f + 0.5f),
            static_cast<uint8_t>(g_.get() * 255.0f + 0.5f),
            static_cast<uint8_t>(b_.get() * 255.0f + 0.5f),
            static_cast<uint8_t>(a_.get() * 255.0f + 0.5f)
        );
    }

private:
    NormalizedF32 r_{NormalizedF32::ZERO};
    NormalizedF32 g_{NormalizedF32::ZERO};
    NormalizedF32 b_{NormalizedF32::ZERO};
    NormalizedF32 a_{NormalizedF32::ZERO};
    friend class ColorSpaceHelper;
};

inline constexpr Color Color::TRANSPARENT{NormalizedF32::ZERO, NormalizedF32::ZERO, NormalizedF32::ZERO, NormalizedF32::ZERO};
inline constexpr Color Color::BLACK{NormalizedF32::ZERO, NormalizedF32::ZERO, NormalizedF32::ZERO, NormalizedF32::ONE};
inline constexpr Color Color::WHITE{NormalizedF32::ONE, NormalizedF32::ONE, NormalizedF32::ONE, NormalizedF32::ONE};
inline constexpr Color Color::RED{NormalizedF32::ONE, NormalizedF32::ZERO, NormalizedF32::ZERO, NormalizedF32::ONE};
inline constexpr Color Color::GREEN{NormalizedF32::ZERO, NormalizedF32::ONE, NormalizedF32::ZERO, NormalizedF32::ONE};
inline constexpr Color Color::BLUE{NormalizedF32::ZERO, NormalizedF32::ZERO, NormalizedF32::ONE, NormalizedF32::ONE};

/// A premultiplied RGBA color value holding four floating-point components.
///
/// Guarantees:
/// - All values are in 0..=1 range.
/// - RGB components are <= A.
class PremultipliedColor {
public:
    constexpr PremultipliedColor() noexcept = default;
    constexpr PremultipliedColor(NormalizedF32 r, NormalizedF32 g, NormalizedF32 b, NormalizedF32 a) noexcept
        : r_(r), g_(g), b_(b), a_(a) {}

    constexpr float red() const noexcept { return r_.get(); }
    constexpr float green() const noexcept { return g_.get(); }
    constexpr float blue() const noexcept { return b_.get(); }
    constexpr float alpha() const noexcept { return a_.get(); }

    constexpr NormalizedF32 red_norm() const noexcept { return r_; }
    constexpr NormalizedF32 green_norm() const noexcept { return g_; }
    constexpr NormalizedF32 blue_norm() const noexcept { return b_; }
    constexpr NormalizedF32 alpha_norm() const noexcept { return a_; }

    constexpr bool operator==(const PremultipliedColor& other) const noexcept = default;

    Color demultiply() const noexcept {
        float a = a_.get();
        if (a == 0.0f) {
            return Color::TRANSPARENT;
        } else {
            return Color(
                NormalizedF32::create_clamped(r_.get() / a),
                NormalizedF32::create_clamped(g_.get() / a),
                NormalizedF32::create_clamped(b_.get() / a),
                a_
            );
        }
    }

    PremultipliedColorU8 to_color_u8() const noexcept {
        return PremultipliedColorU8::from_rgba_unchecked(
            static_cast<uint8_t>(r_.get() * 255.0f + 0.5f),
            static_cast<uint8_t>(g_.get() * 255.0f + 0.5f),
            static_cast<uint8_t>(b_.get() * 255.0f + 0.5f),
            static_cast<uint8_t>(a_.get() * 255.0f + 0.5f)
        );
    }

private:
    NormalizedF32 r_{NormalizedF32::ZERO};
    NormalizedF32 g_{NormalizedF32::ZERO};
    NormalizedF32 b_{NormalizedF32::ZERO};
    NormalizedF32 a_{NormalizedF32::ZERO};
};

inline constexpr PremultipliedColor Color::premultiply() const noexcept {
    if (is_opaque()) {
        return PremultipliedColor(r_, g_, b_, a_);
    } else {
        return PremultipliedColor(
            NormalizedF32::create_clamped(r_.get() * a_.get()),
            NormalizedF32::create_clamped(g_.get() * a_.get()),
            NormalizedF32::create_clamped(b_.get() * a_.get()),
            a_
        );
    }
}

/// The colorspace used to interpret pixel values.
enum class ColorSpace {
    Linear,
    Gamma2,
    SimpleSRGB,
    FullSRGBGamma
};

class ColorSpaceHelper {
public:
    static NormalizedF32 expand_channel(ColorSpace cs, NormalizedF32 x) noexcept {
        switch (cs) {
            case ColorSpace::Linear:
                return x;
            case ColorSpace::Gamma2:
                return x * x;
            case ColorSpace::SimpleSRGB:
                return NormalizedF32::create_clamped(scalar::approx_powf(x.get(), 2.2f));
            case ColorSpace::FullSRGBGamma: {
                float v = x.get();
                if (v <= 0.04045f) {
                    v /= 12.92f;
                } else {
                    v = scalar::approx_powf((v + 0.055f) / 1.055f, 2.4f);
                }
                return NormalizedF32::create_clamped(v);
            }
        }
        return x;
    }

    static Color expand_color(ColorSpace cs, Color color) noexcept {
        return Color(
            expand_channel(cs, color.red_norm()),
            expand_channel(cs, color.green_norm()),
            expand_channel(cs, color.blue_norm()),
            color.alpha_norm()
        );
    }

    static NormalizedF32 compress_channel(ColorSpace cs, NormalizedF32 x) noexcept {
        switch (cs) {
            case ColorSpace::Linear:
                return x;
            case ColorSpace::Gamma2:
                return NormalizedF32::create_clamped(std::sqrt(x.get()));
            case ColorSpace::SimpleSRGB:
                return NormalizedF32::create_clamped(scalar::approx_powf(x.get(), 0.45454545f));
            case ColorSpace::FullSRGBGamma: {
                float v = x.get();
                if (v <= 0.0031308f) {
                    v *= 12.92f;
                } else {
                    v = scalar::approx_powf(v, 0.416666666f) * 1.055f - 0.055f;
                }
                return NormalizedF32::create_clamped(v);
            }
        }
        return x;
    }
};

} // namespace nisaba
