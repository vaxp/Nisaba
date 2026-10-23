#include "orion/animation/skinned_mesh.hpp"
#include <algorithm>

namespace orion {

SkinnedMeshInstance::SkinnedMeshInstance(
    std::shared_ptr<Mesh3D> in_bind,
    std::shared_ptr<Mesh3D> in_dyn,
    std::shared_ptr<Skeleton> in_skel
)
    : bind_mesh(std::move(in_bind)),
      dynamic_mesh(std::move(in_dyn)),
      skeleton(std::move(in_skel)) {}

void SkinnedMeshInstance::update_skinning() noexcept {
    if (!bind_mesh || !dynamic_mesh || !skeleton) return;
    if (bind_mesh->skin_data.empty()) return;

    const auto& pal = skeleton->joint_matrices;
    if (pal.empty()) return;

    const auto& bind_verts = bind_mesh->vertices;
    auto& dyn_verts = dynamic_mesh->vertices;
    const auto& skin = bind_mesh->skin_data;
    const size_t count = std::min({bind_verts.size(), dyn_verts.size(), skin.size()});

    for (size_t i = 0; i < count; ++i) {
        const auto& bv = bind_verts[i];
        const auto& sd = skin[i];

        Vec3 pos = Vec3::zero();
        Vec3 norm = Vec3::zero();

        for (int k = 0; k < 4; ++k) {
            const float w = sd.weights[k];
            if (w <= 1e-5f) continue;
            const uint16_t j_idx = sd.joints[k];
            if (j_idx < pal.size()) {
                const Mat44& m = pal[j_idx];
                pos += m.transform_point(bv.position) * w;
                norm += m.transform_vector(bv.normal) * w;
            }
        }

        dyn_verts[i].position = pos;
        dyn_verts[i].normal = norm.normalized();
    }

    needs_gpu_upload = true;
}

} // namespace orion
