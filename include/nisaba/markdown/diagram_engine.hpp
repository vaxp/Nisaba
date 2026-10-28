#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <memory>
#include "nisaba/color/color.hpp"
#include "nisaba/canvas/canvas_interface.hpp"
#include "nisaba/markdown/style.hpp"
#include "nisaba/text/font_system.hpp"
#include "nisaba/text/glyph_cache.hpp"

namespace nisaba::markdown {

/// Supported Mermaid diagram families.
enum class DiagramKind : uint8_t {
    Flowchart,
    Sequence,
    State,
    XYChart,
    PieChart
};

/// Flowchart / State graph layout orientation.
enum class DiagramDirection : uint8_t {
    TopDown,     // TD / TB
    LeftRight,   // LR
    BottomTop,   // BT
    RightLeft    // RL
};

/// Visual shape of a node in a Flowchart or State diagram.
enum class DiagramNodeShape : uint8_t {
    Rect,        // [Label]
    RoundRect,   // (Label)
    Stadium,     // ([Label])
    Subroutine,  // [[Label]]
    Diamond,     // {Label}
    Hexagon,     // {{Label}}
    Circle,      // ((Label))
    StartState,  // [*] start
    EndState     // [*] end
};

/// Line & arrowhead style of an edge or sequence message.
enum class DiagramEdgeStyle : uint8_t {
    SolidArrow,  // --> or ->>
    OpenSolid,   // --- or ->
    DottedArrow, // -.-> or -->>
    ThickArrow,  // ==>
    BiDirectional// <-->
};

struct DiagramNode {
    std::string id{};
    std::string label{};
    DiagramNodeShape shape{DiagramNodeShape::RoundRect};
    int rank{0};
    int order{0};
    float x{0.0f};
    float y{0.0f};
    float width{110.0f};
    float height{42.0f};
};

struct DiagramEdge {
    size_t from_idx{0};
    size_t to_idx{0};
    std::string label{};
    DiagramEdgeStyle style{DiagramEdgeStyle::SolidArrow};
};

struct SequenceMessage {
    size_t from_actor{0};
    size_t to_actor{0};
    std::string text{};
    DiagramEdgeStyle style{DiagramEdgeStyle::SolidArrow};
    bool is_note{false};
    float y{0.0f};
};

struct ChartSeries {
    bool is_bar{true}; // true = bar, false = line
    std::string label{};
    std::vector<float> values{};
};

struct PieSlice {
    std::string label{};
    float value{0.0f};
};

/// Parsed and laid-out representation of a Mermaid diagram block.
struct DiagramModel {
    DiagramKind kind{DiagramKind::Flowchart};
    DiagramDirection direction{DiagramDirection::TopDown};
    std::string title{};

    // Flowchart / State graph
    std::vector<DiagramNode> nodes{};
    std::vector<DiagramEdge> edges{};

    // Sequence diagram
    std::vector<DiagramNode> actors{};
    std::vector<SequenceMessage> messages{};

    // XYChart
    std::vector<std::string> x_categories{};
    std::string y_axis_title{};
    float y_min{0.0f};
    float y_max{100.0f};
    bool auto_y_range{true};
    std::vector<ChartSeries> series{};

    // PieChart
    std::vector<PieSlice> pie_slices{};

    // Computed bounding dimensions
    float width{0.0f};
    float height{0.0f};
};

/// Sovereign zero-dependency Mermaid Diagram & Chart parser, layout solver, and vector renderer.
class DiagramEngine {
public:
    /// Parses a Mermaid source code string into a structured DiagramModel.
    static std::shared_ptr<DiagramModel> parse(std::string_view mermaid_code);

    /// Solves node placement, layered graph layout, or chart geometry for the given max width.
    static float layout(
        DiagramModel& model,
        float max_width,
        const MarkdownStyle& style,
        text::FontSystem& font_system
    );

    /// Renders the laid-out DiagramModel onto any polymorphic ICanvas (CPU Canvas, GPU GpuCanvas, or PdfCanvas).
    static void render(
        ICanvas& canvas,
        const DiagramModel& model,
        float x,
        float y,
        float max_width,
        const MarkdownStyle& style,
        text::FontSystem& font_system,
        text::GlyphCache& glyph_cache
    );
};

} // namespace nisaba::markdown
