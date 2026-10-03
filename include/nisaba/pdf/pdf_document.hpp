#pragma once

#include <string>
#include <vector>
#include <memory>
#include <span>
#include "nisaba/pdf/pdf_canvas.hpp"
#include "nisaba/pdf/pdf_writer.hpp"
#include "nisaba/markdown/document.hpp"
#include "nisaba/markdown/style.hpp"
#include "nisaba/text/font_system.hpp"
#include "nisaba/text/glyph_cache.hpp"

namespace nisaba::pdf {

enum class PageSize {
    A4,
    Letter,
    A3,
    Legal,
    Custom
};

enum class PageOrientation {
    Portrait,
    Landscape
};

class PdfDocument;

/// Represents an individual page inside a multi-page PDF document.
class PdfPage {
public:
    PdfPage(float width, float height);
    ~PdfPage() = default;

    [[nodiscard]] float width() const noexcept { return width_; }
    [[nodiscard]] float height() const noexcept { return height_; }
    [[nodiscard]] PdfCanvas& canvas() noexcept { return canvas_; }
    [[nodiscard]] const PdfCanvas& canvas() const noexcept { return canvas_; }

private:
    friend class PdfDocument;
    float width_;
    float height_;
    PdfCanvas canvas_;
};

/// High-level sovereign multi-page PDF document generator.
/// Fully self-contained, ISO 32000-1 compliant, with zero third-party dependencies.
class PdfDocument {
public:
    PdfDocument();
    ~PdfDocument() = default;

    /// Adds a new page with custom width and height (in PDF points, 72 pt = 1 inch).
    PdfPage* add_page(float width = 595.28f, float height = 841.89f);

    /// Adds a new page using standard paper sizes (A4, Letter, A3, Legal).
    PdfPage* add_page(PageSize size, PageOrientation orientation = PageOrientation::Portrait);

    /// Number of pages in the document.
    [[nodiscard]] size_t page_count() const noexcept { return pages_.size(); }

    /// Access page by 0-based index.
    [[nodiscard]] PdfPage* page(size_t index);
    [[nodiscard]] const PdfPage* page(size_t index) const;

    // --- Document Metadata ---
    void set_title(std::string_view title) { title_ = std::string(title); }
    void set_author(std::string_view author) { author_ = std::string(author); }
    void set_subject(std::string_view subject) { subject_ = std::string(subject); }
    void set_creator(std::string_view creator) { creator_ = std::string(creator); }

    [[nodiscard]] const std::string& title() const noexcept { return title_; }
    [[nodiscard]] const std::string& author() const noexcept { return author_; }

    /// Assembles the complete PDF binary data.
    [[nodiscard]] std::vector<uint8_t> save_to_bytes();

    /// Writes the PDF directly to a file on disk.
    bool save_to_file(const std::string& path);

    /// Exports a Markdown document directly into this multi-page PDF document
    /// with automatic pagination, flow layout, and rich vector styling.
    bool export_markdown(
        markdown::MarkdownDocument& doc,
        const markdown::MarkdownStyle& style,
        text::FontSystem& font_system,
        text::GlyphCache& glyph_cache,
        float page_width = 595.28f,
        float page_height = 841.89f,
        float margin = 40.0f
    );

private:
    std::vector<std::unique_ptr<PdfPage>> pages_{};
    std::string title_{"Nisaba Document"};
    std::string author_{"Nisaba Sovereign Engine"};
    std::string subject_{""};
    std::string creator_{"Nisaba 2D Graphics Engine"};
};

} // namespace nisaba::pdf
