/// @file layer.cpp
/// @brief Implementation of Lottie layer hierarchy, transform inheritance, and layer rendering.

#include "nisaba/lottie/layer.hpp"
#include "nisaba/canvas/paint.hpp"
#include "nisaba/math/rect.hpp"

#include <algorithm>
#include <charconv>

namespace nisaba::lottie {

namespace {

Color parse_hex_color(std::string_view str) {
    if (str.starts_with('#')) {
        str.remove_prefix(1);
    }
    if (str.length() == 6) {
        uint32_t val = 0;
        auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), val, 16);
        if (ec == std::errc()) {
            uint8_t r = static_cast<uint8_t>((val >> 16) & 0xFF);
            uint8_t g = static_cast<uint8_t>((val >> 8) & 0xFF);
            uint8_t b = static_cast<uint8_t>(val & 0xFF);
            return Color::from_rgba8(r, g, b, 255);
        }
    } else if (str.length() == 8) {
        uint32_t val = 0;
        auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), val, 16);
        if (ec == std::errc()) {
            uint8_t r = static_cast<uint8_t>((val >> 24) & 0xFF);
            uint8_t g = static_cast<uint8_t>((val >> 16) & 0xFF);
            uint8_t b = static_cast<uint8_t>((val >> 8) & 0xFF);
            uint8_t a = static_cast<uint8_t>(val & 0xFF);
            return Color::from_rgba8(r, g, b, a);
        }
    }
    return Color::BLACK;
}

void parse_common_layer_props(LottieLayer& layer, const json::JsonValue& val) {
    layer.index = val.get_int("ind", -1);
    layer.parent_index = val.get_int("parent", -1);
    layer.name = val.get_string("nm", "");
    layer.in_point = val.get_float("ip", 0.0f);
    layer.out_point = val.get_float("op", 0.0f);
    layer.start_time = val.get_float("st", 0.0f);
    layer.time_stretch = val.get_float("sr", 1.0f);
    if (layer.time_stretch == 0.0f) layer.time_stretch = 1.0f;

    layer.matte_type = static_cast<MatteType>(val.get_int("tt", 0));
    layer.is_matte_target = (val.get_int("td", 0) != 0);
    layer.hidden = val.get_bool("hd", false);

    if (const auto* ks = val.get("ks")) {
        layer.transform = LottieTransform::parse(*ks);
    }

    if (const auto* masks_val = val.get("masksProperties")) {
        if (masks_val->is_array()) {
            for (const auto& m_val : masks_val->as_array()) {
                if (m_val.is_object()) {
                    layer.masks.push_back(Mask::parse(m_val));
                }
            }
        }
    }
}

} // anonymous namespace

// ── Mask ──────────────────────────────────────────────────────────────────

Mask Mask::parse(const json::JsonValue& val) {
    Mask m;
    if (const auto* mode_val = val.get("mode")) {
        if (mode_val->is_string()) {
            std::string_view s = mode_val->as_string();
            if (s == "a") m.mode = MaskMode::Add;
            else if (s == "s") m.mode = MaskMode::Subtract;
            else if (s == "i") m.mode = MaskMode::Intersect;
            else if (s == "f") m.mode = MaskMode::Difference;
            else m.mode = MaskMode::None;
        } else if (mode_val->is_number()) {
            m.mode = static_cast<MaskMode>(mode_val->to_int());
        }
    }
    if (const auto* pt = val.get("pt")) {
        m.shape = parse_shape_property(*pt);
    }
    if (const auto* o = val.get("o")) {
        m.opacity = parse_float_property(*o, 100.0f);
    }
    m.inverted = val.get_bool("inv", false);
    return m;
}

// ── LottieLayer ───────────────────────────────────────────────────────────

Transform LottieLayer::compute_global_transform(
    float frame,
    const std::unordered_map<int, std::shared_ptr<LottieLayer>>& layer_map) const {

    Transform local = transform.matrix(frame);
    if (parent_index < 0) {
        return local;
    }

    // Accumulate parent transforms up to root
    Transform accum = local;
    int curr_parent = parent_index;
    int depth = 0;
    while (curr_parent >= 0 && depth < 32) {
        auto it = layer_map.find(curr_parent);
        if (it == layer_map.end() || !it->second) {
            break;
        }
        Transform p_tr = it->second->transform.matrix(frame);
        accum = p_tr.pre_concat(accum);
        curr_parent = it->second->parent_index;
        depth++;
    }
    return accum;
}

std::shared_ptr<LottieLayer> LottieLayer::parse(const json::JsonValue& val) {
    if (!val.is_object()) return nullptr;

    int type_int = val.get_int("ty", 3);
    LayerType layer_type = static_cast<LayerType>(type_int);

    std::shared_ptr<LottieLayer> layer;
    switch (layer_type) {
        case LayerType::Precomp:
            layer = PrecompLayer::parse(val);
            break;
        case LayerType::Solid:
            layer = SolidLayer::parse(val);
            break;
        case LayerType::Image:
            layer = ImageLayer::parse(val);
            break;
        case LayerType::Null:
            layer = NullLayer::parse(val);
            break;
        case LayerType::Shape:
            layer = ShapeLayer::parse(val);
            break;
        default:
            layer = NullLayer::parse(val);
            break;
    }

    if (layer) {
        layer->type = layer_type;
    }
    return layer;
}

// ── ShapeLayer ────────────────────────────────────────────────────────────

void ShapeLayer::render(
    RenderContext& ctx,
    float frame,
    const std::unordered_map<int, std::shared_ptr<LottieLayer>>& layer_map,
    const std::unordered_map<std::string, std::vector<std::shared_ptr<LottieLayer>>>&) const {

    if (!is_active(frame)) return;

    ctx.canvas.save();
    Transform global_tr = compute_global_transform(frame, layer_map);
    ctx.canvas.concat(global_tr);

    float layer_opacity = ctx.parent_opacity * transform.eval_opacity(frame);
    if (layer_opacity > 0.001f) {
        RenderContext layer_ctx(ctx.canvas, layer_opacity);
        for (const auto& item : shapes) {
            if (item) {
                item->render(layer_ctx, frame);
            }
        }
    }

    ctx.canvas.restore();
}

std::shared_ptr<ShapeLayer> ShapeLayer::parse(const json::JsonValue& val) {
    auto layer = std::make_shared<ShapeLayer>();
    parse_common_layer_props(*layer, val);

    const json::JsonValue* items_val = val.get("shapes");
    if (!items_val) items_val = val.get("it");

    if (items_val && items_val->is_array()) {
        for (const auto& item_json : items_val->as_array()) {
            if (auto item = LottieShapeItem::parse(item_json)) {
                layer->shapes.push_back(std::move(item));
            }
        }
    }

    return layer;
}

// ── SolidLayer ────────────────────────────────────────────────────────────

void SolidLayer::render(
    RenderContext& ctx,
    float frame,
    const std::unordered_map<int, std::shared_ptr<LottieLayer>>& layer_map,
    const std::unordered_map<std::string, std::vector<std::shared_ptr<LottieLayer>>>&) const {

    if (!is_active(frame)) return;

    ctx.canvas.save();
    Transform global_tr = compute_global_transform(frame, layer_map);
    ctx.canvas.concat(global_tr);

    float layer_opacity = ctx.parent_opacity * transform.eval_opacity(frame);
    if (width > 0.0f && height > 0.0f && layer_opacity > 0.001f) {
        Paint paint;
        Color c = solid_color;
        c.apply_opacity(layer_opacity);
        paint.set_color(c);
        if (auto r = Rect::from_xywh(0.0f, 0.0f, width, height)) {
            ctx.canvas.fill_rect(*r, paint);
        }
    }

    ctx.canvas.restore();
}

std::shared_ptr<SolidLayer> SolidLayer::parse(const json::JsonValue& val) {
    auto layer = std::make_shared<SolidLayer>();
    parse_common_layer_props(*layer, val);
    layer->width = val.get_float("sw", 0.0f);
    layer->height = val.get_float("sh", 0.0f);
    layer->solid_color = parse_hex_color(val.get_string("sc", "#000000"));
    return layer;
}

// ── PrecompLayer ──────────────────────────────────────────────────────────

void PrecompLayer::render(
    RenderContext& ctx,
    float frame,
    const std::unordered_map<int, std::shared_ptr<LottieLayer>>& layer_map,
    const std::unordered_map<std::string, std::vector<std::shared_ptr<LottieLayer>>>& precomp_assets) const {

    if (!is_active(frame)) return;

    auto it = precomp_assets.find(ref_id);
    if (it == precomp_assets.end()) return;
    const auto& sub_layers = it->second;
    if (sub_layers.empty()) return;

    ctx.canvas.save();
    Transform global_tr = compute_global_transform(frame, layer_map);
    ctx.canvas.concat(global_tr);

    float sub_frame = (frame - start_time) / time_stretch;
    if (has_time_remap) {
        sub_frame = time_remap.evaluate(frame);
    }

    float layer_opacity = ctx.parent_opacity * transform.eval_opacity(frame);
    if (layer_opacity > 0.001f) {
        render_layer_tree(ctx.canvas, sub_layers, sub_frame, precomp_assets, layer_opacity);
    }

    ctx.canvas.restore();
}

std::shared_ptr<PrecompLayer> PrecompLayer::parse(const json::JsonValue& val) {
    auto layer = std::make_shared<PrecompLayer>();
    parse_common_layer_props(*layer, val);
    layer->ref_id = val.get_string("refId", "");
    layer->width = val.get_float("w", 0.0f);
    layer->height = val.get_float("h", 0.0f);
    if (const auto* tm_val = val.get("tm")) {
        layer->time_remap = parse_float_property(*tm_val, 0.0f);
        layer->has_time_remap = true;
    }
    return layer;
}

// ── NullLayer ─────────────────────────────────────────────────────────────

void NullLayer::render(
    RenderContext&,
    float,
    const std::unordered_map<int, std::shared_ptr<LottieLayer>>&,
    const std::unordered_map<std::string, std::vector<std::shared_ptr<LottieLayer>>>&) const {
    // Null layers produce no visible geometry
}

std::shared_ptr<NullLayer> NullLayer::parse(const json::JsonValue& val) {
    auto layer = std::make_shared<NullLayer>();
    parse_common_layer_props(*layer, val);
    return layer;
}

// ── ImageLayer ────────────────────────────────────────────────────────────

void ImageLayer::render(
    RenderContext&,
    float,
    const std::unordered_map<int, std::shared_ptr<LottieLayer>>&,
    const std::unordered_map<std::string, std::vector<std::shared_ptr<LottieLayer>>>&) const {
    // Image layer placeholder
}

std::shared_ptr<ImageLayer> ImageLayer::parse(const json::JsonValue& val) {
    auto layer = std::make_shared<ImageLayer>();
    parse_common_layer_props(*layer, val);
    layer->ref_id = val.get_string("refId", "");
    return layer;
}

// ── Layer Tree Renderer ───────────────────────────────────────────────────

void render_layer_tree(
    ICanvas& canvas,
    const std::vector<std::shared_ptr<LottieLayer>>& layers,
    float frame,
    const std::unordered_map<std::string, std::vector<std::shared_ptr<LottieLayer>>>& precomp_assets,
    float parent_opacity) {

    if (layers.empty()) return;

    // Build index lookup table for parent-child relationship resolution
    std::unordered_map<int, std::shared_ptr<LottieLayer>> layer_map;
    layer_map.reserve(layers.size());
    for (const auto& l : layers) {
        if (l && l->index >= 0) {
            layer_map[l->index] = l;
        }
    }

    RenderContext ctx(canvas, parent_opacity);

    // In Bodymovin JSON, layers are ordered from top to bottom.
    // To render back-to-front (painter's algorithm), traverse backwards from N-1 down to 0.
    for (size_t i = layers.size(); i > 0; --i) {
        const auto& layer = layers[i - 1];
        if (!layer || !layer->is_active(frame)) continue;
        if (layer->is_matte_target) continue; // Matte sources are handled with their targets

        layer->render(ctx, frame, layer_map, precomp_assets);
    }
}

} // namespace nisaba::lottie
