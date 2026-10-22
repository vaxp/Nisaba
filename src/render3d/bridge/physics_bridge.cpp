#include "nisaba/render3d/bridge/physics_bridge.hpp"
#include "nisaba/render3d/geometry/primitives.hpp"
#include "nisaba/physics/collision/box_shape.hpp"
#include "nisaba/physics/collision/sphere_shape.hpp"
#include "nisaba/physics/collision/cylinder_shape.hpp"
#include "nisaba/physics/collision/capsule_shape.hpp"
#include "nisaba/physics/collision/plane_shape.hpp"

namespace nisaba::render3d {

void PhysicsRenderBridge::bind(
    BodyID body_id,
    std::shared_ptr<Renderable3D> renderable,
    const Vec3& local_offset,
    const Quat& local_rotation,
    const Vec3& local_scale
) {
    if (!renderable) return;

    BodyRenderBinding binding;
    binding.body_id = body_id;
    binding.renderable = std::move(renderable);
    binding.local_offset = local_offset;
    binding.local_rotation = local_rotation;
    binding.local_scale = local_scale;
    binding.has_prev = false;

    m_bindings[body_id.raw_id()] = std::move(binding);
}

void PhysicsRenderBridge::unbind(BodyID body_id) {
    m_bindings.erase(body_id.raw_id());
}

void PhysicsRenderBridge::clear() noexcept {
    m_bindings.clear();
}

std::shared_ptr<Renderable3D> PhysicsRenderBridge::get_renderable(BodyID body_id) const noexcept {
    auto it = m_bindings.find(body_id.raw_id());
    if (it != m_bindings.end()) {
        return it->second.renderable;
    }
    return nullptr;
}

bool PhysicsRenderBridge::has_binding(BodyID body_id) const noexcept {
    return m_bindings.find(body_id.raw_id()) != m_bindings.end();
}

std::shared_ptr<Renderable3D> PhysicsRenderBridge::create_and_bind(
    const PhysicsScene& physics_scene,
    BodyID body_id,
    std::shared_ptr<PbrMaterial> material,
    Scene3D* render_scene
) {
    const auto* body = physics_scene.get_body(body_id);
    if (!body) return nullptr;

    const auto* shape = body->get_shape();
    std::shared_ptr<Mesh3D> mesh;

    if (shape) {
        switch (shape->get_type()) {
            case nisaba::physics::ShapeType::Box: {
                const auto* box = static_cast<const nisaba::physics::BoxShape*>(shape);
                mesh = std::make_shared<Mesh3D>(MeshPrimitives::create_cube(box->half_extents * 2.0f));
                break;
            }
            case nisaba::physics::ShapeType::Sphere: {
                const auto* sphere = static_cast<const nisaba::physics::SphereShape*>(shape);
                mesh = std::make_shared<Mesh3D>(MeshPrimitives::create_sphere(sphere->radius));
                break;
            }
            case nisaba::physics::ShapeType::Cylinder: {
                const auto* cyl = static_cast<const nisaba::physics::CylinderShape*>(shape);
                mesh = std::make_shared<Mesh3D>(MeshPrimitives::create_cylinder(cyl->radius, cyl->half_height * 2.0f));
                break;
            }
            case nisaba::physics::ShapeType::Capsule: {
                const auto* cap = static_cast<const nisaba::physics::CapsuleShape*>(shape);
                mesh = std::make_shared<Mesh3D>(MeshPrimitives::create_cylinder(cap->radius, cap->half_height * 2.0f));
                break;
            }
            case nisaba::physics::ShapeType::Plane: {
                const auto* pl = static_cast<const nisaba::physics::PlaneShape*>(shape);
                float w = (pl->half_extent > 0.0f && pl->half_extent < 1000.0f) ? pl->half_extent * 2.0f : 100.0f;
                mesh = std::make_shared<Mesh3D>(MeshPrimitives::create_plane(w, w));
                break;
            }
            default: {
                mesh = std::make_shared<Mesh3D>(MeshPrimitives::create_cube(Vec3(1.0f, 1.0f, 1.0f)));
                break;
            }
        }
    } else {
        mesh = std::make_shared<Mesh3D>(MeshPrimitives::create_cube(Vec3(1.0f, 1.0f, 1.0f)));
    }

    auto renderable = std::make_shared<Renderable3D>(mesh, std::move(material));
    Mat44 tf = Mat44::transform(body->get_position(), body->get_rotation());
    renderable->set_transform(tf);

    if (render_scene) {
        render_scene->add_renderable(renderable);
    }

    bind(body_id, renderable);

    auto& b = m_bindings[body_id.raw_id()];
    b.prev_position = body->get_position();
    b.prev_rotation = body->get_rotation();
    b.has_prev = true;

    return renderable;
}

void PhysicsRenderBridge::record_physics_step(const PhysicsScene& physics_scene) noexcept {
    for (auto& [id_val, b] : m_bindings) {
        const auto* body = physics_scene.get_body(b.body_id);
        if (body) {
            b.prev_position = body->get_position();
            b.prev_rotation = body->get_rotation();
            b.has_prev = true;
        }
    }
}

void PhysicsRenderBridge::sync(const PhysicsScene& physics_scene, float alpha) noexcept {
    for (auto& [id_val, b] : m_bindings) {
        const auto* body = physics_scene.get_body(b.body_id);
        if (!body || !b.renderable) continue;

        const Vec3 cur_pos = body->get_position();
        const Quat cur_rot = body->get_rotation();

        if (alpha >= 1.0f || !b.has_prev) {
            const Vec3 final_pos = cur_pos + cur_rot.rotate_vec(b.local_offset);
            const Quat final_rot = cur_rot * b.local_rotation;
            const Mat44 tf = Mat44::transform(final_pos, final_rot, b.local_scale);
            b.renderable->set_transform(tf);

            b.prev_position = cur_pos;
            b.prev_rotation = cur_rot;
            b.has_prev = true;
        } else {
            const Vec3 interp_pos = Vec3::lerp(b.prev_position, cur_pos, alpha);
            const Quat interp_rot = Quat::slerp(b.prev_rotation, cur_rot, alpha);
            const Vec3 final_pos = interp_pos + interp_rot.rotate_vec(b.local_offset);
            const Quat final_rot = interp_rot * b.local_rotation;
            const Mat44 tf = Mat44::transform(final_pos, final_rot, b.local_scale);
            b.renderable->set_transform(tf);
        }
    }
}

} // namespace nisaba::render3d
