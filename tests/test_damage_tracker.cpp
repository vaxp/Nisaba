#include <iostream>
#include <cassert>
#include <vector>
#include "nisaba/nisaba.hpp"
#include "nisaba/damage/damage.hpp"

using namespace nisaba;
using namespace nisaba::damage;

void test_span_basic() {
    std::cout << "[Test] Span Basics ... ";
    Span s1(10, 50, 100);
    assert(s1.y == 10);
    assert(s1.x0 == 50);
    assert(s1.x1 == 100);
    assert(s1.width() == 50);
    assert(s1.is_valid());

    Span s_empty(10, 100, 50);
    assert(!s_empty.is_valid());
    assert(s_empty.width() == 0);

    Span s2(10, 50, 100);
    assert(s1 == s2);
    std::cout << "PASSED\n";
}

void test_micro_tile_grid() {
    std::cout << "[Test] MicroTileGrid Operations ... ";
    MicroTileGrid<16> grid(1280, 800);
    assert(grid.width() == 1280);
    assert(grid.height() == 800);
    assert(grid.tiles_x() == 80);
    assert(grid.tiles_y() == 50);
    assert(grid.total_tiles() == 4000);
    assert(!grid.has_damage());
    assert(grid.dirty_tile_count() == 0);

    // Mark single pixel inside tile (0, 0)
    grid.mark_dirty(*Rect::from_xywh(5.0f, 5.0f, 2.0f, 2.0f));
    assert(grid.has_damage());
    assert(grid.is_tile_dirty(0, 0));
    assert(!grid.is_tile_dirty(1, 0));
    assert(!grid.is_tile_dirty(0, 1));
    assert(grid.dirty_tile_count() == 1);

    // Mark rect crossing tile boundaries: x=14..34 (tiles 0, 1, 2), y=14..34 (tiles 0, 1, 2)
    grid.mark_dirty(*Rect::from_xywh(14.0f, 14.0f, 20.0f, 20.0f));
    // Should dirty a 3x3 block [0..2, 0..2] = 9 tiles
    for (size_t y = 0; y < 3; ++y) {
        for (size_t x = 0; x < 3; ++x) {
            assert(grid.is_tile_dirty(x, y));
        }
    }
    assert(grid.dirty_tile_count() == 9);

    // Test clear
    grid.clear();
    assert(!grid.has_damage());
    assert(grid.dirty_tile_count() == 0);
    assert(!grid.is_tile_dirty(0, 0));

    // Test out of bounds clipping
    grid.mark_dirty(*Rect::from_xywh(-50.0f, -50.0f, 20.0f, 20.0f));
    assert(!grid.has_damage()); // completely outside

    grid.mark_dirty(*Rect::from_xywh(1275.0f, 795.0f, 50.0f, 50.0f));
    assert(grid.has_damage());
    assert(grid.is_tile_dirty(79, 49)); // bottom-right tile
    std::cout << "PASSED\n";
}

void test_tiled_span_tracker() {
    std::cout << "[Test] TiledSpanTracker & Span Generation ... ";
    TiledSpanTracker<16> tracker(640, 480);

    // Invalidate a 32x32 button at (100, 100) -> tiles (6, 6) and (7, 7)
    tracker.mark_dirty(*Rect::from_xywh(100.0f, 100.0f, 32.0f, 32.0f));
    assert(tracker.has_damage());
    assert(tracker.dirty_tile_count() > 0);

    auto bbox = tracker.bounding_damage();
    assert(bbox.has_value());
    assert(bbox->x() <= 100);
    assert(bbox->y() <= 100);
    assert(bbox->x() + bbox->width() >= 132);
    assert(bbox->y() + bbox->height() >= 132);

    // Generate 1D scanline spans
    std::vector<Span> spans;
    tracker.generate_spans(spans);
    assert(!spans.empty());
    // All generated spans must be valid
    for (const auto& span : spans) {
        assert(span.is_valid());
        assert(span.x0 < span.x1);
        assert(span.y >= 96 && span.y < 144); // within tile rows 6..8 (6*16=96, 9*16=144)
    }

    // Generate coalesced rectangles for Wayland / DRM
    std::vector<ScreenIntRect> damage_rects;
    tracker.generate_damage_rects(damage_rects, 16);
    assert(!damage_rects.empty());
    assert(damage_rects.size() <= 16);

    std::cout << "PASSED\n";
}

void test_buffer_age_tracker() {
    std::cout << "[Test] BufferAgeTracker (Multi-Buffering) ... ";
    BufferAgeTracker tracker(4);

    auto r1 = *ScreenIntRect::from_xywh(10, 10, 50, 50);
    auto r2 = *ScreenIntRect::from_xywh(100, 100, 80, 80);
    auto r3 = *ScreenIntRect::from_xywh(200, 200, 60, 60);

    // Frame 1
    tracker.record_frame_damage({r1});
    // Buffer age 1 -> should have r1
    auto acc1 = tracker.get_accumulated_damage(1);
    assert(acc1.size() == 1);
    assert(acc1[0] == r1);

    // Frame 2
    tracker.record_frame_damage({r2});
    // For double buffering (age 2), current buffer hasn't seen Frame 1 or Frame 2
    auto acc2 = tracker.get_accumulated_damage(2);
    assert(acc2.size() == 2);
    assert(acc2[0] == r1);
    assert(acc2[1] == r2);

    // Frame 3
    tracker.record_frame_damage({r3});
    // Age 2 should contain Frame 2 and Frame 3
    auto acc2_f3 = tracker.get_accumulated_damage(2);
    assert(acc2_f3.size() == 2);
    assert(acc2_f3[0] == r2);
    assert(acc2_f3[1] == r3);

    // Age 0 means full repaint
    auto acc0 = tracker.get_accumulated_damage(0);
    assert(acc0.empty());

    std::cout << "PASSED\n";
}

void test_surface_cache() {
    std::cout << "[Test] SurfaceCache & Multi-Page View Caching ... ";
    SurfaceCache cache(200, 150);
    assert(cache.width() == 200);
    assert(cache.height() == 150);
    assert(cache.is_dirty());
    assert(!cache.is_valid());

    int draw_count = 0;
    auto draw_fn = [&](Canvas& c) {
        draw_count++;
        c.clear(Color::from_rgba8(255, 0, 0, 255));
    };

    // First render: must execute draw_fn
    bool drawn1 = cache.render_if_dirty(draw_fn);
    assert(drawn1);
    assert(draw_count == 1);
    assert(cache.is_valid());
    assert(!cache.is_dirty());

    // Second render without invalidation: must reuse cached surface (0 draws)
    bool drawn2 = cache.render_if_dirty(draw_fn);
    assert(!drawn2);
    assert(draw_count == 1);

    // Blit onto target canvas
    auto target_pix = Pixmap::create(400, 300);
    assert(target_pix.has_value());
    Canvas target_canvas(*target_pix);
    target_canvas.clear(Color::from_rgba8(0, 0, 0, 255));
    bool blitted = cache.draw(target_canvas, 50.0f, 50.0f);
    assert(blitted);

    // Verify blitted pixel at (55, 55) is red
    auto px_val = target_pix->pixel(55, 55);
    assert(px_val.has_value());
    assert(px_val->red() == 255);
    assert(px_val->green() == 0);
    assert(px_val->blue() == 0);

    // Invalidate and verify redraw
    cache.invalidate();
    assert(cache.is_dirty());
    bool drawn3 = cache.render_if_dirty(draw_fn);
    assert(drawn3);
    assert(draw_count == 2);

    // Test SurfaceCacheManager for multi-page switching (Home <-> Settings)
    SurfaceCacheManager mgr;
    int home_draws = 0;
    int settings_draws = 0;

    auto render_home = [&](Canvas& c) { home_draws++; c.clear(Color::from_rgba8(10, 20, 30, 255)); };
    auto render_settings = [&](Canvas& c) { settings_draws++; c.clear(Color::from_rgba8(40, 50, 60, 255)); };

    // Navigate to Home
    mgr.draw_cached("Home", target_canvas, 0, 0, 400, 300, render_home);
    assert(home_draws == 1);

    // Navigate to Settings
    mgr.draw_cached("Settings", target_canvas, 0, 0, 400, 300, render_settings);
    assert(settings_draws == 1);

    // Return to Home: instant 0-draw cache reuse!
    mgr.draw_cached("Home", target_canvas, 0, 0, 400, 300, render_home);
    assert(home_draws == 1); // Still 1! Zero redraw overhead!

    std::cout << "PASSED\n";
}

void test_adaptive_invalidator() {
    std::cout << "[Test] AdaptiveInvalidator (60% Threshold Rule) ... ";
    auto container = Rect::from_xywh(0.0f, 0.0f, 100.0f, 100.0f);
    assert(container.has_value());

    AdaptiveInvalidator inv(*container, 0.60f);
    assert(inv.container_area() == 10000.0f);
    assert(!inv.has_damage());
    assert(inv.damage_ratio() == 0.0f);

    // 1. Small damage: 10x10 rect = 100 px (1% of container)
    inv.add_damage(*Rect::from_xywh(10.0f, 10.0f, 10.0f, 10.0f));
    assert(inv.has_damage());
    assert(inv.damage_count() == 1);
    assert(inv.accumulated_damage_area() == 100.0f);
    assert(!inv.should_collapse());

    TiledSpanTracker<16> tracker(200, 200);
    auto strat1 = inv.resolve(tracker);
    assert(strat1 == InvalidationStrategy::FineGrained);
    assert(tracker.has_damage());

    // 2. Add more damage to push total over 60%: add 70x80 = 5600 px -> total = 5700 px (57% still under)
    inv.clear();
    inv.add_damage(*Rect::from_xywh(0.0f, 0.0f, 70.0f, 80.0f)); // 5600 px (56%)
    assert(!inv.should_collapse());

    // Add another 1000 px -> total = 6600 px (66% >= 60% threshold!)
    inv.add_damage(*Rect::from_xywh(70.0f, 0.0f, 20.0f, 50.0f)); // 1000 px
    assert(inv.accumulated_damage_area() == 6600.0f);
    assert(inv.damage_ratio() == 0.66f);
    assert(inv.should_collapse());

    tracker.clear();
    auto strat2 = inv.resolve(tracker);
    assert(strat2 == InvalidationStrategy::FullContainer);

    // Verify resolving to bounding boxes emits the single container box
    std::vector<Rect> resolved_boxes;
    inv.resolve(resolved_boxes);
    assert(resolved_boxes.size() == 1);
    assert(resolved_boxes[0] == *container);

    std::cout << "PASSED\n";
}

void test_baked_animation() {
    std::cout << "[Test] BakedAnimation (Recorded Replay Engine) ... ";
    BakedAnimation anim(40, 40, 10, AnimationLoopMode::Loop);
    assert(anim.width() == 40);
    assert(anim.height() == 40);
    assert(anim.frame_count() == 10);
    assert(!anim.is_baked());
    assert(anim.memory_footprint() == 40 * 40 * 4 * 10); // 64,000 bytes (~64 KB)

    // Bake 10 frames, each with a unique red color gradient
    bool baked = anim.bake([&](Canvas& c, size_t frame_idx) {
        c.clear(Color::from_rgba8(static_cast<uint8_t>(20 + frame_idx * 20), 100, 200, 255));
    });
    assert(baked);
    assert(anim.is_baked());

    // Verify timeline tick mapping for Loop mode
    assert(anim.tick_to_frame_index(0) == 0);
    assert(anim.tick_to_frame_index(9) == 9);
    assert(anim.tick_to_frame_index(10) == 0);
    assert(anim.tick_to_frame_index(23) == 3);

    // Render frame 5 to target canvas and verify pixel
    auto target_pix = Pixmap::create(100, 100);
    assert(target_pix.has_value());
    Canvas target_canvas(*target_pix);

    bool rendered = anim.render_frame(target_canvas, 10.0f, 10.0f, 5);
    assert(rendered);

    auto px = target_pix->pixel(15, 15);
    assert(px.has_value());
    assert(px->red() == 20 + 5 * 20); // 120
    assert(px->green() == 100);
    assert(px->blue() == 200);

    // Test PingPong mode mapping
    BakedAnimation ping_anim(20, 20, 4, AnimationLoopMode::PingPong);
    // Frames: 0, 1, 2, 3 -> Cycle length is 6: 0, 1, 2, 3, 2, 1
    assert(ping_anim.tick_to_frame_index(0) == 0);
    assert(ping_anim.tick_to_frame_index(1) == 1);
    assert(ping_anim.tick_to_frame_index(2) == 2);
    assert(ping_anim.tick_to_frame_index(3) == 3);
    assert(ping_anim.tick_to_frame_index(4) == 2);
    assert(ping_anim.tick_to_frame_index(5) == 1);
    assert(ping_anim.tick_to_frame_index(6) == 0);

    // Test Clamp mode mapping
    BakedAnimation clamp_anim(20, 20, 4, AnimationLoopMode::Clamp);
    assert(clamp_anim.tick_to_frame_index(0) == 0);
    assert(clamp_anim.tick_to_frame_index(2) == 2);
    assert(clamp_anim.tick_to_frame_index(3) == 3);
    assert(clamp_anim.tick_to_frame_index(10) == 3); // Clamped to 3

    std::cout << "PASSED\n";
}

void test_backdrop_blur_cache() {
    std::cout << "[Test] BackdropBlurCache (Async Frosted Glass & Mirror Bleed) ... ";

    // 1. Create a 120x80 backdrop with high-contrast pattern
    auto bg_pix = Pixmap::create(120, 80);
    assert(bg_pix.has_value());
    Canvas bg_canvas(*bg_pix);
    bg_canvas.clear(Color::from_rgba8(20, 30, 50, 255));
    Paint p1(Color::from_rgba8(255, 100, 50, 255));
    bg_canvas.fill_rect(Rect::from_xywh(20.0f, 20.0f, 40.0f, 40.0f).value(), p1);

    // 2. Initialize BackdropBlurCache synchronously
    BackdropBlurCache cache;
    assert(!cache.is_ready());
    assert(!cache.is_updating());

    cache.set_backdrop(*bg_pix, 8.0f, true); // Mirror blur enabled
    assert(cache.is_ready());
    assert(cache.width() == 120);
    assert(cache.height() == 80);
    assert(cache.blurred_pixmap() != nullptr);

    // 3. Verify mirror reflection at edges (should not have black alpha or 0 values)
    auto edge_px = cache.blurred_pixmap()->pixel(0, 0);
    assert(edge_px.has_value());
    assert(edge_px->alpha() == 255);

    // 4. Test draw_glass_aperture with frosted glass card
    auto screen_pix = Pixmap::create(120, 80);
    assert(screen_pix.has_value());
    Canvas screen_canvas(*screen_pix);
    screen_canvas.clear(Color::from_rgba8(0, 0, 0, 255));

    effects::GlassParams glass_p = effects::GlassParams::dark();
    glass_p.blur_sigma = 8.0f;

    auto card_rect = Rect::from_xywh(10.0f, 10.0f, 60.0f, 40.0f).value();
    cache.draw_glass_aperture(screen_canvas, card_rect, 6.0f, 6.0f, glass_p);

    // Verify center pixel inside glass card was modified from black
    auto card_px = screen_pix->pixel(30, 30);
    assert(card_px.has_value());
    assert(card_px->alpha() == 255);
    assert(card_px->red() > 0 || card_px->blue() > 0);

    // 5. Test Async Worker update
    auto new_bg = Pixmap::create(120, 80);
    assert(new_bg.has_value());
    Canvas new_canvas(*new_bg);
    new_canvas.clear(Color::from_rgba8(100, 200, 50, 255));

    cache.update_async(*new_bg, 6.0f, true);
    cache.wait_async();
    assert(!cache.is_updating());
    assert(cache.is_ready());

    // Render aperture with updated backdrop
    screen_canvas.clear(Color::from_rgba8(0, 0, 0, 255));
    cache.draw_glass_aperture(screen_canvas, card_rect, 6.0f, 6.0f, glass_p);
    auto updated_px = screen_pix->pixel(30, 30);
    assert(updated_px.has_value());
    assert(updated_px->alpha() == 255);

    std::cout << "PASSED\n";
}

int main() {
    std::cout << "====================================================\n";
    std::cout << "Running Nisaba Tiled-Span Damage Tracking Unit Tests\n";
    std::cout << "====================================================\n";

    test_span_basic();
    test_micro_tile_grid();
    test_tiled_span_tracker();
    test_buffer_age_tracker();
    test_surface_cache();
    test_adaptive_invalidator();
    test_baked_animation();
    test_backdrop_blur_cache();

    std::cout << "====================================================\n";
    std::cout << "All Damage Tracking Unit Tests Passed (8/8)!\n";
    std::cout << "====================================================\n";
    return 0;
}

