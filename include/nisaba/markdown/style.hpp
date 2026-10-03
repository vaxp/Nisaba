#pragma once

#include <string>
#include <array>
#include "nisaba/color/color.hpp"

namespace nisaba::markdown {

/// Styling and visual theme parameters for rendering Markdown documents.
struct MarkdownStyle {
    // --- Typography Sizes (in pixels) ---
    float base_font_size{15.0f};
    std::array<float, 6> heading_sizes{30.0f, 24.0f, 20.0f, 17.0f, 15.0f, 13.0f};
    float code_font_size{13.5f};
    float quote_font_size{14.5f};

    // --- Line Heights ---
    float line_height_multiplier{1.45f};
    float heading_line_height_multiplier{1.25f};
    float code_line_height_multiplier{1.35f};

    // --- Spacing & Margins (in pixels) ---
    float paragraph_spacing{12.0f};
    std::array<float, 6> heading_top_margins{24.0f, 20.0f, 16.0f, 14.0f, 12.0f, 10.0f};
    std::array<float, 6> heading_bottom_margins{10.0f, 8.0f, 6.0f, 6.0f, 4.0f, 4.0f};
    float list_item_spacing{4.0f};
    float list_indent{24.0f};
    float quote_left_padding{18.0f};
    float quote_vertical_padding{8.0f};
    float quote_bar_width{4.0f};

    // --- Code Blocks & Spans ---
    float code_block_padding{14.0f};
    float code_block_corner_radius{8.0f};
    float code_span_padding_h{5.0f};
    float code_span_padding_v{2.0f};
    float code_span_corner_radius{4.0f};

    // --- Tables ---
    float table_cell_padding_h{14.0f};
    float table_cell_padding_v{8.0f};
    float table_border_width{1.0f};
    float table_corner_radius{6.0f};

    // --- Thematic Break ---
    float divider_thickness{1.5f};
    float divider_margin{18.0f};

    // --- Colors (RGBA) ---
    Color text_color{Color::from_rgba8(225, 232, 245, 255)};
    Color heading_color{Color::from_rgba8(255, 255, 255, 255)};
    Color link_color{Color::from_rgba8(75, 170, 255, 255)};
    Color link_hover_color{Color::from_rgba8(120, 200, 255, 255)};
    Color quote_bar_color{Color::from_rgba8(0, 220, 255, 200)};
    Color quote_bg_color{Color::from_rgba8(20, 30, 48, 140)};
    Color quote_text_color{Color::from_rgba8(180, 205, 235, 230)};

    Color code_text_color{Color::from_rgba8(230, 240, 255, 255)};
    Color code_block_bg{Color::from_rgba8(14, 20, 32, 240)};
    Color code_block_border{Color::from_rgba8(35, 52, 80, 200)};
    Color code_span_bg{Color::from_rgba8(28, 40, 62, 220)};
    Color code_span_text{Color::from_rgba8(0, 230, 255, 255)};

    Color table_border_color{Color::from_rgba8(38, 55, 85, 220)};
    Color table_header_bg{Color::from_rgba8(22, 34, 54, 255)};
    Color table_row_alt_bg{Color::from_rgba8(16, 24, 38, 120)};
    Color table_header_text{Color::from_rgba8(255, 255, 255, 255)};

    Color divider_color{Color::from_rgba8(35, 50, 75, 200)};

    Color task_checkbox_border{Color::from_rgba8(0, 220, 255, 220)};
    Color task_checkbox_checked{Color::from_rgba8(0, 200, 140, 255)};

    // --- GitHub Callout Alert Styles ---
    struct AlertTheme {
        Color bar_color;
        Color bg_color;
        Color title_color;
    };
    AlertTheme alert_note{
        Color::from_rgba8(31, 111, 235, 255),    // #1f6feb blue
        Color::from_rgba8(15, 30, 56, 180),
        Color::from_rgba8(88, 166, 255, 255)
    };
    AlertTheme alert_tip{
        Color::from_rgba8(35, 134, 54, 255),     // #238636 green
        Color::from_rgba8(16, 44, 26, 180),
        Color::from_rgba8(63, 185, 80, 255)
    };
    AlertTheme alert_important{
        Color::from_rgba8(137, 87, 229, 255),   // #8957e5 purple
        Color::from_rgba8(38, 26, 62, 180),
        Color::from_rgba8(187, 128, 255, 255)
    };
    AlertTheme alert_warning{
        Color::from_rgba8(210, 153, 34, 255),    // #d29922 amber
        Color::from_rgba8(52, 40, 16, 180),
        Color::from_rgba8(240, 180, 41, 255)
    };
    AlertTheme alert_caution{
        Color::from_rgba8(218, 54, 51, 255),     // #da3633 red
        Color::from_rgba8(56, 20, 22, 180),
        Color::from_rgba8(248, 81, 73, 255)
    };

    // --- Syntax Highlighting Palette (GitHub Dark Theme by default) ---
    bool enable_syntax_highlighting{true};
    bool show_line_numbers{false};
    float line_number_margin{38.0f};

    Color syn_keyword{Color::from_rgba8(255, 123, 114, 255)};     // #ff7b72 red/pink
    Color syn_type{Color::from_rgba8(121, 192, 255, 255)};        // #79c0ff light blue
    Color syn_string{Color::from_rgba8(165, 214, 255, 255)};      // #a5d6ff cyan/ice
    Color syn_number{Color::from_rgba8(210, 168, 255, 255)};      // #d2a8ff purple
    Color syn_comment{Color::from_rgba8(139, 148, 158, 220)};     // #8b949e muted gray
    Color syn_preprocessor{Color::from_rgba8(255, 123, 114, 255)};// #ff7b72
    Color syn_operator{Color::from_rgba8(255, 123, 114, 255)};    // #ff7b72
    Color syn_punctuation{Color::from_rgba8(201, 209, 217, 255)}; // #c9d1d9
    Color syn_linenumber{Color::from_rgba8(110, 118, 129, 200)};  // #6e7681

    // --- HTML Element Specific Styling ---
    // <kbd>
    Color kbd_bg_color{Color::from_rgba8(22, 27, 34, 255)};
    Color kbd_border_color{Color::from_rgba8(110, 118, 129, 200)};
    Color kbd_shadow_color{Color::from_rgba8(1, 4, 9, 255)};
    Color kbd_text_color{Color::from_rgba8(240, 246, 252, 255)};
    float kbd_padding_h{6.0f};
    float kbd_padding_v{3.0f};
    float kbd_corner_radius{4.0f};

    // <mark>
    Color mark_bg_color{Color::from_rgba8(187, 128, 9, 120)};
    Color mark_text_color{Color::from_rgba8(255, 255, 255, 255)};

    // <details> / <summary>
    Color details_border_color{Color::from_rgba8(48, 54, 61, 200)};
    Color details_bg_color{Color::from_rgba8(13, 17, 23, 180)};
    Color details_summary_color{Color::from_rgba8(88, 166, 255, 255)};
    Color details_chevron_color{Color::from_rgba8(139, 148, 158, 255)};

    // Footnotes
    Color footnote_text_color{Color::from_rgba8(139, 148, 158, 255)};
    Color footnote_link_color{Color::from_rgba8(88, 166, 255, 255)};
    float footnote_font_size{12.5f};

    // --- Font Family Names ---
    std::string sans_family{"Inter"};
    std::string mono_family{"Fira Mono"};

    /// Modern dark theme matching Nisaba's premium aesthetic and GitHub Dark.
    static MarkdownStyle dark_theme();

    /// Clean light theme matching GitHub Light.
    static MarkdownStyle light_theme();
};

} // namespace nisaba::markdown
