#include "nisaba/effects/color_filter.hpp"
#include "nisaba/effects/color_matrix.hpp"
#include <algorithm>
#include <cmath>

namespace nisaba::effects {

void ColorFilter::invert(PixmapMut& pixmap) noexcept {
    int32_t w = static_cast<int32_t>(pixmap.width());
    int32_t h = static_cast<int32_t>(pixmap.height());

    for (int32_t y = 0; y < h; ++y) {
        auto* row = pixmap.row(static_cast<size_t>(y));
        for (int32_t x = 0; x < w; ++x) {
            auto& p = row[x];
            p.r = static_cast<uint8_t>(p.a - p.r);
            p.g = static_cast<uint8_t>(p.a - p.g);
            p.b = static_cast<uint8_t>(p.a - p.b);
        }
    }
}

void ColorFilter::grayscale(PixmapMut& pixmap) noexcept {
    int32_t w = static_cast<int32_t>(pixmap.width());
    int32_t h = static_cast<int32_t>(pixmap.height());

    for (int32_t y = 0; y < h; ++y) {
        auto* row = pixmap.row(static_cast<size_t>(y));
        for (int32_t x = 0; x < w; ++x) {
            auto& p = row[x];
            uint32_t lum = (77u * p.r + 150u * p.g + 29u * p.b + 128u) >> 8;
            uint8_t l = static_cast<uint8_t>(std::min(lum, static_cast<uint32_t>(p.a)));
            p.r = l;
            p.g = l;
            p.b = l;
        }
    }
}

void ColorFilter::adjust_brightness(PixmapMut& pixmap, float factor) noexcept {
    if (std::abs(factor) < 0.0001f) return;
    factor = std::clamp(factor, -1.0f, 1.0f);

    int32_t w = static_cast<int32_t>(pixmap.width());
    int32_t h = static_cast<int32_t>(pixmap.height());

    for (int32_t y = 0; y < h; ++y) {
        auto* row = pixmap.row(static_cast<size_t>(y));
        for (int32_t x = 0; x < w; ++x) {
            auto& p = row[x];
            if (p.a == 0) continue;

            if (factor > 0.0f) {
                p.r = static_cast<uint8_t>(p.r + (p.a - p.r) * factor + 0.5f);
                p.g = static_cast<uint8_t>(p.g + (p.a - p.g) * factor + 0.5f);
                p.b = static_cast<uint8_t>(p.b + (p.a - p.b) * factor + 0.5f);
            } else {
                float scale = 1.0f + factor; // in [0, 1]
                p.r = static_cast<uint8_t>(p.r * scale + 0.5f);
                p.g = static_cast<uint8_t>(p.g * scale + 0.5f);
                p.b = static_cast<uint8_t>(p.b * scale + 0.5f);
            }
        }
    }
}

void ColorFilter::adjust_contrast(PixmapMut& pixmap, float factor) noexcept {
    if (std::abs(factor - 1.0f) < 0.0001f || factor < 0.0f) return;

    int32_t w = static_cast<int32_t>(pixmap.width());
    int32_t h = static_cast<int32_t>(pixmap.height());

    for (int32_t y = 0; y < h; ++y) {
        auto* row = pixmap.row(static_cast<size_t>(y));
        for (int32_t x = 0; x < w; ++x) {
            auto& p = row[x];
            if (p.a == 0) continue;

            float mid = static_cast<float>(p.a) * 0.5f;

            auto apply = [mid, factor](uint8_t val, uint8_t alpha) -> uint8_t {
                float v = mid + (static_cast<float>(val) - mid) * factor;
                return static_cast<uint8_t>(std::clamp(v + 0.5f, 0.0f, static_cast<float>(alpha)));
            };

            p.r = apply(p.r, p.a);
            p.g = apply(p.g, p.a);
            p.b = apply(p.b, p.a);
        }
    }
}

void ColorFilter::sepia(PixmapMut& pixmap) noexcept {
    int32_t w = static_cast<int32_t>(pixmap.width());
    int32_t h = static_cast<int32_t>(pixmap.height());

    for (int32_t y = 0; y < h; ++y) {
        auto* row = pixmap.row(static_cast<size_t>(y));
        for (int32_t x = 0; x < w; ++x) {
            auto& p = row[x];
            if (p.a == 0) continue;

            float r = static_cast<float>(p.r);
            float g = static_cast<float>(p.g);
            float b = static_cast<float>(p.b);
            float a = static_cast<float>(p.a);

            float nr = 0.393f * r + 0.769f * g + 0.189f * b;
            float ng = 0.349f * r + 0.686f * g + 0.168f * b;
            float nb = 0.272f * r + 0.534f * g + 0.131f * b;

            p.r = static_cast<uint8_t>(std::min(nr, a));
            p.g = static_cast<uint8_t>(std::min(ng, a));
            p.b = static_cast<uint8_t>(std::min(nb, a));
        }
    }
}

void ColorFilter::tint(PixmapMut& pixmap, Color tint_color, float intensity) noexcept {
    if (intensity <= 0.0f) return;
    intensity = std::clamp(intensity, 0.0f, 1.0f);

    auto tc = tint_color.to_color_u8();
    int32_t w = static_cast<int32_t>(pixmap.width());
    int32_t h = static_cast<int32_t>(pixmap.height());

    for (int32_t y = 0; y < h; ++y) {
        auto* row = pixmap.row(static_cast<size_t>(y));
        for (int32_t x = 0; x < w; ++x) {
            auto& p = row[x];
            if (p.a == 0) continue;

            float af = static_cast<float>(p.a) / 255.0f;
            float target_r = static_cast<float>(tc.red()) * af;
            float target_g = static_cast<float>(tc.green()) * af;
            float target_b = static_cast<float>(tc.blue()) * af;

            p.r = static_cast<uint8_t>(p.r + (target_r - p.r) * intensity + 0.5f);
            p.g = static_cast<uint8_t>(p.g + (target_g - p.g) * intensity + 0.5f);
            p.b = static_cast<uint8_t>(p.b + (target_b - p.b) * intensity + 0.5f);
        }
    }
}

void ColorFilter::apply_matrix(PixmapMut& pixmap, const ColorMatrix& matrix) noexcept {
    matrix.apply(pixmap);
}

} // namespace nisaba::effects

