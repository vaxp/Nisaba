#include "orion/scene/renderable.hpp"

namespace orion {

Renderable3D::Renderable3D(std::shared_ptr<Mesh3D> in_mesh, std::shared_ptr<PbrMaterial> in_mat)
    : mesh(std::move(in_mesh)) {
    if (mesh) {
        local_bounds = mesh->get_bounds();
    }
    if (in_mat) {
        materials.push_back(std::move(in_mat));
    }
    update_world_bounds();
}

void Renderable3D::set_transform(const Mat44& in_transform) noexcept {
    transform = in_transform;
    update_world_bounds();
}

void Renderable3D::set_position(const Vec3& pos) noexcept {
    transform.m[3] = pos.x;
    transform.m[7] = pos.y;
    transform.m[11] = pos.z;
    update_world_bounds();
}

void Renderable3D::update_world_bounds() noexcept {
    world_bounds.reset();
    if (!mesh) return;

    const Vec3& min = local_bounds.min_point;
    const Vec3& max = local_bounds.max_point;

    const Vec3 corners[8] = {
        Vec3(min.x, min.y, min.z),
        Vec3(max.x, min.y, min.z),
        Vec3(max.x, max.y, min.z),
        Vec3(min.x, max.y, min.z),
        Vec3(min.x, min.y, max.z),
        Vec3(max.x, min.y, max.z),
        Vec3(max.x, max.y, max.z),
        Vec3(min.x, max.y, max.z)
    };

    for (int i = 0; i < 8; ++i) {
        world_bounds.encapsulate(transform.transform_point(corners[i]));
    }
}

std::shared_ptr<PbrMaterial> Renderable3D::get_material(size_t submesh_idx) const noexcept {
    if (submesh_idx < materials.size() && materials[submesh_idx]) {
        return materials[submesh_idx];
    }
    static auto s_default_mat = PbrMaterial::create_default_dielectric();
    return s_default_mat;
}

void Renderable3D::set_material(size_t submesh_idx, std::shared_ptr<PbrMaterial> mat) {
    if (submesh_idx >= materials.size()) {
        materials.resize(submesh_idx + 1);
    }
    materials[submesh_idx] = std::move(mat);
}

} // namespace orion
