#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <memory>
#include "nisaba/color/color.hpp"
#include "nisaba/canvas/canvas_interface.hpp"
#include "nisaba/text/font_system.hpp"
#include "nisaba/text/glyph_cache.hpp"

namespace nisaba::markdown {

/// Types of nodes in a parsed LaTeX mathematical expression AST.
enum class MathNodeType : uint8_t {
    Row,          // Horizontal sequence of math nodes
    Symbol,       // Variable, number, operator, or Greek symbol
    Text,         // \text{...}, \mathrm{...}, \mathbf{...}
    Fraction,     // \frac{num}{den}
    Sqrt,         // \sqrt[deg]{radicand}
    Script,       // base_{sub}^{sup} (including \sum, \int, \lim with limits)
    Delimited,    // \left( ... \right)
    Matrix,       // \begin{pmatrix/bmatrix/matrix/vmatrix/cases} ... \end{...}
    Accent,       // \hat, \bar, \vec, \tilde, \dot, \overline, \underline
    Space         // \, \; \quad \qquad
};

/// Font style override inside math mode.
enum class MathFontStyle : uint8_t {
    Normal,       // Math italic for Latin variables, upright for digits/operators
    Roman,        // \mathrm, \text, standard functions (\sin, \cos, \lim)
    Bold,         // \mathbf, \boldsymbol
    Blackboard,   // \mathbb
    Calligraphic, // \mathcal
    Monospace     // \mathtt
};

/// Accent decoration kind above or below a math node.
enum class MathAccentKind : uint8_t {
    Hat,
    Bar,
    Vec,
    Tilde,
    Dot,
    DDot,
    Overline,
    Underline
};

/// Matrix / environment bracket kind.
enum class MathMatrixKind : uint8_t {
    Plain,        // matrix
    Paren,        // pmatrix ( ... )
    Bracket,      // bmatrix [ ... ]
    Brace,        // Bmatrix { ... }
    Vert,         // vmatrix | ... |
    DoubleVert,   // Vmatrix || ... ||
    Cases         // cases   { ...
};

/// AST & Layout Box node for a LaTeX mathematical expression.
struct MathNode {
    MathNodeType type{MathNodeType::Row};
    MathFontStyle font_style{MathFontStyle::Normal};
    std::string text{};

    // Children for Row, Fraction, Sqrt, Script, Delimited, Accent
    std::vector<std::shared_ptr<MathNode>> children{};
    std::shared_ptr<MathNode> numerator{};     // Fraction numerator or Sqrt radicand or Script base or Accent base
    std::shared_ptr<MathNode> denominator{};   // Fraction denominator or Sqrt degree or Script sub
    std::shared_ptr<MathNode> superscript{};   // Script sup

    // Delimiters & Matrix
    std::string left_delim{};
    std::string right_delim{};
    MathMatrixKind matrix_kind{MathMatrixKind::Plain};
    std::vector<std::vector<std::shared_ptr<MathNode>>> matrix_rows{};

    // Accent & Operator flags
    MathAccentKind accent_kind{MathAccentKind::Hat};
    bool is_big_operator{false};               // \sum, \prod, \int, \lim
    bool limits_above_below{false};            // Display-style limits placement
    float space_em{0.0f};                      // Explicit horizontal space in ems

    // Computed layout metrics (in pixels, relative to parent or root origin)
    float width{0.0f};
    float ascent{0.0f};   // Height above mathematical baseline
    float descent{0.0f};  // Depth below mathematical baseline
    float x{0.0f};        // X offset relative to root math box left
    float y{0.0f};        // Baseline Y offset relative to root math box top
    float computed_font_size{16.0f};

    [[nodiscard]] float height() const noexcept { return ascent + descent; }
};

/// Sovereign zero-dependency LaTeX / KaTeX mathematical parser, typesetter, and vector renderer.
class MathEngine {
public:
    /// Parses a LaTeX math string into a hierarchical MathNode AST.
    static std::shared_ptr<MathNode> parse(std::string_view latex, bool display_mode = false);

    /// Computes exact 2D typographic box metrics (width, ascent, descent) and positions all sub-nodes.
    static void layout(
        MathNode& root,
        float base_font_size,
        bool display_mode,
        text::FontSystem& font_system
    );

    /// Renders a laid-out MathNode tree onto any polymorphic ICanvas (CPU Canvas, GPU GpuCanvas, or PdfCanvas)
    /// where (x, y) is the top-left corner of the math box.
    static void render(
        ICanvas& canvas,
        const MathNode& root,
        float x,
        float y,
        Color color,
        text::FontSystem& font_system,
        text::GlyphCache& glyph_cache
    );

    /// Converts a LaTeX math expression into a readable Unicode string (for plain-text extraction / TOC).
    static std::string to_plain_text(const MathNode& node);
};

} // namespace nisaba::markdown
