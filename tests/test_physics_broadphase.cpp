#include "orion/physics/physics.hpp"
#include <cassert>
#include <iostream>
#include <cmath>
#include <vector>
#include <unordered_set>

using namespace orion;
using namespace orion::physics;

static int g_tests_passed = 0;
static int g_tests_total = 0;

#define TEST_ASSERT(cond, msg) \
    do { \
        g_tests_total++; \
        if (!(cond)) { \
            std::cerr << "[-] FAILED: " << msg << " at " << __FILE__ << ":" << __LINE__ << std::endl; \
            assert(false && msg); \
        } else { \
            g_tests_passed++; \
        } \
    } while(0)

void test_aabb4_simd_evaluation() {
    std::cout << "[*] Testing AABB4 SIMD 4-Way Overlap & Raycast..." << std::endl;

    AABB4 aabb4;

    // Box 0: center (0, 0, 0), half-extents (1, 1, 1) -> [-1, 1]
    const AABB b0(Vec3(-1.0f, -1.0f, -1.0f), Vec3(1.0f, 1.0f, 1.0f));
    // Box 1: center (5, 0, 0), half-extents (1, 1, 1) -> [4, 6]
    const AABB b1(Vec3(4.0f, -1.0f, -1.0f), Vec3(6.0f, 1.0f, 1.0f));
    // Box 2: center (0, 5, 0), half-extents (1, 1, 1) -> [4, 6] along Y
    const AABB b2(Vec3(-1.0f, 4.0f, -1.0f), Vec3(1.0f, 6.0f, 1.0f));
    // Box 3: center (10, 10, 10), half-extents (1, 1, 1)
    const AABB b3(Vec3(9.0f, 9.0f, 9.0f), Vec3(11.0f, 11.0f, 11.0f));

    aabb4.set_box(0, b0);
    aabb4.set_box(1, b1);
    aabb4.set_box(2, b2);
    aabb4.set_box(3, b3);

    // Verify getter
    TEST_ASSERT(aabb4.get_box(0).min_point.x == -1.0f, "AABB4 get_box(0) min_point.x matches");
    TEST_ASSERT(aabb4.get_box(1).min_point.x == 4.0f, "AABB4 get_box(1) min_point.x matches");

    // 1. Query AABB overlapping only Box 0: center (0.5, 0, 0), extents 0.5 -> [0, 1]
    const AABB q0(Vec3(0.0f, -0.5f, -0.5f), Vec3(1.0f, 0.5f, 0.5f));
    uint32_t mask0 = aabb4.test_overlap(q0);
    TEST_ASSERT(mask0 == (1 << 0), "AABB4 test_overlap hits exclusively Box 0 (mask == 1)");

    // 2. Query AABB overlapping Box 0, Box 1, Box 2: [-1, 6] in X and Y
    const AABB q_multi(Vec3(-0.5f, -0.5f, -0.5f), Vec3(5.5f, 5.5f, 0.5f));
    uint32_t mask_multi = aabb4.test_overlap(q_multi);
    TEST_ASSERT(mask_multi == ((1 << 0) | (1 << 1) | (1 << 2)), "AABB4 test_overlap hits boxes 0, 1, 2 (mask == 7)");

    // 3. Query AABB outside all boxes
    const AABB q_none(Vec3(100.0f, 100.0f, 100.0f), Vec3(101.0f, 101.0f, 101.0f));
    uint32_t mask_none = aabb4.test_overlap(q_none);
    TEST_ASSERT(mask_none == 0, "AABB4 test_overlap outside all boxes returns 0");

    // 4. SIMD Raycast through Box 0 and Box 1
    // Ray origin (-5, 0, 0), direction (+1, 0, 0)
    const Ray ray_x(Vec3(-5.0f, 0.0f, 0.0f), Vec3(1.0f, 0.0f, 0.0f));
    float hit_t[4] = {0.0f};
    uint32_t ray_mask = aabb4.test_ray(ray_x, 50.0f, hit_t);

    TEST_ASSERT(ray_mask == ((1 << 0) | (1 << 1)), "Ray along X hits Box 0 and Box 1");
    TEST_ASSERT(approx_equal(hit_t[0], 4.0f, 0.01f), "Ray entry distance to Box 0 is 4.0 m");
    TEST_ASSERT(approx_equal(hit_t[1], 9.0f, 0.01f), "Ray entry distance to Box 1 is 9.0 m");

    // 5. Short Raycast limited by max_t
    float hit_t_short[4] = {0.0f};
    uint32_t short_mask = aabb4.test_ray(ray_x, 5.0f, hit_t_short);
    TEST_ASSERT(short_mask == (1 << 0), "Short ray (max_t = 5) hits Box 0 but culls Box 1");

    std::cout << "  [+] AABB4 SIMD overlap and slab raycast verified." << std::endl;
}

void test_broadphase_tree_lifecycle() {
    std::cout << "[*] Testing BroadPhase 4-Way Dynamic BVH Tree Construction & Alignment..." << std::endl;

    // Node layout assertion: exactly 128 bytes, 16-byte aligned
    TEST_ASSERT(sizeof(BroadPhase::Node) == 128, "BroadPhase::Node is strictly 128 bytes (2 cache lines)");
    TEST_ASSERT(alignof(BroadPhase::Node) == 16, "BroadPhase::Node is 16-byte aligned");

    BroadPhase bp;
    TEST_ASSERT(bp.get_body_count() == 0, "Initial BroadPhase is empty");
    TEST_ASSERT(bp.get_root_index() == BroadPhase::INVALID_INDEX, "Initial root is INVALID_INDEX");

    // Insert 64 bodies distributed in an 4x4x4 spatial grid
    const int grid_dim = 4;
    const float spacing = 5.0f;
    std::vector<BodyID> created_ids;

    for (int x = 0; x < grid_dim; ++x) {
        for (int y = 0; y < grid_dim; ++y) {
            for (int z = 0; z < grid_dim; ++z) {
                const uint32_t idx = static_cast<uint32_t>(created_ids.size() + 1);
                const BodyID id(idx, 1);
                const Vec3 pos(x * spacing, y * spacing, z * spacing);
                const AABB box(pos - Vec3(1.0f, 1.0f, 1.0f), pos + Vec3(1.0f, 1.0f, 1.0f));

                bp.add_body(id, box, Layers::MOVING);
                created_ids.push_back(id);
            }
        }
    }

    TEST_ASSERT(bp.get_body_count() == 64, "BroadPhase contains 64 bodies");
    TEST_ASSERT(bp.get_root_index() != BroadPhase::INVALID_INDEX, "Tree root is valid");

    // Spatial Query around center body (x = 1, y = 1, z = 1) -> pos (5, 5, 5)
    const AABB local_query(Vec3(4.5f, 4.5f, 4.5f), Vec3(5.5f, 5.5f, 5.5f));
    std::vector<BodyID> query_hits;
    bp.query_aabb(local_query, [&](BodyID hit_id) {
        query_hits.push_back(hit_id);
    });

    TEST_ASSERT(query_hits.size() == 1, "Localized query matches exactly 1 center body");

    // Large volume query encompassing (0,0,0) to (5,5,5) -> should hit 2x2x2 = 8 bodies
    const AABB subgrid_query(Vec3(-0.5f, -0.5f, -0.5f), Vec3(5.5f, 5.5f, 5.5f));
    std::vector<BodyID> subgrid_hits;
    bp.query_aabb(subgrid_query, [&](BodyID hit_id) {
        subgrid_hits.push_back(hit_id);
    });
    TEST_ASSERT(subgrid_hits.size() == 8, "Subgrid query accurately returned 8 bodies");

    // Raycast through entire X axis at y = 0, z = 0
    // Pierces 4 bodies at x = 0, 5, 10, 15
    const Ray ray_axis(Vec3(-10.0f, 0.0f, 0.0f), Vec3(1.0f, 0.0f, 0.0f));
    std::vector<BodyID> ray_hits;
    bp.cast_ray(ray_axis, 100.0f, [&](BodyID hit_id, float hit_dist) {
        (void)hit_dist;
        ray_hits.push_back(hit_id);
    });
    TEST_ASSERT(ray_hits.size() == 4, "Ray piercing along X axis hits exactly 4 bodies");

    std::cout << "  [+] 4-Way BVH grid population and spatial queries verified." << std::endl;
}

void test_fat_aabb_speculative_optimization() {
    std::cout << "[*] Testing Fat AABB Speculative Margin Optimization..." << std::endl;

    BroadPhase bp;
    const BodyID id(1, 1);
    const AABB initial_box(Vec3(-1.0f, -1.0f, -1.0f), Vec3(1.0f, 1.0f, 1.0f));

    bp.add_body(id, initial_box, Layers::MOVING);
    const AABB fat_aabb_initial = bp.get_fat_aabb(id);

    // Fat AABB must be expanded by speculative margin (default 0.05m = 5cm)
    TEST_ASSERT(approx_equal(fat_aabb_initial.min_point.x, -1.05f, 0.001f), "Fat AABB expanded min_point.x by 5cm");
    TEST_ASSERT(approx_equal(fat_aabb_initial.max_point.x, 1.05f, 0.001f), "Fat AABB expanded max_point.x by 5cm");

    // 1. Micro-movement: move body by 1cm (0.01m). It remains inside fat AABB!
    const AABB moved_small(Vec3(-0.99f, -1.0f, -1.0f), Vec3(1.01f, 1.0f, 1.0f));
    bp.update_body(id, moved_small, Vec3(0.01f, 0.0f, 0.0f));

    // Fat AABB in the tree should NOT have changed (zero tree refit!)
    const AABB fat_aabb_small = bp.get_fat_aabb(id);
    TEST_ASSERT(fat_aabb_small.min_point.x == fat_aabb_initial.min_point.x, "Small movement did not trigger tree refit");

    // 2. Macro-movement: move body by 20cm (0.20m). It exits fat AABB!
    const AABB moved_large(Vec3(0.0f, -1.0f, -1.0f), Vec3(2.0f, 1.0f, 1.0f));
    bp.update_body(id, moved_large, Vec3(0.20f, 0.0f, 0.0f));

    // Fat AABB must have re-expanded around new position
    const AABB fat_aabb_large = bp.get_fat_aabb(id);
    TEST_ASSERT(fat_aabb_large.max_point.x >= 2.05f, "Large movement triggered tree re-expansion");

    std::cout << "  [+] Fat AABB speculative margin optimization verified." << std::endl;
}

void test_dynamic_removal_and_recycling() {
    std::cout << "[*] Testing Body Removal & Node Pool Recycling..." << std::endl;

    BroadPhase bp;
    std::vector<BodyID> ids;

    // Add 32 bodies
    for (uint32_t i = 1; i <= 32; ++i) {
        BodyID id(i, 1);
        AABB box(Vec3(i * 3.0f, 0, 0), Vec3(i * 3.0f + 1.0f, 1.0f, 1.0f));
        bp.add_body(id, box, Layers::MOVING);
        ids.push_back(id);
    }
    TEST_ASSERT(bp.get_body_count() == 32, "32 bodies added");

    const size_t initial_nodes = bp.get_node_count();

    // Remove half of the bodies (16 bodies)
    for (size_t i = 0; i < 16; ++i) {
        bp.remove_body(ids[i]);
        TEST_ASSERT(!bp.contains_body(ids[i]), "Removed body is no longer in broad-phase");
    }
    TEST_ASSERT(bp.get_body_count() == 16, "16 bodies remain");

    // Re-add 16 new bodies in the freed spatial region
    for (uint32_t i = 1; i <= 16; ++i) {
        BodyID id(100 + i, 1);
        AABB box(Vec3(i * 3.0f, 0, 0), Vec3(i * 3.0f + 1.0f, 1.0f, 1.0f));
        bp.add_body(id, box, Layers::MOVING);
    }
    TEST_ASSERT(bp.get_body_count() == 32, "Body count back to 32");

    // Test tree rebuild
    bp.rebuild();
    TEST_ASSERT(bp.get_body_count() == 32, "Body count preserved after rebuild");
    TEST_ASSERT(bp.get_node_count() <= initial_nodes + 2, "Rebuilt tree maintains compact node footprint");

    std::cout << "  [+] Dynamic removal and tree rebuild verified." << std::endl;
}

void test_collision_layer_filtering() {
    std::cout << "[*] Testing Collision Layer Pair Filtering..." << std::endl;

    BroadPhase bp;
    ObjectLayerPairFilter filter;
    std::vector<BodyPair> pairs;

    // Two Static boxes overlapping:
    // Box 1 (Static): [-1, 1]
    const BodyID s1(1, 1);
    bp.add_body(s1, AABB(Vec3(-1, -1, -1), Vec3(1, 1, 1)), Layers::STATIC);

    // Box 2 (Static): [-0.5, 0.5] (Overlaps Box 1!)
    const BodyID s2(2, 1);
    bp.add_body(s2, AABB(Vec3(-0.5f, -0.5f, -0.5f), Vec3(0.5f, 0.5f, 0.5f)), Layers::STATIC);

    // Find pairs: Static vs Static must NEVER be reported!
    bp.find_colliding_pairs(filter, pairs);
    TEST_ASSERT(pairs.empty(), "Static vs Static overlapping boxes are filtered out (0 pairs)");

    // Box 3 (Moving): [0, 2] (Overlaps Box 1 and Box 2!)
    const BodyID m3(3, 1);
    bp.add_body(m3, AABB(Vec3(0, -1, -1), Vec3(2, 1, 1)), Layers::MOVING);

    bp.find_colliding_pairs(filter, pairs);
    // Moving Box 3 collides with Static Box 1 and Static Box 2
    TEST_ASSERT(pairs.size() == 2, "Moving body generates 2 pairs with overlapping static bodies");

    // Box 4 (Moving): [0.5, 2.5] (Overlaps Box 3!)
    const BodyID m4(4, 1);
    bp.add_body(m4, AABB(Vec3(0.5f, -1, -1), Vec3(2.5f, 1, 1)), Layers::MOVING);

    bp.find_colliding_pairs(filter, pairs);
    // Box 3 collides with (s1, s2, m4), Box 4 collides with (s1, s2, m3) -> total 5 unique pairs
    TEST_ASSERT(pairs.size() == 5, "Moving vs Moving pairs generated without duplicate canonical pairs");

    std::cout << "  [+] Broad-phase layer filtering verified." << std::endl;
}

void test_large_scale_dense_broadphase_benchmark() {
    std::cout << "[*] Testing Large-Scale Spatial Correctness (BVH4 vs Brute-Force Ground Truth)..." << std::endl;

    BroadPhase bp;
    struct BodyEntry {
        BodyID id;
        AABB box;
    };
    std::vector<BodyEntry> ground_truth;
    const int num_bodies = 200;

    for (int i = 0; i < num_bodies; ++i) {
        const float x = static_cast<float>((i * 17) % 100) - 50.0f;
        const float y = static_cast<float>((i * 31) % 100) - 50.0f;
        const float z = static_cast<float>((i * 47) % 100) - 50.0f;
        const Vec3 center(x, y, z);
        const Vec3 extents(1.5f, 1.5f, 1.5f);
        const AABB box(center - extents, center + extents);
        const BodyID id(i + 1, 1);

        bp.add_body(id, box, Layers::MOVING);
        ground_truth.push_back({id, box});
    }
    TEST_ASSERT(bp.get_body_count() == 200, "200 bodies inserted into BVH4 tree");

    // Perform 20 spatial queries and verify against ground truth
    for (int q = 0; q < 20; ++q) {
        const float qx = static_cast<float>((q * 23) % 80) - 40.0f;
        const float qy = static_cast<float>((q * 37) % 80) - 40.0f;
        const float qz = static_cast<float>((q * 53) % 80) - 40.0f;
        const AABB query_box(Vec3(qx - 8.0f, qy - 8.0f, qz - 8.0f), Vec3(qx + 8.0f, qy + 8.0f, qz + 8.0f));

        std::unordered_set<uint32_t> bvh_results;
        bp.query_aabb(query_box, [&](BodyID hit_id) {
            bvh_results.insert(hit_id.index());
        });

        std::unordered_set<uint32_t> brute_results;
        for (const auto& entry : ground_truth) {
            // Note: broad-phase leaves hold fat_aabb
            const AABB fat_box = bp.get_fat_aabb(entry.id);
            if (fat_box.overlaps(query_box)) {
                brute_results.insert(entry.id.index());
            }
        }

        // Verify 100% equivalence: bvh_results == brute_results
        TEST_ASSERT(bvh_results == brute_results, "BVH4 tree query matches brute-force ground truth 100% exactly");
    }

    // Perform 10 Raycast queries and verify against brute-force ground truth
    for (int r = 0; r < 10; ++r) {
        const float rx = static_cast<float>((r * 19) % 60) - 30.0f;
        const float ry = static_cast<float>((r * 29) % 60) - 30.0f;
        const Ray ray(Vec3(rx, ry, -100.0f), Vec3(0.0f, 0.0f, 1.0f));

        std::unordered_set<uint32_t> bvh_ray_hits;
        bp.cast_ray(ray, 200.0f, [&](BodyID hit_id, float hit_t) {
            (void)hit_t;
            bvh_ray_hits.insert(hit_id.index());
        });

        std::unordered_set<uint32_t> brute_ray_hits;
        for (const auto& entry : ground_truth) {
            const AABB fat_box = bp.get_fat_aabb(entry.id);
            float t_min, t_max;
            if (fat_box.raycast(ray.origin, ray.direction, 200.0f, t_min, t_max)) {
                brute_ray_hits.insert(entry.id.index());
            }
        }

        TEST_ASSERT(bvh_ray_hits == brute_ray_hits, "BVH4 raycast matches brute-force ground truth 100% exactly");
    }

    std::cout << "  [+] Large-scale BVH4 ground truth mathematical equivalence verified." << std::endl;
}

int main() {
    std::cout << "=================================================" << std::endl;
    std::cout << "  Nisaba Sovereign 3D Broad-Phase (BVH4) Tests   " << std::endl;
    std::cout << "=================================================" << std::endl;

    test_aabb4_simd_evaluation();
    test_broadphase_tree_lifecycle();
    test_fat_aabb_speculative_optimization();
    test_dynamic_removal_and_recycling();
    test_collision_layer_filtering();
    test_large_scale_dense_broadphase_benchmark();

    std::cout << "=================================================" << std::endl;
    std::cout << "[+] ALL TESTS PASSED: " << g_tests_passed << "/" << g_tests_total << " assertions verified!" << std::endl;
    std::cout << "=================================================" << std::endl;
    return 0;
}
