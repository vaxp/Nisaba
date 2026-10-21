#pragma once

#include "nisaba/physics/body/body_id.hpp"
#include <vector>
#include <cstdint>

namespace nisaba::physics {

class PhysicsScene;
struct PhysicsSettings;
struct ContactManifold;

/// @brief Represents an isolated group of interacting rigid bodies and contact manifolds.
/// Non-interacting groups are simulated in independent islands, enabling granular sleep states
/// and optimal cache locality during sequential impulse solving.
struct Island {
    uint32_t id = 0;                         ///< Unique sequential identifier for this island
    std::vector<BodyID> bodies;              ///< Dynamic bodies belonging to this island
    std::vector<uint32_t> manifold_indices;  ///< Indices of active manifolds / constraints
    bool is_sleeping = false;                ///< True if all bodies in this island are asleep
};

/// @brief High-performance Disjoint-Set Union (Union-Find) clustering engine.
/// Groups dynamic rigid bodies connected by active contact manifolds into independent simulation islands.
/// Maintains flat reusable buffers to ensure zero allocations during runtime ticks.
class IslandBuilder {
public:
    IslandBuilder() noexcept;
    ~IslandBuilder() = default;

    IslandBuilder(const IslandBuilder&) = delete;
    IslandBuilder& operator=(const IslandBuilder&) = delete;
    IslandBuilder(IslandBuilder&&) noexcept = default;
    IslandBuilder& operator=(IslandBuilder&&) noexcept = default;

    /// @brief Pre-allocates internal Disjoint-Set buffers to accommodate the scene capacity.
    void reserve(size_t body_capacity, size_t manifold_capacity);

    /// @brief Constructs simulation islands from active bodies and active contact manifolds.
    /// Wakes up sleeping bodies that come into contact with active bodies.
    void build_islands(PhysicsScene& scene, const PhysicsSettings& settings);

    /// @brief Evaluates kinetic energy across all bodies in each active island.
    /// If an entire island remains below velocity thresholds for time_before_sleep, puts it to sleep.
    void update_sleeping(PhysicsScene& scene, float dt, const PhysicsSettings& settings);

    /// @brief Wakes up a specific body and all connected bodies within its island.
    void wake_up_island_containing(PhysicsScene& scene, BodyID id);

    /// @brief Returns read-only reference to active simulation islands.
    [[nodiscard]] const std::vector<Island>& get_active_islands() const noexcept { return m_active_islands; }

    /// @brief Returns read-only reference to sleeping simulation islands.
    [[nodiscard]] const std::vector<Island>& get_sleeping_islands() const noexcept { return m_sleeping_islands; }

    /// @brief Returns read-only reference to active simulation islands (for backward compatibility).
    [[nodiscard]] const std::vector<Island>& get_islands() const noexcept { return m_active_islands; }

    /// @brief Returns total count of islands (active + sleeping).
    [[nodiscard]] size_t get_island_count() const noexcept { return m_active_islands.size() + m_sleeping_islands.size(); }

    /// @brief Returns count of active (non-sleeping) islands.
    [[nodiscard]] size_t get_active_island_count() const noexcept { return m_active_islands.size(); }

    /// @brief Returns count of sleeping islands.
    [[nodiscard]] size_t get_sleeping_island_count() const noexcept { return m_sleeping_islands.size(); }

private:
    // Disjoint-Set Union data structures
    std::vector<uint32_t> m_parent;
    std::vector<uint32_t> m_rank;
    std::vector<uint32_t> m_root_to_island;

    std::vector<Island> m_active_islands;
    std::vector<Island> m_sleeping_islands;

    uint32_t find_root(uint32_t x) noexcept;
    void union_sets(uint32_t a, uint32_t b) noexcept;
};

} // namespace nisaba::physics
