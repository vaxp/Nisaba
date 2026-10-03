#include "nisaba/markdown/renderer.hpp"
#include "nisaba/markdown/syntax_highlighter.hpp"
#include "nisaba/path/path_builder.hpp"
#include "nisaba/text/buffer.hpp"
#include <algorithm>
#include <cmath>
#include <numeric>

namespace nisaba::markdown {

namespace {

void render_inlines_to_canvas(
    Canvas& canvas,
    const std::vector<InlineSpan>& inlines,
    float x,
    float y,
    float width,
    float font_size,
    float line_height_mult,
    Color default_color,
    const MarkdownStyle& style,
    text::FontSystem& font_system,
    text::GlyphCache& glyph_cache,
    text::Align text_align = text::Align::Left
) {
    if (inlines.empty()) return;

    std::string text;
    text::AttrsList attrs;

    auto append_span = [&](auto& self, const InlineSpan& span) -> void {
        size_t start = text.size();
        text::Attrs attr;
        attr.set_family(text::Family::sans_serif());

        if (span.text_color) {
            attr.set_color(text::TextColor::rgba(
                static_cast<uint8_t>(span.text_color->red() * 255.0f),
                static_cast<uint8_t>(span.text_color->green() * 255.0f),
                static_cast<uint8_t>(span.text_color->blue() * 255.0f),
                static_cast<uint8_t>(span.text_color->alpha() * 255.0f)
            ));
        }

        switch (span.type) {
            case InlineType::Text:
            case InlineType::Emoji:
                text += span.text;
                break;

            case InlineType::Bold:
                attr.set_weight(text::Weight::Bold);
                if (!span.children.empty()) {
                    for (const auto& ch : span.children) self(self, ch);
                } else {
                    text += span.text;
                }
                attrs.add_span(start, text.size(), attr);
                break;

            case InlineType::Italic:
                attr.set_style(text::Style::Italic);
                if (!span.children.empty()) {
                    for (const auto& ch : span.children) self(self, ch);
                } else {
                    text += span.text;
                }
                attrs.add_span(start, text.size(), attr);
                break;

            case InlineType::BoldItalic:
                attr.set_weight(text::Weight::Bold);
                attr.set_style(text::Style::Italic);
                if (!span.children.empty()) {
                    for (const auto& ch : span.children) self(self, ch);
                } else {
                    text += span.text;
                }
                attrs.add_span(start, text.size(), attr);
                break;

            case InlineType::CodeSpan:
                attr.set_family(text::Family::monospace());
                if (!span.text_color) {
                    attr.set_color(text::TextColor::rgba(
                        static_cast<uint8_t>(style.code_span_text.red() * 255.0f),
                        static_cast<uint8_t>(style.code_span_text.green() * 255.0f),
                        static_cast<uint8_t>(style.code_span_text.blue() * 255.0f),
                        255
                    ));
                }
                text += ' ';
                text += span.text;
                text += ' ';
                attrs.add_span(start, text.size(), attr);
                break;

            case InlineType::Kbd:
                attr.set_family(text::Family::monospace());
                attr.set_color(text::TextColor::rgba(
                    static_cast<uint8_t>(style.kbd_text_color.red() * 255.0f),
                    static_cast<uint8_t>(style.kbd_text_color.green() * 255.0f),
                    static_cast<uint8_t>(style.kbd_text_color.blue() * 255.0f),
                    255
                ));
                text += " ";
                text += span.text;
                text += " ";
                attrs.add_span(start, text.size(), attr);
                break;

            case InlineType::Link:
                if (!span.text_color) {
                    attr.set_color(text::TextColor::rgba(
                        static_cast<uint8_t>(style.link_color.red() * 255.0f),
                        static_cast<uint8_t>(style.link_color.green() * 255.0f),
                        static_cast<uint8_t>(style.link_color.blue() * 255.0f),
                        255
                    ));
                }
                if (!span.children.empty()) {
                    for (const auto& ch : span.children) self(self, ch);
                } else {
                    text += span.text;
                }
                attrs.add_span(start, text.size(), attr);
                break;

            case InlineType::Strikethrough:
                if (!span.children.empty()) {
                    for (const auto& ch : span.children) self(self, ch);
                } else {
                    text += span.text;
                }
                break;

            case InlineType::Underline:
                if (!span.children.empty()) {
                    for (const auto& ch : span.children) self(self, ch);
                } else {
                    text += span.text;
                }
                break;

            case InlineType::Highlight:
                attr.set_color(text::TextColor::rgba(
                    static_cast<uint8_t>(style.mark_text_color.red() * 255.0f),
                    static_cast<uint8_t>(style.mark_text_color.green() * 255.0f),
                    static_cast<uint8_t>(style.mark_text_color.blue() * 255.0f),
                    255
                ));
                text += span.text;
                attrs.add_span(start, text.size(), attr);
                break;

            case InlineType::Subscript:
            case InlineType::Superscript:
                text += span.text;
                break;

            case InlineType::FootnoteRef:
                attr.set_color(text::TextColor::rgba(
                    static_cast<uint8_t>(style.footnote_link_color.red() * 255.0f),
                    static_cast<uint8_t>(style.footnote_link_color.green() * 255.0f),
                    static_cast<uint8_t>(style.footnote_link_color.blue() * 255.0f),
                    255
                ));
                text += "[";
                text += span.text.empty() ? std::to_string(span.footnote_index) : span.text;
                text += "]";
                attrs.add_span(start, text.size(), attr);
                break;

            case InlineType::LineBreak:
                text += '\n';
                break;

            case InlineType::Image:
                text += "[Image: ";
                text += span.text;
                text += "]";
                break;
        }
    };

    for (const auto& span : inlines) {
        append_span(append_span, span);
    }

    if (text.empty()) return;

    float line_h = font_size * line_height_mult;
    text::Buffer buf(text::Metrics(font_size, line_h));
    buf.set_wrap(text::Wrap::Word);
    buf.set_align(text_align);
    buf.set_size(width > 10.0f ? std::optional<float>(width) : std::nullopt, std::nullopt);
    buf.set_text(text, attrs.defaults());

    if (!buf.lines().empty()) {
        buf.lines_mut()[0].attrs_list_mut() = attrs;
    }

    buf.shape_until_scroll(font_system);
    buf.draw(canvas, glyph_cache, font_system, default_color, x, y);
}

void draw_alert_icon(Canvas& canvas, AlertType type, float cx, float cy, float r, Color color) {
    Paint p(color);
    Stroke stroke(1.5f);

    switch (type) {
        case AlertType::Note: {
            // Circle with 'i'
            canvas.stroke_circle(cx, cy, r, p, stroke);
            canvas.fill_circle(cx, cy - r * 0.45f, 1.4f, p);
            canvas.stroke_line(cx, cy - r * 0.15f, cx, cy + r * 0.45f, p, stroke);
            break;
        }
        case AlertType::Tip: {
            // Lightbulb shape
            canvas.stroke_circle(cx, cy - 2.0f, r * 0.8f, p, stroke);
            canvas.stroke_line(cx - 3.0f, cy + 4.0f, cx + 3.0f, cy + 4.0f, p, stroke);
            canvas.stroke_line(cx - 2.0f, cy + 6.5f, cx + 2.0f, cy + 6.5f, p, stroke);
            break;
        }
        case AlertType::Important: {
            // Rounded square with '!'
            auto rect = Rect::from_xywh(cx - r, cy - r, 2.0f * r, 2.0f * r);
            if (rect) canvas.stroke_round_rect(*rect, 3.0f, 3.0f, p, stroke);
            canvas.stroke_line(cx, cy - r * 0.45f, cx, cy + r * 0.15f, p, stroke);
            canvas.fill_circle(cx, cy + r * 0.45f, 1.4f, p);
            break;
        }
        case AlertType::Warning: {
            // Triangle with '!'
            float h = r * 1.7f;
            canvas.stroke_line(cx, cy - h * 0.5f, cx - r, cy + h * 0.5f, p, stroke);
            canvas.stroke_line(cx - r, cy + h * 0.5f, cx + r, cy + h * 0.5f, p, stroke);
            canvas.stroke_line(cx + r, cy + h * 0.5f, cx, cy - h * 0.5f, p, stroke);
            canvas.stroke_line(cx, cy - h * 0.15f, cx, cy + h * 0.15f, p, stroke);
            canvas.fill_circle(cx, cy + h * 0.35f, 1.2f, p);
            break;
        }
        case AlertType::Caution: {
            // Octagon stop sign
            float s = r * 0.85f;
            canvas.stroke_circle(cx, cy, r, p, stroke);
            canvas.stroke_line(cx - s * 0.6f, cy, cx + s * 0.6f, cy, p, Stroke(2.0f));
            break;
        }
        case AlertType::None:
            break;
    }
}

void draw_chevron(Canvas& canvas, float cx, float cy, float s, bool is_open, Color color) {
    Paint p(color);
    Stroke stroke(1.8f);

    if (is_open) {
        // Pointing down: v
        canvas.stroke_line(cx - s, cy - s * 0.4f, cx, cy + s * 0.5f, p, stroke);
        canvas.stroke_line(cx, cy + s * 0.5f, cx + s, cy - s * 0.4f, p, stroke);
    } else {
        // Pointing right: >
        canvas.stroke_line(cx - s * 0.4f, cy - s, cx + s * 0.5f, cy, p, stroke);
        canvas.stroke_line(cx + s * 0.5f, cy, cx - s * 0.4f, cy + s, p, stroke);
    }
}

} // namespace

void MarkdownRenderer::render(
    Canvas& canvas,
    MarkdownDocument& doc,
    float x,
    float y,
    float width,
    const MarkdownStyle& style,
    text::FontSystem& font_system,
    text::GlyphCache& glyph_cache,
    float scroll_y
) {
    if (!doc.is_valid()) return;

    if (doc.layout_width() != width) {
        doc.layout(width, style, font_system, glyph_cache);
    }

    float origin_y = y - scroll_y;

    for (const auto& block_ptr : doc.root()->children) {
        if (!block_ptr) continue;
        const auto& block = *block_ptr;

        float block_top = origin_y + block.layout_y;
        float block_bot = block_top + block.layout_height;

        if (block_bot < 0.0f || block_top > static_cast<float>(canvas.height())) {
            continue;
        }

        switch (block.type) {
            case BlockType::Heading: {
                int lvl = std::clamp(block.heading_level, 1, 6);
                float fsize = style.heading_sizes[static_cast<size_t>(lvl - 1)];

                render_inlines_to_canvas(
                    canvas, block.inlines, x, block_top, width, fsize,
                    style.heading_line_height_multiplier, style.heading_color,
                    style, font_system, glyph_cache
                );

                if (lvl <= 2) {
                    float underline_y = block_bot - 2.0f;
                    Paint line_p(style.divider_color);
                    auto r = Rect::from_xywh(x, underline_y, width, lvl == 1 ? 2.0f : 1.0f);
                    if (r) canvas.fill_rect(*r, line_p);
                }
                break;
            }

            case BlockType::Paragraph: {
                text::Align align = text::Align::Left;
                if (block.block_align == 1) align = text::Align::Center;
                else if (block.block_align == 2) align = text::Align::Right;

                render_inlines_to_canvas(
                    canvas, block.inlines, x, block_top, width, style.base_font_size,
                    style.line_height_multiplier, style.text_color,
                    style, font_system, glyph_cache, align
                );
                break;
            }

            case BlockType::Alert: {
                const MarkdownStyle::AlertTheme* theme = &style.alert_note;
                if (block.alert_type == AlertType::Tip) theme = &style.alert_tip;
                else if (block.alert_type == AlertType::Important) theme = &style.alert_important;
                else if (block.alert_type == AlertType::Warning) theme = &style.alert_warning;
                else if (block.alert_type == AlertType::Caution) theme = &style.alert_caution;

                // 1. Alert Card Background
                auto card = Rect::from_xywh(x, block_top, width, block.layout_height);
                if (card) {
                    Paint bg_p(theme->bg_color);
                    canvas.fill_round_rect(*card, 6.0f, 6.0f, bg_p);

                    // Left vertical accent bar
                    auto bar = Rect::from_xywh(x, block_top, 4.0f, block.layout_height);
                    if (bar) {
                        Paint bar_p(theme->bar_color);
                        canvas.fill_round_rect(*bar, 2.0f, 2.0f, bar_p);
                    }
                }

                // 2. Vector Icon & Title
                draw_alert_icon(canvas, block.alert_type, x + 20.0f, block_top + 16.0f, 7.5f, theme->title_color);

                Paint title_p(theme->title_color);
                canvas.draw_text_debug(block.alert_title, x + 34.0f, block_top + 10.0f, title_p, 1.0f);

                // 3. Render alert child content
                for (const auto& child : block.children) {
                    if (!child) continue;
                    float c_top = block_top + child->layout_y;
                    render_inlines_to_canvas(
                        canvas, child->inlines, x + child->layout_x, c_top, child->layout_width,
                        style.base_font_size, style.line_height_multiplier,
                        style.text_color, style, font_system, glyph_cache
                    );
                }
                break;
            }

            case BlockType::Details: {
                // Collapsible <details> container
                auto card = Rect::from_xywh(x, block_top, width, block.layout_height);
                if (card) {
                    Paint bg_p(style.details_bg_color);
                    canvas.fill_round_rect(*card, 6.0f, 6.0f, bg_p);

                    Paint border_p(style.details_border_color);
                    canvas.stroke_round_rect(*card, 6.0f, 6.0f, border_p, Stroke(1.0f));
                }

                // Chevron icon
                draw_chevron(canvas, x + 16.0f, block_top + 18.0f, 6.0f, block.is_open, style.details_chevron_color);

                // Summary text
                Paint sum_p(style.details_summary_color);
                canvas.draw_text_debug(block.summary_text, x + 28.0f, block_top + 11.0f, sum_p, 0.95f);

                // Render inner children if open
                if (block.is_open) {
                    for (const auto& child : block.children) {
                        if (!child) continue;
                        float c_top = block_top + child->layout_y;
                        render_inlines_to_canvas(
                            canvas, child->inlines, x + child->layout_x, c_top, child->layout_width,
                            style.base_font_size, style.line_height_multiplier,
                            style.text_color, style, font_system, glyph_cache
                        );
                    }
                }
                break;
            }

            case BlockType::CodeBlock: {
                // 1. Card background & border
                auto card_rect = Rect::from_xywh(x, block_top, width, block.layout_height);
                if (card_rect) {
                    Paint bg_p(style.code_block_bg);
                    canvas.fill_round_rect(*card_rect, style.code_block_corner_radius, style.code_block_corner_radius, bg_p);

                    Paint border_p(style.code_block_border);
                    Stroke border_s(1.0f);
                    canvas.stroke_round_rect(*card_rect, style.code_block_corner_radius, style.code_block_corner_radius, border_p, border_s);
                }

                // 2. Language badge in top-right
                if (!block.language.empty()) {
                    float badge_pad = 12.0f;
                    float badge_x = x + width - badge_pad - static_cast<float>(block.language.size()) * 7.5f;
                    Paint badge_p;
                    badge_p.set_color_rgba8(140, 165, 195, 180);
                    canvas.draw_text_debug(block.language, badge_x, block_top + 8.0f, badge_p, 0.75f);
                }

                // 3. Syntax-highlighted code lines
                float code_x = x + style.code_block_padding;
                float code_y = block_top + style.code_block_padding;
                float line_h = style.code_font_size * style.code_line_height_multiplier;

                bool in_multi_comment = false;
                int line_num = 1;

                for (const auto& line : block.code_lines) {
                    float cur_x = code_x;

                    // Optional line numbering
                    if (style.show_line_numbers) {
                        std::string num_str = std::to_string(line_num++);
                        Paint num_p(style.syn_linenumber);
                        canvas.draw_text_debug(num_str, cur_x, code_y + 1.0f, num_p, 0.85f);
                        cur_x += style.line_number_margin;
                    }

                    if (style.enable_syntax_highlighting && !block.language.empty()) {
                        auto tokens = SyntaxHighlighter::tokenize_line(line, block.language, in_multi_comment);
                        for (const auto& tok : tokens) {
                            Paint tok_p(SyntaxHighlighter::token_color(tok.type, style));
                            canvas.draw_text_debug(tok.text, cur_x, code_y + 1.0f, tok_p, 0.9f);
                            cur_x += static_cast<float>(tok.text.size()) * 7.2f;
                        }
                    } else {
                        Paint text_p(style.code_text_color);
                        canvas.draw_text_debug(line, cur_x, code_y + 1.0f, text_p, 0.9f);
                    }

                    code_y += line_h;
                }
                break;
            }

            case BlockType::BlockQuote: {
                auto quote_rect = Rect::from_xywh(x, block_top, width, block.layout_height);
                if (quote_rect) {
                    Paint bg_p(style.quote_bg_color);
                    canvas.fill_round_rect(*quote_rect, 4.0f, 4.0f, bg_p);

                    auto bar_rect = Rect::from_xywh(x, block_top, style.quote_bar_width, block.layout_height);
                    if (bar_rect) {
                        Paint bar_p(style.quote_bar_color);
                        canvas.fill_round_rect(*bar_rect, 2.0f, 2.0f, bar_p);
                    }
                }

                float child_x = x + style.quote_left_padding + style.quote_bar_width;
                for (const auto& child : block.children) {
                    if (!child) continue;
                    float c_top = block_top + child->layout_y + style.quote_vertical_padding;
                    render_inlines_to_canvas(
                        canvas, child->inlines, child_x, c_top, child->layout_width,
                        style.quote_font_size, style.line_height_multiplier,
                        style.quote_text_color, style, font_system, glyph_cache
                    );
                }
                break;
            }

            case BlockType::List: {
                int item_num = block.list_start_number;
                for (const auto& item_ptr : block.children) {
                    if (!item_ptr) continue;
                    const auto& item = *item_ptr;
                    float i_top = block_top + item.layout_y;
                    float bullet_x = x + item.layout_x - 14.0f;

                    if (item.is_task_item) {
                        float box_size = 14.0f;
                        float box_x = x + item.layout_x - 18.0f;
                        float box_y = i_top + 2.0f;

                        auto cb_rect = Rect::from_xywh(box_x, box_y, box_size, box_size);
                        if (cb_rect) {
                            Paint cb_border(style.task_checkbox_border);
                            Stroke cb_s(1.2f);
                            canvas.stroke_round_rect(*cb_rect, 3.0f, 3.0f, cb_border, cb_s);

                            if (item.is_task_checked) {
                                auto inner = Rect::from_xywh(box_x + 3.0f, box_y + 3.0f, box_size - 6.0f, box_size - 6.0f);
                                if (inner) {
                                    Paint fill_p(style.task_checkbox_checked);
                                    canvas.fill_round_rect(*inner, 2.0f, 2.0f, fill_p);
                                }
                            }
                        }
                    } else if (block.is_ordered_list) {
                        std::string num_str = std::to_string(item_num++) + ".";
                        Paint num_p(style.quote_bar_color);
                        canvas.draw_text_debug(num_str, bullet_x - 4.0f, i_top + 2.0f, num_p, 0.85f);
                    } else {
                        Paint bullet_p(style.quote_bar_color);
                        if (item.list_level == 0) {
                            canvas.fill_circle(bullet_x, i_top + style.base_font_size * 0.55f, 3.0f, bullet_p);
                        } else if (item.list_level == 1) {
                            canvas.stroke_circle(bullet_x, i_top + style.base_font_size * 0.55f, 2.8f, bullet_p, Stroke(1.2f));
                        } else {
                            auto sq = Rect::from_xywh(bullet_x - 2.5f, i_top + style.base_font_size * 0.55f - 2.5f, 5.0f, 5.0f);
                            if (sq) canvas.fill_rect(*sq, bullet_p);
                        }
                    }

                    render_inlines_to_canvas(
                        canvas, item.inlines, x + item.layout_x, i_top, item.layout_width,
                        style.base_font_size, style.line_height_multiplier,
                        style.text_color, style, font_system, glyph_cache
                    );
                }
                break;
            }

            case BlockType::Table: {
                size_t num_cols = block.table_alignments.size();
                if (num_cols == 0 && !block.table_rows.empty()) num_cols = block.table_rows[0].cells.size();
                if (num_cols == 0) break;

                // Dynamic Column Width Solver:
                std::vector<float> desired_widths(num_cols, 40.0f);
                for (const auto& row : block.table_rows) {
                    for (size_t c = 0; c < row.cells.size() && c < num_cols; ++c) {
                        std::string cell_txt;
                        for (const auto& span : row.cells[c].inlines) cell_txt += span.text;
                        float txt_w = static_cast<float>(cell_txt.size()) * (style.base_font_size * 0.58f) + 2.0f * style.table_cell_padding_h;
                        desired_widths[c] = std::max(desired_widths[c], txt_w);
                    }
                }

                float total_desired = std::accumulate(desired_widths.begin(), desired_widths.end(), 0.0f);
                float available_w = width - style.table_border_width * static_cast<float>(num_cols + 1);

                std::vector<float> col_widths(num_cols);
                for (size_t c = 0; c < num_cols; ++c) {
                    if (total_desired > 1.0f) {
                        col_widths[c] = std::max(50.0f, (desired_widths[c] / total_desired) * available_w);
                    } else {
                        col_widths[c] = available_w / static_cast<float>(num_cols);
                    }
                }

                // Outer table frame
                auto tbl_rect = Rect::from_xywh(x, block_top, width, block.layout_height);
                if (tbl_rect) {
                    Paint tbl_border(style.table_border_color);
                    Stroke tbl_s(style.table_border_width);
                    canvas.stroke_round_rect(*tbl_rect, style.table_corner_radius, style.table_corner_radius, tbl_border, tbl_s);
                }

                float cur_y = block_top + style.table_border_width;
                bool is_first_row = true;

                for (const auto& row : block.table_rows) {
                    float row_h = row.layout_height > 0.0f ? row.layout_height : (style.base_font_size * style.line_height_multiplier + 2.0f * style.table_cell_padding_v);

                    auto r_bg = Rect::from_xywh(x, cur_y, width, row_h);
                    if (r_bg) {
                        if (row.is_header) {
                            Paint hdr_p(style.table_header_bg);
                            canvas.fill_rect(*r_bg, hdr_p);
                        } else if (!is_first_row) {
                            Paint alt_p(style.table_row_alt_bg);
                            canvas.fill_rect(*r_bg, alt_p);
                        }
                    }

                    float cur_x = x + style.table_border_width;
                    for (size_t c = 0; c < row.cells.size() && c < num_cols; ++c) {
                        const auto& cell = row.cells[c];
                        text::Align align = text::Align::Left;
                        if (cell.align == TableAlign::Center) align = text::Align::Center;
                        else if (cell.align == TableAlign::Right) align = text::Align::Right;

                        render_inlines_to_canvas(
                            canvas, cell.inlines, cur_x + style.table_cell_padding_h,
                            cur_y + style.table_cell_padding_v,
                            col_widths[c] - 2.0f * style.table_cell_padding_h,
                            style.base_font_size, style.line_height_multiplier,
                            row.is_header ? style.table_header_text : style.text_color,
                            style, font_system, glyph_cache, align
                        );

                        cur_x += col_widths[c] + style.table_border_width;
                    }

                    Paint hdiv_p(style.table_border_color);
                    canvas.stroke_line(x, cur_y + row_h, x + width, cur_y + row_h, hdiv_p, Stroke(style.table_border_width));

                    cur_y += row_h + style.table_border_width;
                    is_first_row = false;
                }
                break;
            }

            case BlockType::ThematicBreak: {
                Paint div_p(style.divider_color);
                canvas.stroke_line(x, block_top, x + width, block_top, div_p, Stroke(style.divider_thickness));
                break;
            }

            case BlockType::FootnoteDef: {
                // Footnote definition line
                std::string prefix = "[" + std::to_string(block.footnote_index) + "] ";
                Paint fn_p(style.footnote_link_color);
                canvas.draw_text_debug(prefix, x, block_top + 1.0f, fn_p, 0.9f);

                render_inlines_to_canvas(
                    canvas, block.inlines, x + 24.0f, block_top, width - 24.0f,
                    style.footnote_font_size, style.line_height_multiplier,
                    style.footnote_text_color, style, font_system, glyph_cache
                );
                break;
            }

            case BlockType::HtmlBlock:
            case BlockType::ListItem:
            case BlockType::Document:
                break;
        }
    }
}

void MarkdownRenderer::render(
    ICanvas& canvas,
    MarkdownDocument& doc,
    float x,
    float y,
    float width,
    const MarkdownStyle& style,
    text::FontSystem& font_system,
    text::GlyphCache& glyph_cache,
    float scroll_y
) {
    auto* cpu_canvas = dynamic_cast<Canvas*>(&canvas);
    if (cpu_canvas) {
        render(*cpu_canvas, doc, x, y, width, style, font_system, glyph_cache, scroll_y);
    }
}

} // namespace nisaba::markdown
