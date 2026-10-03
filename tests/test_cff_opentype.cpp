#include <cassert>
#include <iostream>
#include <fstream>
#include <vector>
#include <string_view>
#include "nisaba/nisaba.hpp"
#include "nisaba/text/cff_font.hpp"
#include "nisaba/text/opentype_tables.hpp"
#include "nisaba/text/ttf_font.hpp"

using namespace nisaba;
using namespace nisaba::text;

namespace {

std::string resolve_font(std::string_view filename) {
    std::vector<std::string> candidates = {
        std::string("fonts/") + std::string(filename),
        std::string("../fonts/") + std::string(filename),
        std::string("../../fonts/") + std::string(filename)
    };
    for (const auto& path : candidates) {
        std::ifstream f(path, std::ios::binary);
        if (f.good()) return path;
    }
    return candidates[0];
}

} // namespace

void test_otf_loading_and_cff_cubics() {
    std::cout << "[Test] PostScript / CFF (.otf) Font Loading & Cubic Outlines..." << std::endl;

    std::string font_path = resolve_font("SourceSans3-Regular.otf");
    auto font = TtfFont::from_file(font_path);
    assert(font != nullptr);
    assert(font->is_valid());
    assert(font->is_cff());
    assert(font->cff() != nullptr);
    assert(font->units_per_em() == 1000);
    assert(font->num_glyphs() > 0);

    std::cout << "  Loaded OTF: " << font->family_name()
              << " (" << font->style_name() << "), Glyphs: " << font->num_glyphs()
              << ", UPM: " << font->units_per_em() << std::endl;

    // Verify ASCII glyph lookup & metrics
    uint16_t gid_A = font->glyph_index('A');
    uint16_t gid_O = font->glyph_index('O');
    uint16_t gid_f = font->glyph_index('f');
    uint16_t gid_i = font->glyph_index('i');
    assert(gid_A > 0);
    assert(gid_O > 0);
    assert(gid_f > 0);
    assert(gid_i > 0);

    float adv_A = font->glyph_advance(gid_A, 24.0f);
    assert(adv_A > 0.0f);

    // Extract glyph outline for 'O' (which uses smooth PostScript cubic Bézier curves)
    Path path_O;
    bool ok = font->get_glyph_path(gid_O, path_O, 24.0f);
    assert(ok);
    assert(!path_O.is_empty());

    // Check that the outline contains cubic Bézier verbs
    bool has_cubic = false;
    for (auto v : path_O.verbs()) {
        if (v == PathVerb::Cubic) {
            has_cubic = true;
            break;
        }
    }
    assert(has_cubic);
    std::cout << "  CFF Type 2 Charstring successfully evaluated into Cubic Bézier curves!" << std::endl;

    // Check bounds
    auto bounds = path_O.bounds();
    assert(bounds.width() > 0.0f);
    assert(bounds.height() > 0.0f);

    std::cout << "  -> PASS" << std::endl;
}

void test_opentype_gsub_ligatures() {
    std::cout << "[Test] OpenType GSUB Ligature Substitution..." << std::endl;

    std::string font_path = resolve_font("SourceSans3-Regular.otf");
    auto font = TtfFont::from_file(font_path);
    assert(font != nullptr);
    assert(font->gsub() != nullptr);
    assert(font->gsub()->is_valid());

    uint16_t gid_f = font->glyph_index('f');
    uint16_t gid_t = font->glyph_index('t');
    assert(gid_f > 0 && gid_t > 0);

    // Test 'f' + 'f' -> 'ff' ligature
    std::vector<uint16_t> glyphs_ff = { gid_f, gid_f };
    bool applied_ff = font->apply_ligatures(glyphs_ff);
    assert(applied_ff);
    assert(glyphs_ff.size() == 1);
    assert(glyphs_ff[0] == 687);
    std::cout << "  Successfully merged [f, f] -> ligature 'ff' (glyph ID " << glyphs_ff[0] << ")" << std::endl;

    // Test 'f' + 't' -> 'ft' ligature
    std::vector<uint16_t> glyphs_ft = { gid_f, gid_t };
    bool applied_ft = font->apply_ligatures(glyphs_ft);
    assert(applied_ft);
    assert(glyphs_ft.size() == 1);
    assert(glyphs_ft[0] == 690);
    std::cout << "  Successfully merged [f, t] -> ligature 'ft' (glyph ID " << glyphs_ft[0] << ")" << std::endl;

    // Test 3-component ligature: 'f' + 'f' + 't' -> 'fft'
    std::vector<uint16_t> glyphs_fft = { gid_f, gid_f, gid_t };
    bool applied_fft = font->apply_ligatures(glyphs_fft);
    assert(applied_fft);
    assert(glyphs_fft.size() == 1);
    assert(glyphs_fft[0] == 691);
    std::cout << "  Successfully merged [f, f, t] -> 3-component ligature 'fft' (glyph ID " << glyphs_fft[0] << ")" << std::endl;

    // Test a sequence without ligature: 'a' + 'b'
    uint16_t gid_a = font->glyph_index('a');
    uint16_t gid_b = font->glyph_index('b');
    std::vector<uint16_t> seq = { gid_a, gid_b };
    font->apply_ligatures(seq);
    assert(seq.size() == 2);
    assert(seq[0] == gid_a && seq[1] == gid_b);

    // Test full word: "offset" -> o, f, f, s, e, t (6 glyphs become 5 with 'ff')
    uint16_t gid_o = font->glyph_index('o');
    uint16_t gid_s = font->glyph_index('s');
    uint16_t gid_e = font->glyph_index('e');
    std::vector<uint16_t> word = { gid_o, gid_f, gid_f, gid_s, gid_e, gid_t };
    font->apply_ligatures(word);
    assert(word.size() == 5);
    assert(word[1] == 687); // 'ff' ligature
    std::cout << "  Successfully shaped word 'offset' with ligature substitution!" << std::endl;

    std::cout << "  -> PASS" << std::endl;
}

void test_opentype_gpos_kerning() {
    std::cout << "[Test] OpenType GPOS Pair Kerning..." << std::endl;

    std::string font_path = resolve_font("SourceSans3-Regular.otf");
    auto font = TtfFont::from_file(font_path);
    assert(font != nullptr);
    assert(font->gpos() != nullptr);
    assert(font->gpos()->is_valid());

    uint16_t gid_T = font->glyph_index('T');
    uint16_t gid_o = font->glyph_index('o');
    uint16_t gid_V = font->glyph_index('V');
    uint16_t gid_A = font->glyph_index('A');

    // In Source Sans 3, 'T' followed by 'o', and 'V' followed by 'A' have kerning
    int16_t kern_To = font->get_kerning(gid_T, gid_o);
    int16_t kern_VA = font->get_kerning(gid_V, gid_A);

    std::cout << "  Kerning 'T'-'o': " << kern_To << " font design units" << std::endl;
    std::cout << "  Kerning 'V'-'A': " << kern_VA << " font design units" << std::endl;

    assert(kern_To < 0);
    assert(kern_VA < 0);

    // Test pair positioning details
    GlyphPlacementAdjustment first{}, second{};
    bool pair_ok = font->gpos()->get_pair_adjustment(gid_T, gid_o, first, second);
    assert(pair_ok);
    assert(first.x_advance == kern_To);

    std::cout << "  -> PASS" << std::endl;
}

void test_synthetic_mark_to_base() {
    std::cout << "[Test] OpenType GPOS Mark-to-Base Positioning..." << std::endl;

    // Construct a minimal valid GPOS table in memory with LookupType 4 (Mark-to-Base Format 1)
    // Structure:
    // Header (10 bytes):
    //   majorVersion: 1
    //   minorVersion: 0
    //   scriptListOffset: 10
    //   featureListOffset: 18
    //   lookupListOffset: 34
    // ScriptList (8 bytes at 10):
    //   scriptCount: 0
    // FeatureList (16 bytes at 18):
    //   featureCount: 1
    //   featureRecord: 'mark', offset 8 (at 26)
    //   featureTable (8 bytes at 26):
    //     featureParams: 0
    //     lookupIndexCount: 1
    //     lookupListIndices: 0
    // LookupList (at 34):
    //   lookupCount: 1
    //   offset 0: 4 (at 38)
    // LookupTable (at 38):
    //   lookupType: 4 (MarkToBase)
    //   lookupFlag: 0
    //   subTableCount: 1
    //   subtableOffset 0: 8 (at 46)
    // Subtable (at 46):
    //   posFormat: 1
    //   markCoverageOffset: 12 (at 58)
    //   baseCoverageOffset: 20 (at 66)
    //   markClassCount: 1
    //   markArrayOffset: 28 (at 74)
    //   baseArrayOffset: 40 (at 86)

    std::vector<uint8_t> gpos_data;
    auto write_u16 = [&](uint16_t v) {
        gpos_data.push_back(static_cast<uint8_t>(v >> 8));
        gpos_data.push_back(static_cast<uint8_t>(v & 0xFF));
    };
    auto write_u32 = [&](uint32_t v) {
        gpos_data.push_back(static_cast<uint8_t>(v >> 24));
        gpos_data.push_back(static_cast<uint8_t>((v >> 16) & 0xFF));
        gpos_data.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));
        gpos_data.push_back(static_cast<uint8_t>(v & 0xFF));
    };

    // 0: Header
    write_u16(1); // major
    write_u16(0); // minor
    write_u16(10); // ScriptList
    write_u16(12); // FeatureList
    write_u16(26); // LookupList

    // 10: ScriptList
    write_u16(0); // count

    // 12: FeatureList
    write_u16(1); // count = 1
    write_u32(0x6D61726B); // 'mark'
    write_u16(8); // offset to feature table (relative to 12 -> offset 20)

    // 20: Feature Table
    write_u16(0); // featureParams
    write_u16(1); // lookupIndexCount
    write_u16(0); // lookupListIndex = 0

    // 26: LookupList
    write_u16(1); // count = 1
    write_u16(4); // offset to lookup 0 (relative to 26 -> 30)

    // 30: Lookup 0
    write_u16(4); // lookupType = 4 (MarkToBase)
    write_u16(0); // lookupFlag
    write_u16(1); // subTableCount = 1
    write_u16(8); // offset to subtable 0 (relative to 30 -> 38)

    auto patch_u16 = [&](size_t pos, uint16_t v) {
        gpos_data[pos] = static_cast<uint8_t>(v >> 8);
        gpos_data[pos + 1] = static_cast<uint8_t>(v & 0xFF);
    };

    // 38: Subtable 0 (MarkToBase Format 1)
    size_t sub_start = gpos_data.size();
    write_u16(1);  // posFormat
    size_t mark_cov_pos = gpos_data.size(); write_u16(0);
    size_t base_cov_pos = gpos_data.size(); write_u16(0);
    write_u16(1);  // markClassCount = 1
    size_t mark_array_pos = gpos_data.size(); write_u16(0);
    size_t base_array_pos = gpos_data.size(); write_u16(0);

    // MarkCoverage (Format 1)
    patch_u16(mark_cov_pos, static_cast<uint16_t>(gpos_data.size() - sub_start));
    write_u16(1); // format
    write_u16(1); // count
    write_u16(50); // glyph 50 is mark

    // BaseCoverage (Format 1)
    patch_u16(base_cov_pos, static_cast<uint16_t>(gpos_data.size() - sub_start));
    write_u16(1); // format
    write_u16(1); // count
    write_u16(25); // glyph 25 is base

    // MarkArray
    size_t mark_array_start = gpos_data.size();
    patch_u16(mark_array_pos, static_cast<uint16_t>(mark_array_start - sub_start));
    write_u16(1); // markCount = 1
    write_u16(0); // markClass = 0
    size_t mark_anch_pos = gpos_data.size(); write_u16(0);

    // Mark Anchor 0
    patch_u16(mark_anch_pos, static_cast<uint16_t>(gpos_data.size() - mark_array_start));
    write_u16(1); // format = 1
    write_u16(100); // x = 100
    write_u16(500); // y = 500

    // BaseArray
    size_t base_array_start = gpos_data.size();
    patch_u16(base_array_pos, static_cast<uint16_t>(base_array_start - sub_start));
    write_u16(1); // baseCount = 1
    size_t base_anch_pos = gpos_data.size(); write_u16(0);

    // Base Anchor 0
    patch_u16(base_anch_pos, static_cast<uint16_t>(gpos_data.size() - base_array_start));
    write_u16(1); // format = 1
    write_u16(250); // x = 250
    write_u16(700); // y = 700

    auto gpos = OpenTypeGpos::from_bytes(gpos_data);
    assert(gpos != nullptr);
    assert(gpos->is_valid());

    int16_t dx = 0, dy = 0;
    bool found = gpos->get_mark_to_base_offset(25, 50, dx, dy);
    assert(found);
    assert(dx == 150); // 250 - 100
    assert(dy == 200); // 700 - 500

    std::cout << "  Calculated mark-to-base anchor displacement: dx=" << dx << ", dy=" << dy << std::endl;
    std::cout << "  -> PASS" << std::endl;
}

void test_otf_canvas_rasterization() {
    std::cout << "[Test] PostScript / CFF Canvas Rasterization..." << std::endl;

    std::string font_path = resolve_font("SourceSans3-Regular.otf");
    auto font = TtfFont::from_file(font_path);
    assert(font != nullptr);

    auto pixmap = Pixmap::create(300, 100, PixelFormat::RGBA8888);
    assert(pixmap.has_value());
    pixmap->fill(Color::WHITE);
    Canvas canvas(*pixmap);

    Paint paint;
    paint.shader = Shader(Color::from_rgba8(25, 50, 200, 255));
    paint.anti_alias = true;

    // Render "Nisaba" using glyph paths
    const std::string text = "Nisaba";
    float x = 20.0f;
    float y = 60.0f;
    float font_size = 36.0f;

    for (char c : text) {
        uint16_t gid = font->glyph_index(static_cast<char32_t>(c));
        assert(gid > 0);
        Path glyph_path;
        if (font->get_glyph_path(gid, glyph_path, font_size)) {
            glyph_path.apply_transform(Transform::from_translate(x, y));
            canvas.fill_path(glyph_path, paint);
        }
        x += font->glyph_advance(gid, font_size);
    }

    // Verify canvas has drawn non-white pixels
    size_t painted_pixels = 0;
    for (uint32_t py = 0; py < pixmap->height(); ++py) {
        for (uint32_t px = 0; px < pixmap->width(); ++px) {
            auto p = pixmap->pixel(px, py);
            if (p && (p->r != 255 || p->g != 255 || p->b != 255)) {
                ++painted_pixels;
            }
        }
    }

    assert(painted_pixels > 500);
    std::cout << "  Painted " << painted_pixels << " text pixels cleanly from PostScript cubic curves." << std::endl;
    std::cout << "  -> PASS" << std::endl;
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << " Nisaba CFF & Advanced OpenType Tests   " << std::endl;
    std::cout << "========================================" << std::endl;

    test_otf_loading_and_cff_cubics();
    test_opentype_gsub_ligatures();
    test_opentype_gpos_kerning();
    test_synthetic_mark_to_base();
    test_otf_canvas_rasterization();

    std::cout << "========================================" << std::endl;
    std::cout << " All CFF & OpenType tests PASSED!       " << std::endl;
    std::cout << "========================================" << std::endl;
    return 0;
}
