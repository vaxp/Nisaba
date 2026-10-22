#include "nisaba/render3d/camera/camera.hpp"
#include <algorithm>
#include <cmath>

namespace nisaba::render3d {

Camera3D::Camera3D() noexcept {
    invalidate_view();
    invalidate_projection();
}

void Camera3D::set_look_at(const Vec3& eye, const Vec3& target, const Vec3& up) noexcept {
    m_position = eye;
    m_target = target;
    m_up = up;
    invalidate_view();
}

void Camera3D::set_position(const Vec3& eye) noexcept {
    m_position = eye;
    invalidate_view();
}

void Camera3D::set_target(const Vec3& target) noexcept {
    m_target = target;
    invalidate_view();
}

void Camera3D::set_up(const Vec3& up) noexcept {
    m_up = up;
    invalidate_view();
}

void Camera3D::set_perspective(
    float fov_y_rad,
    float aspect,
    float z_near,
    float z_far,
    bool vulkan_depth
) noexcept {
    m_proj_type = ProjectionType::Perspective;
    m_fov_y = fov_y_rad;
    m_aspect = aspect;
    m_z_near = z_near;
    m_z_far = z_far;
    m_vulkan_depth = vulkan_depth;
    invalidate_projection();
}

void Camera3D::set_orthographic(
    float left,
    float right,
    float bottom,
    float top,
    float z_near,
    float z_far,
    bool vulkan_depth
) noexcept {
    m_proj_type = ProjectionType::Orthographic;
    m_ortho_left = left;
    m_ortho_right = right;
    m_ortho_bottom = bottom;
    m_ortho_top = top;
    m_ortho_near = z_near;
    m_ortho_far = z_far;
    m_vulkan_depth = vulkan_depth;
    invalidate_projection();
}

void Camera3D::set_aspect_ratio(float aspect) noexcept {
    if (std::abs(m_aspect - aspect) > 1e-6f) {
        m_aspect = aspect;
        invalidate_projection();
    }
}

Vec3 Camera3D::get_forward() const noexcept {
    const Vec3 diff = m_target - m_position;
    const float len_sq = diff.length_sq();
    return len_sq > 1e-12f ? diff.normalized() : Vec3(0.0f, 0.0f, -1.0f);
}

Vec3 Camera3D::get_right() const noexcept {
    const Vec3 f = get_forward();
    const Vec3 r = f.cross(m_up);
    const float len_sq = r.length_sq();
    return len_sq > 1e-12f ? r.normalized() : Vec3(1.0f, 0.0f, 0.0f);
}

Vec3 Camera3D::get_camera_up() const noexcept {
    const Vec3 r = get_right();
    const Vec3 f = get_forward();
    return r.cross(f).normalized();
}

float Camera3D::get_near_clip() const noexcept {
    return (m_proj_type == ProjectionType::Perspective) ? m_z_near : m_ortho_near;
}

float Camera3D::get_far_clip() const noexcept {
    return (m_proj_type == ProjectionType::Perspective) ? m_z_far : m_ortho_far;
}

void Camera3D::invalidate_view() noexcept {
    m_view_dirty = true;
    m_vp_dirty = true;
    m_inv_vp_dirty = true;
    m_frustum_dirty = true;
}

void Camera3D::invalidate_projection() noexcept {
    m_proj_dirty = true;
    m_vp_dirty = true;
    m_inv_vp_dirty = true;
    m_frustum_dirty = true;
}

void Camera3D::update_view() const noexcept {
    if (!m_view_dirty) return;
    m_view_matrix = Mat44::look_at(m_position, m_target, m_up);
    m_view_dirty = false;
}

void Camera3D::update_projection() const noexcept {
    if (!m_proj_dirty) return;

    if (m_proj_type == ProjectionType::Perspective) {
        const float tan_half_fov = std::tan(m_fov_y * 0.5f);
        const float f = (tan_half_fov > 1e-6f) ? (1.0f / tan_half_fov) : 1.0f;
        Mat44 p = Mat44::zeros();
        p.m[0] = f / m_aspect;
        p.m[5] = f;

        if (m_vulkan_depth) {
            // Vulkan: Z in [0, 1]
            p.m[10] = m_z_far / (m_z_near - m_z_far);
            p.m[11] = (m_z_far * m_z_near) / (m_z_near - m_z_far);
        } else {
            // OpenGL: Z in [-1, 1]
            p.m[10] = -(m_z_far + m_z_near) / (m_z_far - m_z_near);
            p.m[11] = -(2.0f * m_z_far * m_z_near) / (m_z_far - m_z_near);
        }
        p.m[14] = -1.0f;
        m_proj_matrix = p;
    } else {
        Mat44 p = Mat44::identity();
        const float dx = m_ortho_right - m_ortho_left;
        const float dy = m_ortho_top - m_ortho_bottom;
        p.m[0] = (std::abs(dx) > 1e-6f) ? (2.0f / dx) : 1.0f;
        p.m[3] = (std::abs(dx) > 1e-6f) ? (-(m_ortho_right + m_ortho_left) / dx) : 0.0f;
        p.m[5] = (std::abs(dy) > 1e-6f) ? (2.0f / dy) : 1.0f;
        p.m[7] = (std::abs(dy) > 1e-6f) ? (-(m_ortho_top + m_ortho_bottom) / dy) : 0.0f;

        if (m_vulkan_depth) {
            // Vulkan: Z in [0, 1]
            const float dz = m_ortho_near - m_ortho_far;
            p.m[10] = (std::abs(dz) > 1e-6f) ? (1.0f / dz) : 0.0f;
            p.m[11] = (std::abs(dz) > 1e-6f) ? (m_ortho_near / dz) : 0.0f;
        } else {
            // OpenGL: Z in [-1, 1]
            const float dz = m_ortho_far - m_ortho_near;
            p.m[10] = (std::abs(dz) > 1e-6f) ? (-2.0f / dz) : 0.0f;
            p.m[11] = (std::abs(dz) > 1e-6f) ? (-(m_ortho_far + m_ortho_near) / dz) : 0.0f;
        }
        p.m[15] = 1.0f;
        m_proj_matrix = p;
    }

    m_proj_dirty = false;
}

void Camera3D::update_view_projection() const noexcept {
    if (m_view_dirty) update_view();
    if (m_proj_dirty) update_projection();

    if (m_vp_dirty) {
        m_vp_matrix = m_proj_matrix * m_view_matrix;
        m_vp_dirty = false;
        m_inv_vp_dirty = true;
        m_frustum_dirty = true;
    }
}

const Mat44& Camera3D::get_view_matrix() const noexcept {
    update_view();
    return m_view_matrix;
}

const Mat44& Camera3D::get_projection_matrix() const noexcept {
    update_projection();
    return m_proj_matrix;
}

const Mat44& Camera3D::get_view_projection_matrix() const noexcept {
    update_view_projection();
    return m_vp_matrix;
}

const Mat44& Camera3D::get_inverse_view_projection_matrix() const noexcept {
    if (m_inv_vp_dirty) {
        update_view_projection();
        bool ok = false;
        m_inv_vp_matrix = m_vp_matrix.inverse(ok);
        m_inv_vp_dirty = false;
    }
    return m_inv_vp_matrix;
}

const Frustum& Camera3D::get_frustum() const noexcept {
    if (m_frustum_dirty) {
        update_view_projection();
        m_frustum = Frustum::from_view_projection(m_vp_matrix, m_vulkan_depth);
        m_frustum_dirty = false;
    }
    return m_frustum;
}

Ray Camera3D::screen_point_to_ray(
    float screen_x,
    float screen_y,
    float viewport_w,
    float viewport_h
) const noexcept {
    if (viewport_w <= 0.0f || viewport_h <= 0.0f) {
        return Ray(m_position, get_forward());
    }

    // Normalized Device Coordinates (NDC) in [-1, 1]
    const float ndc_x = (2.0f * screen_x / viewport_w) - 1.0f;
    const float ndc_y = 1.0f - (2.0f * screen_y / viewport_h);

    const Mat44& inv_vp = get_inverse_view_projection_matrix();

    // Vulkan near plane is Z = 0.0f, OpenGL is Z = -1.0f
    const float ndc_z_near = m_vulkan_depth ? 0.0f : -1.0f;
    const float ndc_z_far = 1.0f;

    auto unproject = [&inv_vp](float x, float y, float z) noexcept -> Vec3 {
        const float rx = inv_vp.m[0] * x + inv_vp.m[1] * y + inv_vp.m[2] * z + inv_vp.m[3];
        const float ry = inv_vp.m[4] * x + inv_vp.m[5] * y + inv_vp.m[6] * z + inv_vp.m[7];
        const float rz = inv_vp.m[8] * x + inv_vp.m[9] * y + inv_vp.m[10] * z + inv_vp.m[11];
        const float rw = inv_vp.m[12] * x + inv_vp.m[13] * y + inv_vp.m[14] * z + inv_vp.m[15];

        if (std::abs(rw) > 1e-7f) {
            const float inv_w = 1.0f / rw;
            return Vec3(rx * inv_w, ry * inv_w, rz * inv_w);
        }
        return Vec3(rx, ry, rz);
    };

    const Vec3 near_world = unproject(ndc_x, ndc_y, ndc_z_near);
    const Vec3 far_world = unproject(ndc_x, ndc_y, ndc_z_far);

    const Vec3 dir = (far_world - near_world).normalized();
    if (m_proj_type == ProjectionType::Perspective) {
        return Ray(m_position, dir);
    } else {
        return Ray(near_world, dir);
    }
}

} // namespace nisaba::render3d
