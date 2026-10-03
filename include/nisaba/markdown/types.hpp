#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <memory>
#include <optional>
#include <cstdint>
#include "nisaba/color/color.hpp"
#include "nisaba/math/rect.hpp"

namespace nisaba::markdown {

/// Alignment of table columns in GitHub Flavored Markdown (GFM).
enum class TableAlign : uint8_t {
    None,
    Left,
    Center,
    Right
};

/// Type of inline text span.
enum class InlineType : uint8_t {
    Text,
    Bold,
    Italic,
    BoldItalic,
    Strikethrough,
    CodeSpan,
    Link,
    Image,
    Kbd,          // <kbd> tag (keyboard shortcut keycap)
    Subscript,    // <sub> tag
    Superscript,  // <sup> tag
    Underline,    // <u> or <ins> tag
    Highlight,    // <mark> tag
    Emoji,        // GitHub Emoji (:smile:, :rocket:, etc.)
    FootnoteRef,  // Footnote reference [^1]
    LineBreak     // Hard line break (<br> or two trailing spaces)
};

/// Inline formatting span within a paragraph, heading, or table cell.
struct InlineSpan {
    InlineType type{InlineType::Text};
    std::string text{};
    std::string target{}; // URL or image source path
    std::string title{};  // Optional hover title
    std::vector<InlineSpan> children{};

    // Extended styling overrides (e.g. from <span style="color: ..."> or <mark>)
    std::optional<Color> text_color{std::nullopt};
    std::optional<Color> bg_color{std::nullopt};
    int footnote_index{0};

    // Cached layout bounding box for interactive hit testing
    Rect layout_bounds{};

    InlineSpan() = default;
    explicit InlineSpan(InlineType t, std::string txt = {}, std::string tgt = {})
        : type(t), text(std::move(txt)), target(std::move(tgt)) {}

    static InlineSpan make_text(std::string text) {
        return InlineSpan(InlineType::Text, std::move(text));
    }
    static InlineSpan make_bold(std::string text) {
        return InlineSpan(InlineType::Bold, std::move(text));
    }
    static InlineSpan make_italic(std::string text) {
        return InlineSpan(InlineType::Italic, std::move(text));
    }
    static InlineSpan make_bold_italic(std::string text) {
        return InlineSpan(InlineType::BoldItalic, std::move(text));
    }
    static InlineSpan make_strike(std::string text) {
        return InlineSpan(InlineType::Strikethrough, std::move(text));
    }
    static InlineSpan make_code(std::string text) {
        return InlineSpan(InlineType::CodeSpan, std::move(text));
    }
    static InlineSpan make_link(std::string text, std::string url, std::string title = {}) {
        InlineSpan s(InlineType::Link, std::move(text), std::move(url));
        s.title = std::move(title);
        return s;
    }
    static InlineSpan make_image(std::string alt, std::string src, std::string title = {}) {
        InlineSpan s(InlineType::Image, std::move(alt), std::move(src));
        s.title = std::move(title);
        return s;
    }
    static InlineSpan make_kbd(std::string text) {
        return InlineSpan(InlineType::Kbd, std::move(text));
    }
    static InlineSpan make_subscript(std::string text) {
        return InlineSpan(InlineType::Subscript, std::move(text));
    }
    static InlineSpan make_superscript(std::string text) {
        return InlineSpan(InlineType::Superscript, std::move(text));
    }
    static InlineSpan make_underline(std::string text) {
        return InlineSpan(InlineType::Underline, std::move(text));
    }
    static InlineSpan make_highlight(std::string text) {
        return InlineSpan(InlineType::Highlight, std::move(text));
    }
    static InlineSpan make_emoji(std::string emoji_utf8, std::string shortcode) {
        return InlineSpan(InlineType::Emoji, std::move(emoji_utf8), std::move(shortcode));
    }
    static InlineSpan make_footnote_ref(int index, std::string label) {
        InlineSpan s(InlineType::FootnoteRef, std::move(label));
        s.footnote_index = index;
        return s;
    }
    static InlineSpan make_linebreak() {
        return InlineSpan(InlineType::LineBreak, "\n");
    }
};

/// GitHub Callout Alert types: > [!NOTE], > [!TIP], > [!IMPORTANT], > [!WARNING], > [!CAUTION]
enum class AlertType : uint8_t {
    None,
    Note,      // Blue callout (Information)
    Tip,       // Green callout (Helpful advice)
    Important, // Purple callout (Crucial context)
    Warning,   // Amber / Yellow callout (Cautionary alert)
    Caution    // Red callout (Negative consequences / danger)
};

/// Type of block-level markdown element.
enum class BlockType : uint8_t {
    Document,
    Heading,
    Paragraph,
    BlockQuote,
    CodeBlock,
    List,
    ListItem,
    Table,
    ThematicBreak,
    Alert,       // GitHub callout alert block
    Details,     // <details><summary>...</summary>...</details>
    HtmlBlock,   // Block-level HTML container (e.g. <div align="...">)
    FootnoteDef  // Footnote definition block: [^1]: Content
};

/// Single cell in a markdown table.
struct TableCell {
    std::vector<InlineSpan> inlines{};
    TableAlign align{TableAlign::None};
};

/// Row of cells in a markdown table.
struct TableRow {
    std::vector<TableCell> cells{};
    bool is_header{false};
    float layout_height{0.0f};
};

/// AST Block Node representing a structural markdown element.
struct BlockNode {
    BlockType type{BlockType::Paragraph};
    int heading_level{1};                 // 1..6 for Heading
    std::string anchor_id{};              // GitHub auto-slug anchor id (e.g. "my-heading")
    std::string language{};               // Language info for CodeBlock (e.g. "cpp", "json")
    std::vector<std::string> code_lines{};// Raw lines for CodeBlock
    bool is_ordered_list{false};          // For List
    int list_start_number{1};             // For List
    int list_level{0};                    // Nesting depth for lists (0, 1, 2...)
    bool is_task_item{false};             // For ListItem [ ] or [x]
    bool is_task_checked{false};          // For ListItem
    bool is_loose_list{false};            // For List (loose vs tight spacing)

    // GitHub Alert Callout fields
    AlertType alert_type{AlertType::None};
    std::string alert_title{};            // Default: "Note", "Tip", etc.

    // Collapsible Details / Summary fields
    std::string summary_text{};
    bool is_open{true};
    std::vector<std::shared_ptr<BlockNode>> summary_children{};

    // HTML / Text alignment
    uint8_t block_align{0}; // 0 = Left, 1 = Center, 2 = Right

    // Footnotes
    int footnote_index{0};
    std::string footnote_label{};

    std::vector<InlineSpan> inlines{};    // Inline content for Paragraph, Heading, ListItem
    std::vector<std::shared_ptr<BlockNode>> children{}; // Sub-blocks for Document, BlockQuote, List, ListItem, Details, Alert

    // Table data
    std::vector<TableAlign> table_alignments{};
    std::vector<TableRow> table_rows{};

    // Layout metrics (computed during layout pass)
    float layout_x{0.0f};
    float layout_y{0.0f};
    float layout_width{0.0f};
    float layout_height{0.0f};

    BlockNode() = default;
    explicit BlockNode(BlockType t) : type(t) {}
};

/// Type of interactive hit-test target.
enum class HitTargetType : uint8_t {
    None,
    Link,
    TaskCheckbox,
    DetailsToggle,
    Footnote,
    Image,
    HeadingAnchor
};

/// Result of interactive spatial hit testing on a Markdown document.
struct HitTestResult {
    HitTargetType type{HitTargetType::None};
    std::string target{};  // URL for link, anchor id for heading, footnote ref
    std::string title{};   // Hover tooltip text or alt text
    void* node_ref{nullptr}; // Pointer to BlockNode or related element
    Rect bounds{};         // Bounding rectangle in document coordinates
    int index{0};          // Index (e.g. task checkbox index or footnote number)
    bool is_checked{false};// Current state for task checkbox or details
};

} // namespace nisaba::markdown
