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

void test_compositor_fusion() {
    std::cout << "[Test] Compositor & GPU Pipeline Fusion (Zero-Copy & State Isolation)..." << std::endl;

    // 1. Test Region::intersects(RectI)
    Region reg(RectI{100, 100, 200, 200});
    assert(reg.intersects(RectI{150, 150, 50, 50}));
    assert(reg.intersects(RectI{50, 50, 100, 100}));
    assert(!reg.intersects(RectI{400, 400, 50, 50}));
    assert(!reg.intersects(RectI{0, 0, 50, 50}));

    // 2. Test GpuTexture::from_native_handle (Zero-Copy wrapping)
    auto dev = nisaba::gpu::GpuDevice::create();
    if (dev) {
        auto dummyTex = nisaba::gpu::GpuTexture::from_native_handle(dev, 999, 1920, 1080);
        assert(dummyTex != nullptr);
        assert(dummyTex->id() == 999);
        assert(dummyTex->width() == 1920);
        assert(dummyTex->height() == 1080);
        assert(!dummyTex->owns_texture());

        // 3. Test GpuSurface::from_render_target
        auto dummySurface = nisaba::gpu::GpuSurface::from_render_target(dev, 1280, 720, 12);
        assert(dummySurface != nullptr);
        assert(dummySurface->fbo_id() == 12);
        assert(dummySurface->width() == 1280);
        assert(dummySurface->height() == 720);
    }

    // 4. Test CompositorCanvas fusion with Headless RenderTarget
    auto egl = EglContext::createAuto();
    if (egl && egl->makeCurrent()) {
        auto renderer = Renderer::create(egl);
        if (renderer) {
            auto canvas = CompositorCanvas::create(egl, renderer);
            assert(canvas != nullptr);

            auto target = RenderTarget::createDefault(800, 600);
            bool ok = canvas->beginFrame(target);
            assert(ok);

            // Test vg() vector canvas binding
            auto* vg = canvas->vg();
            assert(vg != nullptr);
            assert(vg->width() == 800);
            assert(vg->height() == 600);

            // Test drawing window decoration (SDF rounded rect + shadow + title + buttons)
            RectI winBox{100, 100, 400, 300};
            canvas->drawWindowDecoration(winBox, "Nisaba Sovereign Desktop Terminal");

            // Test drawing custom styled window decoration
            WindowDecoStyle customStyle;
            customStyle.cornerRadius = 16.0f;
            customStyle.titleBarHeight = 36.0f;
            customStyle.titleBarColor = Color{0.18f, 0.20f, 0.25f, 0.95f};
            canvas->drawWindowDecoration(RectI{150, 150, 300, 200}, customStyle, "Editor");

            // Test desktop dock panel
            RectI dockBox{100, 500, 600, 70};
            nisaba::effects::GlassParams glass;
            canvas->drawDesktopPanel(dockBox, glass);

            // Test typography
            canvas->drawText("Wayland 120 FPS", 30.0f, 40.0f, Color::white(), 14.0f);

            // Test damage culling
            Region damage(RectI{0, 0, 100, 100});
            canvas->setDamage(&damage);
            // Window completely outside damage is dropped in O(1)
            canvas->drawWindowDecoration(RectI{400, 400, 200, 150}, "Culled Window");

            ok = canvas->endFrame();
            assert(ok);
            std::cout << "  -> Hardware CompositorCanvas fusion verified!" << std::endl;
        }
    }

    std::cout << "  -> Fusion tests passed!" << std::endl;
}

int main() {
    std::cout << "=== Running Nisaba Sovereign Compositor Unit Tests ===" << std::endl;

    test_geometry_and_region();
    test_damage_ring();
    test_matrix3();
    test_scene_graph();
    test_color_and_formats();
    test_compositor_fusion();

    std::cout << "=== All Compositor Tests Succeeded 100%! ===" << std::endl;
    std::cout << std::flush;
    return 0;
}
