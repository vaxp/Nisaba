#pragma once

#include <string_view>
#include <vector>
#include <memory>
#include <unordered_map>
#include "nisaba/markdown/types.hpp"

namespace nisaba::markdown {

/// High-performance CommonMark and GFM parser.
class MarkdownParser {
public:
    MarkdownParser() = default;

    /// Parse markdown text into a root Document BlockNode containing all parsed elements.
    static std::shared_ptr<BlockNode> parse(std::string_view markdown);

    /// Parse a single line or segment of markdown into a sequence of inline spans.
    static std::vector<InlineSpan> parse_inlines(
        std::string_view text,
        const std::unordered_map<std::string, std::pair<std::string, std::string>>* ref_map = nullptr
    );

    /// Generate a GitHub-compatible kebab-case slug for headings (e.g. "My Heading!" -> "my-heading").
    static std::string slugify(std::string_view text);

    /// Replace GitHub emoji shortcodes (e.g. ":smile:", ":rocket:") with UTF-8 emojis.
    static std::string replace_emojis(std::string_view text);

private:
    static bool is_thematic_break(std::string_view line);
    static int parse_heading_level(std::string_view line, std::string_view& out_text);
    static bool parse_table_row(std::string_view line, std::vector<std::string>& out_cells);
    static bool parse_table_separator(std::string_view line, std::vector<TableAlign>& out_alignments);
    static AlertType parse_alert_marker(std::string_view line, std::string& out_title);
};

} // namespace nisaba::markdown
