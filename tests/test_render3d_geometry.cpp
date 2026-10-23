#include "orion/orion.hpp"
#include <cassert>
#include <iostream>
#include <iomanip>
#include <cmath>

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

static constexpr float TEST_EPSILON = 1e-4f;

static bool approx(float a, float b, float eps = TEST_EPSILON) {
    return std::abs(a - b) <= eps;
}

void test_vertex_layout_and_strides() {
    std::cout << "[*] Testing VertexLayout & Vertex3D memory layout..." << std::endl;

    TEST_ASSERT(sizeof(Vertex3D) == 64, "Vertex3D must be exactly 64 bytes (cache-line aligned)");
    TEST_ASSERT(alignof(Vertex3D) == 16, "Vertex3D must be 16-byte aligned");

    VertexLayout layout = VertexLayout::standard_pbr();
    TEST_ASSERT(layout.stride == 64, "PBR layout stride must be 64 bytes");
    TEST_ASSERT(layout.attributes.size() == 5, "PBR layout must have 5 attributes");

    TEST_ASSERT(layout.attributes[0].attribute == VertexAttribute::Position, "Attribute 0: Position");
    TEST_ASSERT(layout.attributes[0].offset == 0, "Position offset must be 0");

    TEST_ASSERT(layout.attributes[1].attribute == VertexAttribute::Normal, "Attribute 1: Normal");
    TEST_ASSERT(layout.attributes[1].offset == 16, "Normal offset must be 16");

    TEST_ASSERT(layout.attributes[2].attribute == VertexAttribute::Tangent, "Attribute 2: Tangent");
    TEST_ASSERT(layout.attributes[2].offset == 32, "Tangent offset must be 32");

    TEST_ASSERT(layout.attributes[3].attribute == VertexAttribute::TexCoord0, "Attribute 3: TexCoord0");
    TEST_ASSERT(layout.attributes[3].offset == 48, "TexCoord0 offset must be 48");

    TEST_ASSERT(layout.attributes[4].attribute == VertexAttribute::TexCoord1, "Attribute 4: TexCoord1");
    TEST_ASSERT(layout.attributes[4].offset == 56, "TexCoord1 offset must be 56");
}

void test_vertex_and_index_buffers() {
    std::cout << "[*] Testing VertexBuffer3D and IndexBuffer3D..." << std::endl;

    std::vector<Vertex3D> verts = {
        Vertex3D(Vec3(0.0f, 0.0f, 0.0f), Vec3(0.0f, 1.0f, 0.0f), 0.0f, 0.0f),
        Vertex3D(Vec3(1.0f, 0.0f, 0.0f), Vec3(0.0f, 1.0f, 0.0f), 1.0f, 0.0f),
        Vertex3D(Vec3(0.0f, 0.0f, 1.0f), Vec3(0.0f, 1.0f, 0.0f), 0.0f, 1.0f)
    };

    VertexBuffer3D vb;
    vb.set_vertices(verts);
    TEST_ASSERT(vb.vertex_count() == 3, "Vertex buffer count must be 3");
    TEST_ASSERT(vb.byte_size() == 3 * sizeof(Vertex3D), "Vertex buffer byte size matches 3 * 64");
    TEST_ASSERT(vb.data() != nullptr, "Vertex buffer data pointer valid");

    // 16-bit index buffer
    std::vector<uint16_t> idx16 = {0, 1, 2};
    IndexBuffer3D ib16;
    ib16.set_indices(idx16);
    TEST_ASSERT(ib16.index_count() == 3, "16-bit index count matches");
    TEST_ASSERT(ib16.index_stride() == 2, "16-bit stride is 2");
    TEST_ASSERT(ib16.get_index(1) == 1, "16-bit index retrieval matches");

    // 32-bit index buffer
    std::vector<uint32_t> idx32 = {10, 20, 30, 40};
    IndexBuffer3D ib32;
    ib32.set_indices(idx32);
    TEST_ASSERT(ib32.index_count() == 4, "32-bit index count matches");
    TEST_ASSERT(ib32.index_stride() == 4, "32-bit stride is 4");
    TEST_ASSERT(ib32.get_index(2) == 30, "32-bit index retrieval matches");
}

void test_mesh_cube() {
    std::cout << "[*] Testing procedural Cube generation..." << std::endl;

    Mesh3D cube = MeshPrimitives::create_cube(Vec3(2.0f, 4.0f, 6.0f));

    TEST_ASSERT(cube.vertices.size() == 24, "Cube must have 24 vertices (4 per face)");
    TEST_ASSERT(cube.indices.size() == 36, "Cube must have 36 indices (6 per face)");
    TEST_ASSERT(cube.submeshes.size() == 1, "Cube has 1 submesh");

    // Bounds checking
    const AABB& b = cube.get_bounds();
    TEST_ASSERT(approx(b.min_point.x, -1.0f) && approx(b.max_point.x, 1.0f), "Cube bounds X [-1, 1]");
    TEST_ASSERT(approx(b.min_point.y, -2.0f) && approx(b.max_point.y, 2.0f), "Cube bounds Y [-2, 2]");
    TEST_ASSERT(approx(b.min_point.z, -3.0f) && approx(b.max_point.z, 3.0f), "Cube bounds Z [-3, 3]");

    // Verify all face normals are unit vectors
    for (const auto& v : cube.vertices) {
        TEST_ASSERT(approx(v.normal.length(), 1.0f), "Cube normals must be normalized");
    }

    // Verify GPU buffers uploaded
    TEST_ASSERT(cube.vertex_buffer.vertex_count() == 24, "Cube vertex buffer uploaded");
    TEST_ASSERT(cube.index_buffer.index_count() == 36, "Cube index buffer uploaded");
}

void test_mesh_sphere() {
    std::cout << "[*] Testing procedural Sphere generation..." << std::endl;

    const float radius = 2.5f;
    Mesh3D sphere = MeshPrimitives::create_sphere(radius, 16, 8);

    TEST_ASSERT(!sphere.vertices.empty(), "Sphere has vertices");
    TEST_ASSERT(!sphere.indices.empty(), "Sphere has indices");

    // All vertex positions must lie on the sphere surface of radius 2.5
    for (const auto& v : sphere.vertices) {
        const float dist = v.position.length();
        TEST_ASSERT(approx(dist, radius, 1e-2f), "Sphere vertex must lie on sphere surface");
        TEST_ASSERT(approx(v.normal.length(), 1.0f, 1e-3f), "Sphere normal must be unit length");
    }

    const AABB& b = sphere.get_bounds();
    TEST_ASSERT(approx(b.min_point.x, -radius, 1e-2f) && approx(b.max_point.x, radius, 1e-2f), "Sphere bounds X");
    TEST_ASSERT(approx(b.min_point.y, -radius, 1e-2f) && approx(b.max_point.y, radius, 1e-2f), "Sphere bounds Y");
    TEST_ASSERT(approx(b.min_point.z, -radius, 1e-2f) && approx(b.max_point.z, radius, 1e-2f), "Sphere bounds Z");
}

void test_mesh_plane() {
    std::cout << "[*] Testing procedural Plane generation..." << std::endl;

    Mesh3D plane = MeshPrimitives::create_plane(10.0f, 20.0f, 2, 4);

    TEST_ASSERT(plane.vertices.size() == (2 + 1) * (4 + 1), "Plane vertex count matches (sub_x+1)*(sub_z+1)");
    TEST_ASSERT(plane.indices.size() == 2 * 4 * 6, "Plane index count matches sub_x*sub_z*6");

    const AABB& b = plane.get_bounds();
    TEST_ASSERT(approx(b.min_point.x, -5.0f) && approx(b.max_point.x, 5.0f), "Plane bounds X [-5, 5]");
    TEST_ASSERT(approx(b.min_point.y, 0.0f) && approx(b.max_point.y, 0.0f), "Plane bounds Y [0, 0]");
    TEST_ASSERT(approx(b.min_point.z, -10.0f) && approx(b.max_point.z, 10.0f), "Plane bounds Z [-10, 10]");

    // Normals must point up (0, 1, 0)
    for (const auto& v : plane.vertices) {
        TEST_ASSERT(approx(v.normal.x, 0.0f) && approx(v.normal.y, 1.0f) && approx(v.normal.z, 0.0f), "Plane normals face up");
    }
}

void test_mesh_cylinder() {
    std::cout << "[*] Testing procedural Cylinder generation..." << std::endl;

    const float r = 1.0f;
    const float h = 4.0f;
    Mesh3D cyl = MeshPrimitives::create_cylinder(r, h, 16);

    TEST_ASSERT(!cyl.vertices.empty() && !cyl.indices.empty(), "Cylinder has geometry");

    const AABB& b = cyl.get_bounds();
    TEST_ASSERT(approx(b.min_point.x, -r, 1e-2f) && approx(b.max_point.x, r, 1e-2f), "Cylinder bounds X");
    TEST_ASSERT(approx(b.min_point.y, -h * 0.5f) && approx(b.max_point.y, h * 0.5f), "Cylinder bounds Y");
    TEST_ASSERT(approx(b.min_point.z, -r, 1e-2f) && approx(b.max_point.z, r, 1e-2f), "Cylinder bounds Z");
}

void test_mesh_torus() {
    std::cout << "[*] Testing procedural Torus generation..." << std::endl;

    const float R = 3.0f;
    const float r = 0.5f;
    Mesh3D torus = MeshPrimitives::create_torus(R, r, 16, 8);

    TEST_ASSERT(!torus.vertices.empty() && !torus.indices.empty(), "Torus has geometry");

    const AABB& b = torus.get_bounds();
    TEST_ASSERT(approx(b.min_point.x, -(R + r), 1e-2f) && approx(b.max_point.x, R + r, 1e-2f), "Torus bounds X");
    TEST_ASSERT(approx(b.min_point.y, -r, 1e-2f) && approx(b.max_point.y, r, 1e-2f), "Torus bounds Y");
    TEST_ASSERT(approx(b.min_point.z, -(R + r), 1e-2f) && approx(b.max_point.z, R + r, 1e-2f), "Torus bounds Z");
}

void test_normal_and_tangent_generation() {
    std::cout << "[*] Testing smooth normals & Gram-Schmidt tangent generation..." << std::endl;

    Mesh3D custom;
    // Quad in XY plane facing +Z
    custom.vertices = {
        Vertex3D(Vec3(-1.0f, -1.0f, 0.0f), Vec3(0.0f, 0.0f, 0.0f), 0.0f, 0.0f),
        Vertex3D(Vec3( 1.0f, -1.0f, 0.0f), Vec3(0.0f, 0.0f, 0.0f), 1.0f, 0.0f),
        Vertex3D(Vec3( 1.0f,  1.0f, 0.0f), Vec3(0.0f, 0.0f, 0.0f), 1.0f, 1.0f),
        Vertex3D(Vec3(-1.0f,  1.0f, 0.0f), Vec3(0.0f, 0.0f, 0.0f), 0.0f, 1.0f)
    };
    custom.indices = {0, 1, 2, 0, 2, 3};

    custom.compute_normals();
    for (const auto& v : custom.vertices) {
        TEST_ASSERT(approx(v.normal.x, 0.0f) && approx(v.normal.y, 0.0f) && approx(v.normal.z, 1.0f), "Computed normal points along +Z");
    }

    custom.compute_tangents();
    for (const auto& v : custom.vertices) {
        TEST_ASSERT(approx(v.tangent[0], 1.0f) && approx(v.tangent[1], 0.0f) && approx(v.tangent[2], 0.0f), "Computed tangent points along +X");
        TEST_ASSERT(approx(v.tangent[3], 1.0f), "Tangent handedness sign is positive");
    }
}

int main() {
    std::cout << "========================================================\n"
              << "   NISABA 3D RENDERING PIPELINE: GEOMETRY & MESH TEST   \n"
              << "========================================================" << std::endl;

    test_vertex_layout_and_strides();
    test_vertex_and_index_buffers();
    test_mesh_cube();
    test_mesh_sphere();
    test_mesh_plane();
    test_mesh_cylinder();
    test_mesh_torus();
    test_normal_and_tangent_generation();

    std::cout << "\n[+] ALL TESTS COMPLETED: " << g_tests_passed << "/" << g_tests_total << " assertions passed." << std::endl;
    return (g_tests_passed == g_tests_total) ? 0 : 1;
}
