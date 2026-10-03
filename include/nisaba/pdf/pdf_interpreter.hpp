#pragma once

#include <span>
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include "nisaba/canvas/canvas.hpp"
#include "nisaba/canvas/canvas_interface.hpp"
#include "nisaba/canvas/paint.hpp"
#include "nisaba/shaders/shader.hpp"
#include "nisaba/mesh/mesh.hpp"
#include "nisaba/pdf/pdf_parser.hpp"
#include "nisaba/math/transform.hpp"
#include "nisaba/path/path_builder.hpp"
#include "nisaba/path/stroker.hpp"
#include "nisaba/text/font_system.hpp"
#include "nisaba/pdf/pdf_cmap.hpp"

namespace nisaba::pdf {

/// State of the PDF graphics engine during content stream execution.
struct PdfGraphicsState {
    Transform ctm{};
    Color fill_color{Color::BLACK};
    Color stroke_color{Color::BLACK};
    BlendMode blend_mode{BlendMode::SourceOver};
    float fill_alpha{1.0f};
    float stroke_alpha{1.0f};
    bool alpha_is_shape{false};
    std::optional<Paint> fill_paint{std::nullopt};
    std::optional<Paint> stroke_paint{std::nullopt};
    std::string fill_color_space{"DeviceRGB"};
    std::string stroke_color_space{"DeviceRGB"};
    PdfValue fill_cs_val{};
    PdfValue stroke_cs_val{};
    Stroke stroke{1.0f};

    // Text state
    Transform text_matrix{};
    Transform line_matrix{};
    std::string font_name{"Helvetica"};
    float font_size{12.0f};
    float text_leading{14.0f};
    std::shared_ptr<PdfCMap> current_cmap{nullptr};
    bool clip_pending{false};
    FillRule clip_rule{FillRule::Winding};
};

/// High-performance sovereign PDF Content Stream Interpreter.
/// Executes vector path operators, text, and images onto Nisaba Canvas / ICanvas.
class PdfInterpreter {
public:
    PdfInterpreter(PdfParser& parser);
    ~PdfInterpreter() = default;

    /// Renders a parsed page onto a software Canvas.
    bool interpret_page(
        const ParsedPdfPage& page,
        Canvas& canvas,
        float scale = 1.0f,
        text::FontSystem* font_system = nullptr
    );

    /// Renders a parsed page onto any polymorphic ICanvas.
    bool interpret_page(
        const ParsedPdfPage& page,
        ICanvas& canvas,
        float scale = 1.0f,
        text::FontSystem* font_system = nullptr
    );

private:
    std::shared_ptr<PdfCMap> find_or_load_cmap(
        const PdfDict& resources,
        const std::string& font_name,
        text::FontSystem* font_system = nullptr
    );

    void execute_stream(
        std::span<const uint8_t> stream,
        const PdfDict& resources,
        ICanvas& canvas,
        float page_height,
        float scale,
        Canvas* cpu_canvas,
        text::FontSystem* font_system,
        int depth = 0,
        const Transform& initial_ctm = Transform()
    );

    std::vector<GradientStop> extract_gradient_stops(
        const PdfValue& func_val,
        const std::string& color_space = "DeviceRGB"
    );

    void execute_shading(
        const std::string& sh_name,
        const PdfDict& resources,
        ICanvas& canvas,
        float page_height,
        float scale,
        Canvas* cpu_canvas,
        const PdfGraphicsState& state
    );

    std::optional<Paint> resolve_pattern_paint(
        const std::string& pat_name,
        const PdfDict& resources,
        float page_height,
        float scale,
        Canvas* cpu_canvas,
        const PdfGraphicsState& state
    );

    PdfValue resolve_cs_from_resources(
        const std::string& cs_name,
        const PdfDict& resources
    );

    PdfParser& parser_;
    std::map<PdfRef, std::shared_ptr<PdfCMap>> cmap_cache_;
    std::map<std::string, std::shared_ptr<Pixmap>> pattern_cache_;
    std::map<PdfRef, std::shared_ptr<Pixmap>> image_cache_;
    std::unordered_map<uint64_t, std::shared_ptr<Pixmap>> image_hash_cache_;
};

} // namespace nisaba::pdf
