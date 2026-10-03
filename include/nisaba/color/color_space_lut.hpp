#pragma once

#include <cstdint>
#include <cmath>
#include <algorithm>
#include "nisaba/color/color.hpp"

namespace nisaba {

/// Fast Look-Up Table and conversion utilities for gamma-correct linear sRGB blending.
/// Uses a compact 4.5 KB L1-cache friendly dual LUT:
/// - srgb_to_linear_u12[256]: 8-bit sRGB -> 12-bit linear light (0..4095), 512 bytes.
/// - linear_u12_to_srgb[4096]: 12-bit linear light -> 8-bit sRGB (0..255), 4096 bytes.
/// Provides O(1) single-cycle conversion with 0 roundtrip error across all 256 sRGB levels.
class ColorSpaceLut {
public:
    static const uint16_t srgb_to_linear_u12[256];
    static const uint8_t  linear_u12_to_srgb[4096];

    /// Converts 8-bit sRGB color channel [0..255] to 12-bit linear light [0..4095].
    [[nodiscard]] static inline uint16_t to_linear_u12(uint8_t c) noexcept {
        return srgb_to_linear_u12[c];
    }

    /// Converts 12-bit linear light [0..4095] to 8-bit sRGB color channel [0..255].
    [[nodiscard]] static inline uint8_t to_srgb(uint32_t lin_u12) noexcept {
        if (lin_u12 >= 4096) lin_u12 = 4095;
        return linear_u12_to_srgb[lin_u12];
    }

    /// Exact IEC 61966-2-1 transfer function: sRGB [0..1] to Linear [0..1].
    [[nodiscard]] static inline float to_linear_f32(float s) noexcept {
        s = std::clamp(s, 0.0f, 1.0f);
        if (s <= 0.04045f) {
            return s / 12.92f;
        } else {
            return std::pow((s + 0.055f) / 1.055f, 2.4f);
        }
    }

    /// Exact IEC 61966-2-1 inverse transfer function: Linear [0..1] to sRGB [0..1].
    [[nodiscard]] static inline float to_srgb_f32(float l) noexcept {
        l = std::clamp(l, 0.0f, 1.0f);
        if (l <= 0.0031308f) {
            return l * 12.92f;
        } else {
            return 1.055f * std::pow(l, 1.0f / 2.4f) - 0.055f;
        }
    }

    /// Converts an RGBA Color object from sRGB space to Linear light space.
    [[nodiscard]] static inline Color srgb_to_linear(Color c) noexcept {
        return Color(
            NormalizedF32::create_clamped(to_linear_f32(c.red())),
            NormalizedF32::create_clamped(to_linear_f32(c.green())),
            NormalizedF32::create_clamped(to_linear_f32(c.blue())),
            NormalizedF32::create_clamped(c.alpha())
        );
    }

    /// Converts an RGBA Color object from Linear light space back to sRGB space.
    [[nodiscard]] static inline Color linear_to_srgb(Color c) noexcept {
        return Color(
            NormalizedF32::create_clamped(to_srgb_f32(c.red())),
            NormalizedF32::create_clamped(to_srgb_f32(c.green())),
            NormalizedF32::create_clamped(to_srgb_f32(c.blue())),
            NormalizedF32::create_clamped(c.alpha())
        );
    }

    /// Blends two 8-bit sRGB color channels in linear light given alpha in [0..256].
    /// Formula: sRGB -> Linear -> Lerp(s_lin, d_lin, alpha) -> sRGB.
    [[nodiscard]] static inline uint8_t blend_channel_linear(uint8_t src_srgb, uint8_t dst_srgb, uint32_t a_256) noexcept {
        uint32_t s_lin = srgb_to_linear_u12[src_srgb];
        uint32_t d_lin = srgb_to_linear_u12[dst_srgb];
        uint32_t out_lin = (s_lin * a_256 + d_lin * (256 - a_256) + 128) >> 8;
        if (out_lin > 4095) out_lin = 4095;
        return linear_u12_to_srgb[out_lin];
    }
};

} // namespace nisaba
