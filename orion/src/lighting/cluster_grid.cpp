#include "orion/lighting/cluster_grid.hpp"
#include "orion/math/simd_math.hpp"
#include <cmath>
#include <cstring>
#include <algorithm>

namespace orion {

ClusterGrid::ClusterGrid(uint32_t dim_x, uint32_t dim_y, uint32_t dim_z)
    : m_dim_x(dim_x), m_dim_y(dim_y), m_dim_z(dim_z) {
    const size_t total = static_cast<size_t>(m_dim_x) * m_dim_y * m_dim_z;
    const size_t padded_total = (total + 3) & ~size_t(3);
    m_cluster_aabbs.resize(total);
    m_soa_min_x.resize(padded_total, 0.0f);
    m_soa_min_y.resize(padded_total, 0.0f);
    m_soa_min_z.resize(padded_total, 0.0f);
    m_soa_max_x.resize(padded_total, 0.0f);
    m_soa_max_y.resize(padded_total, 0.0f);
    m_soa_max_z.resize(padded_total, 0.0f);
    m_slice_z0.resize(m_dim_z, 0.1f);
    m_slice_z1.resize(m_dim_z, 100.0f);

    m_cells.resize(total);
    m_cluster_counts.resize(total, 0);
    m_cluster_light_slots.resize(total * MAX_CLUSTER_LOCAL_LIGHTS, 0);
    m_active_clusters.reserve(total);
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

    const float tan_half_y = std::max(1e-4f, std::tan(fov_y_rad * 0.5f));
    const float tan_half_x = std::max(1e-4f, tan_half_y * aspect_ratio);
    m_tan_half_x = tan_half_x;
    m_tan_half_y = tan_half_y;

    for (uint32_t k = 0; k < m_dim_z; ++k) {
        const float t_k0 = static_cast<float>(k) / static_cast<float>(m_dim_z);
        const float t_k1 = static_cast<float>(k + 1) / static_cast<float>(m_dim_z);

        const float z0 = m_z_near * std::pow(m_z_far / m_z_near, t_k0);
        const float z1 = m_z_near * std::pow(m_z_far / m_z_near, t_k1);
        m_slice_z0[k] = z0;
        m_slice_z1[k] = z1;

        for (uint32_t j = 0; j < m_dim_y; ++j) {
            const float v0 = static_cast<float>(j) / static_cast<float>(m_dim_y);
            const float v1 = static_cast<float>(j + 1) / static_cast<float>(m_dim_y);

            const float y_slope0 = (-1.0f + 2.0f * v0) * tan_half_y;
            const float y_slope1 = (-1.0f + 2.0f * v1) * tan_half_y;

            const float min_y = std::min(y_slope0 * z0, y_slope0 * z1);
            const float max_y = std::max(y_slope1 * z0, y_slope1 * z1);

            for (uint32_t i = 0; i < m_dim_x; ++i) {
                const float u0 = static_cast<float>(i) / static_cast<float>(m_dim_x);
                const float u1 = static_cast<float>(i + 1) / static_cast<float>(m_dim_x);

                const float x_slope0 = (-1.0f + 2.0f * u0) * tan_half_x;
                const float x_slope1 = (-1.0f + 2.0f * u1) * tan_half_x;

                const float min_x = std::min(x_slope0 * z0, x_slope0 * z1);
                const float max_x = std::max(x_slope1 * z0, x_slope1 * z1);
                const float min_z = -z1;
                const float max_z = -z0;

                const uint32_t cluster_idx = i + j * m_dim_x + k * (m_dim_x * m_dim_y);

                m_cluster_aabbs[cluster_idx] = AABB(Vec3(min_x, min_y, min_z), Vec3(max_x, max_y, max_z));
                m_soa_min_x[cluster_idx] = min_x;
                m_soa_min_y[cluster_idx] = min_y;
                m_soa_min_z[cluster_idx] = min_z;
                m_soa_max_x[cluster_idx] = max_x;
                m_soa_max_y[cluster_idx] = max_y;
                m_soa_max_z[cluster_idx] = max_z;
            }
        }
    }
}

void ClusterGrid::cull_lights(
    const std::vector<Light3D>& lights,
    const Mat44& view_matrix
) noexcept {
    m_global_lights.clear();
    m_light_indices.clear();

    const uint32_t num_lights = static_cast<uint32_t>(lights.size());
    const float inv_2tan_x = 0.5f / m_tan_half_x;
    const float inv_2tan_y = 0.5f / m_tan_half_y;
    const float f_dim_x = static_cast<float>(m_dim_x);
    const float f_dim_y = static_cast<float>(m_dim_y);
    const uint32_t slice_stride = m_dim_x * m_dim_y;

    for (uint32_t l = 0; l < num_lights; ++l) {
        if (lights[l].type == LightType::Directional) {
            m_global_lights.push_back(l);
        }
    }
    const uint32_t global_count = static_cast<uint32_t>(m_global_lights.size());
    for (uint32_t gl : m_global_lights) {
        m_light_indices.push_back(gl);
    }

    const ClusterCell default_cell{0u, global_count};
    if (m_last_global_count != global_count) {
        std::fill(m_cells.begin(), m_cells.end(), default_cell);
        m_last_global_count = global_count;
    } else {
        for (uint32_t active_idx : m_active_clusters) {
            m_cells[active_idx] = default_cell;
        }
    }
    for (uint32_t active_idx : m_active_clusters) {
        m_cluster_counts[active_idx] = 0;
    }
    m_active_clusters.clear();

    uint32_t total_local_indices = 0;

    for (uint32_t l = 0; l < num_lights; ++l) {
        const auto& light = lights[l];

        if (light.type == LightType::Directional) {
            continue;
        }

        // Transform light position to camera view space
        const Vec3 light_pos_view = view_matrix.transform_point(light.position);
        const float range = light.range;
        const float range_sq = range * range;

        // View space depth is -z
        const float depth = -light_pos_view.z;
        if (depth + range < m_z_near || depth - range > m_z_far) {
            continue;
        }

        const float min_depth = std::max(m_z_near, depth - range);
        const float max_depth = std::min(m_z_far, depth + range);

        uint32_t k_min = 0;
        while (k_min + 1 < m_dim_z && m_slice_z1[k_min] < min_depth) {
            ++k_min;
        }
        uint32_t k_max = k_min;
        while (k_max + 1 < m_dim_z && m_slice_z0[k_max + 1] <= max_depth) {
            ++k_max;
        }

        const float lx = light_pos_view.x;
        const float ly = light_pos_view.y;
        const float lz = light_pos_view.z;

        for (uint32_t k = k_min; k <= k_max; ++k) {
            const float z0 = m_slice_z0[k];
            const float z1 = m_slice_z1[k];

            // Precompute Z distance squared for the entire slice k!
            float dz = 0.0f;
            const float min_z = -z1;
            const float max_z = -z0;
            if (lz < min_z) dz = min_z - lz;
            else if (lz > max_z) dz = lz - max_z;
            const float r_slice_sq = range_sq - dz * dz;
            if (r_slice_sq < 0.0f) continue;

            const float r_slice = std::sqrt(r_slice_sq);
            const float ly_min = ly - r_slice;
            const float ly_max = ly + r_slice;

            // Compute exact Y slope & tile range [j_min, j_max] for slice k using slice-reduced radius
            const float slope_y0 = ly_min / ((ly_min > 0.0f) ? z1 : z0);
            const float slope_y1 = ly_max / ((ly_max > 0.0f) ? z0 : z1);
            const float v_min = slope_y0 * inv_2tan_y + 0.5f;
            const float v_max = slope_y1 * inv_2tan_y + 0.5f;
            if (v_max < 0.0f || v_min > 1.0f) continue;

            const uint32_t j_min = static_cast<uint32_t>(std::clamp(static_cast<int>(std::floor(v_min * f_dim_y)), 0, static_cast<int>(m_dim_y - 1)));
            const uint32_t j_max = static_cast<uint32_t>(std::clamp(static_cast<int>(std::floor(v_max * f_dim_y)), 0, static_cast<int>(m_dim_y - 1)));

            const uint32_t k_offset = k * slice_stride;
            for (uint32_t j = j_min; j <= j_max; ++j) {
                const uint32_t row_base = k_offset + j * m_dim_x;
                // All tiles i in row j of slice k share identical Y bounds!
                const float r_min_y = m_soa_min_y[row_base];
                const float r_max_y = m_soa_max_y[row_base];
                float dy = 0.0f;
                if (ly < r_min_y) dy = r_min_y - ly;
                else if (ly > r_max_y) dy = ly - r_max_y;
                const float rem_r2 = r_slice_sq - dy * dy;
                if (rem_r2 < 0.0f) continue;

                // Exact X tile interval [i_min, i_max] for this row's remaining radius!
                const float rx = std::sqrt(rem_r2);
                const float lx_min = lx - rx;
                const float lx_max = lx + rx;
                const float slope_x0 = lx_min / ((lx_min > 0.0f) ? z1 : z0);
                const float slope_x1 = lx_max / ((lx_max > 0.0f) ? z0 : z1);
                const float u_min = slope_x0 * inv_2tan_x + 0.5f;
                const float u_max = slope_x1 * inv_2tan_x + 0.5f;
                if (u_max < 0.0f || u_min > 1.0f) continue;

                const uint32_t i_min = static_cast<uint32_t>(std::clamp(static_cast<int>(std::floor(u_min * f_dim_x)), 0, static_cast<int>(m_dim_x - 1)));
                const uint32_t i_max = static_cast<uint32_t>(std::clamp(static_cast<int>(std::floor(u_max * f_dim_x)), 0, static_cast<int>(m_dim_x - 1)));

                for (uint32_t i = i_min; i <= i_max; ++i) {
                    const uint32_t c_idx = row_base + i;
                    float dx = 0.0f;
                    const float bmin_x = m_soa_min_x[c_idx];
                    const float bmax_x = m_soa_max_x[c_idx];
                    if (lx < bmin_x) dx = bmin_x - lx;
                    else if (lx > bmax_x) dx = lx - bmax_x;
                    if (dx * dx <= rem_r2) {
                        const uint16_t cnt = m_cluster_counts[c_idx];
                        if (cnt == 0) {
                            m_active_clusters.push_back(c_idx);
                        }
                        if (cnt < MAX_CLUSTER_LOCAL_LIGHTS) {
                            m_cluster_light_slots[c_idx * MAX_CLUSTER_LOCAL_LIGHTS + cnt] = l;
                            m_cluster_counts[c_idx] = cnt + 1;
                            ++total_local_indices;
                        }
                    }
                }
            }
        }
    }

    const size_t active_cluster_count = m_active_clusters.size();
    m_light_indices.resize(global_count + active_cluster_count * global_count + total_local_indices);
    uint32_t write_offset = global_count;

    for (uint32_t c : m_active_clusters) {
        const uint16_t local_cnt = m_cluster_counts[c];
        if (local_cnt == 0) continue;

        const uint32_t cell_offset = write_offset;
        for (uint32_t gl : m_global_lights) {
            m_light_indices[write_offset++] = gl;
        }
        const uint32_t* slot_ptr = &m_cluster_light_slots[c * MAX_CLUSTER_LOCAL_LIGHTS];
        std::memcpy(&m_light_indices[write_offset], slot_ptr, static_cast<size_t>(local_cnt) * sizeof(uint32_t));
        write_offset += local_cnt;
        m_cells[c] = ClusterCell{cell_offset, global_count + local_cnt};
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
