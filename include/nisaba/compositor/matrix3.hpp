#pragma once

#include "nisaba/compositor/geometry.hpp"

#include <array>
#include <cmath>
#include <cstddef>
#include <cstring>
#include <wayland-server-protocol.h>

namespace nisaba::compositor {

/**
 * @brief Inverts a Wayland output transform.
 */
inline constexpr wl_output_transform invertTransform(wl_output_transform tr) noexcept {
    if ((tr & WL_OUTPUT_TRANSFORM_90) && !(tr & WL_OUTPUT_TRANSFORM_FLIPPED)) {
        tr = static_cast<wl_output_transform>(tr ^ WL_OUTPUT_TRANSFORM_180);
    }
    return tr;
}

/**
 * @brief Helper to compose two Wayland output transforms.
 */
inline wl_output_transform composeTransforms(wl_output_transform trA, wl_output_transform trB) noexcept {
    uint32_t flipped = (trA ^ trB) & WL_OUTPUT_TRANSFORM_FLIPPED;
    uint32_t rotMask = WL_OUTPUT_TRANSFORM_90 | WL_OUTPUT_TRANSFORM_180;
    uint32_t rotated;
    if (trB & WL_OUTPUT_TRANSFORM_FLIPPED) {
        rotated = (trB - trA) & rotMask;
    } else {
        rotated = (trA + trB) & rotMask;
    }
    return static_cast<wl_output_transform>(flipped | rotated);
}

/**
 * @brief Sovereign 3x3 Affine Transformation Matrix for 2D compositor projection and mapping.
 * Conforms to OpenGL column-major delivery for row-vector GLSL pipeline: pos3 * proj.
 */
class Matrix3 {
public:
    std::array<float, 9> m{
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f
    };

    constexpr Matrix3() noexcept = default;

    constexpr explicit Matrix3(const std::array<float, 9>& elements) noexcept
        : m(elements) {}

    static constexpr Matrix3 identity() noexcept {
        return Matrix3{};
    }

    [[nodiscard]] const float* data() const noexcept { return m.data(); }
    [[nodiscard]] float* data() noexcept { return m.data(); }

    [[nodiscard]] float operator[](size_t i) const noexcept { return m[i]; }
    [[nodiscard]] float& operator[](size_t i) noexcept { return m[i]; }

    /**
     * @brief Row-by-column matrix multiplication: this = this * other.
     */
    Matrix3& multiply(const Matrix3& other) noexcept {
        std::array<float, 9> product{};

        product[0] = m[0]*other.m[3*0+0] + m[1]*other.m[3*1+0] + m[2]*other.m[3*2+0];
        product[1] = m[0]*other.m[3*0+1] + m[1]*other.m[3*1+1] + m[2]*other.m[3*2+1];
        product[2] = m[0]*other.m[3*0+2] + m[1]*other.m[3*1+2] + m[2]*other.m[3*2+2];

        product[3] = m[3]*other.m[3*0+0] + m[4]*other.m[3*1+0] + m[5]*other.m[3*2+0];
        product[4] = m[3]*other.m[3*0+1] + m[4]*other.m[3*1+1] + m[5]*other.m[3*2+1];
        product[5] = m[3]*other.m[3*0+2] + m[4]*other.m[3*1+2] + m[5]*other.m[3*2+2];

        product[6] = m[6]*other.m[3*0+0] + m[7]*other.m[3*1+0] + m[8]*other.m[3*2+0];
        product[7] = m[6]*other.m[3*0+1] + m[7]*other.m[3*1+1] + m[8]*other.m[3*2+1];
        product[8] = m[6]*other.m[3*0+2] + m[7]*other.m[3*1+2] + m[8]*other.m[3*2+2];

        m = product;
        return *this;
    }

    [[nodiscard]] Matrix3 operator*(const Matrix3& other) const noexcept {
        Matrix3 res = *this;
        res.multiply(other);
        return res;
    }

    Matrix3& translate(float x, float y) noexcept {
        const Matrix3 t({
            1.0f, 0.0f, x,
            0.0f, 1.0f, y,
            0.0f, 0.0f, 1.0f
        });
        return multiply(t);
    }

    Matrix3& scale(float sx, float sy) noexcept {
        const Matrix3 s({
            sx,   0.0f, 0.0f,
            0.0f, sy,   0.0f,
            0.0f, 0.0f, 1.0f
        });
        return multiply(s);
    }

    Matrix3& transform(wl_output_transform t) noexcept {
        static constexpr float kTransforms[8][9] = {
            // WL_OUTPUT_TRANSFORM_NORMAL
            {  1.0f,  0.0f,  0.0f,
               0.0f,  1.0f,  0.0f,
               0.0f,  0.0f,  1.0f },
            // WL_OUTPUT_TRANSFORM_90
            {  0.0f,  1.0f,  0.0f,
              -1.0f,  0.0f,  0.0f,
               0.0f,  0.0f,  1.0f },
            // WL_OUTPUT_TRANSFORM_180
            { -1.0f,  0.0f,  0.0f,
               0.0f, -1.0f,  0.0f,
               0.0f,  0.0f,  1.0f },
            // WL_OUTPUT_TRANSFORM_270
            {  0.0f, -1.0f,  0.0f,
               1.0f,  0.0f,  0.0f,
               0.0f,  0.0f,  1.0f },
            // WL_OUTPUT_TRANSFORM_FLIPPED
            { -1.0f,  0.0f,  0.0f,
               0.0f,  1.0f,  0.0f,
               0.0f,  0.0f,  1.0f },
            // WL_OUTPUT_TRANSFORM_FLIPPED_90
            {  0.0f,  1.0f,  0.0f,
               1.0f,  0.0f,  0.0f,
               0.0f,  0.0f,  1.0f },
            // WL_OUTPUT_TRANSFORM_FLIPPED_180
            {  1.0f,  0.0f,  0.0f,
               0.0f, -1.0f,  0.0f,
               0.0f,  0.0f,  1.0f },
            // WL_OUTPUT_TRANSFORM_FLIPPED_270
            {  0.0f, -1.0f,  0.0f,
              -1.0f,  0.0f,  0.0f,
               0.0f,  0.0f,  1.0f }
        };

        const size_t idx = static_cast<size_t>(t) & 7;
        Matrix3 tr;
        std::memcpy(tr.m.data(), kTransforms[idx], sizeof(kTransforms[idx]));
        return multiply(tr);
    }

    /**
     * @brief Computes orthographic projection matrix from (0,0)-(width,height) to NDC.
     */
    static Matrix3 project(int width, int height, wl_output_transform transform = WL_OUTPUT_TRANSFORM_NORMAL) noexcept {
        static constexpr float kTransforms[8][9] = {
            {  1.0f,  0.0f,  0.0f,   0.0f,  1.0f,  0.0f,   0.0f,  0.0f,  1.0f },
            {  0.0f,  1.0f,  0.0f,  -1.0f,  0.0f,  0.0f,   0.0f,  0.0f,  1.0f },
            { -1.0f,  0.0f,  0.0f,   0.0f, -1.0f,  0.0f,   0.0f,  0.0f,  1.0f },
            {  0.0f, -1.0f,  0.0f,   1.0f,  0.0f,  0.0f,   0.0f,  0.0f,  1.0f },
            { -1.0f,  0.0f,  0.0f,   0.0f,  1.0f,  0.0f,   0.0f,  0.0f,  1.0f },
            {  0.0f,  1.0f,  0.0f,   1.0f,  0.0f,  0.0f,   0.0f,  0.0f,  1.0f },
            {  1.0f,  0.0f,  0.0f,   0.0f, -1.0f,  0.0f,   0.0f,  0.0f,  1.0f },
            {  0.0f, -1.0f,  0.0f,  -1.0f,  0.0f,  0.0f,   0.0f,  0.0f,  1.0f }
        };

        const size_t idx = static_cast<size_t>(transform) & 7;
        const float* t = kTransforms[idx];
        const float x = 2.0f / static_cast<float>(width);
        const float y = 2.0f / static_cast<float>(height);

        Matrix3 res{};
        res.m.fill(0.0f);

        res.m[0] = x * t[0];
        res.m[1] = x * t[1];
        res.m[3] = y * -t[3];
        res.m[4] = y * -t[4];

        res.m[2] = -std::copysign(1.0f, res.m[0] + res.m[1]);
        res.m[5] = -std::copysign(1.0f, res.m[3] + res.m[4]);
        res.m[8] = 1.0f;

        return res;
    }

    /**
     * @brief Creates a combined model-view-projection matrix for a rectangular destination box.
     */
    static Matrix3 projectBox(const RectI& box, wl_output_transform transform, const Matrix3& projection) noexcept {
        Matrix3 mat = Matrix3::identity();
        mat.translate(static_cast<float>(box.x), static_cast<float>(box.y));
        mat.scale(static_cast<float>(box.width), static_cast<float>(box.height));

        if (transform != WL_OUTPUT_TRANSFORM_NORMAL) {
            mat.translate(0.5f, 0.5f);
            mat.transform(transform);
            mat.translate(-0.5f, -0.5f);
        }

        return projection * mat;
    }

    /**
     * @brief Creates UV sampling matrix for source rectangle with optional surface transform.
     */
    static Matrix3 projectTexture(const RectF& fbox, wl_output_transform transform) noexcept {
        Matrix3 mat = Matrix3::identity();
        mat.translate(static_cast<float>(fbox.x), static_cast<float>(fbox.y));
        mat.scale(static_cast<float>(fbox.width), static_cast<float>(fbox.height));
        mat.translate(0.5f, 0.5f);

        if (transform & WL_OUTPUT_TRANSFORM_90) {
            mat.transform(invertTransform(transform));
        } else {
            mat.transform(transform);
        }

        mat.translate(-0.5f, -0.5f);
        return mat;
    }
};

} // namespace nisaba::compositor
