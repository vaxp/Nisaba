#include "nisaba/markdown/style.hpp"

namespace nisaba::markdown {

MarkdownStyle MarkdownStyle::dark_theme() {
    MarkdownStyle s;
    // Default values in header represent the dark theme
    return s;
}

MarkdownStyle MarkdownStyle::light_theme() {
    MarkdownStyle s;
    s.text_color = Color::from_rgba8(35, 45, 60, 255);
    s.heading_color = Color::from_rgba8(15, 20, 30, 255);
    s.link_color = Color::from_rgba8(10, 110, 220, 255);
    s.link_hover_color = Color::from_rgba8(0, 80, 180, 255);
    s.quote_bar_color = Color::from_rgba8(20, 140, 220, 220);
    s.quote_bg_color = Color::from_rgba8(240, 245, 252, 220);
    s.quote_text_color = Color::from_rgba8(70, 85, 105, 240);

    s.code_text_color = Color::from_rgba8(25, 35, 50, 255);
    s.code_block_bg = Color::from_rgba8(244, 247, 251, 255);
    s.code_block_border = Color::from_rgba8(215, 225, 238, 240);
    s.code_span_bg = Color::from_rgba8(235, 240, 248, 255);
    s.code_span_text = Color::from_rgba8(15, 110, 190, 255);

    s.table_border_color = Color::from_rgba8(215, 225, 238, 240);
    s.table_header_bg = Color::from_rgba8(238, 243, 250, 255);
    s.table_row_alt_bg = Color::from_rgba8(248, 250, 253, 200);
    s.table_header_text = Color::from_rgba8(20, 30, 45, 255);

    s.divider_color = Color::from_rgba8(215, 225, 238, 240);

    s.task_checkbox_border = Color::from_rgba8(10, 130, 230, 240);
    s.task_checkbox_checked = Color::from_rgba8(0, 175, 100, 255);

    // Light Alert theme
    s.alert_note.bar_color = Color::from_rgba8(9, 105, 218, 255);
    s.alert_note.bg_color = Color::from_rgba8(235, 245, 255, 230);
    s.alert_note.title_color = Color::from_rgba8(9, 105, 218, 255);

    s.alert_tip.bar_color = Color::from_rgba8(26, 127, 55, 255);
    s.alert_tip.bg_color = Color::from_rgba8(235, 251, 238, 230);
    s.alert_tip.title_color = Color::from_rgba8(26, 127, 55, 255);

    s.alert_important.bar_color = Color::from_rgba8(130, 80, 223, 255);
    s.alert_important.bg_color = Color::from_rgba8(246, 242, 254, 230);
    s.alert_important.title_color = Color::from_rgba8(130, 80, 223, 255);

    s.alert_warning.bar_color = Color::from_rgba8(154, 103, 0, 255);
    s.alert_warning.bg_color = Color::from_rgba8(255, 248, 197, 230);
    s.alert_warning.title_color = Color::from_rgba8(154, 103, 0, 255);

    s.alert_caution.bar_color = Color::from_rgba8(207, 34, 46, 255);
    s.alert_caution.bg_color = Color::from_rgba8(255, 235, 233, 230);
    s.alert_caution.title_color = Color::from_rgba8(207, 34, 46, 255);

    // Light Syntax Highlighting
    s.syn_keyword = Color::from_rgba8(207, 34, 46, 255);       // red
    s.syn_type = Color::from_rgba8(149, 56, 0, 255);          // orange/brown
    s.syn_string = Color::from_rgba8(10, 48, 105, 255);       // deep blue
    s.syn_number = Color::from_rgba8(5, 80, 174, 255);        // blue
    s.syn_comment = Color::from_rgba8(101, 109, 118, 240);    // gray
    s.syn_preprocessor = Color::from_rgba8(207, 34, 46, 255); // red
    s.syn_operator = Color::from_rgba8(207, 34, 46, 255);
    s.syn_punctuation = Color::from_rgba8(36, 41, 47, 255);
    s.syn_linenumber = Color::from_rgba8(140, 149, 159, 220);

    // Light HTML
    s.kbd_bg_color = Color::from_rgba8(246, 248, 250, 255);
    s.kbd_border_color = Color::from_rgba8(175, 184, 193, 255);
    s.kbd_shadow_color = Color::from_rgba8(200, 205, 215, 255);
    s.kbd_text_color = Color::from_rgba8(36, 41, 47, 255);

    s.mark_bg_color = Color::from_rgba8(255, 248, 197, 240);
    s.mark_text_color = Color::from_rgba8(36, 41, 47, 255);

    s.details_border_color = Color::from_rgba8(210, 218, 226, 255);
    s.details_bg_color = Color::from_rgba8(246, 248, 250, 200);
    s.details_summary_color = Color::from_rgba8(9, 105, 218, 255);
    s.details_chevron_color = Color::from_rgba8(101, 109, 118, 255);

    s.footnote_text_color = Color::from_rgba8(101, 109, 118, 255);
    s.footnote_link_color = Color::from_rgba8(9, 105, 218, 255);

    return s;
}

} // namespace nisaba::markdown
