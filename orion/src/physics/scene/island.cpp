#include "orion/physics/scene/island.hpp"
#include "orion/physics/scene/physics_scene.hpp"
#include <algorithm>

namespace orion::physics {

IslandBuilder::IslandBuilder() noexcept {
    reserve(128, 256);
}

void IslandBuilder::reserve(size_t body_capacity, size_t manifold_capacity) {
    (void)manifold_capacity;
    m_parent.reserve(body_capacity);
    m_rank.reserve(body_capacity);
    m_root_to_island.reserve(body_capacity);
    m_active_islands.reserve(std::max(size_t(16), body_capacity / 2));
    m_sleeping_islands.reserve(std::max(size_t(16), body_capacity / 2));
}

uint32_t IslandBuilder::find_root(uint32_t x) noexcept {
    uint32_t root = x;
    while (root != m_parent[root]) {
        root = m_parent[root];
    }
    // Path compression
    uint32_t curr = x;
    while (curr != root) {
        uint32_t next = m_parent[curr];
        m_parent[curr] = root;
        curr = next;
    }
    return root;
}

void IslandBuilder::union_sets(uint32_t a, uint32_t b) noexcept {
    uint32_t root_a = find_root(a);
    uint32_t root_b = find_root(b);
    if (root_a == root_b) return;

    if (m_rank[root_a] < m_rank[root_b]) {
        m_parent[root_a] = root_b;
    } else if (m_rank[root_a] > m_rank[root_b]) {
        m_parent[root_b] = root_a;
    } else {
        m_parent[root_b] = root_a;
        m_rank[root_a]++;
    }
}

void IslandBuilder::build_islands(PhysicsScene& scene, const PhysicsSettings& settings) {
    (void)settings;
    const size_t total_slots = scene.get_body_slot_count();
    if (total_slots == 0) {
        m_active_islands.clear();
        m_sleeping_islands.clear();
        return;
    }

    if (m_parent.size() < total_slots) {
        m_parent.resize(total_slots);
        m_rank.resize(total_slots);
        m_root_to_island.resize(total_slots);
    }

    for (size_t i = 0; i < total_slots; ++i) {
        m_parent[i] = static_cast<uint32_t>(i);
        m_rank[i] = 0;
        m_root_to_island[i] = 0xFFFFFFFF;
    }

    // 1. Process active contact manifolds: wake up sleeping bodies & union dynamic bodies
    const auto& manifolds = scene.get_active_manifolds();
    for (const auto& manifold : manifolds) {
        Body* b_a = scene.get_body(manifold.body_a);
        Body* b_b = scene.get_body(manifold.body_b);
        if (!b_a || !b_b) continue;

        // Wake up sleeping body and its entire island when contacted by an active moving body
        if (b_a->is_active() && b_b->is_sleeping()) {
            wake_up_island_containing(scene, manifold.body_b);
        } else if (b_b->is_active() && b_a->is_sleeping()) {
            wake_up_island_containing(scene, manifold.body_a);
        }

        // Union only if BOTH are dynamic (static/kinematic do NOT connect islands)
        if (b_a->is_dynamic() && b_b->is_dynamic()) {
            union_sets(manifold.body_a.index(), manifold.body_b.index());
        }
    }

    // 2. Clear active island buffers (retaining vector capacity)
    for (auto& island : m_active_islands) {
        island.bodies.clear();
        island.manifold_indices.clear();
        island.is_sleeping = false;
    }
    size_t active_island_count = 0;

    // 3. Cluster active dynamic bodies by Disjoint-Set root
    for (size_t i = 0; i < total_slots; ++i) {
        Body* b = scene.get_body_by_slot(static_cast<uint32_t>(i));
        if (!b || !b->is_dynamic()) continue;

        // If body was already sleeping and untouched, it stays asleep
        if (b->is_sleeping()) continue;

        const uint32_t root = find_root(static_cast<uint32_t>(i));
        uint32_t island_idx = m_root_to_island[root];

        if (island_idx == 0xFFFFFFFF) {
            island_idx = static_cast<uint32_t>(active_island_count++);
            m_root_to_island[root] = island_idx;

            if (island_idx >= m_active_islands.size()) {
                m_active_islands.emplace_back();
            }
            m_active_islands[island_idx].id = island_idx;
            m_active_islands[island_idx].is_sleeping = false;
        }

        m_active_islands[island_idx].bodies.push_back(b->get_id());
    }

    m_active_islands.resize(active_island_count);

    // 4. Assign contact manifolds to corresponding active islands
    for (size_t m = 0; m < manifolds.size(); ++m) {
        const auto& manifold = manifolds[m];
        Body* b_a = scene.get_body(manifold.body_a);
        Body* b_b = scene.get_body(manifold.body_b);
        if (!b_a || !b_b) continue;

        uint32_t target_root = 0xFFFFFFFF;
        if (b_a->is_dynamic() && b_a->is_active()) {
            target_root = find_root(manifold.body_a.index());
        } else if (b_b->is_dynamic() && b_b->is_active()) {
            target_root = find_root(manifold.body_b.index());
        }

        if (target_root != 0xFFFFFFFF && target_root < m_root_to_island.size()) {
            uint32_t island_idx = m_root_to_island[target_root];
            if (island_idx != 0xFFFFFFFF && island_idx < m_active_islands.size()) {
                m_active_islands[island_idx].manifold_indices.push_back(static_cast<uint32_t>(m));
            }
        }
    }
}

void IslandBuilder::update_sleeping(PhysicsScene& scene, float dt, const PhysicsSettings& settings) {
    if (!settings.enable_sleeping) return;

    for (size_t i = 0; i < m_active_islands.size();) {
        auto& island = m_active_islands[i];
        if (island.bodies.empty()) {
            ++i;
            continue;
        }

        bool all_bodies_calm = true;

        for (BodyID id : island.bodies) {
            Body* b = scene.get_body(id);
            if (!b || !b->is_dynamic()) continue;

            const float lin_vel_sq = b->get_linear_velocity().length_sq();
            const float ang_vel_sq = b->get_angular_velocity().length_sq();

            if (lin_vel_sq > settings.linear_sleep_threshold_sq ||
                ang_vel_sq > settings.angular_sleep_threshold_sq) {
                b->reset_sleep_timer();
                all_bodies_calm = false;
            } else {
                b->set_sleep_timer(b->get_sleep_timer() + dt);
                if (b->get_sleep_timer() < settings.time_before_sleep) {
                    all_bodies_calm = false;
                }
            }
        }

        if (all_bodies_calm) {
            // Entire island transitions to sleep!
            for (BodyID id : island.bodies) {
                Body* b = scene.get_body(id);
                if (b) {
                    b->put_to_sleep();
                }
            }
            island.is_sleeping = true;
            m_sleeping_islands.push_back(std::move(island));
            m_active_islands.erase(m_active_islands.begin() + i);
        } else {
            ++i;
        }
    }
}

void IslandBuilder::wake_up_island_containing(PhysicsScene& scene, BodyID id) {
    Body* target = scene.get_body(id);
    if (!target || !target->is_dynamic()) return;

    target->wake_up();

    // 1. Check sleeping islands first
    for (size_t i = 0; i < m_sleeping_islands.size(); ++i) {
        bool found = false;
        for (BodyID b_id : m_sleeping_islands[i].bodies) {
            if (b_id == id) {
                found = true;
                break;
            }
        }
        if (found) {
            for (BodyID b_id : m_sleeping_islands[i].bodies) {
                Body* b = scene.get_body(b_id);
                if (b) b->wake_up();
            }
            m_sleeping_islands[i].is_sleeping = false;
            m_active_islands.push_back(std::move(m_sleeping_islands[i]));
            m_sleeping_islands.erase(m_sleeping_islands.begin() + i);
            return;
        }
    }

    // 2. Check active islands
    for (const auto& island : m_active_islands) {
        bool found = false;
        for (BodyID b_id : island.bodies) {
            if (b_id == id) {
                found = true;
                break;
            }
        }
        if (found) {
            for (BodyID b_id : island.bodies) {
                Body* b = scene.get_body(b_id);
                if (b) b->wake_up();
            }
            break;
        }
    }
}

} // namespace orion::physics
