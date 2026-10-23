#pragma once

#include "orion/math/aabb.hpp"
#include "orion/math/ray.hpp"
#include "orion/physics/body/body_id.hpp"
#include "orion/physics/broadphase/aabb4.hpp"
#include "orion/physics/broadphase/broad_phase_layer.hpp"
#include <vector>
#include <functional>
#include <limits>

namespace orion::physics {

/// @brief Represents a pair of potentially colliding body IDs.
struct BodyPair {
    BodyID body_a;
    BodyID body_b;

    constexpr BodyPair() noexcept = default;
    constexpr BodyPair(BodyID in_a, BodyID in_b) noexcept
        : body_a(in_a < in_b ? in_a : in_b),
          body_b(in_a < in_b ? in_b : in_a) {}

    constexpr bool operator==(const BodyPair& other) const noexcept {
        return body_a == other.body_a && body_b == other.body_b;
    }
};

/// @brief High-performance SIMD 4-way Dynamic Bounding Volume Hierarchy (QuadTree BVH).
/// Each internal node holds 4 AABBs in SoA layout, tested simultaneously via 128-bit SIMD registers.
/// Exactly 128 bytes per node, aligned to 16 bytes (fits strictly in 2 CPU cache lines).
class BroadPhase {
public:
    static constexpr uint32_t INVALID_INDEX = std::numeric_limits<uint32_t>::max();

    /// @brief Exactly 128 bytes, 16-byte aligned (2 cache lines)
    struct alignas(16) Node {
        AABB4 aabb4;                       // 96 bytes: 4 bounding boxes in SoA
        uint32_t children[4] = {           // 16 bytes: child node index or leaf body index
            INVALID_INDEX, INVALID_INDEX, INVALID_INDEX, INVALID_INDEX
        };
        uint32_t parent = INVALID_INDEX;   // 4 bytes: parent node index
        uint16_t num_children = 0;         // 2 bytes: active children (0..4)
        uint8_t  is_leaf_mask = 0;         // 1 byte: bit i == 1 if children[i] is a leaf
        uint8_t  padding[9] = {0};         // 9 bytes: pad to exactly 128 bytes
    };
    static_assert(sizeof(Node) == 128, "BroadPhase::Node must occupy strictly 128 bytes (2 cache lines)");
    static_assert(alignof(Node) == 16, "BroadPhase::Node must be 16-byte aligned");

    struct Leaf {
        AABB fat_aabb;
        AABB tight_bounds;
        BodyID body_id;
        ObjectLayer layer = Layers::MOVING;
        uint32_t node_index = INVALID_INDEX;
        uint8_t child_slot = 0;
        bool is_valid = false;
    };

    explicit BroadPhase(size_t initial_capacity = 256) noexcept;
    ~BroadPhase() = default;

    BroadPhase(const BroadPhase&) = delete;
    BroadPhase& operator=(const BroadPhase&) = delete;
    BroadPhase(BroadPhase&&) noexcept = default;
    BroadPhase& operator=(BroadPhase&&) noexcept = default;

    // --- Body Lifecycle in Broad-Phase ---
    void add_body(BodyID id, const AABB& bounds, ObjectLayer layer = Layers::MOVING) noexcept;
    void remove_body(BodyID id) noexcept;
    void update_body(BodyID id, const AABB& new_bounds, const Vec3& displacement = Vec3::zero()) noexcept;

    [[nodiscard]] bool contains_body(BodyID id) const noexcept;
    [[nodiscard]] const AABB& get_fat_aabb(BodyID id) const noexcept;
    [[nodiscard]] size_t get_body_count() const noexcept { return m_active_body_count; }
    [[nodiscard]] size_t get_node_count() const noexcept { return m_nodes.size() - m_free_nodes.size(); }
    [[nodiscard]] uint32_t get_root_index() const noexcept { return m_root; }

    // --- Spatial Queries (Stack-based, 0 Heap Allocations) ---
    using AABBVisitor = std::function<void(BodyID)>;
    using RayVisitor = std::function<void(BodyID, float)>;
    using PairVisitor = std::function<void(BodyID, BodyID)>;

    /// @brief Queries the BVH with an AABB, invoking visitor for each intersecting leaf.
    void query_aabb(const AABB& aabb, const AABBVisitor& visitor) const noexcept;

    /// @brief Casts a ray through the 4-way BVH, testing 4 nodes simultaneously.
    void cast_ray(const Ray& ray, float max_t, const RayVisitor& visitor) const noexcept;

    /// @brief Finds all potentially colliding pairs between dynamic bodies and scene geometry.
    void find_colliding_pairs(
        const ObjectLayerPairFilter& filter,
        std::vector<BodyPair>& out_pairs
    ) const noexcept;

    /// @brief Optimizes the tree via bottom-up or top-down Surface Area Heuristic.
    void rebuild() noexcept;

    /// @brief Clears all bodies and resets the tree.
    void clear() noexcept;

private:
    uint32_t allocate_node() noexcept;
    void free_node(uint32_t node_idx) noexcept;
    void refit_upwards(uint32_t node_idx) noexcept;
    void insert_leaf_recursive(uint32_t node_idx, uint32_t leaf_idx) noexcept;

    std::vector<Node> m_nodes;
    std::vector<uint32_t> m_free_nodes;
    std::vector<Leaf> m_leaves;
    std::vector<uint32_t> m_active_leaf_indices;

    uint32_t m_root = INVALID_INDEX;
    size_t m_active_body_count = 0;
    float m_speculative_margin = 0.05f; // 5cm default fat AABB margin
};

} // namespace orion::physics
