#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>
#include <optional>
#include <span>
#include "nisaba/math/point.hpp"
#include "nisaba/canvas/canvas.hpp"
#include "nisaba/text/attrs.hpp"
#include "nisaba/text/layout.hpp"
#include "nisaba/text/font_system.hpp"
#include "nisaba/text/glyph_cache.hpp"

namespace nisaba::text {

/// Represents a single paragraph line of text with its attributes and laid out visual lines.
class BufferLine {
public:
    BufferLine(std::string_view line_text, const Attrs& default_attrs);

    std::string_view text() const noexcept { return text_; }
    const AttrsList& attrs_list() const noexcept { return attrs_list_; }
    AttrsList& attrs_list_mut() noexcept { return attrs_list_; }

    const std::vector<LayoutLine>& layout() const noexcept { return layout_lines_; }
    std::vector<LayoutLine>& layout_mut() noexcept { return layout_lines_; }
    bool is_shaped() const noexcept { return is_shaped_; }

    void reset_shaping() noexcept { is_shaped_ = false; layout_lines_.clear(); }

    /// Shapes and lays out this line into visual lines according to wrap and alignment.
    void shape_and_layout(
        FontSystem& font_system,
        float font_size,
        float line_height,
        std::optional<float> wrap_width,
        Wrap wrap,
        Align align,
        Shaping shaping,
        bool enable_kerning = true,
        bool enable_ligatures = false
    );

private:
    std::string text_{};
    AttrsList attrs_list_{};
    std::vector<LayoutLine> layout_lines_{};
    bool is_shaped_{false};
};

/// High-level text buffer managing multi-line text, wrapping, alignment, overflow, and rendering.
class Buffer {
public:
    Buffer(Metrics metrics = Metrics(16.0f, 20.0f));
    ~Buffer() = default;

    // --- Configuration ---
    const Metrics& metrics() const noexcept { return metrics_; }
    void set_metrics(const Metrics& m) noexcept { metrics_ = m; reset_shaping(); }

    std::optional<float> width() const noexcept { return width_; }
    std::optional<float> height() const noexcept { return height_; }
    void set_size(std::optional<float> width, std::optional<float> height) noexcept {
        width_ = width;
        height_ = height;
        reset_shaping();
    }

    Wrap wrap() const noexcept { return wrap_; }
    void set_wrap(Wrap w) noexcept { wrap_ = w; reset_shaping(); }

    Align align() const noexcept { return align_; }
    void set_align(Align a) noexcept { align_ = a; reset_shaping(); }

    Shaping shaping() const noexcept { return shaping_; }
    void set_shaping(Shaping s) noexcept { shaping_ = s; reset_shaping(); }

    Overflow overflow() const noexcept { return overflow_; }
    void set_overflow(Overflow o) noexcept { overflow_ = o; reset_shaping(); }

    std::optional<size_t> max_lines() const noexcept { return max_lines_; }
    void set_max_lines(std::optional<size_t> ml) noexcept { max_lines_ = ml; reset_shaping(); }

    bool kerning_enabled() const noexcept { return enable_kerning_; }
    void set_kerning_enabled(bool enabled) noexcept { enable_kerning_ = enabled; reset_shaping(); }

    bool ligatures_enabled() const noexcept { return enable_ligatures_; }
    void set_ligatures_enabled(bool enabled) noexcept { enable_ligatures_ = enabled; reset_shaping(); }

    float scroll() const noexcept { return scroll_; }
    void set_scroll(float s) noexcept { scroll_ = s; }

    // --- Content ---
    void set_text(std::string_view text, const Attrs& default_attrs = Attrs());
    const std::vector<BufferLine>& lines() const noexcept { return lines_; }
    std::vector<BufferLine>& lines_mut() noexcept { return lines_; }

    void reset_shaping() noexcept;

    /// Runs text shaping and layout on all lines until valid.
    void shape_until_scroll(FontSystem& font_system);

    /// Generates visual layout runs across all wrapped lines.
    std::vector<LayoutRun> layout_runs() const;

    /// Total visual height of all formatted lines.
    float total_height() const;

    /// Maximum visual width across all formatted lines.
    float max_line_width() const;

    /// Draws the buffer onto a Canvas using pre-rendered glyph masks, decorations, and highlights.
    void draw(
        Canvas& canvas,
        GlyphCache& cache,
        FontSystem& font_system,
        Color default_color,
        float origin_x = 0.0f,
        float origin_y = 0.0f
    );

private:
    void apply_ellipsis_truncation(FontSystem& font_system, LayoutLine& line, float max_width) const;

    Metrics metrics_{16.0f, 20.0f};
    std::vector<BufferLine> lines_{};
    std::optional<float> width_{std::nullopt};
    std::optional<float> height_{std::nullopt};
    Wrap wrap_{Wrap::Word};
    Align align_{Align::Left};
    Shaping shaping_{Shaping::Advanced};
    Overflow overflow_{Overflow::Visible};
    std::optional<size_t> max_lines_{std::nullopt};
    bool enable_kerning_{true};
    bool enable_ligatures_{false};
    float scroll_{0.0f};
};

} // namespace nisaba::text
