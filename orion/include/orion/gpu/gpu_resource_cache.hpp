#pragma once

#include "orion/geometry/mesh.hpp"
#include "orion/material/texture.hpp"
#include "orion/material/pbr_material.hpp"
#include "orion/math/mat44.hpp"
#include "orion/math/vec3.hpp"
#include "orion/lighting/light.hpp"
#include "orion/ibl/environment_map.hpp"
#include <memory>
#include <unordered_map>
#include <cstdint>

namespace orion {


struct alignas(16) GpuCameraUboBlock {
    float view[16]{0};
    float projection[16]{0};
    float view_projection[16]{0};
    float camera_position[3]{0};
    float padding{0};
};

static_assert(sizeof(GpuCameraUboBlock) == 208, "GpuCameraUboBlock must be 208 bytes for std140");
static_assert(alignof(GpuCameraUboBlock) == 16, "GpuCameraUboBlock must be 16-byte aligned");

struct GpuMeshHandle {
    uint32_t vao{0};
    uint32_t vbo{0};
    uint32_t ebo{0};
    uint32_t index_count{0};
    bool is_valid{false};
};

struct GpuTextureHandle {
    uint32_t texture_id{0};
    bool is_valid{false};
};

/// @brief Sovereign GPU resource cache managing automated uploading and lifetime tracking
/// of 3D polygonal meshes, PBR textures, and std140 uniform buffer objects.
class GpuResourceCache {
public:
    GpuResourceCache();
    ~GpuResourceCache();

    GpuResourceCache(const GpuResourceCache&) = delete;
    GpuResourceCache& operator=(const GpuResourceCache&) = delete;

    /// @brief Retrieves or lazily uploads a 3D mesh to GPU memory (VAO/VBO/EBO).
    [[nodiscard]] GpuMeshHandle get_or_upload_mesh(const Mesh3D& mesh);

    /// @brief Dynamically updates GPU vertex buffer data for dynamic and skinned meshes via glBufferSubData.
    void update_mesh_buffer(const Mesh3D& mesh);

    /// @brief Retrieves or lazily uploads a 2D texture to GPU memory.
    [[nodiscard]] GpuTextureHandle get_or_upload_texture(const Texture2D& texture);

    /// @brief Updates and binds the std140 Camera uniform buffer to binding point 0.
    void update_camera_ubo(const Mat44& view, const Mat44& proj, const Vec3& eye_pos);

    /// @brief Updates and binds the std140 Lighting uniform buffer to binding point 1.
    void update_lighting_ubo(const GpuLightingUboBlock& lighting_block);

    /// @brief Updates and binds the std140 Material uniform buffer to binding point 2.
    void update_material_ubo(const PbrMaterialUniforms& material_uniforms);

    /// @brief Releases all GPU resources and clears the caches.
    void clear() noexcept;

    /// @brief Retrieves or lazily uploads the precomputed 2D Split-Sum DFG LUT texture.
    [[nodiscard]] uint32_t get_or_create_dfg_lut();

    /// @brief Retrieves or uploads an HDR environment radiance cubemap with all mip levels.
    [[nodiscard]] uint32_t get_or_upload_environment_map(const EnvironmentMap& env_map);

    [[nodiscard]] uint32_t default_white_texture() const noexcept { return m_default_white_tex; }
    [[nodiscard]] uint32_t default_normal_texture() const noexcept { return m_default_normal_tex; }
    [[nodiscard]] uint32_t default_black_texture() const noexcept { return m_default_black_tex; }
    [[nodiscard]] uint32_t dfg_lut_texture() const noexcept { return m_dfg_lut_tex; }

private:
    std::unordered_map<const Mesh3D*, GpuMeshHandle> m_mesh_cache;
    std::unordered_map<const Texture2D*, GpuTextureHandle> m_texture_cache;
    std::unordered_map<const EnvironmentMap*, uint32_t> m_cubemap_cache;

    uint32_t m_camera_ubo{0};
    uint32_t m_lighting_ubo{0};
    uint32_t m_material_ubo{0};
    uint32_t m_default_white_tex{0};
    uint32_t m_default_normal_tex{0};
    uint32_t m_default_black_tex{0};
    uint32_t m_dfg_lut_tex{0};

    void init_ubos();
    void init_default_textures();
};

} // namespace orion
