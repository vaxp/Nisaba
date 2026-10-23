#include "orion/ibl/environment_map.hpp"
#include <cmath>
#include <algorithm>

namespace orion {

EnvironmentMap::EnvironmentMap(uint32_t base_resolution)
    : m_base_resolution(base_resolution) {
    m_mip_levels.resize(NUM_MIP_LEVELS);
    uint32_t res = m_base_resolution;

    for (size_t m = 0; m < NUM_MIP_LEVELS; ++m) {
        m_mip_levels[m].resolution = res;
        m_mip_levels[m].roughness = static_cast<float>(m) / static_cast<float>(NUM_MIP_LEVELS - 1);
        for (int f = 0; f < 6; ++f) {
            m_mip_levels[m].faces[f].resize(res * res * 4, 0.0f);
        }
        res = std::max(1u, res / 2u);
    }
}

Vec3 EnvironmentMap::face_uv_to_direction(CubeFace face, float u, float v) noexcept {
    Vec3 dir(0.0f, 0.0f, 0.0f);
    switch (face) {
        case CubeFace::PositiveX: dir = Vec3( 1.0f, -v,    -u);    break;
        case CubeFace::NegativeX: dir = Vec3(-1.0f, -v,     u);    break;
        case CubeFace::PositiveY: dir = Vec3( u,     1.0f,  v);    break;
        case CubeFace::NegativeY: dir = Vec3( u,    -1.0f, -v);    break;
        case CubeFace::PositiveZ: dir = Vec3( u,    -v,     1.0f); break;
        case CubeFace::NegativeZ: dir = Vec3(-u,    -v,    -1.0f); break;
    }
    return dir.normalized();
}

void EnvironmentMap::direction_to_face_uv(
    const Vec3& dir,
    CubeFace& out_face,
    float& out_u,
    float& out_v
) noexcept {
    const float abs_x = std::abs(dir.x);
    const float abs_y = std::abs(dir.y);
    const float abs_z = std::abs(dir.z);

    if (abs_x >= abs_y && abs_x >= abs_z) {
        if (dir.x > 0.0f) {
            out_face = CubeFace::PositiveX;
            out_u = -dir.z / abs_x;
            out_v = -dir.y / abs_x;
        } else {
            out_face = CubeFace::NegativeX;
            out_u = dir.z / abs_x;
            out_v = -dir.y / abs_x;
        }
    } else if (abs_y >= abs_x && abs_y >= abs_z) {
        if (dir.y > 0.0f) {
            out_face = CubeFace::PositiveY;
            out_u = dir.x / abs_y;
            out_v = dir.z / abs_y;
        } else {
            out_face = CubeFace::NegativeY;
            out_u = dir.x / abs_y;
            out_v = -dir.z / abs_y;
        }
    } else {
        if (dir.z > 0.0f) {
            out_face = CubeFace::PositiveZ;
            out_u = dir.x / abs_z;
            out_v = -dir.y / abs_z;
        } else {
            out_face = CubeFace::NegativeZ;
            out_u = -dir.x / abs_z;
            out_v = -dir.y / abs_z;
        }
    }
}

std::shared_ptr<EnvironmentMap> EnvironmentMap::create_procedural_sky(
    const Vec3& sun_direction,
    const Vec3& sun_color,
    float sun_intensity,
    uint32_t resolution
) noexcept {
    auto env = std::make_shared<EnvironmentMap>(resolution);
    auto& mip0 = env->m_mip_levels[0];
    const Vec3 sun_dir = sun_direction.normalized();

    // Atmospheric scattering model
    const Vec3 zenith_color(0.18f, 0.42f, 0.88f);
    const Vec3 horizon_color(0.68f, 0.78f, 0.90f);
    const Vec3 ground_horizon(0.35f, 0.33f, 0.32f);
    const Vec3 ground_nadir(0.12f, 0.11f, 0.10f);

    for (int f = 0; f < 6; ++f) {
        const CubeFace face = static_cast<CubeFace>(f);
        auto& pixels = mip0.faces[f];

        for (uint32_t y = 0; y < resolution; ++y) {
            const float v = -1.0f + 2.0f * (static_cast<float>(y) + 0.5f) / static_cast<float>(resolution);

            for (uint32_t x = 0; x < resolution; ++x) {
                const float u = -1.0f + 2.0f * (static_cast<float>(x) + 0.5f) / static_cast<float>(resolution);

                const Vec3 D = face_uv_to_direction(face, u, v);
                Vec3 color(0.0f, 0.0f, 0.0f);

                if (D.y >= 0.0f) {
                    // Sky hemisphere
                    const float height_factor = std::pow(D.y, 0.45f);
                    Vec3 sky = Vec3::lerp(horizon_color, zenith_color, height_factor);

                    // Sun phase function (Mie forward scattering)
                    const float cos_gamma = std::clamp(D.dot(sun_dir), -1.0f, 1.0f);
                    constexpr float g = 0.76f;
                    constexpr float g2 = g * g;
                    const float mie_phase = (1.0f - g2) / std::pow(1.0f + g2 - 2.0f * g * cos_gamma, 1.5f);
                    const Vec3 sun_glow = sun_color * (sun_intensity * 0.06f * mie_phase);

                    // Solar disk
                    float sun_disk = 0.0f;
                    if (cos_gamma > 0.9985f) {
                        sun_disk = 15.0f * sun_intensity;
                    }

                    color = sky + sun_glow + sun_color * sun_disk;
                } else {
                    // Ground hemisphere
                    const float ground_factor = std::pow(-D.y, 0.5f);
                    color = Vec3::lerp(ground_horizon, ground_nadir, ground_factor);
                }

                const size_t idx = (y * resolution + x) * 4;
                pixels[idx + 0] = color.x;
                pixels[idx + 1] = color.y;
                pixels[idx + 2] = color.z;
                pixels[idx + 3] = 1.0f;
            }
        }
    }

    env->generate_prefiltered_mipmaps();
    return env;
}

void EnvironmentMap::generate_prefiltered_mipmaps() noexcept {
    // Generate filtered LODs across all mip levels
    for (size_t m = 1; m < m_mip_levels.size(); ++m) {
        const auto& prev_level = m_mip_levels[m - 1];
        auto& curr_level = m_mip_levels[m];
        const uint32_t prev_res = prev_level.resolution;
        const uint32_t curr_res = curr_level.resolution;

        for (int f = 0; f < 6; ++f) {
            const auto& src = prev_level.faces[f];
            auto& dst = curr_level.faces[f];

            for (uint32_t y = 0; y < curr_res; ++y) {
                for (uint32_t x = 0; x < curr_res; ++x) {
                    // 2x2 box filter averaging
                    const uint32_t sx0 = std::min(x * 2, prev_res - 1);
                    const uint32_t sx1 = std::min(x * 2 + 1, prev_res - 1);
                    const uint32_t sy0 = std::min(y * 2, prev_res - 1);
                    const uint32_t sy1 = std::min(y * 2 + 1, prev_res - 1);

                    const size_t idx00 = (sy0 * prev_res + sx0) * 4;
                    const size_t idx10 = (sy0 * prev_res + sx1) * 4;
                    const size_t idx01 = (sy1 * prev_res + sx0) * 4;
                    const size_t idx11 = (sy1 * prev_res + sx1) * 4;

                    Vec3 avg(
                        (src[idx00] + src[idx10] + src[idx01] + src[idx11]) * 0.25f,
                        (src[idx00 + 1] + src[idx10 + 1] + src[idx01 + 1] + src[idx11 + 1]) * 0.25f,
                        (src[idx00 + 2] + src[idx10 + 2] + src[idx01 + 2] + src[idx11 + 2]) * 0.25f
                    );

                    const size_t dst_idx = (y * curr_res + x) * 4;
                    dst[dst_idx + 0] = avg.x;
                    dst[dst_idx + 1] = avg.y;
                    dst[dst_idx + 2] = avg.z;
                    dst[dst_idx + 3] = 1.0f;
                }
            }
        }
    }
}

Vec3 EnvironmentMap::sample_radiance(const Vec3& direction, float roughness) const noexcept {
    if (m_mip_levels.empty()) {
        return Vec3(0.0f, 0.0f, 0.0f);
    }

    CubeFace face = CubeFace::PositiveX;
    float u = 0.0f, v = 0.0f;
    direction_to_face_uv(direction.normalized(), face, u, v);

    // Map roughness to continuous mip level
    const float max_mip = static_cast<float>(m_mip_levels.size() - 1);
    const float mip_float = std::clamp(roughness * max_mip, 0.0f, max_mip);
    const size_t mip_idx = static_cast<size_t>(std::floor(mip_float));

    const auto& level = m_mip_levels[mip_idx];
    const uint32_t res = level.resolution;

    // Convert [-1, 1] to texel coordinates [0, res - 1]
    const float px = (u * 0.5f + 0.5f) * static_cast<float>(res - 1);
    const float py = (v * 0.5f + 0.5f) * static_cast<float>(res - 1);

    const uint32_t x = std::clamp(static_cast<uint32_t>(px), 0u, res - 1);
    const uint32_t y = std::clamp(static_cast<uint32_t>(py), 0u, res - 1);

    const auto& pixels = level.faces[static_cast<int>(face)];
    const size_t idx = (y * res + x) * 4;

    return Vec3(pixels[idx + 0], pixels[idx + 1], pixels[idx + 2]);
}

SphericalHarmonics9 EnvironmentMap::compute_sh9(size_t sample_count) const noexcept {
    SphericalHarmonics9 sh;
    const float inv_samples = 1.0f / static_cast<float>(sample_count);

    for (size_t i = 0; i < sample_count; ++i) {
        // Uniform sphere sampling
        const float u1 = static_cast<float>(i) * inv_samples;
        const float u2 = (std::sqrt(5.0f) * 0.5f - 0.5f) * static_cast<float>(i); // Fibonacci sequence
        const float fract_u2 = u2 - std::floor(u2);

        const float z = 1.0f - 2.0f * u1;
        const float r = std::sqrt(std::max(0.0f, 1.0f - z * z));
        const float phi = 2.0f * PI * fract_u2;
        const float x = r * std::cos(phi);
        const float y = r * std::sin(phi);

        const Vec3 dir(x, y, z);
        const Vec3 rad = sample_radiance(dir, 0.0f);

        // Basis functions
        // Y00 = 0.282095
        sh.coefficients[0] = sh.coefficients[0] + rad * 0.282095f;
        // Y1-1 = 0.488603 * y
        sh.coefficients[1] = sh.coefficients[1] + rad * (0.488603f * y);
        // Y10 = 0.488603 * z
        sh.coefficients[2] = sh.coefficients[2] + rad * (0.488603f * z);
        // Y11 = 0.488603 * x
        sh.coefficients[3] = sh.coefficients[3] + rad * (0.488603f * x);
        // Y2-2 = 1.092548 * x * y
        sh.coefficients[4] = sh.coefficients[4] + rad * (1.092548f * x * y);
        // Y2-1 = 1.092548 * y * z
        sh.coefficients[5] = sh.coefficients[5] + rad * (1.092548f * y * z);
        // Y20 = 0.315392 * (3z^2 - 1)
        sh.coefficients[6] = sh.coefficients[6] + rad * (0.315392f * (3.0f * z * z - 1.0f));
        // Y21 = 1.092548 * x * z
        sh.coefficients[7] = sh.coefficients[7] + rad * (1.092548f * x * z);
        // Y22 = 0.546274 * (x^2 - y^2)
        sh.coefficients[8] = sh.coefficients[8] + rad * (0.546274f * (x * x - y * y));
    }

    // Multiply by solid angle (4*pi / N)
    const float weight = (4.0f * PI) * inv_samples;
    sh.scale(weight);

    return sh;
}

} // namespace orion
