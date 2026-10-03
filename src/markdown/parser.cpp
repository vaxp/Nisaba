#include "nisaba/markdown/parser.hpp"
#include "nisaba/markdown/html_parser.hpp"
#include <cctype>
#include <algorithm>
#include <sstream>
#include <unordered_map>

namespace nisaba::markdown {

namespace {

inline std::string_view trim_start(std::string_view s) noexcept {
    while (!s.empty() && (s.front() == ' ' || s.front() == '\t')) s.remove_prefix(1);
    return s;
}

inline std::string_view trim_end(std::string_view s) noexcept {
    while (!s.empty() && (s.back() == ' ' || s.back() == '\t' || s.back() == '\r')) s.remove_suffix(1);
    return s;
}

inline std::string_view trim(std::string_view s) noexcept {
    return trim_end(trim_start(s));
}

inline bool is_blank(std::string_view s) noexcept {
    return trim(s).empty();
}

std::vector<std::string_view> split_lines(std::string_view text) {
    std::vector<std::string_view> lines;
    size_t start = 0;
    while (start < text.size()) {
        size_t end = text.find('\n', start);
        if (end == std::string_view::npos) {
            lines.push_back(text.substr(start));
            break;
        }
        lines.push_back(text.substr(start, end - start));
        start = end + 1;
    }
    return lines;
}

inline std::string to_lower(std::string_view s) {
    std::string res;
    res.reserve(s.size());
    for (char c : s) res.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
    return res;
}

// GitHub Emojis table (Comprehensive standard GFM set)
static const std::unordered_map<std::string_view, std::string_view> emoji_map = {
    {"smile", "😄"},
    {"rocket", "🚀"},
    {"tada", "🎉"},
    {"sparkles", "✨"},
    {"warning", "⚠️"},
    {"+1", "👍"},
    {"-1", "👎"},
    {"heart", "❤️"},
    {"white_check_mark", "✅"},
    {"x", "❌"},
    {"fire", "🔥"},
    {"star", "⭐"},
    {"bulb", "💡"},
    {"zap", "⚡"},
    {"lock", "🔒"},
    {"key", "🔑"},
    {"memo", "📝"},
    {"book", "📖"},
    {"link", "🔗"},
    {"package", "📦"},
    {"bug", "🐛"},
    {"eyes", "👀"},
    {"gear", "⚙️"},
    {"wrench", "🔧"},
    {"construction", "🚧"},
    {"heavy_check_mark", "✔️"},
    {"information_source", "ℹ️"},
    {"octocat", "🐙"},
    {"v", "✌️"},
    {"100", "💯"},
    {"check", "✓"},
    {"art", "🎨"},
    {"coffee", "☕"},
    {"beer", "🍺"},
    {"trophy", "🏆"},
    {"chart_with_upwards_trend", "📈"},
    {"hammer", "🔨"},
    {"shield", "🛡️"},
    {"computer", "💻"},
    {"books", "📚"},
    {"globe_with_meridians", "🌐"},
    {"thinking", "🤔"},
    {"clap", "👏"},
    {"pray", "🙏"},
    {"runner", "🏃"},
    {"muscle", "💪"},
    {"wave", "👋"},
    {"sparkler", "🎇"},
    {"hourglass", "⏳"},
    {"mag", "🔍"},
    {"bell", "🔔"},
    {"speech_balloon", "💬"},
    {"dart", "🎯"},
    {"gem", "💎"},
    {"crown", "👑"},
    {"battery", "🔋"},
    {"radioactive", "☢️"},
    {"biohazard", "☣️"},
    {"recycle", "♻️"},
    {"ok_hand", "👌"}
};

} // namespace

std::string MarkdownParser::replace_emojis(std::string_view text) {
    std::string result;
    result.reserve(text.size());

    size_t i = 0;
    while (i < text.size()) {
        if (text[i] == ':') {
            size_t end_colon = text.find(':', i + 1);
            if (end_colon != std::string_view::npos && end_colon - i > 1) {
                // Must not contain spaces
                auto candidate = text.substr(i + 1, end_colon - i - 1);
                if (candidate.find(' ') == std::string_view::npos) {
                    auto it = emoji_map.find(candidate);
                    if (it != emoji_map.end()) {
                        result += it->second;
                        i = end_colon + 1;
                        continue;
                    }
                }
            }
        }
        result += text[i];
        i++;
    }
    return result;
}

std::string MarkdownParser::slugify(std::string_view text) {
    std::string slug;
    slug.reserve(text.size());

    bool prev_dash = false;
    for (char c : text) {
        if (std::isalnum(static_cast<unsigned char>(c))) {
            slug.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
            prev_dash = false;
        } else if (c == ' ' || c == '-' || c == '_') {
            if (!prev_dash && !slug.empty()) {
                slug.push_back('-');
                prev_dash = true;
            }
        }
    }

    while (!slug.empty() && slug.back() == '-') {
        slug.pop_back();
    }
    return slug;
}

bool MarkdownParser::is_thematic_break(std::string_view line) {
    auto s = trim(line);
    if (s.size() < 3) return false;
    char c = s[0];
    if (c != '-' && c != '*' && c != '_') return false;

    size_t count = 0;
    for (char ch : s) {
        if (ch == c) {
            count++;
        } else if (ch != ' ' && ch != '\t') {
            return false;
        }
    }
    return count >= 3;
}

int MarkdownParser::parse_heading_level(std::string_view line, std::string_view& out_text) {
    auto s = trim_start(line);
    if (s.empty() || s[0] != '#') return 0;

    int level = 0;
    while (level < 6 && static_cast<size_t>(level) < s.size() && s[level] == '#') {
        level++;
    }

    if (static_cast<size_t>(level) < s.size() && (s[level] == ' ' || s[level] == '\t')) {
        auto content = trim(s.substr(level));
        while (!content.empty() && content.back() == '#') {
            content.remove_suffix(1);
        }
        out_text = trim(content);
        return level;
    }

    return 0;
}

AlertType MarkdownParser::parse_alert_marker(std::string_view line, std::string& out_title) {
    auto s = trim(line);
    if (!s.starts_with(">")) return AlertType::None;
    s.remove_prefix(1);
    s = trim(s);

    if (!s.starts_with("[!")) return AlertType::None;

    size_t close_bracket = s.find(']');
    if (close_bracket == std::string_view::npos) return AlertType::None;

    auto tag = s.substr(2, close_bracket - 2);
    auto remainder = trim(s.substr(close_bracket + 1));

    std::string tag_upper;
    tag_upper.reserve(tag.size());
    for (char c : tag) tag_upper.push_back(static_cast<char>(std::toupper(static_cast<unsigned char>(c))));

    AlertType type = AlertType::None;
    if (tag_upper == "NOTE") {
        type = AlertType::Note;
        out_title = remainder.empty() ? "Note" : std::string(remainder);
    } else if (tag_upper == "TIP") {
        type = AlertType::Tip;
        out_title = remainder.empty() ? "Tip" : std::string(remainder);
    } else if (tag_upper == "IMPORTANT") {
        type = AlertType::Important;
        out_title = remainder.empty() ? "Important" : std::string(remainder);
    } else if (tag_upper == "WARNING") {
        type = AlertType::Warning;
        out_title = remainder.empty() ? "Warning" : std::string(remainder);
    } else if (tag_upper == "CAUTION") {
        type = AlertType::Caution;
        out_title = remainder.empty() ? "Caution" : std::string(remainder);
    }

    return type;
}

bool MarkdownParser::parse_table_row(std::string_view line, std::vector<std::string>& out_cells) {
    auto s = trim(line);
    if (s.empty()) return false;
    if (s.find('|') == std::string_view::npos) return false;

    // Optional leading and trailing pipes
    if (s.front() == '|') s.remove_prefix(1);
    if (!s.empty() && s.back() == '|') s.remove_suffix(1);

    out_cells.clear();
    size_t start = 0;
    bool in_code = false;
    bool is_escaped = false;

    for (size_t i = 0; i <= s.size(); ++i) {
        if (i < s.size() && s[i] == '\\') {
            is_escaped = !is_escaped;
            continue;
        }

        if (i < s.size() && s[i] == '`' && !is_escaped) {
            in_code = !in_code;
        } else if ((i == s.size() || (s[i] == '|' && !is_escaped)) && !in_code) {
            auto cell = trim(s.substr(start, i - start));
            // Unescape escaped pipes \| inside cell content
            std::string cell_str;
            cell_str.reserve(cell.size());
            for (size_t k = 0; k < cell.size(); ++k) {
                if (cell[k] == '\\' && k + 1 < cell.size() && cell[k + 1] == '|') {
                    cell_str.push_back('|');
                    k++;
                } else {
                    cell_str.push_back(cell[k]);
                }
            }
            out_cells.emplace_back(std::move(cell_str));
            start = i + 1;
        }
        is_escaped = false;
    }

    return !out_cells.empty();
}

bool MarkdownParser::parse_table_separator(std::string_view line, std::vector<TableAlign>& out_alignments) {
    std::vector<std::string> cells;
    if (!parse_table_row(line, cells) || cells.empty()) return false;

    out_alignments.clear();
    for (const auto& cell_str : cells) {
        auto c = trim(cell_str);
        if (c.empty()) return false;

        bool left_colon = (c.front() == ':');
        bool right_colon = (c.back() == ':');

        if (left_colon) c.remove_prefix(1);
        if (right_colon && !c.empty()) c.remove_suffix(1);

        c = trim(c);
        if (c.empty()) return false;

        for (char ch : c) {
            if (ch != '-') return false;
        }

        if (left_colon && right_colon) {
            out_alignments.push_back(TableAlign::Center);
        } else if (left_colon) {
            out_alignments.push_back(TableAlign::Left);
        } else if (right_colon) {
            out_alignments.push_back(TableAlign::Right);
        } else {
            out_alignments.push_back(TableAlign::None);
        }
    }

    return true;
}

std::vector<InlineSpan> MarkdownParser::parse_inlines(
    std::string_view text,
    const std::unordered_map<std::string, std::pair<std::string, std::string>>* ref_map
) {
    std::vector<InlineSpan> spans;
    size_t i = 0;
    size_t len = text.size();
    size_t text_start = 0;

    auto flush_text = [&](size_t end) {
        if (end > text_start) {
            auto raw_segment = text.substr(text_start, end - text_start);
            // 1. Emoji shortcode expansion
            std::string processed = replace_emojis(raw_segment);

            // 2. Extended GFM Autolink detection (https://, http://, www., email)
            size_t seg_i = 0;
            size_t seg_start = 0;
            while (seg_i < processed.size()) {
                bool is_boundary = (seg_i == 0 || std::isspace(static_cast<unsigned char>(processed[seg_i - 1])) || processed[seg_i - 1] == '(');

                // A. https:// or http://
                if (is_boundary && (processed.substr(seg_i).starts_with("https://") || processed.substr(seg_i).starts_with("http://"))) {
                    if (seg_i > seg_start) {
                        spans.push_back(InlineSpan::make_text(processed.substr(seg_start, seg_i - seg_start)));
                    }
                    size_t url_end = seg_i;
                    while (url_end < processed.size() && !std::isspace(static_cast<unsigned char>(processed[url_end])) &&
                           processed[url_end] != ')' && processed[url_end] != '>' && processed[url_end] != ']') {
                        url_end++;
                    }
                    while (url_end > seg_i && (processed[url_end - 1] == '.' || processed[url_end - 1] == ',' || processed[url_end - 1] == ';')) {
                        url_end--;
                    }
                    auto url_str = processed.substr(seg_i, url_end - seg_i);
                    spans.push_back(InlineSpan::make_link(url_str, url_str));
                    seg_i = url_end;
                    seg_start = seg_i;
                    continue;
                }

                // B. www. autolink (prepends http://)
                if (is_boundary && processed.substr(seg_i).starts_with("www.")) {
                    if (seg_i > seg_start) {
                        spans.push_back(InlineSpan::make_text(processed.substr(seg_start, seg_i - seg_start)));
                    }
                    size_t url_end = seg_i;
                    while (url_end < processed.size() && !std::isspace(static_cast<unsigned char>(processed[url_end])) &&
                           processed[url_end] != ')' && processed[url_end] != '>' && processed[url_end] != ']') {
                        url_end++;
                    }
                    while (url_end > seg_i && (processed[url_end - 1] == '.' || processed[url_end - 1] == ',' || processed[url_end - 1] == ';')) {
                        url_end--;
                    }
                    auto domain_str = processed.substr(seg_i, url_end - seg_i);
                    spans.push_back(InlineSpan::make_link(domain_str, "http://" + domain_str));
                    seg_i = url_end;
                    seg_start = seg_i;
                    continue;
                }

                seg_i++;
            }

            if (seg_start < processed.size()) {
                spans.push_back(InlineSpan::make_text(processed.substr(seg_start)));
            }
        }
        text_start = end;
    };

    while (i < len) {
        char c = text[i];

        // 1. Escaped characters
        if (c == '\\' && i + 1 < len) {
            flush_text(i);
            spans.push_back(InlineSpan::make_text(std::string(1, text[i + 1])));
            i += 2;
            text_start = i;
            continue;
        }

        // 2. HTML Tags: <kbd>, <sub>, <sup>, <br>, <b>, <i>, <u>, <mark>, <span>, etc.
        if (c == '<' && HtmlParser::is_tag_at(text, i)) {
            auto tag_opt = HtmlParser::parse_tag(text, i);
            if (tag_opt && HtmlParser::is_safe_tag(tag_opt->name)) {
                const auto& tag = *tag_opt;

                // A. HTML Comments: <!-- ... -->
                if (tag.name == "!--") {
                    flush_text(i);
                    i += tag.raw_length;
                    text_start = i;
                    continue;
                }

                // B. Self-closing / void tag: <br> or <br/>
                if (tag.name == "br") {
                    flush_text(i);
                    spans.push_back(InlineSpan::make_linebreak());
                    i += tag.raw_length;
                    text_start = i;
                    continue;
                }

                // C. Closing tags (e.g. </kbd>, </span>) -> ignore or skip
                if (tag.is_closing) {
                    flush_text(i);
                    i += tag.raw_length;
                    text_start = i;
                    continue;
                }

                // D. Container tags with matching </tag>: <kbd>, <sub>, <sup>, <mark>, <u>, <b>, <i>, <s>, <code>, <span>, <a>
                std::string close_tag = "</" + tag.name + ">";
                size_t close_pos = text.find(close_tag, i + tag.raw_length);
                if (close_pos != std::string_view::npos) {
                    flush_text(i);
                    auto inner_content = text.substr(i + tag.raw_length, close_pos - (i + tag.raw_length));

                    if (tag.name == "kbd") {
                        spans.push_back(InlineSpan::make_kbd(std::string(inner_content)));
                    } else if (tag.name == "sub") {
                        spans.push_back(InlineSpan::make_subscript(std::string(inner_content)));
                    } else if (tag.name == "sup") {
                        spans.push_back(InlineSpan::make_superscript(std::string(inner_content)));
                    } else if (tag.name == "mark") {
                        spans.push_back(InlineSpan::make_highlight(std::string(inner_content)));
                    } else if (tag.name == "u" || tag.name == "ins") {
                        spans.push_back(InlineSpan::make_underline(std::string(inner_content)));
                    } else if (tag.name == "b" || tag.name == "strong") {
                        spans.push_back(InlineSpan::make_bold(std::string(inner_content)));
                    } else if (tag.name == "i" || tag.name == "em") {
                        spans.push_back(InlineSpan::make_italic(std::string(inner_content)));
                    } else if (tag.name == "s" || tag.name == "del" || tag.name == "strike") {
                        spans.push_back(InlineSpan::make_strike(std::string(inner_content)));
                    } else if (tag.name == "code") {
                        spans.push_back(InlineSpan::make_code(std::string(inner_content)));
                    } else if (tag.name == "a") {
                        std::string href = tag.get_attr("href");
                        std::string title = tag.get_attr("title");
                        spans.push_back(InlineSpan::make_link(std::string(inner_content), href, title));
                    } else if (tag.name == "span" || tag.name == "font") {
                        std::optional<Color> text_col = std::nullopt;
                        std::optional<Color> bg_col = std::nullopt;

                        std::string style_val = tag.get_attr("style");
                        if (!style_val.empty()) {
                            auto styles = HtmlParser::parse_style(style_val);
                            if (styles.count("color")) text_col = HtmlParser::parse_color(styles["color"]);
                            if (styles.count("background-color")) bg_col = HtmlParser::parse_color(styles["background-color"]);
                            if (styles.count("background")) bg_col = HtmlParser::parse_color(styles["background"]);
                        }
                        std::string col_val = tag.get_attr("color");
                        if (!col_val.empty()) text_col = HtmlParser::parse_color(col_val);

                        auto inner_spans = parse_inlines(inner_content, ref_map);
                        for (auto& s : inner_spans) {
                            if (text_col) s.text_color = text_col;
                            if (bg_col) s.bg_color = bg_col;
                            spans.push_back(std::move(s));
                        }
                    }

                    i = close_pos + close_tag.size();
                    text_start = i;
                    continue;
                }

                // E. Standalone <img src="..." alt="...">
                if (tag.name == "img") {
                    flush_text(i);
                    std::string src = tag.get_attr("src");
                    std::string alt = tag.get_attr("alt");
                    std::string title = tag.get_attr("title");
                    spans.push_back(InlineSpan::make_image(alt, src, title));
                    i += tag.raw_length;
                    text_start = i;
                    continue;
                }
            }
        }

        // 3. Inline Code: `code`
        if (c == '`') {
            size_t code_end = text.find('`', i + 1);
            if (code_end != std::string_view::npos) {
                flush_text(i);
                auto code_str = text.substr(i + 1, code_end - i - 1);
                spans.push_back(InlineSpan::make_code(std::string(code_str)));
                i = code_end + 1;
                text_start = i;
                continue;
            }
        }

        // 4. Images: ![alt](url)
        if (c == '!' && i + 1 < len && text[i + 1] == '[') {
            size_t alt_end = text.find(']', i + 2);
            if (alt_end != std::string_view::npos && alt_end + 1 < len && text[alt_end + 1] == '(') {
                size_t url_end = text.find(')', alt_end + 2);
                if (url_end != std::string_view::npos) {
                    flush_text(i);
                    auto alt = text.substr(i + 2, alt_end - i - 2);
                    auto url = text.substr(alt_end + 2, url_end - alt_end - 2);
                    spans.push_back(InlineSpan::make_image(std::string(alt), std::string(url)));
                    i = url_end + 1;
                    text_start = i;
                    continue;
                }
            }
        }

        // 5. Footnote Reference: [^1] or [^label]
        if (c == '[' && i + 2 < len && text[i + 1] == '^') {
            size_t fn_end = text.find(']', i + 2);
            if (fn_end != std::string_view::npos) {
                flush_text(i);
                auto label = text.substr(i + 2, fn_end - i - 2);
                int idx = 1;
                if (!label.empty() && std::isdigit(static_cast<unsigned char>(label[0]))) {
                    idx = std::atoi(std::string(label).c_str());
                }
                auto fn_span = InlineSpan::make_footnote_ref(idx, std::string(label));
                fn_span.target = "#fn-" + std::string(label);
                spans.push_back(std::move(fn_span));
                i = fn_end + 1;
                text_start = i;
                continue;
            }
        }

        // 6. Links: [text](url), [text][ref], or shortcut [ref]
        if (c == '[') {
            size_t link_text_end = text.find(']', i + 1);
            if (link_text_end != std::string_view::npos) {
                auto link_text = text.substr(i + 1, link_text_end - i - 1);

                // A. Direct URL link: [text](url)
                if (link_text_end + 1 < len && text[link_text_end + 1] == '(') {
                    size_t url_end = text.find(')', link_text_end + 2);
                    if (url_end != std::string_view::npos) {
                        flush_text(i);
                        auto url = text.substr(link_text_end + 2, url_end - link_text_end - 2);
                        auto link_span = InlineSpan::make_link(std::string(link_text), std::string(url));
                        link_span.children = parse_inlines(link_text, ref_map);
                        spans.push_back(std::move(link_span));
                        i = url_end + 1;
                        text_start = i;
                        continue;
                    }
                }

                // B. Reference link: [text][ref]
                if (link_text_end + 1 < len && text[link_text_end + 1] == '[') {
                    size_t ref_end = text.find(']', link_text_end + 2);
                    if (ref_end != std::string_view::npos) {
                        auto ref_id = text.substr(link_text_end + 2, ref_end - link_text_end - 2);
                        if (ref_id.empty()) ref_id = link_text;
                        std::string id_lower = to_lower(ref_id);

                        if (ref_map && ref_map->count(id_lower)) {
                            flush_text(i);
                            const auto& [url, title] = ref_map->at(id_lower);
                            auto link_span = InlineSpan::make_link(std::string(link_text), url, title);
                            link_span.children = parse_inlines(link_text, ref_map);
                            spans.push_back(std::move(link_span));
                            i = ref_end + 1;
                            text_start = i;
                            continue;
                        }
                    }
                }

                // C. Shortcut reference link: [id]
                std::string text_lower = to_lower(link_text);
                if (ref_map && ref_map->count(text_lower)) {
                    flush_text(i);
                    const auto& [url, title] = ref_map->at(text_lower);
                    auto link_span = InlineSpan::make_link(std::string(link_text), url, title);
                    link_span.children = parse_inlines(link_text, ref_map);
                    spans.push_back(std::move(link_span));
                    i = link_text_end + 1;
                    text_start = i;
                    continue;
                }
            }
        }

        // 7. Strikethrough: ~~deleted~~
        if (c == '~' && i + 1 < len && text[i + 1] == '~') {
            size_t strike_end = text.find("~~", i + 2);
            if (strike_end != std::string_view::npos) {
                flush_text(i);
                auto inner = text.substr(i + 2, strike_end - i - 2);
                auto strike_span = InlineSpan::make_strike(std::string(inner));
                strike_span.children = parse_inlines(inner, ref_map);
                spans.push_back(std::move(strike_span));
                i = strike_end + 2;
                text_start = i;
                continue;
            }
        }

        // 8. Bold & Italic: ***, **, *, ___, __, _
        if (c == '*' || c == '_') {
            if (i + 2 < len && text[i + 1] == c && text[i + 2] == c) {
                std::string delim(3, c);
                size_t end_idx = text.find(delim, i + 3);
                if (end_idx != std::string_view::npos) {
                    flush_text(i);
                    auto inner = text.substr(i + 3, end_idx - i - 3);
                    auto bi_span = InlineSpan::make_bold_italic(std::string(inner));
                    bi_span.children = parse_inlines(inner, ref_map);
                    spans.push_back(std::move(bi_span));
                    i = end_idx + 3;
                    text_start = i;
                    continue;
                }
            }

            if (i + 1 < len && text[i + 1] == c) {
                std::string delim(2, c);
                size_t end_idx = text.find(delim, i + 2);
                if (end_idx != std::string_view::npos) {
                    flush_text(i);
                    auto inner = text.substr(i + 2, end_idx - i - 2);
                    auto bold_span = InlineSpan::make_bold(std::string(inner));
                    bold_span.children = parse_inlines(inner, ref_map);
                    spans.push_back(std::move(bold_span));
                    i = end_idx + 2;
                    text_start = i;
                    continue;
                }
            }

            size_t end_idx = text.find(c, i + 1);
            if (end_idx != std::string_view::npos && end_idx > i + 1) {
                flush_text(i);
                auto inner = text.substr(i + 1, end_idx - i - 1);
                auto italic_span = InlineSpan::make_italic(std::string(inner));
                italic_span.children = parse_inlines(inner, ref_map);
                spans.push_back(std::move(italic_span));
                i = end_idx + 1;
                text_start = i;
                continue;
            }
        }

        i++;
    }

    flush_text(len);

    if (spans.empty() && !text.empty()) {
        spans.push_back(InlineSpan::make_text(std::string(text)));
    }

    return spans;
}

std::shared_ptr<BlockNode> MarkdownParser::parse(std::string_view markdown) {
    auto root = std::make_shared<BlockNode>(BlockType::Document);
    auto lines = split_lines(markdown);

    size_t line_idx = 0;
    size_t num_lines = lines.size();

    // Pass 1: Collect Link Reference Definitions [id]: https://... "Title"
    std::unordered_map<std::string, std::pair<std::string, std::string>> ref_map;
    std::vector<bool> is_ref_def_line(num_lines, false);

    for (size_t l = 0; l < num_lines; ++l) {
        auto trimmed = trim(lines[l]);
        if (trimmed.starts_with("[") && !trimmed.starts_with("[^") && trimmed.find("]:") != std::string_view::npos) {
            size_t close_bracket = trimmed.find("]:");
            std::string ref_id = to_lower(trim(trimmed.substr(1, close_bracket - 1)));
            auto remainder = trim(trimmed.substr(close_bracket + 2));
            if (!remainder.empty()) {
                size_t url_end = 0;
                while (url_end < remainder.size() && !std::isspace(static_cast<unsigned char>(remainder[url_end]))) {
                    url_end++;
                }
                std::string url = std::string(remainder.substr(0, url_end));
                std::string title;
                size_t title_start = remainder.find_first_of("\"'", url_end);
                if (title_start != std::string_view::npos) {
                    char quote = remainder[title_start];
                    size_t title_end = remainder.find(quote, title_start + 1);
                    if (title_end != std::string_view::npos) {
                        title = std::string(remainder.substr(title_start + 1, title_end - title_start - 1));
                    }
                }
                ref_map[ref_id] = {std::move(url), std::move(title)};
                is_ref_def_line[l] = true;
            }
        }
    }

    while (line_idx < num_lines) {
        // Skip link reference definition lines from document body
        if (is_ref_def_line[line_idx]) {
            line_idx++;
            continue;
        }

        auto raw_line = lines[line_idx];
        auto trimmed = trim(raw_line);

        // 1. Blank line -> skip
        if (trimmed.empty()) {
            line_idx++;
            continue;
        }

        // 2. Fenced Code Block: ``` or ~~~
        if (trimmed.starts_with("```") || trimmed.starts_with("~~~")) {
            char fence_char = trimmed[0];
            std::string fence(3, fence_char);
            std::string lang = std::string(trim(trimmed.substr(3)));

            auto code_block = std::make_shared<BlockNode>(BlockType::CodeBlock);
            code_block->language = lang;

            line_idx++;
            while (line_idx < num_lines) {
                auto code_line = lines[line_idx];
                if (trim(code_line).starts_with(fence)) {
                    line_idx++;
                    break;
                }
                code_block->code_lines.push_back(std::string(code_line));
                line_idx++;
            }
            root->children.push_back(std::move(code_block));
            continue;
        }

        // 3. Thematic Break: ---, ***, ___
        if (is_thematic_break(raw_line)) {
            root->children.push_back(std::make_shared<BlockNode>(BlockType::ThematicBreak));
            line_idx++;
            continue;
        }

        // 4. ATX Heading: # .. ######
        std::string_view heading_text;
        int h_level = parse_heading_level(raw_line, heading_text);
        if (h_level > 0) {
            auto h_node = std::make_shared<BlockNode>(BlockType::Heading);
            h_node->heading_level = h_level;
            h_node->anchor_id = slugify(heading_text);
            h_node->inlines = parse_inlines(heading_text, &ref_map);
            root->children.push_back(std::move(h_node));
            line_idx++;
            continue;
        }

        // 5. GitHub Alert Callout (> [!NOTE], > [!TIP], > [!IMPORTANT], > [!WARNING], > [!CAUTION])
        std::string alert_title;
        AlertType alert_type = parse_alert_marker(trimmed, alert_title);
        if (alert_type != AlertType::None) {
            auto alert_node = std::make_shared<BlockNode>(BlockType::Alert);
            alert_node->alert_type = alert_type;
            alert_node->alert_title = alert_title;

            line_idx++; // Skip marker line
            std::string alert_content;

            while (line_idx < num_lines) {
                auto q_line = trim_start(lines[line_idx]);
                if (q_line.starts_with(">")) {
                    q_line.remove_prefix(1);
                    if (!q_line.empty() && q_line.front() == ' ') q_line.remove_prefix(1);
                    alert_content += std::string(q_line);
                    alert_content += '\n';
                    line_idx++;
                } else if (is_blank(lines[line_idx])) {
                    break;
                } else {
                    alert_content += std::string(q_line);
                    alert_content += '\n';
                    line_idx++;
                }
            }

            auto sub_doc = parse(alert_content);
            alert_node->children = std::move(sub_doc->children);
            root->children.push_back(std::move(alert_node));
            continue;
        }

        // 6. BlockQuote: > line
        if (trimmed.starts_with(">")) {
            auto quote_node = std::make_shared<BlockNode>(BlockType::BlockQuote);
            std::string quote_content;

            while (line_idx < num_lines) {
                auto q_line = trim_start(lines[line_idx]);
                if (q_line.starts_with(">")) {
                    q_line.remove_prefix(1);
                    if (!q_line.empty() && q_line.front() == ' ') q_line.remove_prefix(1);
                    quote_content += std::string(q_line);
                    quote_content += '\n';
                    line_idx++;
                } else if (is_blank(lines[line_idx])) {
                    break;
                } else {
                    quote_content += std::string(q_line);
                    quote_content += '\n';
                    line_idx++;
                }
            }

            auto sub_doc = parse(quote_content);
            quote_node->children = std::move(sub_doc->children);
            root->children.push_back(std::move(quote_node));
            continue;
        }

        // 7. Collapsible Details: <details> ... </details>
        if (trimmed.starts_with("<details")) {
            auto details_node = std::make_shared<BlockNode>(BlockType::Details);
            details_node->is_open = trimmed.find("open") != std::string_view::npos;
            details_node->summary_text = "Details";

            line_idx++;
            std::string details_body;

            while (line_idx < num_lines) {
                auto d_line = trim(lines[line_idx]);
                if (d_line.starts_with("</details>")) {
                    line_idx++;
                    break;
                }

                // Check for <summary>...</summary>
                if (d_line.starts_with("<summary>") || d_line.starts_with("<summary ")) {
                    size_t s_start = d_line.find('>') + 1;
                    size_t s_end = d_line.find("</summary>");
                    if (s_end != std::string_view::npos) {
                        details_node->summary_text = std::string(trim(d_line.substr(s_start, s_end - s_start)));
                    } else {
                        details_node->summary_text = std::string(trim(d_line.substr(s_start)));
                    }
                    line_idx++;
                    continue;
                }

                details_body += std::string(lines[line_idx]);
                details_body += '\n';
                line_idx++;
            }

            auto sub_doc = parse(details_body);
            details_node->children = std::move(sub_doc->children);
            root->children.push_back(std::move(details_node));
            continue;
        }

        // 8. Block HTML: <div align="..."> or <p align="...">
        if (trimmed.starts_with("<div") || trimmed.starts_with("<p ") || trimmed.starts_with("<p>")) {
            auto tag_opt = HtmlParser::parse_tag(trimmed, 0);
            if (tag_opt) {
                uint8_t align_val = 0; // Left
                std::string align_attr = tag_opt->get_attr("align");
                if (align_attr == "center") align_val = 1;
                else if (align_attr == "right") align_val = 2;

                std::string end_tag = "</" + tag_opt->name + ">";
                std::string block_body;

                line_idx++;
                while (line_idx < num_lines) {
                    if (trim(lines[line_idx]).starts_with(end_tag)) {
                        line_idx++;
                        break;
                    }
                    block_body += std::string(lines[line_idx]);
                    block_body += '\n';
                    line_idx++;
                }

                auto sub_doc = parse(block_body);
                for (auto& child : sub_doc->children) {
                    if (child) {
                        child->block_align = align_val;
                        root->children.push_back(std::move(child));
                    }
                }
                continue;
            }
        }

        // 9. GFM Tables: Header and separator
        if (line_idx + 1 < num_lines) {
            std::vector<std::string> header_cells;
            std::vector<TableAlign> alignments;

            if (parse_table_row(raw_line, header_cells) &&
                parse_table_separator(lines[line_idx + 1], alignments) &&
                !header_cells.empty() && header_cells.size() == alignments.size()) {

                auto table_node = std::make_shared<BlockNode>(BlockType::Table);
                table_node->table_alignments = alignments;

                // Header row
                TableRow header_row;
                header_row.is_header = true;
                for (size_t c = 0; c < header_cells.size(); ++c) {
                    TableCell cell;
                    cell.align = alignments[c];
                    cell.inlines = parse_inlines(header_cells[c], &ref_map);
                    header_row.cells.push_back(std::move(cell));
                }
                table_node->table_rows.push_back(std::move(header_row));

                line_idx += 2; // Skip header and separator

                // Data rows
                while (line_idx < num_lines) {
                    std::vector<std::string> row_cells;
                    if (!parse_table_row(lines[line_idx], row_cells) || row_cells.empty()) {
                        break;
                    }

                    TableRow data_row;
                    data_row.is_header = false;
                    for (size_t c = 0; c < alignments.size(); ++c) {
                        TableCell cell;
                        cell.align = alignments[c];
                        if (c < row_cells.size()) {
                            cell.inlines = parse_inlines(row_cells[c], &ref_map);
                        }
                        data_row.cells.push_back(std::move(cell));
                    }
                    table_node->table_rows.push_back(std::move(data_row));
                    line_idx++;
                }

                root->children.push_back(std::move(table_node));
                continue;
            }
        }

        // 10. Footnote Definition: [^1]: Content
        if (trimmed.starts_with("[^") && trimmed.find("]:") != std::string_view::npos) {
            size_t close_bracket = trimmed.find("]:");
            auto label = trimmed.substr(2, close_bracket - 2);
            auto content = trim(trimmed.substr(close_bracket + 2));

            auto fn_node = std::make_shared<BlockNode>(BlockType::FootnoteDef);
            fn_node->footnote_label = std::string(label);
            fn_node->footnote_index = std::atoi(std::string(label).c_str());
            fn_node->anchor_id = "fn-" + std::string(label);
            fn_node->inlines = parse_inlines(content, &ref_map);
            fn_node->inlines.push_back(InlineSpan::make_link(" ↩", "#fnref-" + std::string(label)));

            root->children.push_back(std::move(fn_node));
            line_idx++;
            continue;
        }

        // 11. Lists (Unordered & Ordered with Indentation / Multi-Level nesting)
        bool is_ul = (trimmed.starts_with("* ") || trimmed.starts_with("- ") || trimmed.starts_with("+ "));
        bool is_ol = false;
        int ol_start = 1;

        if (!is_ul && std::isdigit(static_cast<unsigned char>(trimmed[0]))) {
            size_t dot_pos = trimmed.find_first_not_of("0123456789");
            if (dot_pos != std::string_view::npos && (trimmed[dot_pos] == '.' || trimmed[dot_pos] == ')') &&
                dot_pos + 1 < trimmed.size() && trimmed[dot_pos + 1] == ' ') {
                is_ol = true;
                ol_start = std::atoi(std::string(trimmed.substr(0, dot_pos)).c_str());
            }
        }

        if (is_ul || is_ol) {
            auto list_node = std::make_shared<BlockNode>(BlockType::List);
            list_node->is_ordered_list = is_ol;
            list_node->list_start_number = ol_start;

            while (line_idx < num_lines) {
                auto line_full = lines[line_idx];
                auto cur_trimmed = trim(line_full);
                if (cur_trimmed.empty()) {
                    // Check if subsequent non-empty line continues this list (loose list)
                    size_t lookahead = line_idx + 1;
                    while (lookahead < num_lines && trim(lines[lookahead]).empty()) {
                        lookahead++;
                    }
                    if (lookahead < num_lines) {
                        auto next_t = trim(lines[lookahead]);
                        bool next_is_ul = (next_t.starts_with("* ") || next_t.starts_with("- ") || next_t.starts_with("+ "));
                        bool next_is_ol = false;
                        if (!next_is_ul && std::isdigit(static_cast<unsigned char>(next_t[0]))) {
                            size_t d = next_t.find_first_not_of("0123456789");
                            if (d != std::string_view::npos && (next_t[d] == '.' || next_t[d] == ')') && d + 1 < next_t.size() && next_t[d + 1] == ' ') {
                                next_is_ol = true;
                            }
                        }
                        if ((is_ul && next_is_ul) || (is_ol && next_is_ol)) {
                            list_node->is_loose_list = true;
                            line_idx = lookahead;
                            continue;
                        }
                    }
                    break;
                }

                // Calculate indentation spaces
                size_t indent_spaces = 0;
                while (indent_spaces < line_full.size() && (line_full[indent_spaces] == ' ' || line_full[indent_spaces] == '\t')) {
                    indent_spaces += (line_full[indent_spaces] == '\t' ? 4 : 1);
                }

                int depth_level = static_cast<int>(indent_spaces / 2);

                bool cur_is_ul = (cur_trimmed.starts_with("* ") || cur_trimmed.starts_with("- ") || cur_trimmed.starts_with("+ "));
                bool cur_is_ol = false;
                size_t marker_len = 2;

                if (!cur_is_ul && std::isdigit(static_cast<unsigned char>(cur_trimmed[0]))) {
                    size_t dot = cur_trimmed.find_first_not_of("0123456789");
                    if (dot != std::string_view::npos && (cur_trimmed[dot] == '.' || cur_trimmed[dot] == ')') &&
                        dot + 1 < cur_trimmed.size() && cur_trimmed[dot + 1] == ' ') {
                        cur_is_ol = true;
                        marker_len = dot + 2;
                    }
                }

                if ((is_ul && !cur_is_ul) || (is_ol && !cur_is_ol)) {
                    break;
                }

                auto item_content = trim(cur_trimmed.substr(marker_len));
                auto item_node = std::make_shared<BlockNode>(BlockType::ListItem);
                item_node->list_level = depth_level;

                // GFM Task list checkbox: [ ] or [x] or [X]
                if (item_content.starts_with("[ ] ") || item_content.starts_with("[x] ") || item_content.starts_with("[X] ")) {
                    item_node->is_task_item = true;
                    item_node->is_task_checked = (item_content[1] == 'x' || item_content[1] == 'X');
                    item_content = trim(item_content.substr(4));
                }

                item_node->inlines = parse_inlines(item_content, &ref_map);
                list_node->children.push_back(std::move(item_node));
                line_idx++;
            }

            root->children.push_back(std::move(list_node));
            continue;
        }

        // 12. Paragraph: Accumulate lines until blank line or block element
        auto para_node = std::make_shared<BlockNode>(BlockType::Paragraph);
        std::string para_text;

        while (line_idx < num_lines) {
            auto p_line = trim(lines[line_idx]);
            if (p_line.empty()) break;

            if (p_line.starts_with("```") || p_line.starts_with("~~~") ||
                is_thematic_break(lines[line_idx]) ||
                parse_heading_level(lines[line_idx], heading_text) > 0 ||
                p_line.starts_with(">") ||
                p_line.starts_with("<details") ||
                p_line.starts_with("<div") ||
                p_line.starts_with("* ") || p_line.starts_with("- ") || p_line.starts_with("+ ")) {
                break;
            }

            if (!para_text.empty()) {
                para_text += ' ';
            }
            para_text += std::string(p_line);
            line_idx++;
        }

        para_node->inlines = parse_inlines(para_text, &ref_map);
        root->children.push_back(std::move(para_node));
    }

    return root;
}

} // namespace nisaba::markdown
