#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include "nisaba/markdown/document.hpp"
#include "nisaba/markdown/style.hpp"
#include "nisaba/pdf/pdf_document.hpp"
#include "nisaba/text/font_system.hpp"
#include "nisaba/text/glyph_cache.hpp"

namespace nisaba::markdown {

/// Configuration options for vector PDF export.
struct MarkdownPdfExportOptions {
    pdf::PageSize page_size{pdf::PageSize::A4};
    pdf::PageOrientation orientation{pdf::PageOrientation::Portrait};
    float page_width{595.28f};   // Standard A4 width in pt (72 pt/in)
    float page_height{841.89f};  // Standard A4 height in pt
    float margin_left{42.0f};
    float margin_right{42.0f};
    float margin_top{50.0f};
    float margin_bottom{50.0f};

    // Header and Footer options
    bool show_header{true};
    bool show_footer{true};
    bool show_page_numbers{true};
    std::string header_left{"Nisaba Sovereign Engine"};
    std::string header_right{""};
    std::string footer_left{"Generated with Nisaba 2D"};

    // Document Metadata
    std::string doc_title{"Nisaba Markdown Export"};
    std::string doc_author{"Nisaba Sovereign Engine"};
    std::string doc_subject{"GFM Vector Document"};
};

/// High-level sovereign vector PDF exporter for Markdown documents.
/// Translates AST blocks, GitHub callout alerts, dynamic tables, code blocks,
/// multi-level lists, and footnotes into multi-page ISO 32000-1 PDF documents.
class MarkdownPdfExporter {
public:
    /// Exports MarkdownDocument to a complete PDF binary payload.
    static std::vector<uint8_t> export_to_bytes(
        MarkdownDocument& doc,
        const MarkdownStyle& style,
        text::FontSystem& font_system,
        text::GlyphCache& glyph_cache,
        const MarkdownPdfExportOptions& options = {}
    );

    /// Exports MarkdownDocument directly to a file on disk.
    static bool export_to_file(
        MarkdownDocument& doc,
        const std::string& output_path,
        const MarkdownStyle& style,
        text::FontSystem& font_system,
        text::GlyphCache& glyph_cache,
        const MarkdownPdfExportOptions& options = {}
    );
};

} // namespace nisaba::markdown
