#pragma once

#include "nisaba/physics/math/vec3.hpp"
#include <cstdint>
#include <vector>
#include <memory>
#include <span>
#include <string>

namespace nisaba {
class Pixmap;
}

namespace nisaba::render3d {

using Vec3 = nisaba::physics::Vec3;

struct Vec4 {
    float x{0.0f}, y{0.0f}, z{0.0f}, w{1.0f};

    constexpr Vec4() noexcept = default;
    constexpr Vec4(float in_x, float in_y, float in_z, float in_w = 1.0f) noexcept
        : x(in_x), y(in_y), z(in_z), w(in_w) {}
    constexpr explicit Vec4(const Vec3& v, float in_w = 1.0f) noexcept
        : x(v.x), y(v.y), z(v.z), w(in_w) {}

    [[nodiscard]] constexpr Vec3 xyz() const noexcept { return Vec3(x, y, z); }
};

/// @brief Host and GPU texture buffer supporting bilinear sampling, format conversion,
/// and seamless integration with the sovereign image_io subsystem.
class Texture2D {
public:
    uint32_t width{0};
    uint32_t height{0};
    uint32_t channels{4}; // 1 = R8, 3 = RGB8, 4 = RGBA8
    std::vector<uint8_t> pixels;

    Texture2D() = default;
    Texture2D(uint32_t w, uint32_t h, uint32_t ch = 4);

    void set_pixels(const void* data, size_t size);

    /// @brief Samples the texture using normalized [0, 1] UV coordinates with repeat wrapping and bilinear filtering.
    [[nodiscard]] Vec4 sample_rgba(float u, float v) const noexcept;
    [[nodiscard]] Vec3 sample_rgb(float u, float v) const noexcept;
    [[nodiscard]] float sample_r(float u, float v) const noexcept;

    /// @brief Creates a 1x1 or custom solid color texture.
    [[nodiscard]] static std::shared_ptr<Texture2D> create_solid(
        const Vec4& color,
        uint32_t w = 1,
        uint32_t h = 1
    );

    /// @brief Constructs Texture2D from an existing sovereign Pixmap.
    [[nodiscard]] static std::shared_ptr<Texture2D> from_pixmap(const nisaba::Pixmap& pixmap);

    /// @brief Decodes image from memory (PNG, JPEG, WebP, QOI) via image_io.
    [[nodiscard]] static std::shared_ptr<Texture2D> load_from_memory(std::span<const uint8_t> data);

    /// @brief Loads image from file path via image_io.
    [[nodiscard]] static std::shared_ptr<Texture2D> load_from_file(const std::string& filepath);
};

} // namespace nisaba::render3d
