#include "nisaba/physics/broadphase/broad_phase.hpp"
#include <algorithm>

namespace nisaba::physics {

BroadPhase::BroadPhase(size_t initial_capacity) noexcept {
    m_nodes.reserve(initial_capacity);
    m_free_nodes.reserve(initial_capacity / 2);
    m_leaves.reserve(initial_capacity);
    m_active_leaf_indices.reserve(initial_capacity);
}

void BroadPhase::clear() noexcept {
    m_nodes.clear();
    m_free_nodes.clear();
    m_leaves.clear();
    m_active_leaf_indices.clear();
    m_root = INVALID_INDEX;
    m_active_body_count = 0;
}

uint32_t BroadPhase::allocate_node() noexcept {
    if (!m_free_nodes.empty()) {
        uint32_t idx = m_free_nodes.back();
        m_free_nodes.pop_back();
        m_nodes[idx] = Node();
        return idx;
    }
    uint32_t idx = static_cast<uint32_t>(m_nodes.size());
    m_nodes.emplace_back();
    return idx;
}

void BroadPhase::free_node(uint32_t node_idx) noexcept {
    if (node_idx < m_nodes.size()) {
        m_free_nodes.push_back(node_idx);
    }
}

void BroadPhase::refit_upwards(uint32_t node_idx) noexcept {
    while (node_idx != INVALID_INDEX) {
        Node& node = m_nodes[node_idx];
        AABB union_box;

        for (int i = 0; i < 4; ++i) {
            if (node.children[i] != INVALID_INDEX) {
                union_box.encapsulate(node.aabb4.get_box(i));
            }
        }

        uint32_t parent_idx = node.parent;
        if (parent_idx != INVALID_INDEX) {
            Node& parent_node = m_nodes[parent_idx];
            for (int s = 0; s < 4; ++s) {
                if (parent_node.children[s] == node_idx) {
                    parent_node.aabb4.set_box(s, union_box);
                    break;
                }
            }
        }
        node_idx = parent_idx;
    }
}

void BroadPhase::add_body(BodyID id, const AABB& bounds, ObjectLayer layer) noexcept {
    const uint32_t leaf_idx = id.index();
    if (leaf_idx >= m_leaves.size()) {
        m_leaves.resize(leaf_idx + 1);
    }

    Leaf& leaf = m_leaves[leaf_idx];
    leaf.tight_bounds = bounds;
    leaf.fat_aabb = bounds;
    leaf.fat_aabb.expand(m_speculative_margin);
    leaf.body_id = id;
    leaf.layer = layer;
    leaf.is_valid = true;
    m_active_leaf_indices.push_back(leaf_idx);

    if (m_root == INVALID_INDEX) {
        m_root = allocate_node();
        Node& root_node = m_nodes[m_root];
        root_node.children[0] = leaf_idx;
        root_node.aabb4.set_box(0, leaf.fat_aabb);
        root_node.is_leaf_mask |= (1 << 0);
        root_node.num_children = 1;

        leaf.node_index = m_root;
        leaf.child_slot = 0;
    } else {
        insert_leaf_recursive(m_root, leaf_idx);
    }

    m_active_body_count++;
}

void BroadPhase::insert_leaf_recursive(uint32_t node_idx, uint32_t leaf_idx) noexcept {
    Leaf& leaf = m_leaves[leaf_idx];
    Node& node = m_nodes[node_idx];

    // Case 1: Node has available child slots (< 4)
    if (node.num_children < 4) {
        for (int s = 0; s < 4; ++s) {
            if (node.children[s] == INVALID_INDEX) {
                node.children[s] = leaf_idx;
                node.aabb4.set_box(s, leaf.fat_aabb);
                node.is_leaf_mask |= (1 << s);
                node.num_children++;

                leaf.node_index = node_idx;
                leaf.child_slot = static_cast<uint8_t>(s);
                refit_upwards(node_idx);
                return;
            }
        }
    }

    // Case 2: Node is full (4 children). Choose best child via Surface Area Heuristic (SAH)
    float min_area_increase = LARGE_FLOAT;
    int best_child = 0;

    for (int i = 0; i < 4; ++i) {
        const AABB cur_box = node.aabb4.get_box(i);
        AABB merged_box = cur_box;
        merged_box.encapsulate(leaf.fat_aabb);
        const float area_increase = merged_box.surface_area() - cur_box.surface_area();

        if (area_increase < min_area_increase) {
            min_area_increase = area_increase;
            best_child = i;
        }
    }

    // If the chosen best child is currently a leaf, split it by creating a new internal node
    if (node.is_leaf_mask & (1 << best_child)) {
        const uint32_t old_leaf_idx = node.children[best_child];
        Leaf& old_leaf = m_leaves[old_leaf_idx];

        const uint32_t new_internal_idx = allocate_node();
        Node& new_internal = m_nodes[new_internal_idx];
        new_internal.parent = node_idx;

        // Old leaf into slot 0
        new_internal.children[0] = old_leaf_idx;
        new_internal.aabb4.set_box(0, old_leaf.fat_aabb);
        new_internal.is_leaf_mask |= (1 << 0);
        old_leaf.node_index = new_internal_idx;
        old_leaf.child_slot = 0;

        // New leaf into slot 1
        new_internal.children[1] = leaf_idx;
        new_internal.aabb4.set_box(1, leaf.fat_aabb);
        new_internal.is_leaf_mask |= (1 << 1);
        leaf.node_index = new_internal_idx;
        leaf.child_slot = 1;

        new_internal.num_children = 2;

        // Replace best_child slot in parent node with new_internal
        AABB union_box = old_leaf.fat_aabb;
        union_box.encapsulate(leaf.fat_aabb);

        node.children[best_child] = new_internal_idx;
        node.aabb4.set_box(best_child, union_box);
        node.is_leaf_mask &= ~(1 << best_child); // Slot is now internal node

        refit_upwards(node_idx);
    } else {
        // Best child is already an internal node: descend into it
        const uint32_t child_node_idx = node.children[best_child];
        insert_leaf_recursive(child_node_idx, leaf_idx);
    }
}

void BroadPhase::remove_body(BodyID id) noexcept {
    const uint32_t leaf_idx = id.index();
    if (leaf_idx >= m_leaves.size()) return;

    Leaf& leaf = m_leaves[leaf_idx];
    if (!leaf.is_valid) return;

    const uint32_t node_idx = leaf.node_index;
    const uint8_t slot = leaf.child_slot;

    Node& node = m_nodes[node_idx];
    node.children[slot] = INVALID_INDEX;
    node.aabb4.set_box(slot, AABB());
    node.is_leaf_mask &= ~(1 << slot);
    node.num_children--;

    leaf.is_valid = false;
    m_active_body_count--;

    // Remove from active list
    for (size_t i = 0; i < m_active_leaf_indices.size(); ++i) {
        if (m_active_leaf_indices[i] == leaf_idx) {
            m_active_leaf_indices[i] = m_active_leaf_indices.back();
            m_active_leaf_indices.pop_back();
            break;
        }
    }

    // If node became empty and is not root, remove it from parent
    if (node.num_children == 0 && node_idx != m_root) {
        const uint32_t parent_idx = node.parent;
        if (parent_idx != INVALID_INDEX) {
            Node& parent_node = m_nodes[parent_idx];
            for (int s = 0; s < 4; ++s) {
                if (parent_node.children[s] == node_idx) {
                    parent_node.children[s] = INVALID_INDEX;
                    parent_node.aabb4.set_box(s, AABB());
                    parent_node.num_children--;
                    break;
                }
            }
            free_node(node_idx);
            refit_upwards(parent_idx);
            return;
        }
    }

    refit_upwards(node_idx);
}

void BroadPhase::update_body(BodyID id, const AABB& new_bounds, const Vec3& displacement) noexcept {
    const uint32_t leaf_idx = id.index();
    if (leaf_idx >= m_leaves.size()) return;

    Leaf& leaf = m_leaves[leaf_idx];
    if (!leaf.is_valid) return;

    leaf.tight_bounds = new_bounds;

    // Fat AABB Speculative Optimization: if new tight bounds are contained, skip tree refit!
    if (leaf.fat_aabb.contains(new_bounds)) {
        return;
    }

    // Re-expand fat AABB and update tree
    leaf.fat_aabb = new_bounds;
    leaf.fat_aabb.expand(m_speculative_margin);

    if (displacement.length_sq() > EPSILON * EPSILON) {
        leaf.fat_aabb.encapsulate(new_bounds.min_point + displacement * 2.0f);
        leaf.fat_aabb.encapsulate(new_bounds.max_point + displacement * 2.0f);
    }

    m_nodes[leaf.node_index].aabb4.set_box(leaf.child_slot, leaf.fat_aabb);
    refit_upwards(leaf.node_index);
}

bool BroadPhase::contains_body(BodyID id) const noexcept {
    const uint32_t idx = id.index();
    return idx < m_leaves.size() && m_leaves[idx].is_valid;
}

const AABB& BroadPhase::get_fat_aabb(BodyID id) const noexcept {
    static const AABB invalid_aabb;
    const uint32_t idx = id.index();
    if (idx < m_leaves.size() && m_leaves[idx].is_valid) {
        return m_leaves[idx].fat_aabb;
    }
    return invalid_aabb;
}

void BroadPhase::query_aabb(const AABB& aabb, const AABBVisitor& visitor) const noexcept {
    if (m_root == INVALID_INDEX) return;

    uint32_t stack[64];
    int stack_ptr = 0;
    stack[stack_ptr++] = m_root;

    while (stack_ptr > 0) {
        const uint32_t node_idx = stack[--stack_ptr];
        const Node& node = m_nodes[node_idx];

        const uint32_t mask = node.aabb4.test_overlap(aabb);
        if (mask == 0) continue;

        for (int i = 0; i < 4; ++i) {
            if (mask & (1 << i)) {
                if (node.is_leaf_mask & (1 << i)) {
                    const uint32_t leaf_idx = node.children[i];
                    if (leaf_idx != INVALID_INDEX && m_leaves[leaf_idx].is_valid) {
                        visitor(m_leaves[leaf_idx].body_id);
                    }
                } else {
                    const uint32_t child_node_idx = node.children[i];
                    if (child_node_idx != INVALID_INDEX && stack_ptr < 64) {
                        stack[stack_ptr++] = child_node_idx;
                    }
                }
            }
        }
    }
}

void BroadPhase::cast_ray(const Ray& ray, float max_t, const RayVisitor& visitor) const noexcept {
    if (m_root == INVALID_INDEX) return;

    uint32_t stack[64];
    int stack_ptr = 0;
    stack[stack_ptr++] = m_root;

    while (stack_ptr > 0) {
        const uint32_t node_idx = stack[--stack_ptr];
        const Node& node = m_nodes[node_idx];

        float hit_t[4];
        const uint32_t mask = node.aabb4.test_ray(ray, max_t, hit_t);
        if (mask == 0) continue;

        for (int i = 0; i < 4; ++i) {
            if (mask & (1 << i)) {
                if (node.is_leaf_mask & (1 << i)) {
                    const uint32_t leaf_idx = node.children[i];
                    if (leaf_idx != INVALID_INDEX && m_leaves[leaf_idx].is_valid) {
                        visitor(m_leaves[leaf_idx].body_id, hit_t[i]);
                    }
                } else {
                    const uint32_t child_node_idx = node.children[i];
                    if (child_node_idx != INVALID_INDEX && stack_ptr < 64) {
                        stack[stack_ptr++] = child_node_idx;
                    }
                }
            }
        }
    }
}

void BroadPhase::find_colliding_pairs(
    const ObjectLayerPairFilter& filter,
    std::vector<BodyPair>& out_pairs
) const noexcept {
    out_pairs.clear();

    for (uint32_t leaf_idx : m_active_leaf_indices) {
        const Leaf& leaf_a = m_leaves[leaf_idx];
        if (!leaf_a.is_valid) continue;

        // Static bodies do not initiate queries to avoid duplicate / static-static checks
        if (leaf_a.layer == Layers::STATIC) continue;

        query_aabb(leaf_a.fat_aabb, [&](BodyID id_b) {
            if (leaf_a.body_id == id_b) return;

            const uint32_t b_idx = id_b.index();
            if (b_idx >= m_leaves.size() || !m_leaves[b_idx].is_valid) return;
            const Leaf& leaf_b = m_leaves[b_idx];

            // Canonical order for dynamic pairs: A < B to prevent duplicates
            if (leaf_b.layer != Layers::STATIC && leaf_a.body_id > id_b) return;

            if (filter.should_collide(leaf_a.layer, leaf_b.layer)) {
                out_pairs.emplace_back(leaf_a.body_id, id_b);
            }
        });
    }
}

void BroadPhase::rebuild() noexcept {
    if (m_active_leaf_indices.empty()) {
        clear();
        return;
    }

    std::vector<uint32_t> saved_leaves = m_active_leaf_indices;
    std::vector<Leaf> saved_leaf_data;
    saved_leaf_data.reserve(saved_leaves.size());

    for (uint32_t idx : saved_leaves) {
        saved_leaf_data.push_back(m_leaves[idx]);
    }

    clear();

    for (const Leaf& l : saved_leaf_data) {
        add_body(l.body_id, l.tight_bounds, l.layer);
    }
}

} // namespace nisaba::physics
