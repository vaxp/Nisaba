#pragma once

#include "orion/scene/scene.hpp"
#include "orion/scene/renderable.hpp"
#include "orion/geometry/mesh.hpp"
#include "orion/material/pbr_material.hpp"
#include "orion/material/texture.hpp"
#include "orion/animation/skeleton.hpp"
#include "orion/animation/animation_clip.hpp"
#include "orion/animation/skinned_mesh.hpp"
#include "orion/animation/animator.hpp"
#include <string>
#include <string_view>
#include <vector>
#include <memory>
#include <span>

namespace orion {

/// @brief Result container returned after ingesting a glTF 2.0 or GLB asset.
struct GltfLoadResult {
    bool success{false};
    std::string error_message;
    std::shared_ptr<Scene3D> scene;
    std::vector<std::shared_ptr<Renderable3D>> renderables;
    std::vector<std::shared_ptr<Mesh3D>> meshes;
    std::vector<std::shared_ptr<PbrMaterial>> materials;
    std::vector<std::shared_ptr<Texture2D>> textures;

    std::vector<std::shared_ptr<Skeleton>> skeletons;
    std::vector<std::shared_ptr<AnimationClip>> animations;
    std::vector<std::shared_ptr<SkinnedMeshInstance>> skinned_meshes;
    std::shared_ptr<Animator> animator;
};

/// @brief Sovereign, zero-dependency glTF 2.0 / GLB asset loader for Nisaba 3D.
/// Parses glTF 2.0 JSON, embedded base64 data URIs, binary GLB containers,
/// accessors, buffer views, PBR materials, polygonal meshes, and scene node hierarchies.
class GltfLoader {
public:
    /// @brief Loads and parses a glTF (.gltf) or binary glTF (.glb) file from disk.
    [[nodiscard]] static GltfLoadResult load_from_file(const std::string& filepath);

    /// @brief Loads and parses a glTF or GLB asset from raw memory.
    [[nodiscard]] static GltfLoadResult load_from_memory(
        std::span<const uint8_t> data,
        const std::string& base_dir = ""
    );

    /// @brief Parses a glTF 2.0 JSON string directly, with optional external binary buffers.
    [[nodiscard]] static GltfLoadResult load_from_json(
        std::string_view json_str,
        const std::string& base_dir = "",
        const std::vector<std::vector<uint8_t>>& external_buffers = {}
    );
};

} // namespace orion
