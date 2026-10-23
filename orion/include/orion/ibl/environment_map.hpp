#pragma once

#include "orion/math/vec3.hpp"
#include "orion/ibl/spherical_harmonics.hpp"
#include <vector>
#include <memory>
#include <cstdint>

namespace orion {


enum class CubeFace : uint32_t {
    PositiveX = 0,
    NegativeX = 1,
    PositiveY = 2,
    NegativeY = 3,
    PositiveZ = 4,
    NegativeZ = 5
};

/// @brief Pre-filtered radiance mip level for rough specular reflection sampling.
struct CubemapMipLevel {
    uint32_t resolution{0};
    float roughness{0.0f};
    // 6 faces, each face contains resolution * resolution * 4 floats (RGBA)
    std::vector<float> faces[6];
};

/// @brief High Dynamic Range (HDR) radiance cubemap managing multi-LOD roughness mipmaps,
/// procedural sky dome generation, and analytical Spherical Harmonics projection.
class EnvironmentMap {
public:
    static constexpr uint32_t DEFAULT_CUBEMAP_RES = 128;
    static constexpr uint32_t NUM_MIP_LEVELS = 6;

    EnvironmentMap(uint32_t base_resolution = DEFAULT_CUBEMAP_RES);
    ~EnvironmentMap() = default;

    /// @brief Creates an analytical physically-based procedural sky dome environment.
    [[nodiscard]] static std::shared_ptr<EnvironmentMap> create_procedural_sky(
        const Vec3& sun_direction,
        const Vec3& sun_color = Vec3(1.0f, 0.95f, 0.9f),
        float sun_intensity = 4.0f,
        uint32_t resolution = DEFAULT_CUBEMAP_RES
    ) noexcept;

    /// @brief Generates roughness pre-filtered mipmap levels (levels 0 to NUM_MIP_LEVELS-1).
    void generate_prefiltered_mipmaps() noexcept;

    /// @brief Projects the environment radiance into a 3-Band (9 coefficients) Spherical Harmonics field.
    [[nodiscard]] SphericalHarmonics9 compute_sh9(size_t sample_count = 2048) const noexcept;

    /// @brief Samples the environment radiance on CPU for a given world direction and perceptual roughness.
    [[nodiscard]] Vec3 sample_radiance(const Vec3& direction, float roughness = 0.0f) const noexcept;

    [[nodiscard]] uint32_t base_resolution() const noexcept { return m_base_resolution; }
    [[nodiscard]] size_t mip_levels_count() const noexcept { return m_mip_levels.size(); }
    [[nodiscard]] const CubemapMipLevel& get_mip_level(size_t level) const noexcept { return m_mip_levels[level]; }

    /// @brief Converts normalized cube face (face, u, v) in [-1, 1] to normalized 3D direction vector.
    [[nodiscard]] static Vec3 face_uv_to_direction(CubeFace face, float u, float v) noexcept;

    /// @brief Converts a 3D unit direction vector to corresponding cube face and UV [-1, 1].
    static void direction_to_face_uv(const Vec3& dir, CubeFace& out_face, float& out_u, float& out_v) noexcept;

private:
    uint32_t m_base_resolution{DEFAULT_CUBEMAP_RES};
    std::vector<CubemapMipLevel> m_mip_levels;
};

} // namespace orion
