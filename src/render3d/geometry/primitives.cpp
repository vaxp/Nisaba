#include "nisaba/render3d/geometry/primitives.hpp"
#include <cmath>

namespace nisaba::render3d {

namespace {
constexpr float PI = 3.14159265358979323846f;
constexpr float TWO_PI = 2.0f * PI;
} // namespace

Mesh3D MeshPrimitives::create_cube(const Vec3& size) noexcept {
    Mesh3D mesh;
    const float hx = size.x * 0.5f;
    const float hy = size.y * 0.5f;
    const float hz = size.z * 0.5f;

    mesh.vertices.reserve(24);
    mesh.indices.reserve(36);

    auto add_face = [&](
        const Vec3& p0, const Vec3& p1, const Vec3& p2, const Vec3& p3,
        const Vec3& normal, const Vec3& tangent
    ) {
        const uint32_t base_idx = static_cast<uint32_t>(mesh.vertices.size());

        mesh.vertices.emplace_back(p0, normal, 0.0f, 0.0f, tangent.x, tangent.y, tangent.z, 1.0f);
        mesh.vertices.emplace_back(p1, normal, 1.0f, 0.0f, tangent.x, tangent.y, tangent.z, 1.0f);
        mesh.vertices.emplace_back(p2, normal, 1.0f, 1.0f, tangent.x, tangent.y, tangent.z, 1.0f);
        mesh.vertices.emplace_back(p3, normal, 0.0f, 1.0f, tangent.x, tangent.y, tangent.z, 1.0f);

        mesh.indices.push_back(base_idx + 0);
        mesh.indices.push_back(base_idx + 1);
        mesh.indices.push_back(base_idx + 2);
        mesh.indices.push_back(base_idx + 0);
        mesh.indices.push_back(base_idx + 2);
        mesh.indices.push_back(base_idx + 3);
    };

    // Front (+Z)
    add_face(
        Vec3(-hx, -hy,  hz), Vec3( hx, -hy,  hz), Vec3( hx,  hy,  hz), Vec3(-hx,  hy,  hz),
        Vec3(0.0f, 0.0f, 1.0f), Vec3(1.0f, 0.0f, 0.0f)
    );
    // Back (-Z)
    add_face(
        Vec3( hx, -hy, -hz), Vec3(-hx, -hy, -hz), Vec3(-hx,  hy, -hz), Vec3( hx,  hy, -hz),
        Vec3(0.0f, 0.0f, -1.0f), Vec3(-1.0f, 0.0f, 0.0f)
    );
    // Top (+Y)
    add_face(
        Vec3(-hx,  hy,  hz), Vec3( hx,  hy,  hz), Vec3( hx,  hy, -hz), Vec3(-hx,  hy, -hz),
        Vec3(0.0f, 1.0f, 0.0f), Vec3(1.0f, 0.0f, 0.0f)
    );
    // Bottom (-Y)
    add_face(
        Vec3(-hx, -hy, -hz), Vec3( hx, -hy, -hz), Vec3( hx, -hy,  hz), Vec3(-hx, -hy,  hz),
        Vec3(0.0f, -1.0f, 0.0f), Vec3(1.0f, 0.0f, 0.0f)
    );
    // Right (+X)
    add_face(
        Vec3( hx, -hy,  hz), Vec3( hx, -hy, -hz), Vec3( hx,  hy, -hz), Vec3( hx,  hy,  hz),
        Vec3(1.0f, 0.0f, 0.0f), Vec3(0.0f, 0.0f, -1.0f)
    );
    // Left (-X)
    add_face(
        Vec3(-hx, -hy, -hz), Vec3(-hx, -hy,  hz), Vec3(-hx,  hy,  hz), Vec3(-hx,  hy, -hz),
        Vec3(-1.0f, 0.0f, 0.0f), Vec3(0.0f, 0.0f, 1.0f)
    );

    mesh.compute_bounds();
    mesh.upload_to_buffers();
    return mesh;
}

Mesh3D MeshPrimitives::create_sphere(float radius, uint32_t segments, uint32_t rings) noexcept {
    Mesh3D mesh;
    segments = std::max(3u, segments);
    rings = std::max(2u, rings);

    mesh.vertices.reserve((rings + 1) * (segments + 1));
    mesh.indices.reserve(rings * segments * 6);

    for (uint32_t r = 0; r <= rings; ++r) {
        const float phi = PI * (static_cast<float>(r) / static_cast<float>(rings)) - (PI * 0.5f);
        const float cos_phi = std::cos(phi);
        const float sin_phi = std::sin(phi);
        const float v = static_cast<float>(r) / static_cast<float>(rings);

        for (uint32_t s = 0; s <= segments; ++s) {
            const float theta = TWO_PI * (static_cast<float>(s) / static_cast<float>(segments));
            const float cos_theta = std::cos(theta);
            const float sin_theta = std::sin(theta);
            const float u = static_cast<float>(s) / static_cast<float>(segments);

            const Vec3 norm(cos_phi * cos_theta, sin_phi, cos_phi * sin_theta);
            const Vec3 pos = norm * radius;
            const Vec3 tan(-sin_theta, 0.0f, cos_theta);

            mesh.vertices.emplace_back(pos, norm, u, v, tan.x, tan.y, tan.z, 1.0f);
        }
    }

    for (uint32_t r = 0; r < rings; ++r) {
        for (uint32_t s = 0; s < segments; ++s) {
            const uint32_t i0 = r * (segments + 1) + s;
            const uint32_t i1 = i0 + 1;
            const uint32_t i2 = (r + 1) * (segments + 1) + s;
            const uint32_t i3 = i2 + 1;

            mesh.indices.push_back(i0);
            mesh.indices.push_back(i2);
            mesh.indices.push_back(i1);

            mesh.indices.push_back(i1);
            mesh.indices.push_back(i2);
            mesh.indices.push_back(i3);
        }
    }

    mesh.compute_bounds();
    mesh.upload_to_buffers();
    return mesh;
}

Mesh3D MeshPrimitives::create_plane(float width, float depth, uint32_t sub_x, uint32_t sub_z) noexcept {
    Mesh3D mesh;
    sub_x = std::max(1u, sub_x);
    sub_z = std::max(1u, sub_z);

    const float hx = width * 0.5f;
    const float hz = depth * 0.5f;

    mesh.vertices.reserve((sub_x + 1) * (sub_z + 1));
    mesh.indices.reserve(sub_x * sub_z * 6);

    for (uint32_t z = 0; z <= sub_z; ++z) {
        const float fz = static_cast<float>(z) / static_cast<float>(sub_z);
        const float pos_z = -hz + fz * depth;

        for (uint32_t x = 0; x <= sub_x; ++x) {
            const float fx = static_cast<float>(x) / static_cast<float>(sub_x);
            const float pos_x = -hx + fx * width;

            mesh.vertices.emplace_back(
                Vec3(pos_x, 0.0f, pos_z),
                Vec3(0.0f, 1.0f, 0.0f),
                fx, fz,
                1.0f, 0.0f, 0.0f, 1.0f
            );
        }
    }

    for (uint32_t z = 0; z < sub_z; ++z) {
        for (uint32_t x = 0; x < sub_x; ++x) {
            const uint32_t i0 = z * (sub_x + 1) + x;
            const uint32_t i1 = i0 + 1;
            const uint32_t i2 = (z + 1) * (sub_x + 1) + x;
            const uint32_t i3 = i2 + 1;

            mesh.indices.push_back(i0);
            mesh.indices.push_back(i2);
            mesh.indices.push_back(i1);

            mesh.indices.push_back(i1);
            mesh.indices.push_back(i2);
            mesh.indices.push_back(i3);
        }
    }

    mesh.compute_bounds();
    mesh.upload_to_buffers();
    return mesh;
}

Mesh3D MeshPrimitives::create_cylinder(float radius, float height, uint32_t segments) noexcept {
    Mesh3D mesh;
    segments = std::max(3u, segments);
    const float hh = height * 0.5f;

    // Mantle vertices
    const uint32_t mantle_start = 0;
    for (uint32_t s = 0; s <= segments; ++s) {
        const float u = static_cast<float>(s) / static_cast<float>(segments);
        const float theta = TWO_PI * u;
        const float cos_t = std::cos(theta);
        const float sin_t = std::sin(theta);

        const Vec3 norm(cos_t, 0.0f, sin_t);
        const Vec3 tan(-sin_t, 0.0f, cos_t);

        // Top vertex
        mesh.vertices.emplace_back(
            Vec3(radius * cos_t, hh, radius * sin_t),
            norm, u, 1.0f, tan.x, tan.y, tan.z, 1.0f
        );
        // Bottom vertex
        mesh.vertices.emplace_back(
            Vec3(radius * cos_t, -hh, radius * sin_t),
            norm, u, 0.0f, tan.x, tan.y, tan.z, 1.0f
        );
    }

    // Mantle indices
    for (uint32_t s = 0; s < segments; ++s) {
        const uint32_t top0 = mantle_start + s * 2;
        const uint32_t bot0 = top0 + 1;
        const uint32_t top1 = top0 + 2;
        const uint32_t bot1 = top0 + 3;

        mesh.indices.push_back(top0);
        mesh.indices.push_back(bot0);
        mesh.indices.push_back(top1);

        mesh.indices.push_back(top1);
        mesh.indices.push_back(bot0);
        mesh.indices.push_back(bot1);
    }

    // Top cap
    const uint32_t top_center_idx = static_cast<uint32_t>(mesh.vertices.size());
    mesh.vertices.emplace_back(
        Vec3(0.0f, hh, 0.0f), Vec3(0.0f, 1.0f, 0.0f), 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f
    );
    for (uint32_t s = 0; s <= segments; ++s) {
        const float theta = TWO_PI * static_cast<float>(s) / static_cast<float>(segments);
        const float cos_t = std::cos(theta);
        const float sin_t = std::sin(theta);
        mesh.vertices.emplace_back(
            Vec3(radius * cos_t, hh, radius * sin_t),
            Vec3(0.0f, 1.0f, 0.0f),
            0.5f + 0.5f * cos_t, 0.5f + 0.5f * sin_t,
            1.0f, 0.0f, 0.0f, 1.0f
        );
    }
    for (uint32_t s = 0; s < segments; ++s) {
        mesh.indices.push_back(top_center_idx);
        mesh.indices.push_back(top_center_idx + 1 + s);
        mesh.indices.push_back(top_center_idx + 2 + s);
    }

    // Bottom cap
    const uint32_t bot_center_idx = static_cast<uint32_t>(mesh.vertices.size());
    mesh.vertices.emplace_back(
        Vec3(0.0f, -hh, 0.0f), Vec3(0.0f, -1.0f, 0.0f), 0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 1.0f
    );
    for (uint32_t s = 0; s <= segments; ++s) {
        const float theta = TWO_PI * static_cast<float>(s) / static_cast<float>(segments);
        const float cos_t = std::cos(theta);
        const float sin_t = std::sin(theta);
        mesh.vertices.emplace_back(
            Vec3(radius * cos_t, -hh, radius * sin_t),
            Vec3(0.0f, -1.0f, 0.0f),
            0.5f + 0.5f * cos_t, 0.5f + 0.5f * sin_t,
            -1.0f, 0.0f, 0.0f, 1.0f
        );
    }
    for (uint32_t s = 0; s < segments; ++s) {
        mesh.indices.push_back(bot_center_idx);
        mesh.indices.push_back(bot_center_idx + 2 + s);
        mesh.indices.push_back(bot_center_idx + 1 + s);
    }

    mesh.compute_bounds();
    mesh.upload_to_buffers();
    return mesh;
}

Mesh3D MeshPrimitives::create_torus(
    float major_radius,
    float minor_radius,
    uint32_t radial_segments,
    uint32_t tubular_segments
) noexcept {
    Mesh3D mesh;
    radial_segments = std::max(3u, radial_segments);
    tubular_segments = std::max(3u, tubular_segments);

    mesh.vertices.reserve((radial_segments + 1) * (tubular_segments + 1));
    mesh.indices.reserve(radial_segments * tubular_segments * 6);

    for (uint32_t i = 0; i <= radial_segments; ++i) {
        const float u = static_cast<float>(i) / static_cast<float>(radial_segments);
        const float theta = TWO_PI * u;
        const float cos_theta = std::cos(theta);
        const float sin_theta = std::sin(theta);

        const Vec3 center(major_radius * cos_theta, 0.0f, major_radius * sin_theta);

        for (uint32_t j = 0; j <= tubular_segments; ++j) {
            const float v = static_cast<float>(j) / static_cast<float>(tubular_segments);
            const float phi = TWO_PI * v;
            const float cos_phi = std::cos(phi);
            const float sin_phi = std::sin(phi);

            const Vec3 pos(
                (major_radius + minor_radius * cos_phi) * cos_theta,
                minor_radius * sin_phi,
                (major_radius + minor_radius * cos_phi) * sin_theta
            );

            const Vec3 norm = (pos - center).normalized();
            const Vec3 tan(-sin_theta, 0.0f, cos_theta);

            mesh.vertices.emplace_back(pos, norm, u, v, tan.x, tan.y, tan.z, 1.0f);
        }
    }

    for (uint32_t i = 0; i < radial_segments; ++i) {
        for (uint32_t j = 0; j < tubular_segments; ++j) {
            const uint32_t i0 = i * (tubular_segments + 1) + j;
            const uint32_t i1 = i0 + 1;
            const uint32_t i2 = (i + 1) * (tubular_segments + 1) + j;
            const uint32_t i3 = i2 + 1;

            mesh.indices.push_back(i0);
            mesh.indices.push_back(i2);
            mesh.indices.push_back(i1);

            mesh.indices.push_back(i1);
            mesh.indices.push_back(i2);
            mesh.indices.push_back(i3);
        }
    }

    mesh.compute_bounds();
    mesh.upload_to_buffers();
    return mesh;
}

} // namespace nisaba::render3d
