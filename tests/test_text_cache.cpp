#include <iostream>
#include <vector>
#include <cassert>
#include <fstream>
#include <iomanip>
#include <cmath>

#include "nisaba/nisaba.hpp"

using namespace nisaba;
using namespace nisaba::text;

namespace {

std::string resolve_font(std::string_view filename) {
    std::vector<std::string> candidates = {
        std::string("fonts/") + std::string(filename),
        std::string("../fonts/") + std::string(filename),
        std::string("../../fonts/") + std::string(filename),
        std::string("/home/x/Downloads/thorvg-main/nisaba/fonts/") + std::string(filename)
    };
    for (const auto& path : candidates) {
        std::ifstream f(path, std::ios::binary);
        if (f.good()) return path;
    }
    return "";
}

} // namespace

void test_baked_text() {
    std::cout << "[*] Testing BakedText Pre-Rendering and Direct SIMD Blitting...\n";

    FontSystem font_system;
    auto font_path = resolve_font("Inter-Regular.ttf");
    assert(!font_path.empty() && "Failed to find Inter-Regular.ttf font file!");
    font_system.load_font_file(font_path);
    assert(font_system.font_count() > 0);

    GlyphCache glyph_cache;

    // 1. Bake text block
    BakedText baked = BakedText::bake(
        font_system,
        glyph_cache,
        "Nisaba Sovereign Engine",
        16.0f,
        Color::from_rgba8(255, 255, 255, 255),
        Weight::Bold
    );

    assert(baked.is_valid());
    assert(baked.width() > 100.0f);
    assert(baked.height() > 12.0f);
    assert(baked.pixmap() != nullptr);
    assert(baked.pixmap()->width() > 100);
    assert(baked.pixmap()->height() > 12);

    // 2. Draw onto canvas
    auto pixmap = Pixmap::allocate(300, 100);
    assert(pixmap.has_value());
    Canvas canvas(*pixmap);
    canvas.clear(Color::BLACK);

    canvas.draw_baked_text(baked, 20.0f, 30.0f);

    // Verify non-black pixels exist inside the text region
    bool found_text_pixel = false;
    for (uint32_t y = 30; y < 70; ++y) {
        for (uint32_t x = 20; x < 250; ++x) {
            auto px = pixmap->as_ref().pixel(x, y);
            if (px && px->red() > 200 && px->alpha() > 200) {
                found_text_pixel = true;
                break;
            }
        }
        if (found_text_pixel) break;
    }
    assert(found_text_pixel && "Expected bright white text pixels in canvas!");

    // 3. Test Opacity modulation (50% alpha)
    auto pixmap_half = Pixmap::allocate(300, 100);
    Canvas canvas_half(*pixmap_half);
    canvas_half.clear(Color::TRANSPARENT);
    canvas_half.draw_baked_text(baked, 20.0f, 30.0f, 0.5f);

    bool found_half_pixel = false;
    for (uint32_t y = 30; y < 70; ++y) {
        for (uint32_t x = 20; x < 250; ++x) {
            auto px = pixmap_half->as_ref().pixel(x, y);
            if (px && px->alpha() > 50 && px->alpha() < 180) {
                found_half_pixel = true;
                break;
            }
        }
        if (found_half_pixel) break;
    }
    assert(found_half_pixel && "Expected modulated alpha pixels when drawing with 0.5 opacity!");

    // 4. Test Scissor Clipping
    auto pixmap_clip = Pixmap::allocate(300, 100);
    Canvas canvas_clip(*pixmap_clip);
    canvas_clip.clear(Color::BLACK);

    // Restrict scissor clip to [0, 0, 50, 100]
    canvas_clip.set_scissor_clip(*ScreenIntRect::from_xywh(0, 0, 50, 100));
    canvas_clip.draw_baked_text(baked, 20.0f, 30.0f);

    // Pixels at x >= 50 must remain pitch black
    for (uint32_t y = 30; y < 70; ++y) {
        for (uint32_t x = 50; x < 250; ++x) {
            auto px = pixmap_clip->as_ref().pixel(x, y);
            assert(px && px->red() == 0);
        }
    }

    std::cout << "  [+] BakedText test passed!\n";
}

void test_text_cache_lru() {
    std::cout << "[*] Testing TextCache LRU, Hit Ratios, and Eviction Policies...\n";

    FontSystem font_system;
    font_system.load_font_file(resolve_font("Inter-Regular.ttf"));
    GlyphCache glyph_cache;

    // Small cache capacity of 3 items
    TextCache cache(3);
    assert(cache.size() == 0);
    assert(cache.max_entries() == 3);

    // 1. Initial query: Miss
    const auto& t1 = cache.get_or_bake(font_system, glyph_cache, "Item 1", 14.0f, Color::WHITE);
    assert(t1.is_valid());
    assert(cache.size() == 1);
    assert(cache.hits() == 0);
    assert(cache.misses() == 1);
    assert(cache.hit_ratio() == 0.0);

    // 2. Identical query: Hit
    const auto& t1_hit = cache.get_or_bake(font_system, glyph_cache, "Item 1", 14.0f, Color::WHITE);
    assert(t1_hit.is_valid());
    assert(t1_hit.pixmap() == t1.pixmap()); // Same underlying pre-rendered pixmap
    assert(cache.hits() == 1);
    assert(cache.misses() == 1);
    assert(cache.hit_ratio() == 0.5);

    // 3. Fill cache up to capacity
    cache.get_or_bake(font_system, glyph_cache, "Item 2", 14.0f, Color::WHITE);
    cache.get_or_bake(font_system, glyph_cache, "Item 3", 14.0f, Color::WHITE);
    assert(cache.size() == 3);
    assert(cache.hits() == 1);
    assert(cache.misses() == 3);

    // Access "Item 1" to mark it most recently used: Order becomes [Item 1, Item 3, Item 2]
    cache.get_or_bake(font_system, glyph_cache, "Item 1", 14.0f, Color::WHITE);
    assert(cache.hits() == 2);

    // 4. Insert 4th item -> must evict "Item 2" (least recently used)
    cache.get_or_bake(font_system, glyph_cache, "Item 4", 14.0f, Color::WHITE);
    assert(cache.size() == 3); // Still within max capacity
    assert(cache.misses() == 4);

    // "Item 1" should still be cached (Hit)
    cache.get_or_bake(font_system, glyph_cache, "Item 1", 14.0f, Color::WHITE);
    assert(cache.hits() == 3);

    // "Item 2" was evicted -> querying it causes a Miss
    cache.get_or_bake(font_system, glyph_cache, "Item 2", 14.0f, Color::WHITE);
    assert(cache.misses() == 5);

    // 5. Test clear()
    cache.clear();
    assert(cache.size() == 0);
    assert(cache.hits() == 0);
    assert(cache.misses() == 0);

    std::cout << "  [+] TextCache LRU test passed!\n";
}

void test_text_cache_draw() {
    std::cout << "[*] Testing TextCache::draw Direct Canvas Integration...\n";

    FontSystem font_system;
    font_system.load_font_file(resolve_font("Inter-Regular.ttf"));
    GlyphCache glyph_cache;

    TextCache cache(100);

    auto pixmap = Pixmap::allocate(400, 80);
    Canvas canvas(*pixmap);
    canvas.clear(Color::from_rgba8(20, 20, 30, 255));

    // First draw: Bakes and blits
    cache.draw(canvas, font_system, glyph_cache, "Direct SIMD Blit", 15.0f, 25.0f, 15.0f, Color::from_rgba8(0, 255, 200, 255));
    assert(cache.misses() == 1);
    assert(cache.hits() == 0);

    // Second draw: O(1) Instant Cache Hit
    cache.draw(canvas, font_system, glyph_cache, "Direct SIMD Blit", 15.0f, 50.0f, 15.0f, Color::from_rgba8(0, 255, 200, 255));
    assert(cache.hits() == 1);
    assert(cache.misses() == 1);

    // Verify colored text pixels exist
    bool found_cyan_text = false;
    for (uint32_t y = 20; y < 70; ++y) {
        for (uint32_t x = 15; x < 200; ++x) {
            auto px = pixmap->as_ref().pixel(x, y);
            if (px && px->green() > 200 && px->blue() > 150) {
                found_cyan_text = true;
                break;
            }
        }
        if (found_cyan_text) break;
    }
    assert(found_cyan_text && "Expected cyan text pixels in canvas!");

    std::cout << "  [+] TextCache::draw test passed!\n";
}

void test_text_performance_benchmark() {
    std::cout << "\n[*] Running 1,000 Iterations Performance Benchmark (Uncached vs TextCache vs BakedText)...\n";

    FontSystem font_system;
    font_system.load_font_file(resolve_font("Inter-Regular.ttf"));
    GlyphCache glyph_cache;

    auto pixmap = Pixmap::allocate(500, 100);
    Canvas canvas(*pixmap);

    constexpr int ITERS = 1000;
    const std::string sample_text = "GPU PIPELINE ALPHA: NODE_01 // ACTIVE THROUGHPUT";

    // 1. Uncached Text Rendering (Buffer construction + shaping + per-glyph mask blending)
    auto t0_raw = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < ITERS; ++i) {
        Buffer buf(Metrics(14.0f, 18.0f));
        Attrs attrs;
        attrs.set_color(TextColor::rgb(255, 255, 255));
        buf.set_text(sample_text, attrs);
        buf.draw(canvas, glyph_cache, font_system, Color::WHITE, 20.0f, 40.0f);
    }
    auto t1_raw = std::chrono::high_resolution_clock::now();
    double raw_ms = std::chrono::duration<double, std::milli>(t1_raw - t0_raw).count();

    // 2. TextCache::draw (Transparent LRU Lookup + Direct SIMD Blit)
    TextCache cache(100);
    // Warm up cache with 1 query
    cache.draw(canvas, font_system, glyph_cache, sample_text, 20.0f, 40.0f, 14.0f, Color::WHITE);

    auto t0_cache = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < ITERS; ++i) {
        cache.draw(canvas, font_system, glyph_cache, sample_text, 20.0f, 40.0f, 14.0f, Color::WHITE);
    }
    auto t1_cache = std::chrono::high_resolution_clock::now();
    double cache_ms = std::chrono::duration<double, std::milli>(t1_cache - t0_cache).count();

    // 3. Direct BakedText Blit (Pure SIMD Row Composite)
    BakedText baked = BakedText::bake(font_system, glyph_cache, sample_text, 14.0f, Color::WHITE);

    auto t0_baked = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < ITERS; ++i) {
        canvas.draw_baked_text(baked, 20.0f, 40.0f);
    }
    auto t1_baked = std::chrono::high_resolution_clock::now();
    double baked_ms = std::chrono::duration<double, std::milli>(t1_baked - t0_baked).count();

    double speedup_cache = raw_ms / cache_ms;
    double speedup_baked = raw_ms / baked_ms;

    std::cout << "  --------------------------------------------------------------------------------\n";
    std::cout << "  Benchmark (" << ITERS << " calls)        | Total Time  | Per-Draw Latency | Speedup\n";
    std::cout << "  --------------------------------------------------------------------------------\n";
    std::cout << "  A) Uncached Text (Raw)         | " << std::fixed << std::setprecision(2) << raw_ms << " ms | "
              << std::setprecision(4) << (raw_ms / ITERS) << " ms     | Baseline (1.0x)\n";
    std::cout << "  B) TextCache (Transparent LRU) | " << std::setprecision(2) << cache_ms << " ms | "
              << std::setprecision(4) << (cache_ms / ITERS) << " ms     | "
              << std::setprecision(1) << speedup_cache << "x FASTER\n";
    std::cout << "  C) BakedText (Direct SIMD)     | " << std::setprecision(2) << baked_ms << " ms | "
              << std::setprecision(4) << (baked_ms / ITERS) << " ms     | "
              << std::setprecision(1) << speedup_baked << "x FASTER\n";
    std::cout << "  --------------------------------------------------------------------------------\n";
}

int main() {
    std::cout << "========================================================\n"
              << "       NISABA SOVEREIGN TEXT CACHING UNIT TESTS\n"
              << "========================================================\n";

    test_baked_text();
    test_text_cache_lru();
    test_text_cache_draw();
    test_text_performance_benchmark();

    std::cout << "\n[SUCCESS] All Text Caching unit tests passed flawlessly!\n";
    return 0;
}
