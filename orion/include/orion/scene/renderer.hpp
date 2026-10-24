#pragma once

#include "orion/scene/view.hpp"
#include "orion/scene/sort_key.hpp"
#include "orion/lighting/shadow_map.hpp"
#include <vector>

namespace orion {

/// @brief Compact draw command representation sorted by a single 64-bit key.
struct DrawCommand64 {
    uint64_t sort_key{0};
    const Renderable3D* renderable{nullptr};
    const Mesh3D* mesh{nullptr};
    uint32_t submesh_index{0};
    const PbrMaterial* material{nullptr};
    Mat44 world_transform;
    Mat44 normal_matrix;
    float distance_to_camera{0.0f};

    [[nodiscard]] constexpr bool operator<(const DrawCommand64& rhs) const noexcept {
        return sort_key < rhs.sort_key;
    }
};

/// @brief Telemetry and performance statistics for a rendered frame.
struct RenderStats {
    uint32_t total_objects{0};
    uint32_t visible_objects{0};
    uint32_t culled_objects{0};
    uint32_t total_draw_calls{0};
    uint32_t opaque_draw_calls{0};
    uint32_t transparent_draw_calls{0};
    uint32_t instanced_batches{0};
    uint32_t total_triangles{0};

    void reset() noexcept {
        total_objects = 0;
        visible_objects = 0;
        culled_objects = 0;
        total_draw_calls = 0;
        opaque_draw_calls = 0;
        transparent_draw_calls = 0;
        instanced_batches = 0;
        total_triangles = 0;
    }
};

/// @brief Sovereign 3D Renderer orchestrating frustum culling, 64-bit draw call sorting,
/// Early-Z front-to-back opaque clustering, back-to-front transparency, and shadow maps.
class Renderer3D {
public:
    std::vector<DrawCommand64> draw_queue;
    std::vector<const Renderable3D*> m_visible_renderables;
    RenderStats last_stats;
    ShadowMap shadow_map;

    Renderer3D(uint32_t shadow_res = 1024);

    /// @brief Extracts visible geometry, calculates 64-bit sort keys, and sorts draw calls.
    void prepare_frame(const View3D& view) noexcept;

    /// @brief Dispatches sorted draw calls and shadow passes.
    void execute_frame(const View3D& view) noexcept;

    /// @brief High-level render entry point executing prepare_frame and execute_frame.
    void render(const View3D& view) noexcept;

    [[nodiscard]] const RenderStats& get_stats() const noexcept { return last_stats; }
    [[nodiscard]] const std::vector<DrawCommand64>& get_draw_queue() const noexcept { return draw_queue; }
    [[nodiscard]] const ShadowMap& get_shadow_map() const noexcept { return shadow_map; }
};

} // namespace orion
