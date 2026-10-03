#pragma once

/// @file layer.hpp
/// @brief Lottie layer hierarchy, transform inheritance, and layer rendering.

#include "nisaba/lottie/lottie_types.hpp"
#include "nisaba/lottie/property.hpp"
#include "nisaba/lottie/shape.hpp"
#include "nisaba/canvas/canvas.hpp"
#include "nisaba/color/color.hpp"
#include "nisaba/math/transform.hpp"

#include <vector>
#include <memory>
#include <string>
#include <unordered_map>
#include <optional>

namespace nisaba::lottie {

// Forward declarations
class LottieLayer;

struct Mask {
    MaskMode mode = MaskMode::Add;
    ShapeProperty shape;
    FloatProperty opacity{100.0f};
    bool inverted = false;

    static Mask parse(const json::JsonValue& val);
};

class LottieLayer {
public:
    virtual ~LottieLayer() = default;

    int index = -1;
    int parent_index = -1;
    std::string name;
    LayerType type = LayerType::Null;
    float in_point = 0.0f;
    float out_point = 0.0f;
    float start_time = 0.0f;
    float time_stretch = 1.0f;
    LottieTransform transform;
    MatteType matte_type = MatteType::None;
    bool is_matte_target = false;
    bool hidden = false;
    std::vector<Mask> masks;

    [[nodiscard]] bool is_active(float frame) const noexcept {
        return !hidden && (frame >= in_point && frame <= out_point);
    }

    [[nodiscard]] Transform compute_global_transform(
        float frame,
        const std::unordered_map<int, std::shared_ptr<LottieLayer>>& layer_map) const;

    virtual void render(
        RenderContext& ctx,
        float frame,
        const std::unordered_map<int, std::shared_ptr<LottieLayer>>& layer_map,
        const std::unordered_map<std::string, std::vector<std::shared_ptr<LottieLayer>>>& precomp_assets) const = 0;

    static std::shared_ptr<LottieLayer> parse(const json::JsonValue& val);
};

// Shape Layer ("ty": 4)
class ShapeLayer : public LottieLayer {
public:
    std::vector<std::shared_ptr<LottieShapeItem>> shapes;

    void render(
        RenderContext& ctx,
        float frame,
        const std::unordered_map<int, std::shared_ptr<LottieLayer>>& layer_map,
        const std::unordered_map<std::string, std::vector<std::shared_ptr<LottieLayer>>>& precomp_assets) const override;

    static std::shared_ptr<ShapeLayer> parse(const json::JsonValue& val);
};

// Solid Color Layer ("ty": 1)
class SolidLayer : public LottieLayer {
public:
    float width = 0.0f;
    float height = 0.0f;
    Color solid_color = Color::BLACK;

    void render(
        RenderContext& ctx,
        float frame,
        const std::unordered_map<int, std::shared_ptr<LottieLayer>>& layer_map,
        const std::unordered_map<std::string, std::vector<std::shared_ptr<LottieLayer>>>& precomp_assets) const override;

    static std::shared_ptr<SolidLayer> parse(const json::JsonValue& val);
};

// Precomposition Layer ("ty": 0)
class PrecompLayer : public LottieLayer {
public:
    std::string ref_id;
    float width = 0.0f;
    float height = 0.0f;
    FloatProperty time_remap{0.0f};
    bool has_time_remap = false;

    void render(
        RenderContext& ctx,
        float frame,
        const std::unordered_map<int, std::shared_ptr<LottieLayer>>& layer_map,
        const std::unordered_map<std::string, std::vector<std::shared_ptr<LottieLayer>>>& precomp_assets) const override;

    static std::shared_ptr<PrecompLayer> parse(const json::JsonValue& val);
};

// Null / Controller Layer ("ty": 3)
class NullLayer : public LottieLayer {
public:
    void render(
        RenderContext& ctx,
        float frame,
        const std::unordered_map<int, std::shared_ptr<LottieLayer>>& layer_map,
        const std::unordered_map<std::string, std::vector<std::shared_ptr<LottieLayer>>>& precomp_assets) const override;

    static std::shared_ptr<NullLayer> parse(const json::JsonValue& val);
};

// Image / Asset Layer ("ty": 2)
class ImageLayer : public LottieLayer {
public:
    std::string ref_id;

    void render(
        RenderContext& ctx,
        float frame,
        const std::unordered_map<int, std::shared_ptr<LottieLayer>>& layer_map,
        const std::unordered_map<std::string, std::vector<std::shared_ptr<LottieLayer>>>& precomp_assets) const override;

    static std::shared_ptr<ImageLayer> parse(const json::JsonValue& val);
};

// Layer tree renderer (evaluates layers in proper order and hierarchy)
void render_layer_tree(
    ICanvas& canvas,
    const std::vector<std::shared_ptr<LottieLayer>>& layers,
    float frame,
    const std::unordered_map<std::string, std::vector<std::shared_ptr<LottieLayer>>>& precomp_assets,
    float parent_opacity = 1.0f);

} // namespace nisaba::lottie
