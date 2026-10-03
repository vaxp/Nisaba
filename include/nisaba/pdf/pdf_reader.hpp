#pragma once

#include <string>
#include <vector>
#include <memory>
#include <span>
#include "nisaba/canvas/canvas.hpp"
#include "nisaba/canvas/canvas_interface.hpp"
#include "nisaba/pdf/pdf_parser.hpp"
#include "nisaba/pdf/pdf_interpreter.hpp"
#include "nisaba/pdf/pdf_navigation.hpp"
#include "nisaba/text/font_system.hpp"

namespace nisaba::gpu {
class GpuCanvas;
}

namespace nisaba::pdf {

/// High-level sovereign PDF Viewer and Document Reader.
/// Parses PDF documents and renders them directly onto Nisaba Canvas / ICanvas.
class PdfReader {
public:
    PdfReader();
    ~PdfReader();

    PdfReader(const PdfReader&) = delete;
    PdfReader& operator=(const PdfReader&) = delete;
    PdfReader(PdfReader&&) noexcept;
    PdfReader& operator=(PdfReader&&) noexcept;

    /// Opens a PDF document from a file path with optional password.
    bool open_from_file(const std::string& path, const std::string& password = "");

    /// Opens a PDF document from an in-memory buffer with optional password.
    bool open_from_memory(std::span<const uint8_t> data, const std::string& password = "");

    /// Authenticates with password (for encrypted documents).
    bool authenticate(const std::string& password);

    /// Checks if the document is encrypted.
    [[nodiscard]] bool is_encrypted() const noexcept;

    /// Checks if the document has been successfully authenticated.
    [[nodiscard]] bool is_authenticated() const noexcept;

    /// Number of pages available.
    [[nodiscard]] size_t page_count() const noexcept;

    /// Returns the bounding box / MediaBox of a page in PDF points.
    [[nodiscard]] Rect page_box(size_t index) const noexcept;

    /// Renders a page onto a software CPU Canvas.
    bool render_page(
        size_t index,
        Canvas& canvas,
        float scale = 1.0f,
        text::FontSystem* font_system = nullptr
    );

    /// Renders a page onto any polymorphic ICanvas (CPU or GPU).
    bool render_page(
        size_t index,
        ICanvas& canvas,
        float scale = 1.0f,
        text::FontSystem* font_system = nullptr
    );

    /// Returns the full interactive outline / bookmarks hierarchy of the document.
    [[nodiscard]] std::vector<PdfOutlineItem> outlines();

    /// Returns all interactive link annotations on a given page.
    [[nodiscard]] std::vector<PdfLinkAnnotation> page_links(size_t page_index);

    /// Resolves a destination to an absolute page index.
    [[nodiscard]] std::optional<size_t> resolve_destination_page(const PdfDestination& dest);

    /// Direct GPU acceleration: renders page directly into GpuCanvas and flushes GPU pipeline.
    bool render_page_gpu(
        size_t index,
        gpu::GpuCanvas& gpu_canvas,
        float scale = 1.0f,
        text::FontSystem* font_system = nullptr
    );

    // Document Information Metadata
    [[nodiscard]] std::string title() const;
    [[nodiscard]] std::string author() const;
    [[nodiscard]] std::string creator() const;
    [[nodiscard]] std::string subject() const;

private:
    std::vector<uint8_t> file_bytes_{};
    PdfParser parser_{};
    std::unique_ptr<PdfInterpreter> interpreter_{};
};

} // namespace nisaba::pdf
