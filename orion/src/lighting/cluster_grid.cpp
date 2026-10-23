#include "orion/lighting/cluster_grid.hpp"
#include <cmath>
#include <algorithm>

namespace orion {

static inline bool test_sphere_aabb(const Vec3& center, float radius, const AABB& aabb) noexcept {
    float dist_sq = 0.0f;
    for (int i = 0; i < 3; ++i) {
        float p = (i == 0) ? center.x : ((i == 1) ? center.y : center.z);
        float bmin = (i == 0) ? aabb.min_point.x : ((i == 1) ? aabb.min_point.y : aabb.min_point.z);
        float bmax = (i == 0) ? aabb.max_point.x : ((i == 1) ? aabb.max_point.y : aabb.max_point.z);
        if (p < bmin) {
            float d = bmin - p;
            dist_sq += d * d;
        } else if (p > bmax) {
            float d = p - bmax;
            dist_sq += d * d;
        }
    }
    return dist_sq <= (radius * radius);
}

ClusterGrid::ClusterGrid(uint32_t dim_x, uint32_t dim_y, uint32_t dim_z)
    : m_dim_x(dim_x), m_dim_y(dim_y), m_dim_z(dim_z) {
    const size_t total = static_cast<size_t>(m_dim_x) * m_dim_y * m_dim_z;
    m_cluster_aabbs.resize(total);
    m_cells.resize(total);
    m_cluster_temp_lists.resize(total);
    for (auto& list : m_cluster_temp_lists) {
        list.reserve(32);
    }
    m_light_indices.reserve(total * 8);
    m_global_lights.reserve(8);
}

void ClusterGrid::build_frustum_grid(
    float fov_y_rad,
    float aspect_ratio,
    float z_near,
    float z_far
) noexcept {
    m_z_near = std::max(1e-3f, z_near);
    m_z_far = std::max(m_z_near + 1.0f, z_far);
    m_log_factor = static_cast<float>(m_dim_z) / std::log(m_z_far / m_z_near);

    const float tan_half_y = std::tan(fov_y_rad * 0.5f);
    const float tan_half_x = tan_half_y * aspect_ratio;

    for (uint32_t k = 0; k < m_dim_z; ++k) {
        const float t_k0 = static_cast<float>(k) / static_cast<float>(m_dim_z);
        const float t_k1 = static_cast<float>(k + 1) / static_cast<float>(m_dim_z);

        const float z0 = m_z_near * std::pow(m_z_far / m_z_near, t_k0);
        const float z1 = m_z_near * std::pow(m_z_far / m_z_near, t_k1);

        for (uint32_t j = 0; j < m_dim_y; ++j) {
            const float v0 = static_cast<float>(j) / static_cast<float>(m_dim_y);
            const float v1 = static_cast<float>(j + 1) / static_cast<float>(m_dim_y);

            const float y_slope0 = (-1.0f + 2.0f * v0) * tan_half_y;
            const float y_slope1 = (-1.0f + 2.0f * v1) * tan_half_y;

            for (uint32_t i = 0; i < m_dim_x; ++i) {
                const float u0 = static_cast<float>(i) / static_cast<float>(m_dim_x);
                const float u1 = static_cast<float>(i + 1) / static_cast<float>(m_dim_x);

                const float x_slope0 = (-1.0f + 2.0f * u0) * tan_half_x;
                const float x_slope1 = (-1.0f + 2.0f * u1) * tan_half_x;

                const uint32_t cluster_idx = i + j * m_dim_x + k * (m_dim_x * m_dim_y);

                // View space: Looking down -Z axis
                Vec3 p[8] = {
                    Vec3(x_slope0 * z0, y_slope0 * z0, -z0),
                    Vec3(x_slope1 * z0, y_slope0 * z0, -z0),
                    Vec3(x_slope0 * z0, y_slope1 * z0, -z0),
                    Vec3(x_slope1 * z0, y_slope1 * z0, -z0),
                    Vec3(x_slope0 * z1, y_slope0 * z1, -z1),
                    Vec3(x_slope1 * z1, y_slope0 * z1, -z1),
                    Vec3(x_slope0 * z1, y_slope1 * z1, -z1),
                    Vec3(x_slope1 * z1, y_slope1 * z1, -z1)
                };

                Vec3 min_pt = p[0];
                Vec3 max_pt = p[0];
                for (int pt = 1; pt < 8; ++pt) {
                    min_pt = min_pt.min(p[pt]);
                    max_pt = max_pt.max(p[pt]);
                }

                m_cluster_aabbs[cluster_idx] = AABB(min_pt, max_pt);
            }
        }
    }
}

void ClusterGrid::cull_lights(
    const std::vector<Light3D>& lights,
    const Mat44& view_matrix
) noexcept {
    const size_t total_clusters = m_cells.size();
    for (size_t c = 0; c < total_clusters; ++c) {
        m_cluster_temp_lists[c].clear();
    }
    m_global_lights.clear();
    m_light_indices.clear();

    const uint32_t num_lights = static_cast<uint32_t>(lights.size());

    for (uint32_t l = 0; l < num_lights; ++l) {
        const auto& light = lights[l];

        if (light.type == LightType::Directional) {
            m_global_lights.push_back(l);
            continue;
        }

        // Transform light position to camera view space
        const Vec3 light_pos_view = view_matrix.transform_point(light.position);
        const float range = light.range;

        // View space depth is -z
        const float depth = -light_pos_view.z;
        if (depth + range < m_z_near || depth - range > m_z_far) {
            continue;
        }

        const float min_depth = std::max(m_z_near, depth - range);
        const float max_depth = std::min(m_z_far, depth + range);

        const uint32_t k_min = static_cast<uint32_t>(std::clamp(
            static_cast<int>(std::floor(std::log(min_depth / m_z_near) * m_log_factor)),
            0, static_cast<int>(m_dim_z - 1)
        ));
        const uint32_t k_max = static_cast<uint32_t>(std::clamp(
            static_cast<int>(std::floor(std::log(max_depth / m_z_near) * m_log_factor)),
            0, static_cast<int>(m_dim_z - 1)
        ));

        for (uint32_t k = k_min; k <= k_max; ++k) {
            for (uint32_t j = 0; j < m_dim_y; ++j) {
                for (uint32_t i = 0; i < m_dim_x; ++i) {
                    const uint32_t cluster_idx = i + j * m_dim_x + k * (m_dim_x * m_dim_y);
                    if (test_sphere_aabb(light_pos_view, range, m_cluster_aabbs[cluster_idx])) {
                        m_cluster_temp_lists[cluster_idx].push_back(l);
                    }
                }
            }
        }
    }

    // Flatten into contiguous light index buffer
    for (size_t c = 0; c < total_clusters; ++c) {
        const uint32_t offset = static_cast<uint32_t>(m_light_indices.size());
        
        // Add global directional lights first
        for (uint32_t gl : m_global_lights) {
            m_light_indices.push_back(gl);
        }
        // Add cluster-specific local lights
        for (uint32_t ll : m_cluster_temp_lists[c]) {
            m_light_indices.push_back(ll);
        }

        const uint32_t count = static_cast<uint32_t>(m_light_indices.size() - offset);
        m_cells[c] = ClusterCell{offset, count};
    }
}

uint32_t ClusterGrid::get_cluster_index(
    float screen_u,
    float screen_v,
    float view_depth
) const noexcept {
    uint32_t x = 0, y = 0, z = 0;
    get_cluster_coords(screen_u, screen_v, view_depth, x, y, z);
    return x + y * m_dim_x + z * (m_dim_x * m_dim_y);
}

void ClusterGrid::get_cluster_coords(
    float screen_u,
    float screen_v,
    float view_depth,
    uint32_t& out_x,
    uint32_t& out_y,
    uint32_t& out_z
) const noexcept {
    const float u = std::clamp(screen_u, 0.0f, 0.99999f);
    const float v = std::clamp(screen_v, 0.0f, 0.99999f);
    out_x = static_cast<uint32_t>(u * static_cast<float>(m_dim_x));
    out_y = static_cast<uint32_t>(v * static_cast<float>(m_dim_y));

    const float d = std::clamp(view_depth, m_z_near, m_z_far);
    const int z_idx = static_cast<int>(std::floor(std::log(d / m_z_near) * m_log_factor));
    out_z = static_cast<uint32_t>(std::clamp(z_idx, 0, static_cast<int>(m_dim_z - 1)));
}

const AABB& ClusterGrid::get_cluster_aabb(uint32_t index) const noexcept {
    return m_cluster_aabbs[index];
}

const ClusterCell& ClusterGrid::get_cell(uint32_t index) const noexcept {
    return m_cells[index];
}

std::span<const uint32_t> ClusterGrid::get_cluster_lights(uint32_t index) const noexcept {
    if (index >= m_cells.size()) {
        return {};
    }
    const auto& cell = m_cells[index];
    if (cell.count == 0 || cell.offset >= m_light_indices.size()) {
        return {};
    }
    return std::span<const uint32_t>(&m_light_indices[cell.offset], cell.count);
}

} // namespace orion
