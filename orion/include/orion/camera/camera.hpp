#pragma once

#include "orion/math/vec3.hpp"
#include "orion/math/mat44.hpp"
#include "orion/math/ray.hpp"
#include "orion/camera/frustum.hpp"
#include <cmath>

namespace orion {


enum class ProjectionType : uint8_t {
    Perspective = 0,
    Orthographic = 1
};

/// @brief High-performance 3D Camera with cached transformations, Gribb-Hartmann frustum culling,
/// and Vulkan-first clip-space depth conventions. Zero heap allocation.
class alignas(16) Camera3D {
public:
    Camera3D() noexcept;

    // --- Configuration ---

    /// @brief Sets view orientation using eye, target, and world up.
    void set_look_at(const Vec3& eye, const Vec3& target, const Vec3& up = Vec3(0.0f, 1.0f, 0.0f)) noexcept;

    /// @brief Moves camera position without changing target.
    void set_position(const Vec3& eye) noexcept;

    /// @brief Changes look-at target without changing camera position.
    void set_target(const Vec3& target) noexcept;

    /// @brief Sets camera world up vector.
    void set_up(const Vec3& up) noexcept;

    /// @brief Configures perspective projection.
    /// @param fov_y_rad Vertical field of view in radians.
    /// @param aspect Viewport aspect ratio (width / height).
    /// @param z_near Near clipping plane distance (> 0).
    /// @param z_far Far clipping plane distance (> z_near).
    /// @param vulkan_depth True for Vulkan Z in [0, 1], false for OpenGL Z in [-1, 1].
    void set_perspective(
        float fov_y_rad,
        float aspect,
        float z_near,
        float z_far,
        bool vulkan_depth = true
    ) noexcept;

    /// @brief Configures orthographic projection.
    void set_orthographic(
        float left,
        float right,
        float bottom,
        float top,
        float z_near,
        float z_far,
        bool vulkan_depth = true
    ) noexcept;

    /// @brief Updates viewport aspect ratio while preserving existing FOV and clipping planes.
    void set_aspect_ratio(float aspect) noexcept;

    // --- Getters ---

    [[nodiscard]] const Vec3& get_position() const noexcept { return m_position; }
    [[nodiscard]] const Vec3& get_target() const noexcept { return m_target; }
    [[nodiscard]] const Vec3& get_up() const noexcept { return m_up; }

    /// @brief Normalized direction vector pointing from eye to target.
    [[nodiscard]] Vec3 get_forward() const noexcept;

    /// @brief Normalized right vector perpendicular to forward and up.
    [[nodiscard]] Vec3 get_right() const noexcept;

    /// @brief Normalized true up vector perpendicular to right and forward.
    [[nodiscard]] Vec3 get_camera_up() const noexcept;

    [[nodiscard]] float get_near_clip() const noexcept;
    [[nodiscard]] float get_far_clip() const noexcept;
    [[nodiscard]] float get_fov_y() const noexcept { return m_fov_y; }
    [[nodiscard]] float get_aspect_ratio() const noexcept { return m_aspect; }
    [[nodiscard]] ProjectionType get_projection_type() const noexcept { return m_proj_type; }
    [[nodiscard]] bool is_vulkan_depth() const noexcept { return m_vulkan_depth; }

    // --- Matrix & Culling Accessors (Cached & SIMD Accelerated) ---

    /// @brief Retrieves the 4x4 View matrix (recomputed lazily if dirty).
    [[nodiscard]] const Mat44& get_view_matrix() const noexcept;

    /// @brief Retrieves the 4x4 Projection matrix (recomputed lazily if dirty).
    [[nodiscard]] const Mat44& get_projection_matrix() const noexcept;

    /// @brief Retrieves the combined View-Projection matrix (VP = P * V).
    [[nodiscard]] const Mat44& get_view_projection_matrix() const noexcept;

    /// @brief Retrieves the inverse View-Projection matrix (VP^-1) for unprojection and raycasting.
    [[nodiscard]] const Mat44& get_inverse_view_projection_matrix() const noexcept;

    /// @brief Retrieves the cached Gribb-Hartmann 6-plane viewing frustum.
    [[nodiscard]] const Frustum& get_frustum() const noexcept;

    /// @brief Unprojects 2D screen viewport coordinates into a 3D world space Ray (for mouse picking).
    /// @param screen_x X pixel coordinate in viewport [0, viewport_w].
    /// @param screen_y Y pixel coordinate in viewport [0, viewport_h].
    /// @param viewport_w Viewport width in pixels.
    /// @param viewport_h Viewport height in pixels.
    [[nodiscard]] Ray screen_point_to_ray(
        float screen_x,
        float screen_y,
        float viewport_w,
        float viewport_h
    ) const noexcept;

private:
    void invalidate_view() noexcept;
    void invalidate_projection() noexcept;
    void update_view() const noexcept;
    void update_projection() const noexcept;
    void update_view_projection() const noexcept;

    Vec3 m_position{0.0f, 0.0f, 5.0f};
    Vec3 m_target{0.0f, 0.0f, 0.0f};
    Vec3 m_up{0.0f, 1.0f, 0.0f};

    ProjectionType m_proj_type{ProjectionType::Perspective};
    float m_fov_y{60.0f * (3.14159265358979323846f / 180.0f)};
    float m_aspect{16.0f / 9.0f};
    float m_z_near{0.1f};
    float m_z_far{1000.0f};

    float m_ortho_left{-10.0f};
    float m_ortho_right{10.0f};
    float m_ortho_bottom{-10.0f};
    float m_ortho_top{10.0f};
    float m_ortho_near{0.1f};
    float m_ortho_far{1000.0f};

    bool m_vulkan_depth{true};

    // Lazily evaluated caches
    mutable bool m_view_dirty{true};
    mutable bool m_proj_dirty{true};
    mutable bool m_vp_dirty{true};
    mutable bool m_inv_vp_dirty{true};
    mutable bool m_frustum_dirty{true};

    mutable Mat44 m_view_matrix{};
    mutable Mat44 m_proj_matrix{};
    mutable Mat44 m_vp_matrix{};
    mutable Mat44 m_inv_vp_matrix{};
    mutable Frustum m_frustum{};
};

} // namespace orion
