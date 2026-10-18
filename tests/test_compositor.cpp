#include "nisaba/compositor/compositor.hpp"
#include <cassert>
#include <iostream>
#include <cmath>

using namespace nisaba::compositor;

void test_geometry_and_region() {
    std::cout << "[Test] Geometry & Region..." << std::endl;

    RectI r1{0, 0, 100, 100};
    RectI r2{50, 50, 100, 100};
    assert(r1.intersects(r2));

    RectI inter = r1.intersection(r2);
    assert(inter.x == 50 && inter.y == 50 && inter.width == 50 && inter.height == 50);

    Region reg1(r1);
    assert(!reg1.isEmpty());
    assert(reg1.rects().size() == 1);

    Region reg2(r2);
    reg1.unite(reg2);
    assert(!reg1.isEmpty());
    RectI ext = reg1.extents();
    assert(ext.x == 0 && ext.y == 0 && ext.width == 150 && ext.height == 150);

    reg1.clear();
    assert(reg1.isEmpty());

    std::cout << "  -> Region tests passed!" << std::endl;
}

void test_damage_ring() {
    std::cout << "[Test] DamageRing multi-buffer rotation..." << std::endl;

    DamageRing ring;
    assert(!ring.hasPendingDamage());

    ring.addRect(RectI{10, 10, 50, 50});
    assert(ring.hasPendingDamage());

    // First time seeing buffer 1: returns full redraw
    Region d1 = ring.rotate(1, 1920, 1080);
    assert(!d1.isEmpty());
    assert(d1.extents().width == 1920 && d1.extents().height == 1080);
    assert(!ring.hasPendingDamage());

    // Add damage for next frame
    ring.addRect(RectI{100, 100, 20, 20});
    // First time seeing buffer 2: returns full redraw
    Region d2 = ring.rotate(2, 1920, 1080);
    assert(d2.extents().width == 1920);

    // Second time seeing buffer 1: returns accumulated damage since buffer 1 was last rendered!
    Region d1_accum = ring.rotate(1, 1920, 1080);
    assert(!d1_accum.isEmpty());
    RectI ext = d1_accum.extents();
    assert(ext.x == 100 && ext.y == 100 && ext.width == 20 && ext.height == 20);

    std::cout << "  -> DamageRing tests passed!" << std::endl;
}

void test_matrix3() {
    std::cout << "[Test] Matrix3 & Wayland transforms..." << std::endl;

    Matrix3 id = Matrix3::identity();
    assert(id[0] == 1.0f && id[4] == 1.0f && id[8] == 1.0f);
    assert(id[1] == 0.0f && id[2] == 0.0f);

    Matrix3 t = Matrix3::identity();
    t.translate(10.0f, 20.0f);
    assert(t[2] == 10.0f && t[5] == 20.0f);

    Matrix3 s = Matrix3::identity();
    s.scale(2.0f, 3.0f);
    assert(s[0] == 2.0f && s[4] == 3.0f);

    // Test Wayland transform inversion
    assert(invertTransform(WL_OUTPUT_TRANSFORM_NORMAL) == WL_OUTPUT_TRANSFORM_NORMAL);
    assert(invertTransform(WL_OUTPUT_TRANSFORM_90) == WL_OUTPUT_TRANSFORM_270);
    assert(invertTransform(WL_OUTPUT_TRANSFORM_270) == WL_OUTPUT_TRANSFORM_90);

    // Projection matrix test
    Matrix3 proj = Matrix3::project(1920, 1080, WL_OUTPUT_TRANSFORM_NORMAL);
    assert(proj[0] != 0.0f);

    std::cout << "  -> Matrix3 tests passed!" << std::endl;
}

void test_scene_graph() {
    std::cout << "[Test] SceneTree & Direct Scanout evaluation..." << std::endl;

    auto tree = SceneTree::create();
    assert(tree->children().empty());

    auto rect = tree->createChildRect(100, 100, Color::red());
    assert(tree->children().size() == 1);
    assert(rect->color().r == 1.0f);

    rect->setPosition(50, 60);
    PointI absP = rect->absolutePosition();
    assert(absP.x == 50 && absP.y == 60);

    // Hit test
    SceneNode* hit = tree->nodeAt(PointI{70, 80});
    assert(hit == rect.get());

    SceneNode* miss = tree->nodeAt(PointI{10, 10});
    assert(miss == nullptr);

    // Direct scanout candidate test:
    // When no buffer covers the entire screen, scanout candidate should be null
    assert(tree->findDirectScanoutCandidate(1920, 1080) == nullptr);

    std::cout << "  -> Scene graph tests passed!" << std::endl;
}

void test_color_and_formats() {
    std::cout << "[Test] Color & DRM Formats..." << std::endl;

    Color c1(1.0f, 0.5f, 0.25f, 0.5f);
    Color premul = c1.premultiplied();
    assert(std::abs(premul.r - 0.5f) < 1e-5f);
    assert(std::abs(premul.g - 0.25f) < 1e-5f);
    assert(std::abs(premul.a - 0.5f) < 1e-5f);

    DrmFormatSet set;
    set.add(DRM_FORMAT_XRGB8888, DRM_FORMAT_MOD_LINEAR);
    assert(set.has(DRM_FORMAT_XRGB8888, DRM_FORMAT_MOD_LINEAR));
    assert(!set.has(DRM_FORMAT_ARGB8888, DRM_FORMAT_MOD_LINEAR));

    std::string str = fourccToString(DRM_FORMAT_XRGB8888);
    assert(str.size() == 4);

    std::cout << "  -> Color and Formats tests passed!" << std::endl;
}

int main() {
    std::cout << "=== Running Nisaba Sovereign Compositor Unit Tests ===" << std::endl;

    test_geometry_and_region();
    test_damage_ring();
    test_matrix3();
    test_scene_graph();
    test_color_and_formats();

    std::cout << "=== All Compositor Tests Succeeded 100%! ===" << std::endl;
    return 0;
}
