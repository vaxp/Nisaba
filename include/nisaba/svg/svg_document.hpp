#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <optional>
#include <memory>
#include <map>
#include "nisaba/math/rect.hpp"
#include "nisaba/math/transform.hpp"
#include "nisaba/color/color.hpp"
#include "nisaba/path/path.hpp"
#include "nisaba/path/stroker.hpp"
#include "nisaba/shaders/shader.hpp"
#include "nisaba/canvas/pixmap.hpp"

namespace nisaba {
    class Canvas;
}

namespace nisaba::svg {

/// Coordinate space interpretation for SVG gradients.
enum class SvgGradientUnits : uint8_t {
    ObjectBoundingBox, ///< Coordinates [0, 1] relative to the geometry bounding box
    UserSpaceOnUse,    ///< Absolute coordinates in the current user coordinate space
};

/// An SVG linear or radial gradient definition.
struct SvgGradient {
    enum class Kind : uint8_t { Linear, Radial };

    Kind kind{Kind::Linear};
    std::string id{};
    std::string href_id{};
    SvgGradientUnits units{SvgGradientUnits::ObjectBoundingBox};
    SpreadMode spread_mode{SpreadMode::Pad};
    Transform gradient_transform{};
    std::vector<GradientStop> stops{};

    // Linear gradient coordinates (defaults: 0,0 to 1,0)
    float x1{0.0f};
    float y1{0.0f};
    float x2{1.0f};
    float y2{0.0f};

    // Radial gradient coordinates (defaults: cx=0.5, cy=0.5, r=0.5)
    float cx{0.5f};
    float cy{0.5f};
    float r{0.5f};
    float fx{0.5f};
    float fy{0.5f};
};

/// Paint and styling properties for an SVG element.
struct SvgPaintStyle {
    std::optional<Color> fill{Color::BLACK};     ///< Solid fill color (nullopt if fill="none" or server reference)
    std::string fill_url{};                      ///< Paint server URL id if fill="url(#id)"
    std::optional<Color> stroke{std::nullopt};   ///< Solid stroke color (nullopt if no stroke or "none")
    std::string stroke_url{};                    ///< Paint server URL id if stroke="url(#id)"
    float stroke_width{1.0f};                    ///< Stroke line width
    LineCap line_cap{LineCap::Butt};             ///< Line cap style
    LineJoin line_join{LineJoin::Miter};         ///< Line join style
    float opacity{1.0f};                         ///< Group/Element opacity multiplier
    float fill_opacity{1.0f};                    ///< Fill opacity multiplier
    float stroke_opacity{1.0f};                  ///< Stroke opacity multiplier
    FillRule fill_rule{FillRule::Winding};       ///< Fill rule (Winding or EvenOdd)
    std::string clip_path_url{};                 ///< ClipPath reference if clip-path="url(#id)"
    std::string mask_url{};                      ///< Mask reference if mask="url(#id)"
};

/// The discriminator type of an SVG element.
enum class SvgElementType : uint8_t {
    Path,
    Image,
};

/// A single rendered vector element or embedded raster image parsed from an SVG document.
struct SvgElement {
    SvgElementType type{SvgElementType::Path};
    Path path{};
    SvgPaintStyle style{};
    Transform transform{};
    std::string id{};

    // Image-specific properties (when type == SvgElementType::Image)
    std::shared_ptr<Pixmap> image{};
    Rect image_bounds{};
};

/// An SVG <mask> container with child elements to be rasterized as a luminance/alpha mask.
struct SvgMask {
    std::string id{};
    std::vector<SvgElement> elements{};
};

/// An in-memory parsed representation of an SVG vector document or icon.
class SvgDocument {
public:
    SvgDocument() noexcept {
        auto r = Rect::from_xywh(0.0f, 0.0f, 100.0f, 100.0f);
        if (r) view_box_ = *r;
    }

    /// Parses an SVG XML string into an SvgDocument.
    static std::optional<SvgDocument> parse(std::string_view svg_xml);

    /// Reads and parses an SVG file from the filesystem.
    static std::optional<SvgDocument> from_file(const std::string& path);

    /// Renders the SVG document onto a Canvas, optionally scaling to fit target_bounds and applying a tint color.
    void render(Canvas& canvas, const Rect* target_bounds = nullptr, std::optional<Color> tint = std::nullopt) const;

    [[nodiscard]] Rect view_box() const noexcept { return view_box_; }
    [[nodiscard]] float width() const noexcept { return width_; }
    [[nodiscard]] float height() const noexcept { return height_; }
    [[nodiscard]] const std::vector<SvgElement>& elements() const noexcept { return elements_; }
    [[nodiscard]] const std::map<std::string, SvgGradient>& gradients() const noexcept { return gradients_; }
    [[nodiscard]] const std::map<std::string, Path>& clip_paths() const noexcept { return clip_paths_; }
    [[nodiscard]] const std::map<std::string, SvgMask>& masks() const noexcept { return masks_; }
    [[nodiscard]] const std::map<std::string, SvgElement>& defined_elements() const noexcept { return defined_elements_; }

    void set_view_box(Rect vb) noexcept { view_box_ = vb; }
    void set_dimensions(float w, float h) noexcept { width_ = w; height_ = h; }
    void add_element(SvgElement elem) { elements_.push_back(std::move(elem)); }
    void add_gradient(SvgGradient grad) { gradients_[grad.id] = std::move(grad); }
    void add_clip_path(std::string id, Path path) { clip_paths_[std::move(id)] = std::move(path); }
    void add_mask(SvgMask mask) { masks_[mask.id] = std::move(mask); }
    void add_defined_element(std::string id, SvgElement elem) { defined_elements_[std::move(id)] = std::move(elem); }

private:
    void render_element(Canvas& canvas, const SvgElement& elem, std::optional<Color> tint) const;

    Rect view_box_{};
    float width_{100.0f};
    float height_{100.0f};
    std::vector<SvgElement> elements_;
    std::map<std::string, SvgGradient> gradients_;
    std::map<std::string, Path> clip_paths_;
    std::map<std::string, SvgMask> masks_;
    std::map<std::string, SvgElement> defined_elements_;
    std::map<std::string, std::vector<SvgElement>> defined_groups_;
};

} // namespace nisaba::svg
