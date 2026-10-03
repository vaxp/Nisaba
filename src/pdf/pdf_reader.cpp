#include "nisaba/pdf/pdf_reader.hpp"
#include "nisaba/gpu/gpu_canvas.hpp"
#include <fstream>

namespace nisaba::pdf {

PdfReader::PdfReader() {
    interpreter_ = std::make_unique<PdfInterpreter>(parser_);
}

PdfReader::~PdfReader() = default;

PdfReader::PdfReader(PdfReader&&) noexcept = default;
PdfReader& PdfReader::operator=(PdfReader&&) noexcept = default;

bool PdfReader::open_from_file(const std::string& path, const std::string& password) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) return false;

    auto size = file.tellg();
    if (size <= 0) return false;

    file.seekg(0, std::ios::beg);
    file_bytes_.resize(static_cast<size_t>(size));
    file.read(reinterpret_cast<char*>(file_bytes_.data()), size);

    return open_from_memory(file_bytes_, password);
}

bool PdfReader::open_from_memory(std::span<const uint8_t> data, const std::string& password) {
    if (!parser_.load(data)) {
        return false;
    }
    if (parser_.is_encrypted()) {
        if (!password.empty()) {
            if (!parser_.authenticate(password)) {
                return false;
            }
        } else if (!parser_.is_authenticated()) {
            return false;
        }
    }
    return true;
}

bool PdfReader::authenticate(const std::string& password) {
    return parser_.authenticate(password);
}

bool PdfReader::is_encrypted() const noexcept {
    return parser_.is_encrypted();
}

bool PdfReader::is_authenticated() const noexcept {
    return parser_.is_authenticated();
}

size_t PdfReader::page_count() const noexcept {
    return parser_.page_count();
}

Rect PdfReader::page_box(size_t index) const noexcept {
    const auto* p = parser_.page(index);
    if (!p) return Rect::from_xywh(0.0f, 0.0f, 595.28f, 841.89f).value_or(Rect());
    return p->media_box;
}

bool PdfReader::render_page(
    size_t index,
    Canvas& canvas,
    float scale,
    text::FontSystem* font_system
) {
    const auto* p = parser_.page(index);
    if (!p) return false;
    return interpreter_->interpret_page(*p, canvas, scale, font_system);
}

bool PdfReader::render_page(
    size_t index,
    ICanvas& canvas,
    float scale,
    text::FontSystem* font_system
) {
    const auto* p = parser_.page(index);
    if (!p) return false;
    return interpreter_->interpret_page(*p, canvas, scale, font_system);
}

std::string PdfReader::title() const {
    return parser_.info_field("Title");
}

std::string PdfReader::author() const {
    return parser_.info_field("Author");
}

std::string PdfReader::creator() const {
    return parser_.info_field("Creator");
}

std::string PdfReader::subject() const {
    return parser_.info_field("Subject");
}

std::vector<PdfOutlineItem> PdfReader::outlines() {
    return parser_.read_outlines();
}

std::vector<PdfLinkAnnotation> PdfReader::page_links(size_t page_index) {
    return parser_.read_page_links(page_index);
}

std::optional<size_t> PdfReader::resolve_destination_page(const PdfDestination& dest) {
    if (dest.page_index >= 0 && static_cast<size_t>(dest.page_index) < page_count()) {
        return static_cast<size_t>(dest.page_index);
    }
    if (!dest.named_dest.empty()) {
        auto resolved = parser_.resolve_named_destination(dest.named_dest);
        if (resolved && resolved->page_index >= 0 && static_cast<size_t>(resolved->page_index) < page_count()) {
            return static_cast<size_t>(resolved->page_index);
        }
    }
    return std::nullopt;
}

bool PdfReader::render_page_gpu(
    size_t index,
    gpu::GpuCanvas& gpu_canvas,
    float scale,
    text::FontSystem* font_system
) {
    bool ok = render_page(index, static_cast<ICanvas&>(gpu_canvas), scale, font_system);
    if (ok) {
        gpu_canvas.flush();
    }
    return ok;
}

} // namespace nisaba::pdf
