#include "nisaba/text/buffer.hpp"
#include "nisaba/text/bidi.hpp"
#include <algorithm>
#include <cmath>

namespace nisaba::text {

BufferLine::BufferLine(std::string_view line_text, const Attrs& default_attrs)
    : text_(line_text), attrs_list_(default_attrs) {}

void BufferLine::shape_and_layout(
    FontSystem& font_system,
    float font_size,
    float line_height,
    std::optional<float> wrap_width,
    Wrap wrap,
    Align align,
    Shaping shaping
) {
    layout_lines_.clear();
    is_shaped_ = true;

    if (text_.empty()) {
        LayoutLine empty_line;
        empty_line.line_height_opt = line_height;
        layout_lines_.push_back(std::move(empty_line));
        return;
    }

    bool is_pure_ascii = true;
    for (unsigned char c : text_) {
        if (c >= 128) {
            is_pure_ascii = false;
            break;
        }
    }

    // -------------------------------------------------------------------------
    // ULTRA-FAST PATH: Pure ASCII with Uniform Attributes (Standard Case)
    // -------------------------------------------------------------------------
    if (is_pure_ascii && attrs_list_.spans().empty()) {
        const Attrs& def_attrs = attrs_list_.defaults();
        auto [primary_fid, _] = font_system.match_font(def_attrs.family, def_attrs.weight, def_attrs.style);
        const TtfFont* primary_font = font_system.get_font(primary_fid);
        float scale = (primary_font != nullptr) ? primary_font->scale_for_size(font_size) : 0.0f;

        std::vector<LayoutGlyph> all_glyphs;
        all_glyphs.resize(text_.size());
        float cur_x = 0.0f;

        for (size_t i = 0; i < text_.size(); ++i) {
            uint8_t c = static_cast<uint8_t>(text_[i]);
            uint16_t gid = primary_font ? primary_font->ascii_glyph_index(c) : 0;
            float adv = primary_font ? (static_cast<float>(primary_font->ascii_advance(c)) * scale) : (font_size * 0.5f);

            LayoutGlyph& g = all_glyphs[i];
            g.start = i;
            g.end = i + 1;
            g.font_size = font_size;
            g.font_weight = def_attrs.weight;
            g.line_height_opt = line_height;
            g.font_id = primary_fid;
            g.glyph_id = gid;
            g.x = cur_x;
            g.y = 0.0f;
            g.w = adv;
            g.level = 0;
            g.x_offset = 0.0f;
            g.y_offset = 0.0f;
            g.color_opt = def_attrs.color_opt;
            g.metadata = def_attrs.metadata;

            cur_x += adv;
        }

        // Single line without wrapping
        if (!wrap_width.has_value() || wrap == Wrap::None) {
            LayoutLine single_line;
            single_line.line_height_opt = line_height;
            single_line.w = cur_x;
            single_line.glyphs = std::move(all_glyphs);

            if (wrap_width.has_value() && *wrap_width > single_line.w) {
                float spare = *wrap_width - single_line.w;
                float offset = 0.0f;
                if (align == Align::Right || align == Align::End) {
                    offset = spare;
                } else if (align == Align::Center) {
                    offset = spare * 0.5f;
                }
                if (offset > 0.0f) {
                    for (auto& g : single_line.glyphs) {
                        g.x += offset;
                    }
                }
            }

            layout_lines_.push_back(std::move(single_line));
            return;
        }

        // Wrapping logic for pure ASCII
        float max_w = *wrap_width;
        auto finalize_ascii_line = [&](LayoutLine& line) {
            if (line.glyphs.empty()) return;
            float lx = 0.0f;
            for (auto& g : line.glyphs) {
                g.x = lx;
                lx += g.w;
            }
            line.w = lx;
            if (wrap_width.has_value() && *wrap_width > line.w) {
                float spare = *wrap_width - line.w;
                float offset = 0.0f;
                if (align == Align::Right || align == Align::End) {
                    offset = spare;
                } else if (align == Align::Center) {
                    offset = spare * 0.5f;
                }
                if (offset > 0.0f) {
                    for (auto& g : line.glyphs) g.x += offset;
                }
            }
        };

        if (wrap == Wrap::Glyph) {
            LayoutLine cur_line;
            cur_line.line_height_opt = line_height;
            float cur_line_w = 0.0f;

            for (auto& g : all_glyphs) {
                if (cur_line_w + g.w > max_w && !cur_line.glyphs.empty()) {
                    finalize_ascii_line(cur_line);
                    layout_lines_.push_back(std::move(cur_line));
                    cur_line = LayoutLine{};
                    cur_line.line_height_opt = line_height;
                    cur_line_w = 0.0f;
                }
                cur_line_w += g.w;
                cur_line.glyphs.push_back(g);
            }
            if (!cur_line.glyphs.empty()) {
                finalize_ascii_line(cur_line);
                layout_lines_.push_back(std::move(cur_line));
            }
            return;
        } else {
            // Word or WordOrGlyph wrapping for pure ASCII
            struct Word {
                std::vector<LayoutGlyph> glyphs;
                float width{0.0f};
                bool is_space{false};
            };

            std::vector<Word> words;
            words.reserve(all_glyphs.size() / 4 + 1);
            Word current_word;

            for (size_t i = 0; i < all_glyphs.size(); ++i) {
                char c = text_[i];
                bool is_sp = (c == ' ' || c == '\t');

                if (!current_word.glyphs.empty() && current_word.is_space != is_sp) {
                    words.push_back(std::move(current_word));
                    current_word = Word{};
                }

                current_word.is_space = is_sp;
                current_word.glyphs.push_back(all_glyphs[i]);
                current_word.width += all_glyphs[i].w;
            }
            if (!current_word.glyphs.empty()) {
                words.push_back(std::move(current_word));
            }

            LayoutLine cur_line;
            cur_line.line_height_opt = line_height;
            float cur_line_w = 0.0f;

            for (auto& w : words) {
                if (cur_line_w + w.width > max_w && !cur_line.glyphs.empty()) {
                    if (!w.is_space) {
                        finalize_ascii_line(cur_line);
                        layout_lines_.push_back(std::move(cur_line));
                        cur_line = LayoutLine{};
                        cur_line.line_height_opt = line_height;
                        cur_line_w = 0.0f;
                    }
                }

                if (w.width > max_w && wrap == Wrap::WordOrGlyph && !w.is_space) {
                    for (auto& g : w.glyphs) {
                        if (cur_line_w + g.w > max_w && !cur_line.glyphs.empty()) {
                            finalize_ascii_line(cur_line);
                            layout_lines_.push_back(std::move(cur_line));
                            cur_line = LayoutLine{};
                            cur_line.line_height_opt = line_height;
                            cur_line_w = 0.0f;
                        }
                        cur_line_w += g.w;
                        cur_line.glyphs.push_back(g);
                    }
                } else {
                    for (auto& g : w.glyphs) {
                        cur_line.glyphs.push_back(g);
                    }
                    cur_line_w += w.width;
                }
            }

            if (!cur_line.glyphs.empty()) {
                finalize_ascii_line(cur_line);
                layout_lines_.push_back(std::move(cur_line));
            }
            return;
        }
    }

    // -------------------------------------------------------------------------
    // FAST LTR PATH: UTF-8 without RTL / Complex BiDi (e.g. European languages, 'Bézier', etc.)
    // -------------------------------------------------------------------------
    auto decoded = Bidi::decode_utf8(text_);
    bool has_rtl = false;
    for (const auto& [cp, _] : decoded) {
        if (cp >= 0x0590 && cp <= 0x08FF) {
            auto t = Bidi::classify_char(cp);
            if (t == CharType::ArabicLetter || t == CharType::RTL) {
                has_rtl = true;
                break;
            }
        }
    }

    if (!has_rtl && attrs_list_.spans().empty()) {
        const Attrs& def_attrs = attrs_list_.defaults();
        auto [primary_fid, _] = font_system.match_font(def_attrs.family, def_attrs.weight, def_attrs.style);
        const TtfFont* primary_font = font_system.get_font(primary_fid);
        float scale = (primary_font != nullptr) ? primary_font->scale_for_size(font_size) : 0.0f;

        std::vector<LayoutGlyph> all_glyphs;
        all_glyphs.resize(decoded.size());
        float cur_x = 0.0f;

        for (size_t i = 0; i < decoded.size(); ++i) {
            char32_t cp = decoded[i].first;
            size_t byte_idx = decoded[i].second;

            uint32_t actual_fid = primary_fid;
            uint16_t gid = 0;
            float adv = font_size * 0.5f;

            if (primary_font != nullptr) {
                gid = primary_font->glyph_index(cp);
                adv = primary_font->glyph_advance_scaled(gid, scale);
            }

            if (gid == 0 && font_system.font_count() > 1) {
                auto [fb_fid, fb_gid] = font_system.find_glyph_or_fallback(primary_fid, cp);
                actual_fid = fb_fid;
                const TtfFont* fb_font = font_system.get_font(fb_fid);
                if (fb_font != nullptr) {
                    gid = fb_gid;
                    adv = fb_font->glyph_advance(gid, font_size);
                }
            }

            LayoutGlyph& g = all_glyphs[i];
            g.start = byte_idx;
            g.end = (i + 1 < decoded.size()) ? decoded[i + 1].second : text_.size();
            g.font_size = font_size;
            g.font_weight = def_attrs.weight;
            g.line_height_opt = line_height;
            g.font_id = actual_fid;
            g.glyph_id = gid;
            g.x = cur_x;
            g.y = 0.0f;
            g.w = adv;
            g.level = 0;
            g.x_offset = 0.0f;
            g.y_offset = 0.0f;
            g.color_opt = def_attrs.color_opt;
            g.metadata = def_attrs.metadata;

            cur_x += adv;
        }

        // Single line without wrapping
        if (!wrap_width.has_value() || wrap == Wrap::None) {
            LayoutLine single_line;
            single_line.line_height_opt = line_height;
            single_line.w = cur_x;
            single_line.glyphs = std::move(all_glyphs);

            if (wrap_width.has_value() && *wrap_width > single_line.w) {
                float spare = *wrap_width - single_line.w;
                float offset = 0.0f;
                if (align == Align::Right || align == Align::End) {
                    offset = spare;
                } else if (align == Align::Center) {
                    offset = spare * 0.5f;
                }
                if (offset > 0.0f) {
                    for (auto& g : single_line.glyphs) {
                        g.x += offset;
                    }
                }
            }

            layout_lines_.push_back(std::move(single_line));
            return;
        }

        float max_w = *wrap_width;
        auto finalize_ltr_line = [&](LayoutLine& line) {
            if (line.glyphs.empty()) return;
            float lx = 0.0f;
            for (auto& g : line.glyphs) {
                g.x = lx;
                lx += g.w;
            }
            line.w = lx;
            if (wrap_width.has_value() && *wrap_width > line.w) {
                float spare = *wrap_width - line.w;
                float offset = 0.0f;
                if (align == Align::Right || align == Align::End) {
                    offset = spare;
                } else if (align == Align::Center) {
                    offset = spare * 0.5f;
                }
                if (offset > 0.0f) {
                    for (auto& g : line.glyphs) g.x += offset;
                }
            }
        };

        if (wrap == Wrap::Glyph) {
            LayoutLine cur_line;
            cur_line.line_height_opt = line_height;
            float cur_line_w = 0.0f;

            for (auto& g : all_glyphs) {
                if (cur_line_w + g.w > max_w && !cur_line.glyphs.empty()) {
                    finalize_ltr_line(cur_line);
                    layout_lines_.push_back(std::move(cur_line));
                    cur_line = LayoutLine{};
                    cur_line.line_height_opt = line_height;
                    cur_line_w = 0.0f;
                }
                cur_line_w += g.w;
                cur_line.glyphs.push_back(g);
            }
            if (!cur_line.glyphs.empty()) {
                finalize_ltr_line(cur_line);
                layout_lines_.push_back(std::move(cur_line));
            }
            return;
        } else {
            struct Word {
                std::vector<LayoutGlyph> glyphs;
                float width{0.0f};
                bool is_space{false};
            };

            std::vector<Word> words;
            words.reserve(all_glyphs.size() / 4 + 1);
            Word current_word;
            bool prev_was_cjk = false;
            bool prev_no_break_after = false;

            for (size_t i = 0; i < all_glyphs.size(); ++i) {
                char32_t cp = decoded[i].first;
                bool is_sp = (cp == ' ' || cp == '\t');
                bool is_cjk_char = Bidi::is_cjk(cp);
                bool no_break_before = is_cjk_char && Bidi::is_cjk_no_break_before(cp);
                bool no_break_after = is_cjk_char && Bidi::is_cjk_no_break_after(cp);

                bool should_split = false;
                if (!current_word.glyphs.empty()) {
                    if (current_word.is_space != is_sp) {
                        should_split = true;
                    } else if (!is_sp) {
                        if (is_cjk_char) {
                            if (!no_break_before && !prev_no_break_after) should_split = true;
                        } else if (prev_was_cjk && !no_break_before) {
                            should_split = true;
                        }
                    }
                }

                if (should_split) {
                    words.push_back(std::move(current_word));
                    current_word = Word{};
                }

                current_word.is_space = is_sp;
                current_word.glyphs.push_back(all_glyphs[i]);
                current_word.width += all_glyphs[i].w;

                prev_was_cjk = is_cjk_char;
                prev_no_break_after = no_break_after;
            }
            if (!current_word.glyphs.empty()) {
                words.push_back(std::move(current_word));
            }

            LayoutLine cur_line;
            cur_line.line_height_opt = line_height;
            float cur_line_w = 0.0f;

            for (auto& w : words) {
                if (cur_line_w + w.width > max_w && !cur_line.glyphs.empty()) {
                    if (!w.is_space) {
                        finalize_ltr_line(cur_line);
                        layout_lines_.push_back(std::move(cur_line));
                        cur_line = LayoutLine{};
                        cur_line.line_height_opt = line_height;
                        cur_line_w = 0.0f;
                    }
                }

                if (w.width > max_w && wrap == Wrap::WordOrGlyph && !w.is_space) {
                    for (auto& g : w.glyphs) {
                        if (cur_line_w + g.w > max_w && !cur_line.glyphs.empty()) {
                            finalize_ltr_line(cur_line);
                            layout_lines_.push_back(std::move(cur_line));
                            cur_line = LayoutLine{};
                            cur_line.line_height_opt = line_height;
                            cur_line_w = 0.0f;
                        }
                        cur_line_w += g.w;
                        cur_line.glyphs.push_back(g);
                    }
                } else {
                    for (auto& g : w.glyphs) {
                        cur_line.glyphs.push_back(g);
                    }
                    cur_line_w += w.width;
                }
            }

            if (!cur_line.glyphs.empty()) {
                finalize_ltr_line(cur_line);
                layout_lines_.push_back(std::move(cur_line));
            }
            return;
        }
    }

    Direction base_dir = Direction::LeftToRight;
    std::vector<ShapedChar> shaped_chars;

    if (is_pure_ascii) {
        shaped_chars.resize(text_.size());
        for (size_t i = 0; i < text_.size(); ++i) {
            shaped_chars[i] = ShapedChar{
                static_cast<char32_t>(static_cast<unsigned char>(text_[i])),
                i,
                0,
                false
            };
        }
    } else {
        base_dir = Bidi::detect_base_direction(text_);
        auto runs = Bidi::segment_runs(text_, base_dir);
        if (shaping == Shaping::Advanced) {
            shaped_chars = Bidi::shape_text(text_, runs);
        } else {
            shaped_chars.reserve(decoded.size());
            for (const auto& [cp, byte_idx] : decoded) {
                shaped_chars.push_back(ShapedChar{cp, byte_idx, 0, false});
            }
        }
    }

    if (shaped_chars.empty()) {
        LayoutLine empty_line;
        empty_line.line_height_opt = line_height;
        layout_lines_.push_back(std::move(empty_line));
        return;
    }

    // Measure glyphs and create initial list of LayoutGlyph
    std::vector<LayoutGlyph> all_glyphs;
    all_glyphs.reserve(shaped_chars.size());

    // Cache font matching & font lookup across identical attributes
    uint32_t last_primary_fid = 0;
    const TtfFont* last_primary_font = nullptr;
    std::optional<Attrs> last_attrs = std::nullopt;

    for (size_t i = 0; i < shaped_chars.size(); ++i) {
        const auto& sc = shaped_chars[i];
        Attrs attrs = attrs_list_.spans().empty() ? attrs_list_.defaults() : attrs_list_.get(sc.byte_index);

        if (!last_attrs.has_value() || *last_attrs != attrs) {
            auto [primary_fid, _] = font_system.match_font(attrs.family, attrs.weight, attrs.style);
            last_primary_fid = primary_fid;
            last_primary_font = font_system.get_font(primary_fid);
            last_attrs = attrs;
        }

        uint32_t actual_fid = last_primary_fid;
        const TtfFont* font = last_primary_font;
        uint16_t gid = 0;

        if (font != nullptr) {
            gid = font->glyph_index(sc.codepoint);
        }

        if (gid == 0 && font_system.font_count() > 1) {
            auto [fb_fid, fb_gid] = font_system.find_glyph_or_fallback(last_primary_fid, sc.codepoint);
            actual_fid = fb_fid;
            gid = fb_gid;
            font = font_system.get_font(actual_fid);
        }

        float adv = (font != nullptr) ? font->glyph_advance(gid, font_size) : (font_size * 0.5f);

        LayoutGlyph g;
        g.start = sc.byte_index;
        g.end = sc.byte_index + 1;
        g.font_size = font_size;
        g.font_weight = attrs.weight;
        g.line_height_opt = line_height;
        g.font_id = actual_fid;
        g.glyph_id = gid;
        g.w = adv;
        g.level = sc.level;
        g.color_opt = attrs.color_opt;
        g.metadata = attrs.metadata;

        all_glyphs.push_back(g);
    }

    // Line wrapping algorithm
    auto finalize_line = [&](LayoutLine& line) {
        if (line.glyphs.empty()) return;
        float cur_x = 0.0f;
        for (auto& g : line.glyphs) {
            g.x = cur_x;
            cur_x += g.w;
        }
        line.w = cur_x;

        // Apply alignment
        if (wrap_width.has_value() && *wrap_width > line.w) {
            float spare = *wrap_width - line.w;
            float offset = 0.0f;
            if (align == Align::Right) {
                offset = spare;
            } else if (align == Align::Center) {
                offset = spare * 0.5f;
            } else if (align == Align::End) {
                offset = (base_dir == Direction::RightToLeft) ? 0.0f : spare;
            }
            if (offset > 0.0f) {
                for (auto& g : line.glyphs) {
                    g.x += offset;
                }
            }
        }
    };

    if (!wrap_width.has_value() || wrap == Wrap::None) {
        LayoutLine single_line;
        single_line.line_height_opt = line_height;
        single_line.glyphs = std::move(all_glyphs);
        finalize_line(single_line);
        layout_lines_.push_back(std::move(single_line));
        return;
    }

    float max_w = *wrap_width;

    if (wrap == Wrap::Glyph) {
        LayoutLine cur_line;
        cur_line.line_height_opt = line_height;
        float cur_line_w = 0.0f;

        for (auto& g : all_glyphs) {
            if (cur_line_w + g.w > max_w && !cur_line.glyphs.empty()) {
                finalize_line(cur_line);
                layout_lines_.push_back(std::move(cur_line));
                cur_line = LayoutLine{};
                cur_line.line_height_opt = line_height;
                cur_line_w = 0.0f;
            }
            cur_line_w += g.w;
            cur_line.glyphs.push_back(g);
        }
        if (!cur_line.glyphs.empty()) {
            finalize_line(cur_line);
            layout_lines_.push_back(std::move(cur_line));
        }
    } else {
        // Word or WordOrGlyph wrapping
        struct Word {
            std::vector<LayoutGlyph> glyphs;
            float width{0.0f};
            bool is_space{false};
        };

        std::vector<Word> words;
        Word current_word;
        bool prev_was_cjk = false;
        bool prev_no_break_after = false;

        for (size_t i = 0; i < all_glyphs.size(); ++i) {
            const auto& g = all_glyphs[i];
            char32_t cp = (i < shaped_chars.size()) ? shaped_chars[i].codepoint : 0;
            bool is_sp = (cp == ' ' || cp == '\t');
            bool is_cjk_char = is_pure_ascii ? false : Bidi::is_cjk(cp);
            bool no_break_before = is_cjk_char && Bidi::is_cjk_no_break_before(cp);
            bool no_break_after = is_cjk_char && Bidi::is_cjk_no_break_after(cp);

            bool should_split = false;
            if (!current_word.glyphs.empty()) {
                if (current_word.is_space != is_sp) {
                    should_split = true;
                } else if (!is_sp) {
                    // CJK ideographs/syllabaries can break individually unless punctuation rules forbid it
                    if (is_cjk_char) {
                        if (!no_break_before && !prev_no_break_after) {
                            should_split = true;
                        }
                    } else if (prev_was_cjk && !no_break_before) {
                        should_split = true;
                    }
                }
            }

            if (should_split) {
                words.push_back(std::move(current_word));
                current_word = Word{};
            }

            current_word.is_space = is_sp;
            current_word.glyphs.push_back(g);
            current_word.width += g.w;

            prev_was_cjk = is_cjk_char;
            prev_no_break_after = no_break_after;
        }
        if (!current_word.glyphs.empty()) {
            words.push_back(std::move(current_word));
        }

        LayoutLine cur_line;
        cur_line.line_height_opt = line_height;
        float cur_line_w = 0.0f;

        for (auto& w : words) {
            if (cur_line_w + w.width > max_w && !cur_line.glyphs.empty()) {
                // If the word causing overflow is whitespace at the end of line, skip line break
                if (!w.is_space) {
                    finalize_line(cur_line);
                    layout_lines_.push_back(std::move(cur_line));
                    cur_line = LayoutLine{};
                    cur_line.line_height_opt = line_height;
                    cur_line_w = 0.0f;
                }
            }

            // If a single word is wider than max_w
            if (w.width > max_w && wrap == Wrap::WordOrGlyph && !w.is_space) {
                for (const auto& g : w.glyphs) {
                    if (cur_line_w + g.w > max_w && !cur_line.glyphs.empty()) {
                        finalize_line(cur_line);
                        layout_lines_.push_back(std::move(cur_line));
                        cur_line = LayoutLine{};
                        cur_line.line_height_opt = line_height;
                        cur_line_w = 0.0f;
                    }
                    cur_line_w += g.w;
                    cur_line.glyphs.push_back(g);
                }
            } else {
                for (const auto& g : w.glyphs) {
                    cur_line.glyphs.push_back(g);
                }
                cur_line_w += w.width;
            }
        }

        if (!cur_line.glyphs.empty()) {
            finalize_line(cur_line);
            layout_lines_.push_back(std::move(cur_line));
        }
    }
}

Buffer::Buffer(Metrics metrics) : metrics_(metrics) {}

void Buffer::reset_shaping() noexcept {
    for (auto& line : lines_) {
        line.reset_shaping();
    }
}

void Buffer::set_text(std::string_view text, const Attrs& default_attrs) {
    lines_.clear();
    size_t line_count = 1;
    for (char c : text) {
        if (c == '\n') ++line_count;
    }
    lines_.reserve(line_count);

    size_t start = 0;
    for (size_t i = 0; i < text.size(); ++i) {
        if (text[i] == '\n') {
            size_t end = i;
            if (end > start && text[end - 1] == '\r') {
                end--;
            }
            lines_.emplace_back(text.substr(start, end - start), default_attrs);
            start = i + 1;
        }
    }
    if (start <= text.size()) {
        lines_.emplace_back(text.substr(start), default_attrs);
    }
}

void Buffer::shape_until_scroll(FontSystem& font_system) {
    for (auto& line : lines_) {
        if (!line.is_shaped()) {
            line.shape_and_layout(
                font_system,
                metrics_.font_size,
                metrics_.line_height,
                width_,
                wrap_,
                align_,
                shaping_
            );
        }
    }
}

std::vector<LayoutRun> Buffer::layout_runs() const {
    std::vector<LayoutRun> runs;
    runs.reserve(lines_.size());
    float current_top = 0.0f;

    for (size_t line_i = 0; line_i < lines_.size(); ++line_i) {
        const auto& line = lines_[line_i];
        for (const auto& layout_line : line.layout()) {
            float lh = layout_line.line_height_opt.value_or(metrics_.line_height);
            runs.push_back(LayoutRun{
                line_i,
                line.text(),
                layout_line.glyphs,
                current_top,
                lh
            });
            current_top += lh;
        }
    }
    return runs;
}

float Buffer::total_height() const {
    float h = 0.0f;
    for (const auto& line : lines_) {
        for (const auto& layout_line : line.layout()) {
            h += layout_line.line_height_opt.value_or(metrics_.line_height);
        }
    }
    return h;
}

void Buffer::draw(
    Canvas& canvas,
    GlyphCache& cache,
    FontSystem& font_system,
    Color default_color,
    float origin_x,
    float origin_y
) {
    shape_until_scroll(font_system);
    auto runs = layout_runs();

    for (const auto& run : runs) {
        float line_baseline = origin_y + run.line_top + metrics_.font_size;

        for (const auto& glyph : run.glyphs) {
            if (glyph.glyph_id == 0) continue;

            const TtfFont* font = font_system.get_font(glyph.font_id);
            if (!font) continue;

            float glyph_x = origin_x + glyph.x;
            float glyph_y = line_baseline + glyph.y;

            const CachedGlyph* cached = cache.get_or_render(
                *font,
                glyph.font_id,
                glyph.glyph_id,
                glyph.font_size,
                glyph_x,
                glyph_y
            );

            if (!cached || cached->data.empty()) continue;

            Point dst_pos = Point::from_xy(
                glyph_x + static_cast<float>(cached->offset_x),
                glyph_y + static_cast<float>(cached->offset_y)
            );

            Paint paint;
            if (glyph.color_opt.has_value()) {
                paint.set_color(glyph.color_opt->to_color());
            } else {
                paint.set_color(default_color);
            }

            canvas.draw_glyph(dst_pos, cached->as_mask_ref(), paint);
        }
    }
}

} // namespace nisaba::text
