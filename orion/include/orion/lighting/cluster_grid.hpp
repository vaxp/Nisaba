#pragma once

#include "orion/math/aabb.hpp"
#include "orion/math/mat44.hpp"
#include "orion/math/vec3.hpp"
#include "orion/lighting/light.hpp"
#include <vector>
#include <span>
#include <cstdint>

namespace orion {


/// @brief Represents a single clustered froxel's offset and count in the flattened light index array.
struct ClusterCell {
    uint32_t offset{0};
    uint32_t count{0};
};

/// @brief Clustered 3D Light Grid (Froxels) dividing the camera viewing frustum into
/// 3D cells (e.g. 16x9x24) with logarithmic depth distribution for O(1) light lookup and culling.
class ClusterGrid {
public:
    static constexpr uint32_t DEFAULT_CLUSTERS_X = 16;
    static constexpr uint32_t DEFAULT_CLUSTERS_Y = 9;
    static constexpr uint32_t DEFAULT_CLUSTERS_Z = 24;
    static constexpr uint32_t TOTAL_CLUSTERS = DEFAULT_CLUSTERS_X * DEFAULT_CLUSTERS_Y * DEFAULT_CLUSTERS_Z; // 3456

    ClusterGrid(
        uint32_t dim_x = DEFAULT_CLUSTERS_X,
        uint32_t dim_y = DEFAULT_CLUSTERS_Y,
        uint32_t dim_z = DEFAULT_CLUSTERS_Z
    );

    /// @brief Pre-calculates cluster bounding boxes in view space based on camera frustum projection.
    void build_frustum_grid(
        float fov_y_rad,
        float aspect_ratio,
        float z_near,
        float z_far
    ) noexcept;

    /// @brief Culls dynamic scene lights against the view-space cluster grid.
    /// Runs with zero steady-state heap allocations.
    void cull_lights(
        const std::vector<Light3D>& lights,
        const Mat44& view_matrix
    ) noexcept;

    /// @brief Retrieves the 1D linear cluster index for a given screen UV [0, 1] and positive view depth.
    [[nodiscard]] uint32_t get_cluster_index(
        float screen_u,
        float screen_v,
        float view_depth
    ) const noexcept;

    /// @brief Retrieves the 3D discrete cluster coordinates (x, y, z) for a given screen UV and positive view depth.
    void get_cluster_coords(
        float screen_u,
        float screen_v,
        float view_depth,
        uint32_t& out_x,
        uint32_t& out_y,
        uint32_t& out_z
    ) const noexcept;

    /// @brief Returns the view-space bounding box for a given cluster.
    [[nodiscard]] const AABB& get_cluster_aabb(uint32_t index) const noexcept;

    /// @brief Returns the cell metadata (offset and light count) for a cluster.
    [[nodiscard]] const ClusterCell& get_cell(uint32_t index) const noexcept;

    /// @brief Returns a slice of light indices active in the specified cluster cell.
    [[nodiscard]] std::span<const uint32_t> get_cluster_lights(uint32_t index) const noexcept;

    /// @brief Returns total active light indices in the flattened buffer across all clusters.
    [[nodiscard]] size_t total_light_indices() const noexcept { return m_light_indices.size(); }

    [[nodiscard]] uint32_t dim_x() const noexcept { return m_dim_x; }
    [[nodiscard]] uint32_t dim_y() const noexcept { return m_dim_y; }
    [[nodiscard]] uint32_t dim_z() const noexcept { return m_dim_z; }
    [[nodiscard]] float z_near() const noexcept { return m_z_near; }
    [[nodiscard]] float z_far() const noexcept { return m_z_far; }

private:
    uint32_t m_dim_x{DEFAULT_CLUSTERS_X};
    uint32_t m_dim_y{DEFAULT_CLUSTERS_Y};
    uint32_t m_dim_z{DEFAULT_CLUSTERS_Z};
    float m_z_near{0.1f};
    float m_z_far{100.0f};
    float m_log_factor{1.0f};

    std::vector<AABB> m_cluster_aabbs;
    std::vector<ClusterCell> m_cells;
    std::vector<uint32_t> m_light_indices;
    std::vector<uint32_t> m_global_lights;

    // Temporary per-cluster light accumulators for zero-allocation reuse
    std::vector<std::vector<uint32_t>> m_cluster_temp_lists;
};

} // namespace orion
