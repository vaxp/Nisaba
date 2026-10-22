#include "nisaba/render3d/render3d.hpp"
#include <iostream>
#include <cmath>
#include <cassert>
#include <filesystem>

#define TEST_ASSERT(cond, msg) \
    do { \
        if (!(cond)) { \
            std::cerr << "[FAIL] " << msg << " (" << #cond << ") at " << __FILE__ << ":" << __LINE__ << std::endl; \
            return 1; \
        } \
    } while (0)

using namespace nisaba::render3d;

int main() {
    std::cout << "===================================================\n"
              << "Nisaba Sovereign 3D Skeletal Animation & Skinning Test\n"
              << "===================================================\n";

    std::string model_path = "third_party/models/Characters/CesiumMan.glb";
    if (!std::filesystem::exists(model_path)) {
        std::cout << "[SKIP] Character model not found at: " << model_path << "\n";
        return 0;
    }

    std::cout << "[*] Loading rigged character glTF asset..." << std::endl;
    auto load_res = GltfLoader::load_from_file(model_path);
    TEST_ASSERT(load_res.success, "glTF asset loading must succeed");
    TEST_ASSERT(!load_res.skeletons.empty(), "Skeleton must be parsed from glTF skins");
    TEST_ASSERT(!load_res.animations.empty(), "Animations must be parsed from glTF animations");
    TEST_ASSERT(!load_res.skinned_meshes.empty(), "SkinnedMeshInstance must be constructed");

    auto skeleton = load_res.skeletons[0];
    auto clip = load_res.animations[0];
    auto skinned_mesh = load_res.skinned_meshes[0];

    std::cout << "[*] Skeleton: \"" << skeleton->name << "\" with "
              << skeleton->joints.size() << " joints" << std::endl;
    TEST_ASSERT(skeleton->joints.size() == 19, "CesiumMan must have exactly 19 joints");

    std::cout << "[*] Animation Clip: \"" << clip->name << "\" duration="
              << clip->duration << "s, channels=" << clip->channels.size() << std::endl;
    TEST_ASSERT(clip->duration > 1.9f && clip->duration < 2.1f, "Walk cycle duration ~2.0s");
    TEST_ASSERT(clip->channels.size() == 57, "CesiumMan must have 57 animation channels (19 joints * 3 TRS)");

    // 1. Test Bind Pose Evaluation
    std::cout << "[*] Evaluating Bind Pose..." << std::endl;
    skeleton->reset_to_bind_pose();
    TEST_ASSERT(skeleton->joint_matrices.size() == 19, "Must have 19 skin matrices");

    // In bind pose, skin matrices should be close to Identity
    for (size_t i = 0; i < skeleton->joint_matrices.size(); ++i) {
        const auto& m = skeleton->joint_matrices[i];
        float diag_sum = m(0,0) + m(1,1) + m(2,2) + m(3,3);
        TEST_ASSERT(std::abs(diag_sum - 4.0f) < 0.2f, "Bind pose skin matrix must be close to Identity");
    }

    // 2. Test Keyframe Animation Sampling & Skeleton Evaluation
    std::cout << "[*] Evaluating Animation at t = 0.5s..." << std::endl;
    clip->apply(*skeleton, 0.5f);
    skeleton->update_matrices();

    // Verify joints have moved from bind pose
    bool any_matrix_changed = false;
    for (size_t i = 0; i < skeleton->joint_matrices.size(); ++i) {
        const auto& m = skeleton->joint_matrices[i];
        float off_diag = std::abs(m(0,3)) + std::abs(m(1,3)) + std::abs(m(2,3));
        if (off_diag > 0.01f) {
            any_matrix_changed = true;
            break;
        }
    }
    TEST_ASSERT(any_matrix_changed, "Joint transforms must move during animation");

    // 3. Test Skinned Mesh Deformation
    std::cout << "[*] Testing Linear Blend Skinning Deformation..." << std::endl;
    const auto& bind_verts = skinned_mesh->bind_mesh->vertices;
    auto& dyn_verts = skinned_mesh->dynamic_mesh->vertices;
    TEST_ASSERT(!bind_verts.empty(), "Bind mesh must have vertices");
    TEST_ASSERT(bind_verts.size() == dyn_verts.size(), "Dynamic mesh must match vertex count");

    skinned_mesh->update_skinning();
    TEST_ASSERT(skinned_mesh->needs_gpu_upload, "Flag needs_gpu_upload must be set after skinning");

    bool vertices_deformed = false;
    float max_delta = 0.0f;
    for (size_t i = 0; i < bind_verts.size(); ++i) {
        float d = (dyn_verts[i].position - bind_verts[i].position).length();
        if (d > max_delta) max_delta = d;
        if (d > 0.01f) {
            vertices_deformed = true;
        }
    }
    std::cout << "[*] Maximum vertex displacement during walk step: " << max_delta << " meters" << std::endl;
    TEST_ASSERT(vertices_deformed, "Vertices must deform during skeletal animation");
    TEST_ASSERT(max_delta > 0.05f, "Limb displacement must be significant during walk cycle");

    // 4. Test Animator controller
    std::cout << "[*] Testing Animator playback controller..." << std::endl;
    auto animator = load_res.animator;
    TEST_ASSERT(animator != nullptr, "Animator must be instantiated");
    TEST_ASSERT(animator->is_playing, "Animator should auto-play by default");

    animator->update(0.25f);
    TEST_ASSERT(animator->current_time >= 0.24f, "Animator time must advance with dt");

    std::cout << "\n[PASS] All 3D Skeletal Animation & Mesh Skinning tests passed 100%!" << std::endl;
    return 0;
}
