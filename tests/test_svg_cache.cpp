#include <iostream>
#include <vector>
#include <cassert>
#include <chrono>
#include <iomanip>
#include <cmath>

#include "nisaba/nisaba.hpp"

using namespace nisaba;
using namespace nisaba::svg;

namespace {

const std::string simple_rect_svg = R"(
    <svg viewBox="0 0 100 100">
        <rect x="10" y="10" width="80" height="80" fill="#00ff00"/>
    </svg>
)";

const std::string icon_svg = R"(
    <svg viewBox="0 0 64 64">
        <rect x="4" y="4" width="56" height="56" rx="10" fill="#202530" stroke="#00d4ff" stroke-width="2"/>
        <circle cx="32" cy="32" r="16" fill="#ff0055"/>
        <path d="M 24 32 L 32 24 L 40 32 L 32 40 Z" fill="#ffffff"/>
    </svg>
)";

const std::string monochrome_icon_svg = R"(
    <svg viewBox="0 0 48 48">
        <circle cx="24" cy="24" r="20" fill="#ffffff"/>
    </svg>
)";

} // namespace

void test_baked_svg_basic() {
    std::cout << "[*] Testing BakedSvg basic pre-rendering and SIMD drawing...\n";

    auto doc = SvgDocument::parse(simple_rect_svg);
    assert(doc.has_value());

    BakedSvg baked = BakedSvg::bake(*doc, 100, 100);
    assert(baked.is_valid());
    assert(baked.width() == 100);
    assert(baked.height() == 100);

    // Center pixel (50, 50) must be green (#00ff00)
    auto p_center = baked.pixmap().as_ref().pixel(50, 50);
    assert(p_center.has_value());
    assert(p_center->green() == 255);
    assert(p_center->red() == 0);
    assert(p_center->blue() == 0);
    assert(p_center->alpha() == 255);

    // Corner pixel (5, 5) must be transparent (0 alpha)
    auto p_corner = baked.pixmap().as_ref().pixel(5, 5);
    assert(p_corner.has_value());
    assert(p_corner->alpha() == 0);

    // Draw onto canvas
    auto dest_pix = Pixmap::allocate(200, 200);
    assert(dest_pix.has_value());
    Canvas canvas(*dest_pix);
    canvas.clear(Color::BLACK);

    baked.draw(canvas, 50.0f, 50.0f);

    // Check pixel at (100, 100) on destination canvas
    auto dest_p = dest_pix->as_ref().pixel(100, 100);
    assert(dest_p.has_value());
    assert(dest_p->green() == 255);
    assert(dest_p->red() == 0);

    std::cout << "  BakedSvg basic drawing verified.\n";
}

void test_baked_svg_tint() {
    std::cout << "[*] Testing BakedSvg dynamic tinting...\n";

    auto doc = SvgDocument::parse(monochrome_icon_svg);
    assert(doc.has_value());

    // Bake with orange tint
    Color tint_color = Color::from_rgba8(255, 128, 0, 255);
    BakedSvg baked = BakedSvg::bake(*doc, 48, 48, tint_color);
    assert(baked.is_valid());

    // Center pixel should be tinted orange
    auto p = baked.pixmap().as_ref().pixel(24, 24);
    assert(p.has_value());
    assert(p->red() == 255);
    assert(p->green() == 128);
    assert(p->blue() == 0);
    assert(p->alpha() == 255);

    std::cout << "  BakedSvg dynamic tinting verified.\n";
}

void test_svg_cache_lru() {
    std::cout << "[*] Testing SvgCache LRU eviction and hit/miss metrics...\n";

    SvgCache cache(2); // Capacity: 2
    assert(cache.capacity() == 2);
    assert(cache.entries_count() == 0);
    assert(cache.hits() == 0);
    assert(cache.misses() == 0);

    auto doc1 = SvgDocument::parse(simple_rect_svg);
    auto doc2 = SvgDocument::parse(icon_svg);
    auto doc3 = SvgDocument::parse(monochrome_icon_svg);
    assert(doc1.has_value() && doc2.has_value() && doc3.has_value());

    // First lookup: doc1 -> Miss
    const BakedSvg& b1 = cache.get_or_bake(*doc1, 64, 64);
    assert(b1.is_valid());
    assert(cache.misses() == 1);
    assert(cache.hits() == 0);
    assert(cache.entries_count() == 1);

    // Second lookup: doc1 -> Hit
    const BakedSvg& b1_hit = cache.get_or_bake(*doc1, 64, 64);
    assert(b1_hit.is_valid());
    assert(cache.misses() == 1);
    assert(cache.hits() == 1);
    assert(cache.entries_count() == 1);

    // Third lookup: doc2 -> Miss
    const BakedSvg& b2 = cache.get_or_bake(*doc2, 64, 64);
    assert(b2.is_valid());
    assert(cache.misses() == 2);
    assert(cache.hits() == 1);
    assert(cache.entries_count() == 2);

    // Fourth lookup: doc3 -> Miss (evicts doc1, since doc2 was accessed more recently)
    const BakedSvg& b3 = cache.get_or_bake(*doc3, 64, 64);
    assert(b3.is_valid());
    assert(cache.misses() == 3);
    assert(cache.hits() == 1);
    assert(cache.entries_count() == 2);

    // doc2 should still be in cache -> Hit
    const BakedSvg& b2_hit = cache.get_or_bake(*doc2, 64, 64);
    assert(b2_hit.is_valid());
    assert(cache.misses() == 3);
    assert(cache.hits() == 2);

    // doc1 was evicted -> Miss
    const BakedSvg& b1_again = cache.get_or_bake(*doc1, 64, 64);
    assert(b1_again.is_valid());
    assert(cache.misses() == 4);
    assert(cache.hits() == 2);

    // Clear cache
    cache.clear();
    assert(cache.entries_count() == 0);
    assert(cache.hits() == 0);
    assert(cache.misses() == 0);

    std::cout << "  SvgCache LRU eviction verified.\n";
}

void test_svg_cache_variants() {
    std::cout << "[*] Testing SvgCache key differentiation (resolution & tint)...\n";

    SvgCache cache(10);
    auto doc = SvgDocument::parse(monochrome_icon_svg);
    assert(doc.has_value());

    const Color color_red = Color::from_rgba8(255, 0, 0, 255);
    const Color color_blue = Color::from_rgba8(0, 0, 255, 255);

    // 1. Same doc, size 32x32
    cache.get_or_bake(*doc, 32, 32);
    assert(cache.entries_count() == 1);

    // 2. Same doc, size 64x64 -> Different entry!
    cache.get_or_bake(*doc, 64, 64);
    assert(cache.entries_count() == 2);

    // 3. Same doc, size 32x32, with Tint Red -> Different entry!
    cache.get_or_bake(*doc, 32, 32, color_red);
    assert(cache.entries_count() == 3);

    // 4. Same doc, size 32x32, with Tint Blue -> Different entry!
    cache.get_or_bake(*doc, 32, 32, color_blue);
    assert(cache.entries_count() == 4);

    // 5. Query size 32x32 with Tint Red again -> Hit!
    cache.get_or_bake(*doc, 32, 32, color_red);
    assert(cache.entries_count() == 4);
    assert(cache.hits() == 1);

    std::cout << "  SvgCache key differentiation verified.\n";
}

void test_svg_cache_draw() {
    std::cout << "[*] Testing SvgCache high-level draw() methods...\n";

    auto dest_pix = Pixmap::allocate(200, 200);
    assert(dest_pix.has_value());
    Canvas canvas(*dest_pix);
    canvas.clear(Color::BLACK);

    SvgCache cache(16);

    // Draw using SvgDocument
    auto doc = SvgDocument::parse(simple_rect_svg);
    assert(doc.has_value());
    cache.draw(canvas, *doc, 20.0f, 20.0f, 80.0f, 80.0f);
    assert(cache.misses() == 1);
    assert(cache.hits() == 0);

    // Second draw -> Hit!
    cache.draw(canvas, *doc, 100.0f, 20.0f, 80.0f, 80.0f);
    assert(cache.misses() == 1);
    assert(cache.hits() == 1);

    // Draw using SVG XML string overload (miss for string hash key)
    cache.draw(canvas, simple_rect_svg, 20.0f, 100.0f, 80.0f, 80.0f);
    assert(cache.misses() == 2);
    assert(cache.hits() == 1);

    // Second draw with XML string -> Hit!
    cache.draw(canvas, simple_rect_svg, 100.0f, 100.0f, 80.0f, 80.0f);
    assert(cache.misses() == 2);
    assert(cache.hits() == 2);

    std::cout << "  SvgCache draw() methods verified.\n";
}

void test_svg_cache_benchmark() {
    std::cout << "\n[*] Running Performance Benchmark: Vector SVG Rasterization vs. SvgCache vs. BakedSvg...\n";

    auto doc = SvgDocument::parse(icon_svg);
    assert(doc.has_value());

    auto pixmap = Pixmap::allocate(400, 400);
    assert(pixmap.has_value());
    Canvas canvas(*pixmap);

    constexpr int ITERS = 1000;
    auto target_rect = Rect::from_xywh(50.0f, 50.0f, 64.0f, 64.0f);
    assert(target_rect.has_value());

    // 1. Uncached Raw SVG Rendering (Parses Béziers & Scanline Rasterizes every time)
    auto t0_raw = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < ITERS; ++i) {
        canvas.draw_svg(*doc, *target_rect);
    }
    auto t1_raw = std::chrono::high_resolution_clock::now();
    double raw_ms = std::chrono::duration<double, std::milli>(t1_raw - t0_raw).count();

    // 2. SvgCache (Transparent LRU Cache -> $O(1)$ SIMD Direct Blit)
    SvgCache cache(16);
    auto t0_cache = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < ITERS; ++i) {
        cache.draw(canvas, *doc, 50.0f, 50.0f, 64.0f, 64.0f);
    }
    auto t1_cache = std::chrono::high_resolution_clock::now();
    double cache_ms = std::chrono::duration<double, std::milli>(t1_cache - t0_cache).count();

    // 3. Direct BakedSvg Blit
    BakedSvg baked = BakedSvg::bake(*doc, 64, 64);
    auto t0_baked = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < ITERS; ++i) {
        baked.draw(canvas, 50.0f, 50.0f);
    }
    auto t1_baked = std::chrono::high_resolution_clock::now();
    double baked_ms = std::chrono::duration<double, std::milli>(t1_baked - t0_baked).count();

    double speedup_cache = raw_ms / cache_ms;
    double speedup_baked = raw_ms / baked_ms;

    std::cout << "  --------------------------------------------------------------------------------\n";
    std::cout << "  Benchmark (" << ITERS << " calls)        | Total Time  | Per-Draw Latency | Speedup\n";
    std::cout << "  --------------------------------------------------------------------------------\n";
    std::cout << "  A) Uncached SVG (Raw Vector)   | " << std::fixed << std::setprecision(2) << raw_ms << " ms | "
              << std::setprecision(4) << (raw_ms / ITERS) << " ms     | Baseline (1.0x)\n";
    std::cout << "  B) SvgCache (Transparent LRU) | " << std::setprecision(2) << cache_ms << " ms | "
              << std::setprecision(4) << (cache_ms / ITERS) << " ms     | "
              << std::setprecision(1) << speedup_cache << "x FASTER\n";
    std::cout << "  C) BakedSvg (Direct SIMD)      | " << std::setprecision(2) << baked_ms << " ms | "
              << std::setprecision(4) << (baked_ms / ITERS) << " ms     | "
              << std::setprecision(1) << speedup_baked << "x FASTER\n";
    std::cout << "  --------------------------------------------------------------------------------\n";
}

int main() {
    std::cout << "========================================================\n"
              << "       NISABA SOVEREIGN SVG CACHING UNIT TESTS\n"
              << "========================================================\n";

    test_baked_svg_basic();
    test_baked_svg_tint();
    test_svg_cache_lru();
    test_svg_cache_variants();
    test_svg_cache_draw();
    test_svg_cache_benchmark();

    std::cout << "\n[SUCCESS] All SVG Caching unit tests passed flawlessly!\n";
    return 0;
}
