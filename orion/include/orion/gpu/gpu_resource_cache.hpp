#pragma once

#include "orion/geometry/mesh.hpp"
#include "orion/material/texture.hpp"
#include "orion/material/pbr_material.hpp"
#include "orion/math/mat44.hpp"
#include "orion/math/vec3.hpp"
#include "orion/lighting/light.hpp"
#include "orion/lighting/cluster_grid.hpp"
#include "orion/ibl/environment_map.hpp"
#include "orion/animation/skeleton.hpp"
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

struct alignas(16) GpuSkinningUboBlock {
    static constexpr size_t MAX_BONES = 128;
    float bone_matrices[MAX_BONES * 16]{0};
};

static_assert(sizeof(GpuSkinningUboBlock) == 8192, "GpuSkinningUboBlock must be 8192 bytes for std140");

struct alignas(16) GpuInstanceTransform {
    float model[16]{0};
    float normal_matrix[16]{0};
};

struct alignas(16) GpuInstancingUboBlock {
    static constexpr size_t MAX_INSTANCES = 128;
    GpuInstanceTransform instances[MAX_INSTANCES]{};
};

static_assert(sizeof(GpuInstancingUboBlock) == 16384, "GpuInstancingUboBlock must be 16384 bytes for std140");

struct GpuMeshHandle {
    uint32_t vao{0};
    uint32_t vbo{0};
    uint32_t skin_vbo{0};
    uint32_t ebo{0};
    uint32_t index_count{0};
    bool has_skinning{false};
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

    /// @brief Updates and binds the std140 Skeletal Skinning bone palette UBO to binding point 3.
    void update_skinning_ubo(const Skeleton& skeleton);

    /// @brief Updates and binds the std140 Hardware Instancing transform UBO to binding point 4.
    void update_instancing_ubo(const GpuInstanceTransform* instances, size_t instance_count);

    /// @brief Uploads Froxel ClusterGrid cells and light index lists into GPU lookup textures.
    void update_cluster_textures(const ClusterGrid& grid);

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
    [[nodiscard]] uint32_t cluster_cells_texture() const noexcept { return m_cluster_cells_tex; }
    [[nodiscard]] uint32_t cluster_indices_texture() const noexcept { return m_cluster_indices_tex; }

private:
    std::unordered_map<const Mesh3D*, GpuMeshHandle> m_mesh_cache;
    std::unordered_map<const Texture2D*, GpuTextureHandle> m_texture_cache;
    std::unordered_map<const EnvironmentMap*, uint32_t> m_cubemap_cache;

    uint32_t m_camera_ubo{0};
    uint32_t m_lighting_ubo{0};
    uint32_t m_material_ubo{0};
    uint32_t m_skinning_ubo{0};
    uint32_t m_instancing_ubo{0};
    uint32_t m_default_white_tex{0};
    uint32_t m_default_normal_tex{0};
    uint32_t m_default_black_tex{0};
    uint32_t m_dfg_lut_tex{0};
    uint32_t m_cluster_cells_tex{0};
    uint32_t m_cluster_indices_tex{0};
    std::vector<float> m_cluster_cells_scratch;
    std::vector<float> m_cluster_indices_scratch;

    void init_ubos();
    void init_default_textures();
};

} // namespace orion
