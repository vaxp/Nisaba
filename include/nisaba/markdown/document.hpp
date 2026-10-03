#pragma once

#include <string_view>
#include <memory>
#include <string>
#include <functional>
#include "nisaba/markdown/types.hpp"
#include "nisaba/markdown/style.hpp"
#include "nisaba/text/font_system.hpp"
#include "nisaba/text/glyph_cache.hpp"

namespace nisaba::markdown {

/// Represents a parsed Markdown document tree with layout and measurement capabilities.
class MarkdownDocument {
public:
    MarkdownDocument();
    explicit MarkdownDocument(std::shared_ptr<BlockNode> root);

    /// Parses markdown from a UTF-8 string.
    static MarkdownDocument from_string(std::string_view markdown_text);

    /// Parses markdown from a file on disk.
    static MarkdownDocument from_file(std::string_view file_path);

    [[nodiscard]] bool is_valid() const noexcept { return root_ != nullptr; }
    [[nodiscard]] const std::shared_ptr<BlockNode>& root() const noexcept { return root_; }

    /// Computes layout positions and total document height given available width and styling.
    float layout(
        float max_width,
        const MarkdownStyle& style,
        text::FontSystem& font_system,
        text::GlyphCache& cache
    );

    [[nodiscard]] float total_height() const noexcept { return total_height_; }
    [[nodiscard]] float layout_width() const noexcept { return layout_width_; }

    /// Performs interactive hit testing at document coordinate (x, y).
    /// Detects clicks on hyperlinks, task checkboxes, collapsible details toggles, footnotes, and images.
    [[nodiscard]] HitTestResult hit_test(float x, float y) const;

    /// Extracts the Table of Contents as pairs of (Heading Title, Anchor ID).
    [[nodiscard]] std::vector<std::pair<std::string, std::string>> table_of_contents() const;

    /// Toggles the expanded/collapsed state of a <details> node.
    bool toggle_details(void* node_ref);

    /// Toggles the checked state of a task list item.
    bool toggle_task_item(void* node_ref);

    /// Cursor type enumeration for interactive hover state.
    enum class CursorType : uint8_t {
        Default,
        Pointer, // Hand cursor (over links, checkboxes, details summaries)
        Text     // I-beam cursor (over text content)
    };

    /// Handles mouse click event at document coordinates (x, y).
    /// Returns true if document state changed (e.g. details toggled or checkbox toggled), requiring relayout/rerender.
    /// If clicked on a hyperlink or footnote, on_link_click(target) is called.
    bool handle_click(
        float x,
        float y,
        const std::function<void(const std::string& target)>& on_link_click = nullptr
    );

    /// Returns recommended cursor type for the mouse position (x, y).
    [[nodiscard]] CursorType get_cursor_at(float x, float y) const;

private:
    std::shared_ptr<BlockNode> root_{nullptr};
    float total_height_{0.0f};
    float layout_width_{0.0f};
};

} // namespace nisaba::markdown
