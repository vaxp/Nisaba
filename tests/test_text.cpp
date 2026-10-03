#include <cassert>
#include <iostream>
#include <fstream>
#include <string_view>
#include "nisaba/nisaba.hpp"

using namespace nisaba;
using namespace nisaba::text;

namespace {

std::string resolve_font(std::string_view filename) {
    std::vector<std::string> candidates = {
        std::string("fonts/") + std::string(filename),
        std::string("../fonts/") + std::string(filename),
        std::string("../../fonts/") + std::string(filename),
        std::string("fonts/") + std::string(filename),
        std::string("../fonts/") + std::string(filename)
    };
    for (const auto& path : candidates) {
        std::ifstream f(path, std::ios::binary);
        if (f.good()) return path;
    }
    return candidates[0];
}

} // namespace

void test_attrs() {
    std::cout << "[Test] Text Attributes & Metrics..." << std::endl;

    TextColor c1 = TextColor::rgb(255, 128, 0);
    assert(c1.r == 255 && c1.g == 128 && c1.b == 0 && c1.a == 255);
    assert(c1.to_u32() != 0);

    Metrics m(18.0f, 24.0f);
    assert(m.font_size == 18.0f);
    assert(m.line_height == 24.0f);

    Attrs default_attrs;
    default_attrs.set_weight(Weight::Bold).set_color(c1);

    AttrsList list(default_attrs);
    assert(list.get(0).weight == Weight::Bold);

    Attrs span_attrs;
    span_attrs.set_weight(Weight::Normal).set_style(Style::Italic);
    list.add_span(5, 10, span_attrs);

    assert(list.get(2).weight == Weight::Bold);
    assert(list.get(5).style == Style::Italic);
    assert(list.get(9).style == Style::Italic);
    assert(list.get(10).style == Style::Normal);

    std::cout << "  -> PASS" << std::endl;
}

void test_ttf_font() {
    std::cout << "[Test] Sovereign TrueType Font Parser..." << std::endl;

    auto font = TtfFont::from_file(resolve_font("Inter-Regular.ttf"));
    assert(font != nullptr);
    assert(font->is_valid());
    assert(font->units_per_em() > 0);
    assert(font->num_glyphs() > 0);
    assert(!font->family_name().empty());

    std::cout << "  Loaded font: " << font->family_name()
              << " (" << font->style_name() << "), Glyphs: " << font->num_glyphs()
              << ", UPM: " << font->units_per_em() << std::endl;

    // Check glyph mapping
    uint16_t gid_A = font->glyph_index('A');
    uint16_t gid_B = font->glyph_index('B');
    uint16_t gid_space = font->glyph_index(' ');
    assert(gid_A != 0);
    assert(gid_B != 0);
    assert(gid_space != 0);

    // Check metrics
    float adv_A = font->glyph_advance(gid_A, 16.0f);
    float adv_sp = font->glyph_advance(gid_space, 16.0f);
    assert(adv_A > 0.0f);
    assert(adv_sp > 0.0f);

    // Vector outline extraction into Nisaba Path
    Path path_A;
    bool ok_A = font->get_glyph_path(gid_A, path_A, 32.0f);
    assert(ok_A);
    assert(!path_A.is_empty());
    assert(path_A.bounds().width() > 0.0f);
    assert(path_A.bounds().height() > 0.0f);

    std::cout << "  Glyph 'A' bounds: "
              << path_A.bounds().width() << "x" << path_A.bounds().height() << std::endl;

    std::cout << "  -> PASS" << std::endl;
}

void test_bidi_and_arabic() {
    std::cout << "[Test] BiDi Analysis & Arabic Contextual Shaping..." << std::endl;

    // 1. Direction detection
    assert(Bidi::detect_base_direction("Hello World") == Direction::LeftToRight);
    assert(Bidi::detect_base_direction("مرحبا بالعالم") == Direction::RightToLeft);

    // 2. Run segmentation
    std::string mixed = "English مرحبا بالعالم English";
    auto runs = Bidi::segment_runs(mixed);
    assert(runs.size() >= 3);
    assert(!runs[0].is_rtl());
    assert(runs[1].is_rtl());
    assert(!runs[2].is_rtl());

    // 3. Arabic Contextual Shaping
    std::string arabic_word = "سلام"; // Sin, Lam, Alif, Mim
    auto shaped = Bidi::shape_text(arabic_word, Bidi::segment_runs(arabic_word));
    assert(!shaped.empty());

    // In 'سلام', 'ل' + 'ا' forms Lam-Alif ligature (0xFEFB or 0xFEFC)!
    // Sin at start should be initial form (0xFEB3)
    // Lam-Alif in middle should be final form ligature (0xFEF6 or 0xFEFC)
    // Mim at end should be isolated (0xFEE1)
    bool has_lam_alif = false;
    for (const auto& sc : shaped) {
        if (sc.codepoint == 0xFEFB || sc.codepoint == 0xFEFC) {
            has_lam_alif = true;
        }
    }
    assert(has_lam_alif);

    std::cout << "  Arabic shaping successfully synthesized ligatures and forms." << std::endl;
    std::cout << "  -> PASS" << std::endl;
}

void test_font_system() {
    std::cout << "[Test] Font System & Multilingual Fallback..." << std::endl;

    FontSystem fs;
    auto id_inter = fs.load_font_file(resolve_font("Inter-Regular.ttf"));
    auto id_arabic = fs.load_font_file(resolve_font("NotoSansArabic.ttf"));

    assert(id_inter.has_value());
    assert(id_arabic.has_value());
    assert(fs.font_count() == 2);

    // Match font
    auto [matched_id, font_ptr] = fs.match_font(Family::sans_serif());
    assert(font_ptr != nullptr);

    // Multilingual Fallback:
    // Inter does not have Arabic glyphs (e.g. 0x0628 = Ba 'ب'), but NotoSansArabic does!
    auto [actual_fid, gid] = fs.find_glyph_or_fallback(*id_inter, 0x0628);
    assert(actual_fid == *id_arabic);
    assert(gid != 0);

    std::cout << "  Successfully resolved Arabic fallback glyph from NotoSansArabic." << std::endl;
    std::cout << "  -> PASS" << std::endl;
}

void test_buffer_layout() {
    std::cout << "[Test] Multi-line Buffer Layout & Word Wrapping..." << std::endl;

    FontSystem fs;
    fs.load_font_file(resolve_font("Inter-Regular.ttf"));

    Buffer buffer(Metrics(16.0f, 22.0f));
    buffer.set_text("First line of text.\nSecond line with several words that should wrap when narrow.");

    // Unconstrained layout
    buffer.shape_until_scroll(fs);
    auto runs = buffer.layout_runs();
    assert(runs.size() == 2);

    // Apply width constraint
    buffer.set_size(150.0f, std::nullopt);
    buffer.set_wrap(Wrap::Word);
    buffer.shape_until_scroll(fs);

    auto wrapped_runs = buffer.layout_runs();
    assert(wrapped_runs.size() > 2); // Should have wrapped into multiple lines!

    std::cout << "  Unconstrained lines: " << runs.size()
              << ", Wrapped lines at 150px: " << wrapped_runs.size() << std::endl;

    std::cout << "  -> PASS" << std::endl;
}

void test_rendering_to_canvas() {
    std::cout << "[Test] Canvas Text Rendering with Alpha Masks..." << std::endl;

    FontSystem fs;
    fs.load_font_file(resolve_font("Inter-Regular.ttf"));
    fs.load_font_file(resolve_font("NotoSansArabic.ttf"));

    GlyphCache cache;

    auto pixmap = Pixmap::allocate(400, 200);
    assert(pixmap.has_value());

    Canvas canvas(*pixmap);
    canvas.clear(Color::from_rgba8(20, 24, 32, 255));

    Buffer buffer(Metrics(20.0f, 26.0f));
    Attrs title_attrs;
    title_attrs.set_color(TextColor::rgb(0, 210, 255));

    buffer.set_text("Nisaba Native Engine\nمحرك سيادي متكامل", title_attrs);
    buffer.draw(canvas, cache, fs, Color::WHITE, 20.0f, 20.0f);

    assert(cache.size() > 0);
    std::cout << "  Cached unique glyphs: " << cache.size() << std::endl;

    // Check that some pixels were rendered
    bool has_drawn_pixels = false;
    for (uint32_t y = 0; y < pixmap->height(); ++y) {
        for (uint32_t x = 0; x < pixmap->width(); ++x) {
            auto p = pixmap->pixel(x, y);
            if (p && (p->red() > 30 || p->green() > 30 || p->blue() > 40)) {
                has_drawn_pixels = true;
                break;
            }
        }
        if (has_drawn_pixels) break;
    }
    assert(has_drawn_pixels);

    std::cout << "  -> PASS" << std::endl;
}

void test_cursor_and_selection() {
    std::cout << "[Test] Interactive Cursor & Hit-Testing..." << std::endl;

    FontSystem fs;
    fs.load_font_file(resolve_font("Inter-Regular.ttf"));

    Buffer buffer(Metrics(16.0f, 20.0f));
    buffer.set_text("Hello World\nLine Two");
    buffer.shape_until_scroll(fs);

    // Hit test near top-left
    Cursor c_start = CursorController::hit_test(buffer, 0.0f, 5.0f);
    assert(c_start.line == 0);
    assert(c_start.index == 0);

    // Move cursor right
    Cursor c_next = CursorController::move(buffer, c_start, Motion::Right);
    assert(c_next.line == 0);
    assert(c_next.index == 1);

    // Move cursor down
    Cursor c_down = CursorController::move(buffer, c_next, Motion::Down);
    assert(c_down.line == 1);

    // Selection
    Selection sel(c_start, c_next);
    assert(!sel.is_empty());
    assert(sel.normalized().start.index <= sel.normalized().end.index);

    std::cout << "  -> PASS" << std::endl;
}

void test_multilingual_cjk_russian() {
    std::cout << "[Test] Global Multilingual Support (Japanese, Chinese, Russian, Korean)..." << std::endl;

    FontSystem fs;
    auto inter_id = fs.load_font_file(resolve_font("Inter-Regular.ttf"));
    auto arabic_id = fs.load_font_file(resolve_font("NotoSansArabic.ttf"));
    auto cjk_id = fs.load_font_file(resolve_font("DroidSansFallbackFull.ttf"));

    assert(inter_id.has_value());
    assert(arabic_id.has_value());
    assert(cjk_id.has_value());

    // 1. Russian (Cyrillic)
    const TtfFont* inter_font = fs.get_font(*inter_id);
    assert(inter_font != nullptr);
    uint16_t gid_cyr = inter_font->glyph_index(0x041F); // 'П'
    assert(gid_cyr != 0);

    // 2. Japanese & Chinese & Korean (CJK)
    const TtfFont* cjk_font = fs.get_font(*cjk_id);
    assert(cjk_font != nullptr);
    uint16_t gid_jp_ko = cjk_font->glyph_index(0x3053); // Hiragana 'こ'
    uint16_t gid_zh_ni = cjk_font->glyph_index(0x4F60); // Hanzi '你'
    uint16_t gid_kr_ga = cjk_font->glyph_index(0xAC00); // Hangul '가'
    assert(gid_jp_ko != 0);
    assert(gid_zh_ni != 0);
    assert(gid_kr_ga != 0);

    // 3. CJK Line Wrapping test (spaceless sentence should wrap at character boundaries!)
    Buffer cjk_buffer(Metrics(16.0f, 22.0f));
    std::string spaceless_jp = "今日は世界中のロボットと組み込みシステムを制御する日です。";
    cjk_buffer.set_text(spaceless_jp);
    cjk_buffer.set_size(120.0f, std::nullopt); // Narrow width
    cjk_buffer.set_wrap(Wrap::Word);
    cjk_buffer.shape_until_scroll(fs);

    auto runs = cjk_buffer.layout_runs();
    assert(runs.size() > 1); // Must have wrapped across multiple lines despite having NO spaces!

    // 4. Multilingual mixed rendering to Canvas
    auto pixmap = Pixmap::allocate(500, 250);
    assert(pixmap.has_value());
    Canvas canvas(*pixmap);
    canvas.clear(Color::BLACK);

    GlyphCache cache;
    Buffer mixed_buffer(Metrics(16.0f, 22.0f));
    std::string mixed_text =
        "English: Sovereign Robot\n"
        "Русский: Робототехника\n"
        "日本語: ロボット工学\n"
        "中文: 智能机器人操作系统\n"
        "العربية: محرك الأنظمة الذكية";
    mixed_buffer.set_text(mixed_text);
    mixed_buffer.draw(canvas, cache, fs, Color::WHITE, 20.0f, 20.0f);

    assert(cache.size() >= 10);
    std::cout << "  Rendered " << runs.size() << " CJK lines, and cached "
              << cache.size() << " global glyphs across all scripts." << std::endl;
    std::cout << "  -> PASS" << std::endl;
}

int main() {
    std::cout << "==========================================" << std::endl;
    std::cout << "   Nisaba Sovereign Text Engine Test Suite" << std::endl;
    std::cout << "==========================================" << std::endl;

    test_attrs();
    test_ttf_font();
    test_bidi_and_arabic();
    test_font_system();
    test_buffer_layout();
    test_rendering_to_canvas();
    test_cursor_and_selection();
    test_multilingual_cjk_russian();

    std::cout << "==========================================" << std::endl;
    std::cout << "All 8 text test suites passed successfully!" << std::endl;
    std::cout << "==========================================" << std::endl;
    return 0;
}
