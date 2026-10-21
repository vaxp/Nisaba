#pragma once

#include "nisaba/physics/math/simd_math.hpp"
#include "nisaba/physics/math/vec3.hpp"
#include "nisaba/physics/math/aabb.hpp"
#include "nisaba/physics/math/ray.hpp"

namespace nisaba::physics {

/// @brief Represents 4 Axis-Aligned Bounding Boxes in Structure of Arrays (SoA) layout.
/// Used by the 4-way SIMD Dynamic BVH tree to evaluate overlap and raycasts across
/// 4 child nodes simultaneously in parallel SIMD registers.
struct alignas(16) AABB4 {
    // 4 Min points (X, Y, Z)
    alignas(16) float min_x[4] = { LARGE_FLOAT, LARGE_FLOAT, LARGE_FLOAT, LARGE_FLOAT };
    alignas(16) float min_y[4] = { LARGE_FLOAT, LARGE_FLOAT, LARGE_FLOAT, LARGE_FLOAT };
    alignas(16) float min_z[4] = { LARGE_FLOAT, LARGE_FLOAT, LARGE_FLOAT, LARGE_FLOAT };

    // 4 Max points (X, Y, Z)
    alignas(16) float max_x[4] = {-LARGE_FLOAT,-LARGE_FLOAT,-LARGE_FLOAT,-LARGE_FLOAT };
    alignas(16) float max_y[4] = {-LARGE_FLOAT,-LARGE_FLOAT,-LARGE_FLOAT,-LARGE_FLOAT };
    alignas(16) float max_z[4] = {-LARGE_FLOAT,-LARGE_FLOAT,-LARGE_FLOAT,-LARGE_FLOAT };

    constexpr AABB4() noexcept = default;

    /// @brief Set one of the 4 bounding boxes (index 0..3)
    void set_box(int index, const AABB& aabb) noexcept {
        min_x[index] = aabb.min_point.x;
        min_y[index] = aabb.min_point.y;
        min_z[index] = aabb.min_point.z;

        max_x[index] = aabb.max_point.x;
        max_y[index] = aabb.max_point.y;
        max_z[index] = aabb.max_point.z;
    }

    /// @brief Retrieve one of the 4 bounding boxes as an AABB
    [[nodiscard]] AABB get_box(int index) const noexcept {
        return AABB(
            Vec3(min_x[index], min_y[index], min_z[index]),
            Vec3(max_x[index], max_y[index], max_z[index])
        );
    }

    /// @brief Tests a query AABB against all 4 boxes simultaneously.
    /// @return 4-bit bitmask where bit i is 1 if box i overlaps query_aabb.
    [[nodiscard]] uint32_t test_overlap(const AABB& query_aabb) const noexcept {
#if defined(NISABA_PHYSICS_SIMD_SSE)
        const __m128 q_min_x = _mm_set1_ps(query_aabb.min_point.x);
        const __m128 q_min_y = _mm_set1_ps(query_aabb.min_point.y);
        const __m128 q_min_z = _mm_set1_ps(query_aabb.min_point.z);

        const __m128 q_max_x = _mm_set1_ps(query_aabb.max_point.x);
        const __m128 q_max_y = _mm_set1_ps(query_aabb.max_point.y);
        const __m128 q_max_z = _mm_set1_ps(query_aabb.max_point.z);

        const __m128 b_min_x = _mm_load_ps(min_x);
        const __m128 b_min_y = _mm_load_ps(min_y);
        const __m128 b_min_z = _mm_load_ps(min_z);

        const __m128 b_max_x = _mm_load_ps(max_x);
        const __m128 b_max_y = _mm_load_ps(max_y);
        const __m128 b_max_z = _mm_load_ps(max_z);

        // Overlap condition: (q_min <= b_max) && (q_max >= b_min) across all 3 axes
        const __m128 test_x = _mm_and_ps(_mm_cmple_ps(q_min_x, b_max_x), _mm_cmpge_ps(q_max_x, b_min_x));
        const __m128 test_y = _mm_and_ps(_mm_cmple_ps(q_min_y, b_max_y), _mm_cmpge_ps(q_max_y, b_min_y));
        const __m128 test_z = _mm_and_ps(_mm_cmple_ps(q_min_z, b_max_z), _mm_cmpge_ps(q_max_z, b_min_z));

        const __m128 overlap = _mm_and_ps(_mm_and_ps(test_x, test_y), test_z);
        return static_cast<uint32_t>(_mm_movemask_ps(overlap));
#elif defined(NISABA_PHYSICS_SIMD_NEON)
        const float32x4_t q_min_x = vdupq_n_f32(query_aabb.min_point.x);
        const float32x4_t q_min_y = vdupq_n_f32(query_aabb.min_point.y);
        const float32x4_t q_min_z = vdupq_n_f32(query_aabb.min_point.z);

        const float32x4_t q_max_x = vdupq_n_f32(query_aabb.max_point.x);
        const float32x4_t q_max_y = vdupq_n_f32(query_aabb.max_point.y);
        const float32x4_t q_max_z = vdupq_n_f32(query_aabb.max_point.z);

        const float32x4_t b_min_x = vld1q_f32(min_x);
        const float32x4_t b_min_y = vld1q_f32(min_y);
        const float32x4_t b_min_z = vld1q_f32(min_z);

        const float32x4_t b_max_x = vld1q_f32(max_x);
        const float32x4_t b_max_y = vld1q_f32(max_y);
        const float32x4_t b_max_z = vld1q_f32(max_z);

        const uint32x4_t test_x = vandq_u32(vcleq_f32(q_min_x, b_max_x), vcgeq_f32(q_max_x, b_min_x));
        const uint32x4_t test_y = vandq_u32(vcleq_f32(q_min_y, b_max_y), vcgeq_f32(q_max_y, b_min_y));
        const uint32x4_t test_z = vandq_u32(vcleq_f32(q_min_z, b_max_z), vcgeq_f32(q_max_z, b_min_z));

        const uint32x4_t overlap = vandq_u32(vandq_u32(test_x, test_y), test_z);

        uint32_t mask = 0;
        if (vgetq_lane_u32(overlap, 0)) mask |= (1 << 0);
        if (vgetq_lane_u32(overlap, 1)) mask |= (1 << 1);
        if (vgetq_lane_u32(overlap, 2)) mask |= (1 << 2);
        if (vgetq_lane_u32(overlap, 3)) mask |= (1 << 3);
        return mask;
#else
        uint32_t mask = 0;
        for (int i = 0; i < 4; ++i) {
            if (query_aabb.min_point.x <= max_x[i] && query_aabb.max_point.x >= min_x[i] &&
                query_aabb.min_point.y <= max_y[i] && query_aabb.max_point.y >= min_y[i] &&
                query_aabb.min_point.z <= max_z[i] && query_aabb.max_point.z >= min_z[i]) {
                mask |= (1 << i);
            }
        }
        return mask;
#endif
    }

    /// @brief Tests a Ray against all 4 boxes simultaneously using the Kay-Kajiya slab algorithm in SIMD.
    /// @param ray Query ray
    /// @param max_t Maximum distance along ray
    /// @param out_hit_t Output entry distances for all 4 boxes
    /// @return 4-bit bitmask where bit i is 1 if ray intersects box i within [0, max_t]
    [[nodiscard]] uint32_t test_ray(const Ray& ray, float max_t, float out_hit_t[4]) const noexcept {
#if defined(NISABA_PHYSICS_SIMD_SSE)
        const __m128 r_orig_x = _mm_set1_ps(ray.origin.x);
        const __m128 r_orig_y = _mm_set1_ps(ray.origin.y);
        const __m128 r_orig_z = _mm_set1_ps(ray.origin.z);

        const __m128 r_inv_x = _mm_set1_ps(ray.inv_direction.x);
        const __m128 r_inv_y = _mm_set1_ps(ray.inv_direction.y);
        const __m128 r_inv_z = _mm_set1_ps(ray.inv_direction.z);

        const __m128 zero = _mm_setzero_ps();
        const __m128 ray_max = _mm_set1_ps(max_t);

        // Slab X
        __m128 t1_x = _mm_mul_ps(_mm_sub_ps(_mm_load_ps(min_x), r_orig_x), r_inv_x);
        __m128 t2_x = _mm_mul_ps(_mm_sub_ps(_mm_load_ps(max_x), r_orig_x), r_inv_x);
        __m128 t_min_x = _mm_min_ps(t1_x, t2_x);
        __m128 t_max_x = _mm_max_ps(t1_x, t2_x);

        // Slab Y
        __m128 t1_y = _mm_mul_ps(_mm_sub_ps(_mm_load_ps(min_y), r_orig_y), r_inv_y);
        __m128 t2_y = _mm_mul_ps(_mm_sub_ps(_mm_load_ps(max_y), r_orig_y), r_inv_y);
        __m128 t_min_y = _mm_min_ps(t1_y, t2_y);
        __m128 t_max_y = _mm_max_ps(t1_y, t2_y);

        // Slab Z
        __m128 t1_z = _mm_mul_ps(_mm_sub_ps(_mm_load_ps(min_z), r_orig_z), r_inv_z);
        __m128 t2_z = _mm_mul_ps(_mm_sub_ps(_mm_load_ps(max_z), r_orig_z), r_inv_z);
        __m128 t_min_z = _mm_min_ps(t1_z, t2_z);
        __m128 t_max_z = _mm_max_ps(t1_z, t2_z);

        // Intersection of intervals
        __m128 t_enter = _mm_max_ps(_mm_max_ps(t_min_x, t_min_y), t_min_z);
        __m128 t_exit  = _mm_min_ps(_mm_min_ps(t_max_x, t_max_y), t_max_z);

        // Clamped entry distance: max(0, t_enter)
        __m128 t_hit = _mm_max_ps(zero, t_enter);

        // Hit condition: (t_enter <= t_exit) && (t_hit <= ray_max)
        __m128 hit_cond = _mm_and_ps(_mm_cmple_ps(t_enter, t_exit), _mm_cmple_ps(t_hit, ray_max));

        _mm_store_ps(out_hit_t, t_hit);
        return static_cast<uint32_t>(_mm_movemask_ps(hit_cond));
#else
        uint32_t mask = 0;
        for (int i = 0; i < 4; ++i) {
            float t1x = (min_x[i] - ray.origin.x) * ray.inv_direction.x;
            float t2x = (max_x[i] - ray.origin.x) * ray.inv_direction.x;
            float tmin_x = std::min(t1x, t2x);
            float tmax_x = std::max(t1x, t2x);

            float t1y = (min_y[i] - ray.origin.y) * ray.inv_direction.y;
            float t2y = (max_y[i] - ray.origin.y) * ray.inv_direction.y;
            float tmin_y = std::min(t1y, t2y);
            float tmax_y = std::max(t1y, t2y);

            float t1z = (min_z[i] - ray.origin.z) * ray.inv_direction.z;
            float t2z = (max_z[i] - ray.origin.z) * ray.inv_direction.z;
            float tmin_z = std::min(t1z, t2z);
            float tmax_z = std::max(t1z, t2z);

            float t_enter = std::max({tmin_x, tmin_y, tmin_z});
            float t_exit = std::min({tmax_x, tmax_y, tmax_z});

            float t_hit = std::max(0.0f, t_enter);
            out_hit_t[i] = t_hit;

            if (t_enter <= t_exit && t_hit <= max_t) {
                mask |= (1 << i);
            }
        }
        return mask;
#endif
    }
};

} // namespace nisaba::physics
