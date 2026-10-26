/// @file layer.cpp
/// @brief Implementation of Lottie layers, masks, track mattes, effects, images, typography, and 3D compositing.

#include "nisaba/lottie/layer.hpp"
#include "nisaba/canvas/paint.hpp"
#include "nisaba/canvas/mask.hpp"
#include "nisaba/effects/blur.hpp"
#include "nisaba/effects/drop_shadow.hpp"
#include "nisaba/effects/color_matrix.hpp"
#include "nisaba/path/path_builder.hpp"
#include "nisaba/path/path_ops.hpp"
#include "nisaba/path/stroker.hpp"
#include "nisaba/text/bidi.hpp"
#include "nisaba/math/rect.hpp"

#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif
#include <cmath>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include <algorithm>
#include <charconv>

namespace nisaba::lottie {

// ── LottieAssetRegistry ───────────────────────────────────────────────────

const LottieGlyphChar* LottieAssetRegistry::find_char(std::string_view ch, std::string_view family, std::string_view style) const noexcept {
    for (const auto& gc : chars) {
        if (gc.ch == ch && (family.empty() || gc.family == family) && (style.empty() || gc.style == style)) {
            return &gc;
        }
    }
    for (const auto& gc : chars) {
        if (gc.ch == ch) return &gc;
    }
    return nullptr;
}

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

Color parse_json_rgb_array(const json::JsonValue& val, Color fallback = Color::BLACK) {
    if (!val.is_array()) return fallback;
    const auto& arr = val.as_array();
    if (arr.size() < 3) return fallback;
    float r = arr[0].to_float();
    float g = arr[1].to_float();
    float b = arr[2].to_float();
    float a = (arr.size() >= 4) ? arr[3].to_float() : 1.0f;
    if (r > 1.0f || g > 1.0f || b > 1.0f) {
        r /= 255.0f;
        g /= 255.0f;
        b /= 255.0f;
        if (a > 1.0f) a /= 255.0f;
    }
    return Color::from_rgba(
        std::clamp(r, 0.0f, 1.0f),
        std::clamp(g, 0.0f, 1.0f),
        std::clamp(b, 0.0f, 1.0f),
        std::clamp(a, 0.0f, 1.0f)
    ).value_or(fallback);
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

    layer.blend_mode = map_lottie_blend_mode(val.get_int("bm", 0));
    layer.matte_type = static_cast<MatteType>(val.get_int("tt", 0));
    layer.matte_parent_index = val.get_int("tp", -1);
    layer.is_matte_target = (val.get_int("td", 0) != 0);
    layer.hidden = val.get_bool("hd", false);
    layer.auto_orient = (val.get_int("ao", 0) == 1);

    if (const auto* ks = val.get("ks")) {
        layer.transform = LottieTransform::parse(*ks);
        layer.transform.auto_orient = layer.auto_orient;
        if (val.get_int("ddd", 0) == 1) {
            layer.transform.is_3d = true;
        }
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

    if (const auto* ef_val = val.get("ef")) {
        if (ef_val->is_array()) {
            for (const auto& e_val : ef_val->as_array()) {
                if (e_val.is_object()) {
                    auto eff = LayerEffect::parse(e_val);
                    if (eff.type != EffectType::Unknown && eff.enabled) {
                        layer.effects.push_back(std::move(eff));
                    }
                }
            }
        }
    }
}

bool point_in_path_approx(const Path& p, float x, float y) {
    if (p.is_empty()) return false;
    Rect b = p.bounds();
    if (x < b.left() || x > b.right() || y < b.top() || y > b.bottom()) {
        return false;
    }
    // Ray-casting across path segments
    PathSegmentsIter iter = p.segments();
    iter.set_auto_close(true);
    Point last = Point::zero();
    int crossings = 0;

    auto test_edge = [&](Point p0, Point p1) {
        if ((p0.y <= y && p1.y > y) || (p1.y <= y && p0.y > y)) {
            float vt = (y - p0.y) / (p1.y - p0.y);
            if (x < p0.x + vt * (p1.x - p0.x)) {
                crossings++;
            }
        }
    };

    while (auto seg = iter.next()) {
        switch (seg->type) {
            case PathSegment::Type::MoveTo:
                last = seg->p0;
                break;
            case PathSegment::Type::LineTo:
                test_edge(last, seg->p0);
                last = seg->p0;
                break;
            case PathSegment::Type::QuadTo: {
                Point prev = last;
                for (int i = 1; i <= 4; ++i) {
                    float t = i * 0.25f;
                    float mt = 1.0f - t;
                    Point cur = Point::from_xy(
                        mt * mt * last.x + 2.0f * mt * t * seg->p0.x + t * t * seg->p1.x,
                        mt * mt * last.y + 2.0f * mt * t * seg->p0.y + t * t * seg->p1.y
                    );
                    test_edge(prev, cur);
                    prev = cur;
                }
                last = seg->p1;
                break;
            }
            case PathSegment::Type::CubicTo: {
                Point prev = last;
                for (int i = 1; i <= 6; ++i) {
                    float t = i / 6.0f;
                    float mt = 1.0f - t;
                    Point cur = Point::from_xy(
                        mt * mt * mt * last.x + 3.0f * mt * mt * t * seg->p0.x + 3.0f * mt * t * t * seg->p1.x + t * t * t * seg->p2.x,
                        mt * mt * mt * last.y + 3.0f * mt * mt * t * seg->p0.y + 3.0f * mt * t * t * seg->p1.y + t * t * t * seg->p2.y
                    );
                    test_edge(prev, cur);
                    prev = cur;
                }
                last = seg->p2;
                break;
            }
            case PathSegment::Type::Close:
                break;
        }
    }
    return (crossings % 2) != 0;
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
    if (const auto* x = val.get("x")) {
        m.expansion = parse_float_property(*x, 0.0f);
    }
    if (const auto* f = val.get("f")) {
        m.feather = parse_point_property(*f, Point::zero());
    }
    m.inverted = val.get_bool("inv", false);
    return m;
}

// ── LayerEffect ("ef") ────────────────────────────────────────────────────

LayerEffect LayerEffect::parse(const json::JsonValue& val) {
    LayerEffect eff;
    int ty = val.get_int("ty", -1);
    eff.name = val.get_string("nm", "");
    eff.enabled = (val.get_int("en", 1) != 0);

    switch (ty) {
        case 20: eff.type = EffectType::Tint; break;
        case 21: eff.type = EffectType::Fill; break;
        case 23: eff.type = EffectType::Tritone; break;
        case 25: eff.type = EffectType::DropShadow; break;
        case 29: eff.type = EffectType::GaussianBlur; break;
        default: eff.type = EffectType::Unknown; break;
    }

    const auto* sub_ef = val.get("ef");
    if (!sub_ef || !sub_ef->is_array()) return eff;
    const auto& params = sub_ef->as_array();

    if (eff.type == EffectType::GaussianBlur) {
        for (size_t i = 0; i < params.size(); ++i) {
            if (!params[i].is_object()) continue;
            const auto* v = params[i].get("v");
            if (!v) continue;
            if (i == 0) eff.blurriness = parse_float_property(*v, 0.0f);
            else if (i == 1) eff.blur_dimensions = static_cast<int>(parse_float_property(*v, 1.0f).evaluate(0.0f));
        }
    } else if (eff.type == EffectType::DropShadow) {
        for (size_t i = 0; i < params.size(); ++i) {
            if (!params[i].is_object()) continue;
            const auto* v = params[i].get("v");
            if (!v) continue;
            if (i == 0) eff.shadow_color = parse_color_property(*v, Color::BLACK);
            else if (i == 1) eff.shadow_opacity = parse_float_property(*v, 128.0f);
            else if (i == 2) eff.shadow_direction = parse_float_property(*v, 135.0f);
            else if (i == 3) eff.shadow_distance = parse_float_property(*v, 5.0f);
            else if (i == 4) eff.shadow_softness = parse_float_property(*v, 5.0f);
        }
    } else if (eff.type == EffectType::Fill) {
        for (size_t i = 0; i < params.size(); ++i) {
            if (!params[i].is_object()) continue;
            const auto* v = params[i].get("v");
            if (!v) continue;
            int pty = params[i].get_int("ty", -1);
            if (pty == 2 || i == 2) eff.shadow_color = parse_color_property(*v, Color::RED);
            else if (i == 6) eff.shadow_opacity = parse_float_property(*v, 1.0f);
        }
    } else if (eff.type == EffectType::Tint) {
        for (size_t i = 0; i < params.size(); ++i) {
            if (!params[i].is_object()) continue;
            const auto* v = params[i].get("v");
            if (!v) continue;
            if (i == 0) eff.tint_black = parse_color_property(*v, Color::BLACK);
            else if (i == 1) eff.tint_white = parse_color_property(*v, Color::WHITE);
            else if (i == 2) eff.tint_amount = parse_float_property(*v, 100.0f);
        }
    } else if (eff.type == EffectType::Tritone) {
        for (size_t i = 0; i < params.size(); ++i) {
            if (!params[i].is_object()) continue;
            const auto* v = params[i].get("v");
            if (!v) continue;
            if (i == 0) eff.tint_white = parse_color_property(*v, Color::WHITE);
            else if (i == 1) eff.tint_mid = parse_color_property(*v, Color::from_rgba8(128, 128, 128, 255));
            else if (i == 2) eff.tint_black = parse_color_property(*v, Color::BLACK);
            else if (i == 3) eff.tint_amount = parse_float_property(*v, 0.0f);
        }
    }
    return eff;
}

// ── TextDocument & TextAnimator ("ty": 5) ─────────────────────────────────

TextDocument TextDocument::parse(const json::JsonValue& val) {
    TextDocument doc;
    if (!val.is_object()) return doc;
    doc.text = std::string(val.get_string("t", ""));
    doc.font_name = std::string(val.get_string("f", ""));
    doc.font_size = val.get_float("s", 24.0f);
    doc.line_height = val.get_float("lh", doc.font_size * 1.2f);
    doc.tracking = val.get_float("tr", 0.0f);
    doc.baseline_shift = val.get_float("ls", 0.0f);
    doc.stroke_width = val.get_float("sw", 0.0f);
    doc.stroke_over_fill = val.get_bool("of", false);
    int j = val.get_int("j", 0);
    if (j >= 0 && j <= 2) doc.justify = static_cast<TextJustify>(j);

    if (const auto* fc = val.get("fc")) doc.fill_color = parse_json_rgb_array(*fc, Color::BLACK);
    if (const auto* sc = val.get("sc")) doc.stroke_color = parse_json_rgb_array(*sc, Color::BLACK);
    if (const auto* ps = val.get("ps"); ps && ps->is_array() && ps->as_array().size() >= 2) {
        doc.box_position = Point::from_xy(ps->as_array()[0].to_float(), ps->as_array()[1].to_float());
    }
    if (const auto* sz = val.get("sz"); sz && sz->is_array() && sz->as_array().size() >= 2) {
        doc.box_size = Point::from_xy(sz->as_array()[0].to_float(), sz->as_array()[1].to_float());
    }
    return doc;
}

float TextRangeSelector::compute_factor(size_t index, size_t total_count, float frame) const noexcept {
    if (total_count == 0) return 0.0f;
    float s = start.evaluate(frame) + offset.evaluate(frame);
    float e = end.evaluate(frame) + offset.evaluate(frame);
    if (units == 1) {
        // Percentage [0..100] mapped to character index
        s *= 0.01f * static_cast<float>(total_count);
        e *= 0.01f * static_cast<float>(total_count);
    }
    if (s > e) std::swap(s, e);

    float char_start = static_cast<float>(index);
    float char_end = static_cast<float>(index + 1);
    if (e <= char_start || s >= char_end) return 0.0f;

    float span = e - s;
    if (span <= 1e-5f) return 0.0f;

    float overlap_start = std::max(s, char_start);
    float overlap_end = std::min(e, char_end);
    float coverage = std::clamp(overlap_end - overlap_start, 0.0f, 1.0f);

    if (shape == 1) {
        return coverage; // Square
    }
    float mid = (char_start + 0.5f - s) / span;
    mid = std::clamp(mid, 0.0f, 1.0f);
    switch (shape) {
        case 2: return mid; // Ramp Up
        case 3: return 1.0f - mid; // Ramp Down
        case 4: return 1.0f - std::abs(2.0f * mid - 1.0f); // Triangle
        case 5: return std::sin(mid * static_cast<float>(M_PI)); // Round
        case 6: return 0.5f * (1.0f - std::cos(mid * 2.0f * static_cast<float>(M_PI))); // Smooth
        default: return coverage;
    }
}

TextRangeSelector TextRangeSelector::parse(const json::JsonValue& val) {
    TextRangeSelector sel;
    if (!val.is_object()) return sel;
    if (const auto* s = val.get("s")) sel.start = parse_float_property(*s, 0.0f);
    if (const auto* e = val.get("e")) sel.end = parse_float_property(*e, 100.0f);
    if (const auto* o = val.get("o")) sel.offset = parse_float_property(*o, 0.0f);
    sel.units = val.get_int("r", 1);
    sel.based_on = val.get_int("b", 1);
    sel.shape = val.get_int("sh", 1);
    return sel;
}

TextAnimator TextAnimator::parse(const json::JsonValue& val) {
    TextAnimator anim;
    if (!val.is_object()) return anim;
    anim.name = val.get_string("nm", "");
    if (const auto* s = val.get("s")) {
        if (s->is_object()) anim.selector = TextRangeSelector::parse(*s);
        else if (s->is_array() && !s->as_array().empty()) anim.selector = TextRangeSelector::parse(s->as_array()[0]);
    }
    if (const auto* a = val.get("a"); a && a->is_object()) {
        if (const auto* p = a->get("p")) { anim.position = parse_point_property(*p, Point::zero()); anim.has_position = true; }
        if (const auto* sc = a->get("s")) { anim.scale = parse_point_property(*sc, Point::from_xy(100.0f, 100.0f)); anim.has_scale = true; }
        if (const auto* r = a->get("r")) { anim.rotation = parse_float_property(*r, 0.0f); anim.has_rotation = true; }
        if (const auto* o = a->get("o")) { anim.opacity = parse_float_property(*o, 100.0f); anim.has_opacity = true; }
        if (const auto* fc = a->get("fc")) { anim.fill_color = parse_color_property(*fc, Color::BLACK); anim.has_fill_color = true; }
        if (const auto* sc = a->get("sc")) { anim.stroke_color = parse_color_property(*sc, Color::BLACK); anim.has_stroke_color = true; }
        if (const auto* sw = a->get("sw")) { anim.stroke_width = parse_float_property(*sw, 0.0f); anim.has_stroke_width = true; }
        if (const auto* t = a->get("t")) { anim.tracking = parse_float_property(*t, 0.0f); anim.has_tracking = true; }
    }
    return anim;
}

// ── LottieLayer Hierarchy & Transforms ────────────────────────────────────

void link_layer_hierarchy(std::vector<std::shared_ptr<LottieLayer>>& layers) {
    std::unordered_map<int, const LottieLayer*> index_map;
    index_map.reserve(layers.size());
    for (const auto& l : layers) {
        if (l && l->index >= 0) {
            index_map[l->index] = l.get();
        }
    }

    for (size_t i = 0; i < layers.size(); ++i) {
        auto& l = layers[i];
        if (!l) continue;

        l->parent_ptr = nullptr;
        if (l->parent_index >= 0) {
            auto it = index_map.find(l->parent_index);
            if (it != index_map.end() && it->second != l.get()) {
                l->parent_ptr = it->second;
            }
        }

        l->matte_layer_ptr = nullptr;
        if (l->matte_type != MatteType::None) {
            if (l->matte_parent_index >= 0) {
                auto it = index_map.find(l->matte_parent_index);
                if (it != index_map.end()) {
                    l->matte_layer_ptr = it->second;
                }
            } else if (i > 0 && layers[i - 1]) {
                // In Bodymovin, if "tp" is omitted, the layer immediately above (i - 1) is the matte source
                l->matte_layer_ptr = layers[i - 1].get();
            }
        }
    }
}

Transform LottieLayer::compute_global_transform(float frame) const noexcept {
    Transform accum = transform.matrix(frame);
    const LottieLayer* curr = parent_ptr;
    int depth = 0;
    while (curr && depth < 32) {
        Transform p_tr = curr->transform.matrix(frame);
        accum = p_tr.pre_concat(accum);
        curr = curr->parent_ptr;
        depth++;
    }
    return accum;
}

Transform LottieLayer::compute_global_transform(
    float frame,
    const std::unordered_map<int, std::shared_ptr<LottieLayer>>& layer_map) const {
    if (parent_ptr || parent_index < 0) {
        return compute_global_transform(frame);
    }
    Transform accum = transform.matrix(frame);
    int curr_parent = parent_index;
    int depth = 0;
    while (curr_parent >= 0 && depth < 32) {
        auto it = layer_map.find(curr_parent);
        if (it == layer_map.end() || !it->second) break;
        Transform p_tr = it->second->transform.matrix(frame);
        accum = p_tr.pre_concat(accum);
        curr_parent = it->second->parent_index;
        depth++;
    }
    return accum;
}

bool LottieLayer::has_3d_hierarchy() const noexcept {
    const LottieLayer* curr = this;
    int depth = 0;
    while (curr && depth < 32) {
        if (curr->transform.is_3d) return true;
        curr = curr->parent_ptr;
        depth++;
    }
    return false;
}

Transform4x4 LottieLayer::compute_global_transform_3d(float frame, float comp_w, float comp_h) const noexcept {
    Transform4x4 accum = transform.matrix_3d(frame);
    const LottieLayer* curr = parent_ptr;
    int depth = 0;
    while (curr && depth < 32) {
        Transform4x4 p_tr = curr->transform.matrix_3d(frame);
        accum = p_tr * accum;
        curr = curr->parent_ptr;
        depth++;
    }
    // Apply centered perspective projection so Z rotations have realistic 3D foreshortening
    float cx = comp_w * 0.5f;
    float cy = comp_h * 0.5f;
    float fov_d = std::max(comp_w, comp_h) * 2.0f;
    Transform4x4 persp = Transform4x4::from_translate(cx, cy, 0.0f)
                       * Transform4x4::from_perspective(fov_d)
                       * Transform4x4::from_translate(-cx, -cy, 0.0f);
    return persp * accum;
}

void LottieLayer::render_with_assets(
    RenderContext& ctx,
    float frame,
    const LottieAssetRegistry& assets) const {
    std::unordered_map<int, std::shared_ptr<LottieLayer>> empty_map;
    render(ctx, frame, empty_map, assets.precomps);
}

void LottieLayer::render(
    RenderContext& ctx,
    float frame,
    const std::unordered_map<int, std::shared_ptr<LottieLayer>>&,
    const std::unordered_map<std::string, std::vector<std::shared_ptr<LottieLayer>>>& precomp_assets) const {
    LottieAssetRegistry temp_assets;
    temp_assets.precomps = precomp_assets;
    render_with_assets(ctx, frame, temp_assets);
}

bool LottieLayer::hit_test(float, float, float, const LottieAssetRegistry&, std::string&) const {
    return false;
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
        case LayerType::Text:
            layer = TextLayer::parse(val);
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

// ── ShapeLayer ("ty": 4) ──────────────────────────────────────────────────

void ShapeLayer::render_with_assets(
    RenderContext& ctx,
    float frame,
    const LottieAssetRegistry&) const {

    if (!is_active(frame)) return;

    ctx.canvas.save();
    Transform global_tr = compute_global_transform(frame);
    ctx.canvas.concat(global_tr);

    float layer_opacity = ctx.parent_opacity * transform.eval_opacity(frame);
    if (layer_opacity > 0.001f) {
        RenderContext layer_ctx(ctx.canvas, layer_opacity);
        layer_ctx.active_blend_mode = blend_mode;
        for (const auto& item : shapes) {
            if (item && !item->hidden) {
                item->render(layer_ctx, frame);
            }
        }
    }

    ctx.canvas.restore();
}

bool ShapeLayer::hit_test(float x, float y, float frame, const LottieAssetRegistry&, std::string& out_hit_name) const {
    if (!is_active(frame)) return false;
    Transform global_tr = compute_global_transform(frame);
    auto inv = global_tr.invert();
    if (!inv) return false;
    Point local_pt = Point::from_xy(x, y);
    inv->map_points(&local_pt, 1);

    for (auto it = shapes.rbegin(); it != shapes.rend(); ++it) {
        const auto& item = *it;
        if (!item || item->hidden) continue;
        Path p;
        item->collect_geometry(frame, p);
        if (point_in_path_approx(p, local_pt.x, local_pt.y)) {
            out_hit_name = !name.empty() ? name : item->name;
            return true;
        }
    }
    return false;
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

// ── SolidLayer ("ty": 1) ──────────────────────────────────────────────────

void SolidLayer::render_with_assets(
    RenderContext& ctx,
    float frame,
    const LottieAssetRegistry&) const {

    if (!is_active(frame)) return;

    ctx.canvas.save();
    Transform global_tr = compute_global_transform(frame);
    ctx.canvas.concat(global_tr);

    float layer_opacity = ctx.parent_opacity * transform.eval_opacity(frame);
    if (width > 0.0f && height > 0.0f && layer_opacity > 0.001f) {
        Paint paint;
        Color c = color_override.value_or(solid_color);
        c.apply_opacity(layer_opacity);
        paint.set_color(c);
        paint.blend_mode = blend_mode;
        if (auto r = Rect::from_xywh(0.0f, 0.0f, width, height)) {
            ctx.canvas.fill_rect(*r, paint);
        }
    }

    ctx.canvas.restore();
}

bool SolidLayer::hit_test(float x, float y, float frame, const LottieAssetRegistry&, std::string& out_hit_name) const {
    if (!is_active(frame) || width <= 0.0f || height <= 0.0f) return false;
    Transform global_tr = compute_global_transform(frame);
    auto inv = global_tr.invert();
    if (!inv) return false;
    Point pt = Point::from_xy(x, y);
    inv->map_points(&pt, 1);
    if (pt.x >= 0.0f && pt.x <= width && pt.y >= 0.0f && pt.y <= height) {
        out_hit_name = name;
        return true;
    }
    return false;
}

std::shared_ptr<SolidLayer> SolidLayer::parse(const json::JsonValue& val) {
    auto layer = std::make_shared<SolidLayer>();
    parse_common_layer_props(*layer, val);
    layer->width = val.get_float("sw", 0.0f);
    layer->height = val.get_float("sh", 0.0f);
    layer->solid_color = parse_hex_color(val.get_string("sc", "#000000"));
    return layer;
}

// ── PrecompLayer ("ty": 0) ────────────────────────────────────────────────

void PrecompLayer::render_with_assets(
    RenderContext& ctx,
    float frame,
    const LottieAssetRegistry& assets) const {

    if (!is_active(frame)) return;

    auto it = assets.precomps.find(ref_id);
    if (it == assets.precomps.end()) return;
    const auto& sub_layers = it->second;
    if (sub_layers.empty()) return;

    ctx.canvas.save();
    Transform global_tr = compute_global_transform(frame);
    ctx.canvas.concat(global_tr);

    if (width > 0.0f && height > 0.0f) {
        if (auto clip_r = Rect::from_xywh(0.0f, 0.0f, width, height)) {
            ctx.canvas.clip_rect(*clip_r);
        }
    }

    float sub_frame = (frame - start_time) / time_stretch;
    if (has_time_remap) {
        // Note: Lottie "tm" is in seconds; convert to frames if needed or evaluate directly
        sub_frame = time_remap.evaluate(frame);
    }

    float layer_opacity = ctx.parent_opacity * transform.eval_opacity(frame);
    if (layer_opacity > 0.001f) {
        render_layer_tree_with_assets(ctx.canvas, sub_layers, sub_frame, assets, layer_opacity);
    }

    ctx.canvas.restore();
}

bool PrecompLayer::hit_test(float x, float y, float frame, const LottieAssetRegistry& assets, std::string& out_hit_name) const {
    if (!is_active(frame)) return false;
    auto it = assets.precomps.find(ref_id);
    if (it == assets.precomps.end()) return false;

    Transform global_tr = compute_global_transform(frame);
    auto inv = global_tr.invert();
    if (!inv) return false;
    Point pt = Point::from_xy(x, y);
    inv->map_points(&pt, 1);

    float sub_frame = has_time_remap ? time_remap.evaluate(frame) : (frame - start_time) / time_stretch;
    for (const auto& sub : it->second) {
        if (sub && sub->hit_test(pt.x, pt.y, sub_frame, assets, out_hit_name)) {
            if (out_hit_name.empty()) out_hit_name = name;
            return true;
        }
    }
    return false;
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

// ── NullLayer ("ty": 3) ───────────────────────────────────────────────────

void NullLayer::render_with_assets(
    RenderContext&,
    float,
    const LottieAssetRegistry&) const {
    // Null layers produce no visible geometry
}

std::shared_ptr<NullLayer> NullLayer::parse(const json::JsonValue& val) {
    auto layer = std::make_shared<NullLayer>();
    parse_common_layer_props(*layer, val);
    return layer;
}

// ── ImageLayer ("ty": 2) ──────────────────────────────────────────────────

void ImageLayer::render_with_assets(
    RenderContext& ctx,
    float frame,
    const LottieAssetRegistry& assets) const {

    if (!is_active(frame)) return;
    auto it = assets.images.find(ref_id);
    if (it == assets.images.end() || !it->second.pixmap.has_value()) return;

    const Pixmap& pm = *it->second.pixmap;
    if (pm.width() == 0 || pm.height() == 0) return;

    float layer_opacity = ctx.parent_opacity * transform.eval_opacity(frame);
    if (layer_opacity <= 0.001f) return;

    ctx.canvas.save();
    Transform global_tr = compute_global_transform(frame);

    // Scale image to asset's declared dimensions if they differ from decoded pixmap size
    float sx = (it->second.width > 0.0f) ? (it->second.width / static_cast<float>(pm.width())) : 1.0f;
    float sy = (it->second.height > 0.0f) ? (it->second.height / static_cast<float>(pm.height())) : 1.0f;
    if (std::abs(sx - 1.0f) > 1e-4f || std::abs(sy - 1.0f) > 1e-4f) {
        global_tr = global_tr.pre_concat(Transform::from_scale(sx, sy));
    }

    ctx.canvas.concat(global_tr);

    PixmapPaint paint;
    paint.opacity = layer_opacity;
    paint.blend_mode = blend_mode;
    paint.quality = FilterQuality::Bilinear;

    ctx.canvas.draw_pixmap(0, 0, pm.as_ref(), paint);
    ctx.canvas.restore();
}

bool ImageLayer::hit_test(float x, float y, float frame, const LottieAssetRegistry& assets, std::string& out_hit_name) const {
    if (!is_active(frame)) return false;
    auto it = assets.images.find(ref_id);
    if (it == assets.images.end()) return false;
    float w = it->second.width;
    float h = it->second.height;
    if (w <= 0.0f && it->second.pixmap) w = static_cast<float>(it->second.pixmap->width());
    if (h <= 0.0f && it->second.pixmap) h = static_cast<float>(it->second.pixmap->height());
    if (w <= 0.0f || h <= 0.0f) return false;

    Transform global_tr = compute_global_transform(frame);
    auto inv = global_tr.invert();
    if (!inv) return false;
    Point pt = Point::from_xy(x, y);
    inv->map_points(&pt, 1);
    if (pt.x >= 0.0f && pt.x <= w && pt.y >= 0.0f && pt.y <= h) {
        out_hit_name = name;
        return true;
    }
    return false;
}

std::shared_ptr<ImageLayer> ImageLayer::parse(const json::JsonValue& val) {
    auto layer = std::make_shared<ImageLayer>();
    parse_common_layer_props(*layer, val);
    layer->ref_id = val.get_string("refId", "");
    return layer;
}

// ── TextLayer ("ty": 5) ───────────────────────────────────────────────────

const TextDocument& TextLayer::evaluate_document(float frame) const noexcept {
    if (keyframes.empty()) return static_doc;
    for (size_t i = keyframes.size(); i > 0; --i) {
        if (frame >= keyframes[i - 1].time) {
            return keyframes[i - 1].start_value;
        }
    }
    return keyframes.front().start_value;
}

void TextLayer::render_with_assets(
    RenderContext& ctx,
    float frame,
    const LottieAssetRegistry& assets) const {

    if (!is_active(frame)) return;
    const TextDocument& doc = evaluate_document(frame);
    std::string_view raw_text = text_override.has_value() ? std::string_view(*text_override) : std::string_view(doc.text);
    if (raw_text.empty() || doc.font_size <= 0.0f) return;

    float base_opacity = ctx.parent_opacity * transform.eval_opacity(frame);
    if (base_opacity <= 0.001f) return;

    // Split text into lines by '\r' or '\n'
    std::vector<std::string_view> lines;
    size_t pos = 0;
    while (pos <= raw_text.size()) {
        size_t next_r = raw_text.find_first_of("\r\n", pos);
        if (next_r == std::string_view::npos) {
            lines.push_back(raw_text.substr(pos));
            break;
        }
        lines.push_back(raw_text.substr(pos, next_r - pos));
        if (raw_text[next_r] == '\r' && next_r + 1 < raw_text.size() && raw_text[next_r + 1] == '\n') {
            pos = next_r + 2;
        } else {
            pos = next_r + 1;
        }
    }

    // Resolve font definition & TtfFont if available
    std::string font_family;
    std::string font_style;
    const text::TtfFont* ttf_font = nullptr;
    auto fit = assets.fonts.find(doc.font_name);
    if (fit != assets.fonts.end()) {
        font_family = fit->second.family;
        font_style = fit->second.style;
        if (assets.font_system && fit->second.loaded_font_id) {
            ttf_font = assets.font_system->get_font(*fit->second.loaded_font_id);
        }
    }
    if (!ttf_font && assets.font_system) {
        auto [fid, fptr] = assets.font_system->match_font(
            font_family.empty() ? text::Family::sans_serif() : text::Family(font_family)
        );
        ttf_font = fptr;
    }

    float scale_100 = doc.font_size / 100.0f;
    float tracking_px = (doc.tracking / 1000.0f) * doc.font_size;

    // Count total characters for range selectors
    struct GlyphItem {
        std::string utf8_cluster;
        char32_t shaped_cp = 0;
        const LottieGlyphChar* lottie_char = nullptr;
        float advance = 0.0f;
        size_t line_idx = 0;
        size_t global_char_idx = 0;
    };

    std::vector<std::vector<GlyphItem>> line_glyphs(lines.size());
    std::vector<float> line_widths(lines.size(), 0.0f);
    size_t total_chars = 0;

    for (size_t li = 0; li < lines.size(); ++li) {
        std::string_view ln = lines[li];
        if (ln.empty()) continue;

        // Use Nisaba's sovereign BiDi + Arabic contextual shaping!
        auto runs = text::Bidi::segment_runs(ln);
        auto shaped = text::Bidi::shape_text(ln, runs);

        for (size_t ci = 0; ci < shaped.size(); ++ci) {
            const auto& sc = shaped[ci];
            GlyphItem gi;
            gi.shaped_cp = sc.codepoint;
            gi.line_idx = li;
            gi.global_char_idx = total_chars++;

            // Encode codepoint to UTF-8 for pre-baked Lottie "chars" lookup
            char32_t lookup_cp = (sc.original_codepoint != 0) ? sc.original_codepoint : sc.codepoint;
            char buf[5] = {0};
            if (lookup_cp < 0x80) {
                buf[0] = static_cast<char>(lookup_cp);
                gi.utf8_cluster = std::string(buf, 1);
            } else if (lookup_cp < 0x800) {
                buf[0] = static_cast<char>(0xC0 | (lookup_cp >> 6));
                buf[1] = static_cast<char>(0x80 | (lookup_cp & 0x3F));
                gi.utf8_cluster = std::string(buf, 2);
            } else if (lookup_cp < 0x10000) {
                buf[0] = static_cast<char>(0xE0 | (lookup_cp >> 12));
                buf[1] = static_cast<char>(0x80 | ((lookup_cp >> 6) & 0x3F));
                buf[2] = static_cast<char>(0x80 | (lookup_cp & 0x3F));
                gi.utf8_cluster = std::string(buf, 3);
            } else {
                buf[0] = static_cast<char>(0xF0 | (lookup_cp >> 18));
                buf[1] = static_cast<char>(0x80 | ((lookup_cp >> 12) & 0x3F));
                buf[2] = static_cast<char>(0x80 | ((lookup_cp >> 6) & 0x3F));
                buf[3] = static_cast<char>(0x80 | (lookup_cp & 0x3F));
                gi.utf8_cluster = std::string(buf, 4);
            }

            gi.lottie_char = assets.find_char(gi.utf8_cluster, font_family, font_style);
            if (gi.lottie_char) {
                gi.advance = gi.lottie_char->width * scale_100;
            } else if (ttf_font) {
                uint16_t gid = ttf_font->glyph_index(sc.codepoint);
                if (gid == 0 && sc.original_codepoint != 0) {
                    gid = ttf_font->glyph_index(sc.original_codepoint);
                }
                gi.advance = sc.is_mark ? 0.0f : ttf_font->glyph_advance(gid, doc.font_size);
            } else {
                gi.advance = (sc.codepoint == ' ') ? (doc.font_size * 0.3f) : (doc.font_size * 0.6f);
            }

            line_widths[li] += gi.advance;
            if (ci + 1 < shaped.size()) {
                line_widths[li] += tracking_px;
            }
            line_glyphs[li].push_back(std::move(gi));
        }
    }

    ctx.canvas.save();
    Transform global_tr = compute_global_transform(frame);
    ctx.canvas.concat(global_tr);

    for (size_t li = 0; li < lines.size(); ++li) {
        float x_cursor = doc.box_position.x;
        if (doc.justify == TextJustify::Right) {
            x_cursor += doc.box_size.x - line_widths[li];
        } else if (doc.justify == TextJustify::Center) {
            x_cursor += (doc.box_size.x - line_widths[li]) * 0.5f;
        }
        float y_cursor = doc.box_position.y + static_cast<float>(li) * doc.line_height - doc.baseline_shift;

        for (size_t ci = 0; ci < line_glyphs[li].size(); ++ci) {
            const auto& gi = line_glyphs[li][ci];

            // Accumulate TextAnimator overrides for this character
            Point anim_pos = Point::zero();
            Point anim_scale = Point::from_xy(1.0f, 1.0f);
            float anim_rot = 0.0f;
            float anim_op = 1.0f;
            Color fill_c = doc.fill_color;
            Color stroke_c = doc.stroke_color;
            float stroke_w = doc.stroke_width;
            float extra_tracking = 0.0f;

            for (const auto& anim : animators) {
                float factor = anim.selector.compute_factor(gi.global_char_idx, total_chars, frame);
                if (factor <= 1e-5f) continue;

                if (anim.has_position) {
                    Point p = anim.position.evaluate(frame);
                    anim_pos.x += p.x * factor;
                    anim_pos.y += p.y * factor;
                }
                if (anim.has_scale) {
                    Point s = anim.scale.evaluate(frame);
                    anim_scale.x *= 1.0f + ((s.x * 0.01f) - 1.0f) * factor;
                    anim_scale.y *= 1.0f + ((s.y * 0.01f) - 1.0f) * factor;
                }
                if (anim.has_rotation) {
                    anim_rot += anim.rotation.evaluate(frame) * factor;
                }
                if (anim.has_opacity) {
                    float target_op = std::clamp(anim.opacity.evaluate(frame) * 0.01f, 0.0f, 1.0f);
                    anim_op *= (1.0f - factor) + target_op * factor;
                }
                if (anim.has_fill_color) {
                    Color tc = anim.fill_color.evaluate(frame);
                    fill_c = Color::from_rgba(
                        fill_c.red() + (tc.red() - fill_c.red()) * factor,
                        fill_c.green() + (tc.green() - fill_c.green()) * factor,
                        fill_c.blue() + (tc.blue() - fill_c.blue()) * factor,
                        fill_c.alpha()
                    ).value_or(fill_c);
                }
                if (anim.has_stroke_color) {
                    Color tc = anim.stroke_color.evaluate(frame);
                    stroke_c = Color::from_rgba(
                        stroke_c.red() + (tc.red() - stroke_c.red()) * factor,
                        stroke_c.green() + (tc.green() - stroke_c.green()) * factor,
                        stroke_c.blue() + (tc.blue() - stroke_c.blue()) * factor,
                        stroke_c.alpha()
                    ).value_or(stroke_c);
                }
                if (anim.has_stroke_width) {
                    stroke_w += anim.stroke_width.evaluate(frame) * factor;
                }
                if (anim.has_tracking) {
                    extra_tracking += anim.tracking.evaluate(frame) * factor;
                }
            }

            float char_op = base_opacity * anim_op;
            if (char_op > 0.001f) {
                ctx.canvas.save();
                ctx.canvas.translate(x_cursor + anim_pos.x, y_cursor + anim_pos.y);
                if (std::abs(anim_rot) > 1e-4f) {
                    Transform r_tr = Transform::from_rotate(anim_rot);
                    ctx.canvas.concat(r_tr);
                }
                if (std::abs(anim_scale.x - 1.0f) > 1e-4f || std::abs(anim_scale.y - 1.0f) > 1e-4f) {
                    ctx.canvas.scale(anim_scale.x, anim_scale.y);
                }

                Path glyph_path;
                if (gi.lottie_char) {
                    for (const auto& sg : gi.lottie_char->shapes) {
                        if (sg) sg->collect_geometry(frame, glyph_path);
                    }
                    glyph_path.apply_transform(Transform::from_scale(scale_100, scale_100));
                } else if (ttf_font) {
                    uint16_t gid = ttf_font->glyph_index(gi.shaped_cp);
                    if (gid != 0) {
                        ttf_font->get_glyph_path(gid, glyph_path, doc.font_size);
                    }
                } else if (gi.shaped_cp != ' ' && gi.shaped_cp != '\t') {
                    // Procedural fallback glyph box so headless tests without external TTF files still render visible text geometry
                    float gw = gi.advance * 0.75f;
                    float gh = doc.font_size * 0.7f;
                    PathBuilder pb;
                    pb.move_to(0.0f, -gh);
                    pb.line_to(gw, -gh);
                    pb.line_to(gw, 0.0f);
                    pb.line_to(0.0f, 0.0f);
                    pb.close();
                    if (auto p = pb.finish()) glyph_path = std::move(*p);
                }

                if (!glyph_path.is_empty()) {
                    auto draw_fill = [&]() {
                        Color fc = fill_c;
                        fc.apply_opacity(char_op);
                        Paint fp(fc);
                        fp.blend_mode = blend_mode;
                        ctx.canvas.fill_path(glyph_path, fp, nisaba::FillRule::Winding);
                    };
                    auto draw_stroke = [&]() {
                        if (stroke_w > 0.0f) {
                            Color sc = stroke_c;
                            sc.apply_opacity(char_op);
                            Paint sp(sc);
                            sp.blend_mode = blend_mode;
                            Stroke st(stroke_w);
                            st.line_cap = nisaba::LineCap::Round;
                            st.line_join = nisaba::LineJoin::Round;
                            ctx.canvas.stroke_path(glyph_path, sp, st);
                        }
                    };

                    if (doc.stroke_over_fill) {
                        draw_fill();
                        draw_stroke();
                    } else {
                        draw_stroke();
                        draw_fill();
                    }
                }

                ctx.canvas.restore();
            }

            x_cursor += gi.advance + tracking_px + extra_tracking;
        }
    }

    ctx.canvas.restore();
}

bool TextLayer::hit_test(float x, float y, float frame, const LottieAssetRegistry&, std::string& out_hit_name) const {
    if (!is_active(frame)) return false;
    const TextDocument& doc = evaluate_document(frame);
    if (doc.text.empty() || doc.font_size <= 0.0f) return false;

    Transform global_tr = compute_global_transform(frame);
    auto inv = global_tr.invert();
    if (!inv) return false;
    Point pt = Point::from_xy(x, y);
    inv->map_points(&pt, 1);

    float approx_w = static_cast<float>(doc.text.size()) * doc.font_size * 0.6f;
    float approx_h = doc.font_size * 1.2f;
    float x0 = doc.box_position.x;
    if (doc.justify == TextJustify::Center) x0 -= approx_w * 0.5f;
    else if (doc.justify == TextJustify::Right) x0 -= approx_w;
    float y0 = doc.box_position.y - doc.font_size;

    if (pt.x >= x0 && pt.x <= x0 + approx_w && pt.y >= y0 && pt.y <= y0 + approx_h) {
        out_hit_name = name;
        return true;
    }
    return false;
}

std::shared_ptr<TextLayer> TextLayer::parse(const json::JsonValue& val) {
    auto layer = std::make_shared<TextLayer>();
    parse_common_layer_props(*layer, val);

    if (const auto* t_obj = val.get("t"); t_obj && t_obj->is_object()) {
        if (const auto* d_obj = t_obj->get("d"); d_obj && d_obj->is_object()) {
            if (const auto* k_val = d_obj->get("k")) {
                if (k_val->is_array()) {
                    for (const auto& kf_json : k_val->as_array()) {
                        if (!kf_json.is_object()) continue;
                        Keyframe<TextDocument> kf;
                        kf.time = kf_json.get_float("t", 0.0f);
                        kf.is_hold = true;
                        if (const auto* s_val = kf_json.get("s")) {
                            kf.start_value = TextDocument::parse(*s_val);
                        }
                        layer->keyframes.push_back(std::move(kf));
                    }
                    if (!layer->keyframes.empty()) {
                        layer->static_doc = layer->keyframes.front().start_value;
                    }
                } else if (k_val->is_object()) {
                    layer->static_doc = TextDocument::parse(*k_val);
                }
            }
        }
        if (const auto* a_arr = t_obj->get("a"); a_arr && a_arr->is_array()) {
            for (const auto& a_json : a_arr->as_array()) {
                if (a_json.is_object()) {
                    layer->animators.push_back(TextAnimator::parse(a_json));
                }
            }
        }
    }
    return layer;
}

// ── Compositing Helpers: Masks, Track Mattes, Effects, 3D ─────────────────

namespace {

Path build_expanded_mask_path(const Mask& m, float frame) {
    BezierData bz = m.shape.evaluate(frame);
    Path p = bz.to_path();
    if (p.is_empty()) return p;

    float exp = m.expansion.evaluate(frame);
    if (std::abs(exp) > 1e-3f) {
        Stroke st(std::abs(exp) * 2.0f);
        st.line_join = nisaba::LineJoin::Round;
        if (auto stroked = stroke_path(p, st, 1.0f)) {
            if (exp > 0.0f) {
                if (auto u = path_union(p, *stroked)) p = std::move(*u);
            } else {
                if (auto d = path_difference(p, *stroked)) p = std::move(*d);
            }
        }
    }
    return p;
}

std::optional<Path> combine_layer_masks_vector(const LottieLayer& layer, float frame, float comp_w, float comp_h) {
    if (layer.masks.empty()) return std::nullopt;

    Path accum;
    bool has_accum = false;

    Path full_comp;
    {
        PathBuilder pb;
        pb.move_to(-comp_w, -comp_h);
        pb.line_to(comp_w * 2.0f, -comp_h);
        pb.line_to(comp_w * 2.0f, comp_h * 2.0f);
        pb.line_to(-comp_w, comp_h * 2.0f);
        pb.close();
        if (auto fp = pb.finish()) full_comp = std::move(*fp);
    }

    for (const auto& m : layer.masks) {
        if (m.mode == MaskMode::None) continue;
        float op = m.opacity.evaluate(frame);
        if (op <= 0.0f && m.mode == MaskMode::Add) continue;

        Path mp = build_expanded_mask_path(m, frame);
        if (mp.is_empty()) continue;

        if (m.inverted) {
            if (auto inv_p = path_difference(full_comp, mp)) {
                mp = std::move(*inv_p);
            }
        }

        if (!has_accum) {
            if (m.mode == MaskMode::Subtract) {
                if (auto diff = path_difference(full_comp, mp)) {
                    accum = std::move(*diff);
                } else {
                    accum = std::move(mp);
                }
            } else {
                accum = std::move(mp);
            }
            has_accum = true;
            continue;
        }

        switch (m.mode) {
            case MaskMode::Add:
                if (auto u = path_union(accum, mp)) accum = std::move(*u);
                else accum.add_path(mp);
                break;
            case MaskMode::Subtract:
                if (auto d = path_difference(accum, mp)) accum = std::move(*d);
                break;
            case MaskMode::Intersect:
                if (auto in = path_intersect(accum, mp)) accum = std::move(*in);
                break;
            case MaskMode::Difference:
                if (auto x = path_xor(accum, mp)) accum = std::move(*x);
                break;
            default:
                break;
        }
    }

    if (!has_accum) return std::nullopt;
    Transform global_tr = layer.compute_global_transform(frame);
    accum.apply_transform(global_tr);
    return accum;
}

void apply_layer_effects_to_pixmap(Pixmap& pm, const std::vector<LayerEffect>& effects, float frame) {
    if (effects.empty() || pm.width() == 0 || pm.height() == 0) return;

    for (const auto& eff : effects) {
        if (!eff.enabled) continue;
        switch (eff.type) {
            case EffectType::GaussianBlur: {
                float blur_val = eff.blurriness.evaluate(frame);
                if (blur_val > 0.1f) {
                    float sigma = blur_val * 0.35f;
                    PixmapMut mut = pm.as_mut();
                    effects::gaussian_blur_pixmap(mut, sigma);
                }
                break;
            }
            case EffectType::DropShadow: {
                Color sc = eff.shadow_color.evaluate(frame);
                float sop = eff.shadow_opacity.evaluate(frame);
                // In AE Drop Shadow, opacity can be [0..255] or [0..100]
                float alpha_scale = (sop > 100.0f) ? (sop / 255.0f) : (sop / 100.0f);
                sc.apply_opacity(std::clamp(alpha_scale, 0.0f, 1.0f));

                float angle_rad = (eff.shadow_direction.evaluate(frame) - 90.0f) * static_cast<float>(M_PI / 180.0);
                float dist = eff.shadow_distance.evaluate(frame);
                int32_t dx = static_cast<int32_t>(std::round(std::cos(angle_rad) * dist));
                int32_t dy = static_cast<int32_t>(std::round(std::sin(angle_rad) * dist));
                float sigma = std::max(0.0f, eff.shadow_softness.evaluate(frame) * 0.4f);

                nisaba::Mask shadow_mask = nisaba::Mask::from_pixmap(pm.as_ref(), MaskType::Alpha);
                if (sigma > 0.2f) {
                    effects::gaussian_blur_mask(shadow_mask, sigma);
                }

                auto composite = Pixmap::allocate(pm.width(), pm.height());
                if (composite) {
                    PixmapMut dst_mut = composite->as_mut();
                    effects::draw_shadow_mask(dst_mut, dx, dy, shadow_mask.as_submask(), sc);
                    Canvas comp_canvas(*composite);
                    comp_canvas.draw_pixmap(0, 0, pm.as_ref(), PixmapPaint{});
                    pm = std::move(*composite);
                }
                break;
            }
            case EffectType::Fill: {
                Color fc = eff.shadow_color.evaluate(frame);
                float fop = eff.shadow_opacity.evaluate(frame);
                if (fop > 1.0f) fop *= 0.01f;
                fop = std::clamp(fop, 0.0f, 1.0f);

                // Replace RGB with fc while preserving pixel alpha * fop
                std::array<float, 20> mat = {
                    0.0f, 0.0f, 0.0f, 0.0f, fc.red(),
                    0.0f, 0.0f, 0.0f, 0.0f, fc.green(),
                    0.0f, 0.0f, 0.0f, 0.0f, fc.blue(),
                    0.0f, 0.0f, 0.0f, fop,  0.0f
                };
                effects::ColorMatrix cm(mat);
                cm.apply(pm);
                break;
            }
            case EffectType::Tint:
            case EffectType::Tritone: {
                Color cb = eff.tint_black.evaluate(frame);
                Color cw = eff.tint_white.evaluate(frame);
                float amt = std::clamp(eff.tint_amount.evaluate(frame) * 0.01f, 0.0f, 1.0f);
                if (amt > 1e-4f) {
                    // Map luminance Y = 0.2126 R + 0.7152 G + 0.0722 B between cb and cw
                    float dr = cw.red() - cb.red();
                    float dg = cw.green() - cb.green();
                    float db = cw.blue() - cb.blue();
                    float inv = 1.0f - amt;
                    std::array<float, 20> mat = {
                        inv + amt * dr * 0.2126f, amt * dr * 0.7152f,       amt * dr * 0.0722f,       0.0f, amt * cb.red(),
                        amt * dg * 0.2126f,       inv + amt * dg * 0.7152f, amt * dg * 0.0722f,       0.0f, amt * cb.green(),
                        amt * db * 0.2126f,       amt * db * 0.7152f,       inv + amt * db * 0.0722f, 0.0f, amt * cb.blue(),
                        0.0f,                     0.0f,                     0.0f,                     1.0f, 0.0f
                    };
                    effects::ColorMatrix cm(mat);
                    cm.apply(pm);
                }
                break;
            }
            default:
                break;
        }
    }
}

void apply_track_matte_to_pixmap(Pixmap& layer_pm, const Pixmap& matte_pm, MatteType matte_type) {
    if (matte_type == MatteType::None) return;
    uint32_t w = std::min(layer_pm.width(), matte_pm.width());
    uint32_t h = std::min(layer_pm.height(), matte_pm.height());
    if (w == 0 || h == 0) return;

    MaskType mtype = (matte_type == MatteType::Luma || matte_type == MatteType::InvertedLuma)
        ? MaskType::Luminance
        : MaskType::Alpha;

    nisaba::Mask mask = nisaba::Mask::from_pixmap(matte_pm.as_ref(), mtype);
    if (matte_type == MatteType::InvertedAlpha || matte_type == MatteType::InvertedLuma) {
        mask.invert();
    }

    PixmapMut mut = layer_pm.as_mut();
    for (uint32_t y = 0; y < h; ++y) {
        PremultipliedColorU8* row = mut.row(y);
        for (uint32_t x = 0; x < w; ++x) {
            uint8_t ma = mask.get(x, y);
            if (ma == 0) {
                row[x] = PremultipliedColorU8::TRANSPARENT;
            } else if (ma < 255) {
                uint8_t r = static_cast<uint8_t>((row[x].red() * ma + 127) / 255);
                uint8_t g = static_cast<uint8_t>((row[x].green() * ma + 127) / 255);
                uint8_t b = static_cast<uint8_t>((row[x].blue() * ma + 127) / 255);
                uint8_t a = static_cast<uint8_t>((row[x].alpha() * ma + 127) / 255);
                row[x] = PremultipliedColorU8::from_rgba_unchecked(r, g, b, a);
            }
        }
    }
}

void render_single_layer_complete(
    ICanvas& canvas,
    const LottieLayer& layer,
    float frame,
    const LottieAssetRegistry& assets,
    float parent_opacity
) {
    bool needs_matte = (layer.matte_type != MatteType::None && layer.matte_layer_ptr != nullptr);
    bool needs_effects = !layer.effects.empty();
    bool needs_3d = layer.has_3d_hierarchy();
    bool has_feathered_mask = false;
    for (const auto& m : layer.masks) {
        Point f = m.feather.evaluate(frame);
        if (f.x > 0.1f || f.y > 0.1f) {
            has_feathered_mask = true;
            break;
        }
    }

    if (!needs_matte && !needs_effects && !needs_3d && !has_feathered_mask) {
        // Fast direct vector rendering path (0 offscreen allocation)
        bool has_masks = !layer.masks.empty();
        if (has_masks) {
            canvas.save();
            if (auto mask_path = combine_layer_masks_vector(layer, frame, assets.comp_width, assets.comp_height)) {
                Path device_mask = *mask_path;
                device_mask.apply_transform(canvas.current_transform());
                canvas.clip_path(device_mask, nisaba::FillRule::Winding);
            }
        }

        RenderContext ctx(canvas, parent_opacity);
        layer.render_with_assets(ctx, frame, assets);

        if (has_masks) {
            canvas.restore();
        }
        return;
    }

    // Offscreen compositing path for Track Mattes, Layer Effects, Feathered Masks, or 3D Layers
    uint32_t off_w = static_cast<uint32_t>(std::max(1.0f, std::ceil(assets.comp_width)));
    uint32_t off_h = static_cast<uint32_t>(std::max(1.0f, std::ceil(assets.comp_height)));
    if (off_w > 4096) off_w = 4096;
    if (off_h > 4096) off_h = 4096;

    auto layer_pm_opt = Pixmap::allocate(off_w, off_h);
    if (!layer_pm_opt) return;
    Pixmap& layer_pm = *layer_pm_opt;

    {
        Canvas off_canvas(layer_pm);
        if (!layer.masks.empty() && !has_feathered_mask) {
            if (auto mask_path = combine_layer_masks_vector(layer, frame, assets.comp_width, assets.comp_height)) {
                off_canvas.clip_path(*mask_path, nisaba::FillRule::Winding);
            }
        }
        RenderContext off_ctx(off_canvas, needs_3d ? 1.0f : parent_opacity);
        layer.render_with_assets(off_ctx, frame, assets);
    }

    // Apply feathered masks if present
    if (has_feathered_mask) {
        if (auto mask_path = combine_layer_masks_vector(layer, frame, assets.comp_width, assets.comp_height)) {
            if (auto m_alloc = nisaba::Mask::allocate(off_w, off_h)) {
                m_alloc->fill_path(*mask_path, nisaba::FillRule::Winding, true);
                float max_f = 0.0f;
                for (const auto& m : layer.masks) {
                    Point f = m.feather.evaluate(frame);
                    max_f = std::max({max_f, f.x, f.y});
                }
                if (max_f > 0.2f) {
                    effects::gaussian_blur_mask(*m_alloc, max_f * 0.35f);
                }
                PixmapMut mut = layer_pm.as_mut();
                for (uint32_t y = 0; y < off_h; ++y) {
                    PremultipliedColorU8* row = mut.row(y);
                    for (uint32_t x = 0; x < off_w; ++x) {
                        uint8_t ma = m_alloc->get(x, y);
                        if (ma < 255) {
                            row[x] = PremultipliedColorU8::from_rgba_unchecked(
                                static_cast<uint8_t>((row[x].red() * ma + 127) / 255),
                                static_cast<uint8_t>((row[x].green() * ma + 127) / 255),
                                static_cast<uint8_t>((row[x].blue() * ma + 127) / 255),
                                static_cast<uint8_t>((row[x].alpha() * ma + 127) / 255)
                            );
                        }
                    }
                }
            }
        }
    }

    // Apply Layer Effects (Blur, DropShadow, Fill, Tint, Tritone)
    if (needs_effects) {
        apply_layer_effects_to_pixmap(layer_pm, layer.effects, frame);
    }

    // Apply Track Matte if active
    if (needs_matte && layer.matte_layer_ptr) {
        const LottieLayer& matte_layer = *layer.matte_layer_ptr;
        if (frame >= matte_layer.in_point && frame < matte_layer.out_point) {
            if (auto matte_pm = Pixmap::allocate(off_w, off_h)) {
                Canvas matte_canvas(*matte_pm);
                RenderContext matte_ctx(matte_canvas, 1.0f);
                // Temporarily render matte layer even if hidden/is_matte_target
                bool saved_hidden = matte_layer.hidden;
                const_cast<LottieLayer&>(matte_layer).hidden = false;
                matte_layer.render_with_assets(matte_ctx, frame, assets);
                const_cast<LottieLayer&>(matte_layer).hidden = saved_hidden;

                apply_track_matte_to_pixmap(layer_pm, *matte_pm, layer.matte_type);
            }
        } else {
            // Matte layer is inactive at this frame
            if (layer.matte_type == MatteType::Alpha || layer.matte_type == MatteType::Luma) {
                return; // Clipped out completely
            }
        }
    }

    // Composite onto target canvas (either 2D or 3D projective)
    PixmapPaint paint;
    paint.blend_mode = layer.blend_mode;
    paint.quality = FilterQuality::Bilinear;
    paint.opacity = needs_3d ? parent_opacity : 1.0f;

    if (needs_3d) {
        Transform tr2d = layer.compute_global_transform(frame);
        Transform4x4 tr3d = layer.compute_global_transform_3d(frame, assets.comp_width, assets.comp_height);
        if (auto inv2d = tr2d.invert()) {
            Transform4x4 undo2d(
                inv2d->sx, inv2d->kx, 0.0f, inv2d->tx,
                inv2d->ky, inv2d->sy, 0.0f, inv2d->ty,
                0.0f,      0.0f,      1.0f, 0.0f,
                0.0f,      0.0f,      0.0f, 1.0f
            );
            canvas.draw_pixmap_3d(layer_pm.as_ref(), tr3d * undo2d, paint.opacity, paint.blend_mode);
            return;
        }
    }

    canvas.draw_pixmap(0, 0, layer_pm.as_ref(), paint);
}

} // anonymous namespace

// ── Layer Tree Renderer ───────────────────────────────────────────────────

void render_layer_tree_with_assets(
    ICanvas& canvas,
    const std::vector<std::shared_ptr<LottieLayer>>& layers,
    float frame,
    const LottieAssetRegistry& assets,
    float parent_opacity) {

    if (layers.empty()) return;

    // Ensure hierarchy pointers are linked if caller created layers manually
    bool needs_linking = false;
    for (const auto& l : layers) {
        if (l && ((l->parent_index >= 0 && !l->parent_ptr) || (l->matte_type != MatteType::None && !l->matte_layer_ptr))) {
            needs_linking = true;
            break;
        }
    }
    if (needs_linking) {
        auto mutable_copy = layers;
        link_layer_hierarchy(mutable_copy);
    }

    // In Bodymovin JSON, layers are ordered from top to bottom.
    // To render back-to-front (painter's algorithm), traverse backwards from N-1 down to 0.
    for (size_t i = layers.size(); i > 0; --i) {
        const auto& layer = layers[i - 1];
        if (!layer || !layer->is_active(frame)) continue;
        if (layer->is_matte_target) continue; // Matte sources are rendered by their matte consumers

        render_single_layer_complete(canvas, *layer, frame, assets, parent_opacity);
    }
}

void render_layer_tree(
    ICanvas& canvas,
    const std::vector<std::shared_ptr<LottieLayer>>& layers,
    float frame,
    const std::unordered_map<std::string, std::vector<std::shared_ptr<LottieLayer>>>& precomp_assets,
    float parent_opacity) {

    LottieAssetRegistry assets;
    assets.precomps = precomp_assets;
    assets.comp_width = static_cast<float>(canvas.canvas_width() > 0 ? canvas.canvas_width() : 512);
    assets.comp_height = static_cast<float>(canvas.canvas_height() > 0 ? canvas.canvas_height() : 512);
    render_layer_tree_with_assets(canvas, layers, frame, assets, parent_opacity);
}

} // namespace nisaba::lottie
