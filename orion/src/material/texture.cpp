#include "orion/material/texture.hpp"
#include "orion/orionimg/orionimg.hpp"
#include <cmath>
#include <algorithm>
#include <cstring>

namespace orion {

Texture2D::Texture2D(uint32_t w, uint32_t h, uint32_t ch)
    : width(w), height(h), channels(ch) {
    pixels.resize(static_cast<size_t>(w) * h * ch, 255);
}

void Texture2D::set_pixels(const void* data, size_t size) {
    pixels.resize(size);
    if (data && size > 0) {
        std::memcpy(pixels.data(), data, size);
    }
}

Vec4 Texture2D::sample_rgba(float u, float v) const noexcept {
    if (width == 0 || height == 0 || pixels.empty()) {
        return Vec4(1.0f, 1.0f, 1.0f, 1.0f);
    }

    // Repeat wrap addressing
    if (u < 0.0f || u > 1.0f) {
        u = u - std::floor(u);
    }
    if (v < 0.0f || v > 1.0f) {
        v = v - std::floor(v);
    }

    const float x = u * static_cast<float>(width > 1 ? width - 1 : 0);
    const float y = v * static_cast<float>(height > 1 ? height - 1 : 0);

    const uint32_t x0 = static_cast<uint32_t>(x);
    const uint32_t y0 = static_cast<uint32_t>(y);
    const uint32_t x1 = std::min(x0 + 1, width - 1);
    const uint32_t y1 = std::min(y0 + 1, height - 1);

    const float fx = x - static_cast<float>(x0);
    const float fy = y - static_cast<float>(y0);

    auto get_pixel = [this](uint32_t px, uint32_t py) noexcept -> Vec4 {
        const size_t idx = (static_cast<size_t>(py) * width + px) * channels;
        if (channels >= 4) {
            return Vec4(
                pixels[idx + 0] / 255.0f,
                pixels[idx + 1] / 255.0f,
                pixels[idx + 2] / 255.0f,
                pixels[idx + 3] / 255.0f
            );
        } else if (channels == 3) {
            return Vec4(
                pixels[idx + 0] / 255.0f,
                pixels[idx + 1] / 255.0f,
                pixels[idx + 2] / 255.0f,
                1.0f
            );
        } else if (channels == 1) {
            const float val = pixels[idx] / 255.0f;
            return Vec4(val, val, val, 1.0f);
        }
        return Vec4(1.0f, 1.0f, 1.0f, 1.0f);
    };

    const Vec4 c00 = get_pixel(x0, y0);
    const Vec4 c10 = get_pixel(x1, y0);
    const Vec4 c01 = get_pixel(x0, y1);
    const Vec4 c11 = get_pixel(x1, y1);

    const float r0 = c00.x * (1.0f - fx) + c10.x * fx;
    const float g0 = c00.y * (1.0f - fx) + c10.y * fx;
    const float b0 = c00.z * (1.0f - fx) + c10.z * fx;
    const float a0 = c00.w * (1.0f - fx) + c10.w * fx;

    const float r1 = c01.x * (1.0f - fx) + c11.x * fx;
    const float g1 = c01.y * (1.0f - fx) + c11.y * fx;
    const float b1 = c01.z * (1.0f - fx) + c11.z * fx;
    const float a1 = c01.w * (1.0f - fx) + c11.w * fx;

    return Vec4(
        r0 * (1.0f - fy) + r1 * fy,
        g0 * (1.0f - fy) + g1 * fy,
        b0 * (1.0f - fy) + b1 * fy,
        a0 * (1.0f - fy) + a1 * fy
    );
}

Vec3 Texture2D::sample_rgb(float u, float v) const noexcept {
    return sample_rgba(u, v).xyz();
}

float Texture2D::sample_r(float u, float v) const noexcept {
    return sample_rgba(u, v).x;
}

std::shared_ptr<Texture2D> Texture2D::create_solid(const Vec4& color, uint32_t w, uint32_t h) {
    auto tex = std::make_shared<Texture2D>(w, h, 4);
    const uint8_t r = static_cast<uint8_t>(std::clamp(color.x, 0.0f, 1.0f) * 255.0f);
    const uint8_t g = static_cast<uint8_t>(std::clamp(color.y, 0.0f, 1.0f) * 255.0f);
    const uint8_t b = static_cast<uint8_t>(std::clamp(color.z, 0.0f, 1.0f) * 255.0f);
    const uint8_t a = static_cast<uint8_t>(std::clamp(color.w, 0.0f, 1.0f) * 255.0f);

    const size_t total_pixels = static_cast<size_t>(w) * h;
    for (size_t i = 0; i < total_pixels; ++i) {
        tex->pixels[i * 4 + 0] = r;
        tex->pixels[i * 4 + 1] = g;
        tex->pixels[i * 4 + 2] = b;
        tex->pixels[i * 4 + 3] = a;
    }
    return tex;
}

std::shared_ptr<Texture2D> Texture2D::from_image(const orionimg::Image& image) {
    const uint32_t w = image.width();
    const uint32_t h = image.height();
    auto tex = std::make_shared<Texture2D>(w, h, 4);

    const uint8_t* src_data = image.data();
    const size_t stride = image.stride_bytes();

    for (uint32_t y = 0; y < h; ++y) {
        const uint8_t* row_src = src_data + y * stride;
        uint8_t* row_dst = tex->pixels.data() + y * w * 4;

        if (image.format() == orionimg::PixelFormat::RGBA8888) {
            std::memcpy(row_dst, row_src, w * 4);
        } else if (image.format() == orionimg::PixelFormat::BGRA8888) {
            for (uint32_t x = 0; x < w; ++x) {
                row_dst[x * 4 + 0] = row_src[x * 4 + 2]; // R
                row_dst[x * 4 + 1] = row_src[x * 4 + 1]; // G
                row_dst[x * 4 + 2] = row_src[x * 4 + 0]; // B
                row_dst[x * 4 + 3] = row_src[x * 4 + 3]; // A
            }
        } else {
            auto ref = image.as_ref();
            for (uint32_t x = 0; x < w; ++x) {
                auto p = ref.pixel(x, y);
                if (p.has_value()) {
                    row_dst[x * 4 + 0] = p->red();
                    row_dst[x * 4 + 1] = p->green();
                    row_dst[x * 4 + 2] = p->blue();
                    row_dst[x * 4 + 3] = p->alpha();
                }
            }
        }
    }
    return tex;
}

std::shared_ptr<Texture2D> Texture2D::load_from_memory(std::span<const uint8_t> data) {
    auto result = orionimg::load_image_from_memory(data);
    if (!result.has_value() || !result.value.has_value()) {
        std::cerr << "[Texture2D] load_image_from_memory failed! error: " << (int)result.error
                  << " detected_format: " << (int)orionimg::detect_format(data)
                  << " data_len: " << data.size() << std::endl;
        return nullptr;
    }
    return from_image(result.value.value());
}

std::shared_ptr<Texture2D> Texture2D::load_from_file(const std::string& filepath) {
    auto result = orionimg::load_image_file(filepath);
    if (!result.has_value() || !result.value.has_value()) {
        return nullptr;
    }
    return from_image(result.value.value());
}

} // namespace orion
