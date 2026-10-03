#include "nisaba/svg/svg_document.hpp"
#include "nisaba/svg/path_parser.hpp"
#include "nisaba/canvas/canvas.hpp"
#include "nisaba/image/image_io.hpp"
#include <fstream>
#include <sstream>
#include <map>
#include <cctype>
#include <cmath>
#include <algorithm>

namespace nisaba::svg {

namespace {

std::string_view trim(std::string_view s) noexcept {
    while (!s.empty() && std::isspace(static_cast<unsigned char>(s.front()))) s.remove_prefix(1);
    while (!s.empty() && std::isspace(static_cast<unsigned char>(s.back()))) s.remove_suffix(1);
    return s;
}

float parse_float(std::string_view s, float default_val = 0.0f) noexcept {
    s = trim(s);
    if (s.empty()) return default_val;
    if (s.ends_with("px") || s.ends_with("pt")) {
        s.remove_suffix(2);
    }
    char* end = nullptr;
    std::string str(s);
    float val = std::strtof(str.c_str(), &end);
    return (end != str.c_str()) ? val : default_val;
}

float parse_coord(std::string_view s, float default_val = 0.0f) noexcept {
    s = trim(s);
    if (s.empty()) return default_val;
    if (s.back() == '%') {
        float pct = parse_float(s.substr(0, s.size() - 1), default_val * 100.0f);
        return pct / 100.0f;
    }
    return parse_float(s, default_val);
}

std::string extract_url_id(std::string_view v) {
    v = trim(v);
    if (v.starts_with("url(") && v.ends_with(")")) {
        v = v.substr(4, v.size() - 5);
        v = trim(v);
        if (!v.empty() && (v.front() == '\'' || v.front() == '"')) {
            v.remove_prefix(1);
        }
        if (!v.empty() && (v.back() == '\'' || v.back() == '"')) {
            v.remove_suffix(1);
        }
        v = trim(v);
        if (v.starts_with("#")) {
            v.remove_prefix(1);
        }
        return std::string(v);
    }
    if (v.starts_with("#")) {
        return std::string(v.substr(1));
    }
    return std::string(v);
}

std::vector<uint8_t> base64_decode(std::string_view input) {
    std::vector<uint8_t> out;
    out.reserve((input.size() * 3) / 4);

    auto decode_char = [](char c) -> int {
        if (c >= 'A' && c <= 'Z') return c - 'A';
        if (c >= 'a' && c <= 'z') return c - 'a' + 26;
        if (c >= '0' && c <= '9') return c - '0' + 52;
        if (c == '+' || c == '-') return 62;
        if (c == '/' || c == '_') return 63;
        return -1;
    };

    uint32_t val = 0;
    int valb = -8;
    for (char c : input) {
        if (std::isspace(static_cast<unsigned char>(c))) continue;
        if (c == '=') break; // padding
        int d = decode_char(c);
        if (d < 0) continue;
        val = (val << 6) | static_cast<uint32_t>(d);
        valb += 6;
        if (valb >= 0) {
            out.push_back(static_cast<uint8_t>((val >> valb) & 0xFF));
            valb -= 8;
        }
    }
    return out;
}

std::optional<Color> parse_color(std::string_view str) noexcept {
    str = trim(str);
    if (str.empty() || str == "none" || str == "transparent") {
        return std::nullopt;
    }
    if (str == "currentColor" || str == "inherit") {
        return Color::BLACK;
    }

    if (str.front() == '#') {
        std::string_view hex = str.substr(1);
        if (hex.size() == 3) {
            auto h = [](char c) -> uint8_t {
                if (c >= '0' && c <= '9') return static_cast<uint8_t>(c - '0');
                if (c >= 'a' && c <= 'f') return static_cast<uint8_t>(c - 'a' + 10);
                if (c >= 'A' && c <= 'F') return static_cast<uint8_t>(c - 'A' + 10);
                return 0;
            };
            uint8_t r = h(hex[0]) * 17;
            uint8_t g = h(hex[1]) * 17;
            uint8_t b = h(hex[2]) * 17;
            return Color::from_rgba8(r, g, b, 255);
        }
        if (hex.size() == 6 || hex.size() == 8) {
            char* end = nullptr;
            std::string hstr(hex);
            uint32_t val = static_cast<uint32_t>(std::strtoul(hstr.c_str(), &end, 16));
            if (hex.size() == 6) {
                uint8_t r = static_cast<uint8_t>((val >> 16) & 0xFF);
                uint8_t g = static_cast<uint8_t>((val >> 8) & 0xFF);
                uint8_t b = static_cast<uint8_t>(val & 0xFF);
                return Color::from_rgba8(r, g, b, 255);
            } else {
                uint8_t r = static_cast<uint8_t>((val >> 24) & 0xFF);
                uint8_t g = static_cast<uint8_t>((val >> 16) & 0xFF);
                uint8_t b = static_cast<uint8_t>((val >> 8) & 0xFF);
                uint8_t a = static_cast<uint8_t>(val & 0xFF);
                return Color::from_rgba8(r, g, b, a);
            }
        }
    }

    if (str.starts_with("rgb(") && str.ends_with(")")) {
        auto inner = str.substr(4, str.size() - 5);
        float r = 0, g = 0, b = 0;
        size_t c1 = inner.find(',');
        size_t c2 = (c1 != std::string_view::npos) ? inner.find(',', c1 + 1) : std::string_view::npos;
        if (c1 != std::string_view::npos && c2 != std::string_view::npos) {
            r = parse_float(inner.substr(0, c1));
            g = parse_float(inner.substr(c1 + 1, c2 - c1 - 1));
            b = parse_float(inner.substr(c2 + 1));
            return Color::from_rgba8(static_cast<uint8_t>(r), static_cast<uint8_t>(g), static_cast<uint8_t>(b), 255);
        }
    }

    if (str.starts_with("rgba(") && str.ends_with(")")) {
        auto inner = str.substr(5, str.size() - 6);
        float r = 0, g = 0, b = 0, a = 1.0f;
        size_t c1 = inner.find(',');
        size_t c2 = (c1 != std::string_view::npos) ? inner.find(',', c1 + 1) : std::string_view::npos;
        size_t c3 = (c2 != std::string_view::npos) ? inner.find(',', c2 + 1) : std::string_view::npos;
        if (c1 != std::string_view::npos && c2 != std::string_view::npos && c3 != std::string_view::npos) {
            r = parse_float(inner.substr(0, c1));
            g = parse_float(inner.substr(c1 + 1, c2 - c1 - 1));
            b = parse_float(inner.substr(c2 + 1, c3 - c2 - 1));
            a = parse_float(inner.substr(c3 + 1));
            return Color::from_rgba8(
                static_cast<uint8_t>(r),
                static_cast<uint8_t>(g),
                static_cast<uint8_t>(b),
                static_cast<uint8_t>(a * 255.0f + 0.5f)
            );
        }
    }

    // Named colors
    if (str == "black") return Color::BLACK;
    if (str == "white") return Color::WHITE;
    if (str == "red") return Color::from_rgba8(255, 0, 0, 255);
    if (str == "green") return Color::from_rgba8(0, 128, 0, 255);
    if (str == "lime") return Color::from_rgba8(0, 255, 0, 255);
    if (str == "blue") return Color::from_rgba8(0, 0, 255, 255);
    if (str == "yellow") return Color::from_rgba8(255, 255, 0, 255);
    if (str == "cyan") return Color::from_rgba8(0, 255, 255, 255);
    if (str == "magenta") return Color::from_rgba8(255, 0, 255, 255);
    if (str == "gray" || str == "grey") return Color::from_rgba8(128, 128, 128, 255);
    if (str == "orange") return Color::from_rgba8(255, 165, 0, 255);
    if (str == "purple") return Color::from_rgba8(128, 0, 128, 255);
    if (str == "gold") return Color::from_rgba8(255, 215, 0, 255);
    if (str == "silver") return Color::from_rgba8(192, 192, 192, 255);

    return Color::BLACK;
}

Transform parse_transform(std::string_view str) noexcept {
    Transform ts;
    size_t pos = 0;
    while (pos < str.size()) {
        size_t open_p = str.find('(', pos);
        if (open_p == std::string_view::npos) break;
        size_t close_p = str.find(')', open_p);
        if (close_p == std::string_view::npos) break;

        std::string_view name = trim(str.substr(pos, open_p - pos));
        std::string_view args_str = str.substr(open_p + 1, close_p - open_p - 1);

        std::vector<float> args;
        size_t a_pos = 0;
        while (a_pos < args_str.size()) {
            while (a_pos < args_str.size() && (std::isspace(args_str[a_pos]) || args_str[a_pos] == ',')) a_pos++;
            if (a_pos >= args_str.size()) break;
            size_t next_comma = args_str.find_first_of(" ,\t\r\n", a_pos);
            if (next_comma == std::string_view::npos) next_comma = args_str.size();
            args.push_back(parse_float(args_str.substr(a_pos, next_comma - a_pos)));
            a_pos = next_comma;
        }

        if (name.ends_with("translate") && !args.empty()) {
            float tx = args[0];
            float ty = (args.size() > 1) ? args[1] : 0.0f;
            ts = ts.pre_translate(tx, ty);
        } else if (name.ends_with("scale") && !args.empty()) {
            float sx = args[0];
            float sy = (args.size() > 1) ? args[1] : sx;
            ts = ts.pre_scale(sx, sy);
        } else if (name.ends_with("rotate") && !args.empty()) {
            float deg = args[0];
            if (args.size() >= 3) {
                float cx = args[1], cy = args[2];
                ts = ts.pre_translate(cx, cy).pre_rotate(deg).pre_translate(-cx, -cy);
            } else {
                ts = ts.pre_rotate(deg);
            }
        } else if (name.ends_with("skewX") && !args.empty()) {
            float rad = args[0] * static_cast<float>(M_PI) / 180.0f;
            ts = ts.pre_concat(Transform(1.0f, 0.0f, std::tan(rad), 1.0f, 0.0f, 0.0f));
        } else if (name.ends_with("skewY") && !args.empty()) {
            float rad = args[0] * static_cast<float>(M_PI) / 180.0f;
            ts = ts.pre_concat(Transform(1.0f, std::tan(rad), 0.0f, 1.0f, 0.0f, 0.0f));
        } else if (name.ends_with("matrix") && args.size() >= 6) {
            Transform m(args[0], args[1], args[2], args[3], args[4], args[5]);
            ts = ts.pre_concat(m);
        }

        pos = close_p + 1;
    }
    return ts;
}

std::map<std::string, std::string> parse_attributes(std::string_view tag_content) {
    std::map<std::string, std::string> attrs;
    size_t pos = 0;
    while (pos < tag_content.size()) {
        while (pos < tag_content.size() && std::isspace(tag_content[pos])) pos++;
        if (pos >= tag_content.size()) break;

        size_t eq_pos = tag_content.find('=', pos);
        if (eq_pos == std::string_view::npos) break;

        std::string_view key = trim(tag_content.substr(pos, eq_pos - pos));
        pos = eq_pos + 1;
        while (pos < tag_content.size() && std::isspace(tag_content[pos])) pos++;
        if (pos >= tag_content.size()) break;

        char quote = tag_content[pos];
        if (quote == '"' || quote == '\'') {
            pos++;
            size_t close_q = tag_content.find(quote, pos);
            if (close_q == std::string_view::npos) break;
            attrs[std::string(key)] = std::string(tag_content.substr(pos, close_q - pos));
            pos = close_q + 1;
        } else {
            size_t val_end = tag_content.find_first_of(" \t\r\n>", pos);
            if (val_end == std::string_view::npos) val_end = tag_content.size();
            attrs[std::string(key)] = std::string(tag_content.substr(pos, val_end - pos));
            pos = val_end;
        }
    }
    return attrs;
}

void apply_style_string(SvgPaintStyle& style, std::string_view style_str) {
    size_t pos = 0;
    while (pos < style_str.size()) {
        size_t sc = style_str.find(';', pos);
        if (sc == std::string_view::npos) sc = style_str.size();
        std::string_view prop = trim(style_str.substr(pos, sc - pos));
        pos = sc + 1;

        size_t colon = prop.find(':');
        if (colon != std::string_view::npos) {
            std::string_view k = trim(prop.substr(0, colon));
            std::string_view v = trim(prop.substr(colon + 1));

            if (k == "fill") {
                if (v.starts_with("url(") || v.starts_with("url")) {
                    style.fill_url = extract_url_id(v);
                    style.fill = std::nullopt;
                } else {
                    style.fill = parse_color(v);
                    style.fill_url.clear();
                }
            } else if (k == "stroke") {
                if (v.starts_with("url(") || v.starts_with("url")) {
                    style.stroke_url = extract_url_id(v);
                    style.stroke = std::nullopt;
                } else {
                    style.stroke = parse_color(v);
                    style.stroke_url.clear();
                }
            } else if (k == "stroke-width") {
                style.stroke_width = parse_float(v, 1.0f);
            } else if (k == "opacity") {
                style.opacity = parse_float(v, 1.0f);
            } else if (k == "fill-opacity") {
                style.fill_opacity = parse_float(v, 1.0f);
            } else if (k == "stroke-opacity") {
                style.stroke_opacity = parse_float(v, 1.0f);
            } else if (k == "stroke-linecap") {
                if (v == "round") style.line_cap = LineCap::Round;
                else if (v == "square") style.line_cap = LineCap::Square;
                else style.line_cap = LineCap::Butt;
            } else if (k == "stroke-linejoin") {
                if (v == "round") style.line_join = LineJoin::Round;
                else if (v == "bevel") style.line_join = LineJoin::Bevel;
                else style.line_join = LineJoin::Miter;
            } else if (k == "fill-rule") {
                if (v == "evenodd") style.fill_rule = FillRule::EvenOdd;
                else style.fill_rule = FillRule::Winding;
            } else if (k == "clip-path") {
                style.clip_path_url = extract_url_id(v);
            } else if (k == "mask") {
                style.mask_url = extract_url_id(v);
            }
        }
    }
}

Shader create_shader_from_gradient(
    const SvgGradient& grad,
    const Rect& bounds,
    float opacity,
    std::optional<Color> tint
) {
    if (grad.stops.empty()) {
        return Shader::from_color(Color::TRANSPARENT);
    }

    std::vector<GradientStop> stops = grad.stops;
    if (stops.size() == 1) {
        stops.push_back(stops[0]);
        stops[0].position = NormalizedF32::ZERO;
        stops[1].position = NormalizedF32::ONE;
    }

    for (auto& s : stops) {
        if (tint.has_value()) {
            s.color = *tint;
        }
        if (opacity < 1.0f) {
            s.color.apply_opacity(opacity);
        }
    }

    if (grad.kind == SvgGradient::Kind::Linear) {
        Point p0, p1;
        if (grad.units == SvgGradientUnits::ObjectBoundingBox) {
            float bx = bounds.left();
            float by = bounds.top();
            float bw = std::max(bounds.width(), 1.0f);
            float bh = std::max(bounds.height(), 1.0f);
            p0 = Point::from_xy(bx + grad.x1 * bw, by + grad.y1 * bh);
            p1 = Point::from_xy(bx + grad.x2 * bw, by + grad.y2 * bh);
        } else { // UserSpaceOnUse
            p0 = Point::from_xy(grad.x1, grad.y1);
            p1 = Point::from_xy(grad.x2, grad.y2);
        }

        if (std::abs(p1.x - p0.x) < 1e-5f && std::abs(p1.y - p0.y) < 1e-5f) {
            p1.x += 0.001f;
        }

        auto lin = LinearGradient::create(p0, p1, stops, grad.spread_mode, grad.gradient_transform);
        if (lin) {
            return Shader(*lin);
        }
    } else { // Radial
        Point center;
        float radius = 0.0f;
        if (grad.units == SvgGradientUnits::ObjectBoundingBox) {
            float bx = bounds.left();
            float by = bounds.top();
            float bw = std::max(bounds.width(), 1.0f);
            float bh = std::max(bounds.height(), 1.0f);
            center = Point::from_xy(bx + grad.cx * bw, by + grad.cy * bh);
            radius = grad.r * std::max(bw, bh);
        } else { // UserSpaceOnUse
            center = Point::from_xy(grad.cx, grad.cy);
            radius = grad.r;
        }

        if (radius <= 0.0f) radius = 0.001f;

        auto rad = RadialGradient::create(center, radius, stops, grad.spread_mode, grad.gradient_transform);
        if (rad) {
            return Shader(*rad);
        }
    }

    return Shader::from_color(stops.empty() ? Color::BLACK : stops.front().color);
}

struct SvgUseInstance {
    std::string target_id;
    float x{0.0f};
    float y{0.0f};
    SvgPaintStyle style;
    Transform transform;
    bool in_defs{false};
    bool in_mask{false};
    std::string group_id{};
    std::string mask_id{};
};

enum class ScopeType : uint8_t {
    Svg,
    Defs,
    Group,
    ClipPath,
    Mask,
    LinearGradient,
    RadialGradient,
};

struct ParseScope {
    ScopeType type{ScopeType::Svg};
    std::string id{};
    SvgPaintStyle style{};
    Transform transform{};
    std::optional<SvgGradient> gradient{};
    Path clip_path{};
    std::vector<SvgElement> mask_elements{};
};

} // namespace

std::optional<SvgDocument> SvgDocument::parse(std::string_view xml) {
    SvgDocument doc;

    size_t pos = 0;
    std::vector<ParseScope> scope_stack;
    scope_stack.push_back(ParseScope{ .type = ScopeType::Svg });

    std::vector<SvgUseInstance> use_instances;

    auto in_defs = [&]() -> bool {
        for (const auto& s : scope_stack) {
            if (s.type == ScopeType::Defs) return true;
        }
        return false;
    };

    auto current_mask_scope = [&]() -> ParseScope* {
        for (auto it = scope_stack.rbegin(); it != scope_stack.rend(); ++it) {
            if (it->type == ScopeType::Mask) return &(*it);
        }
        return nullptr;
    };

    auto current_clip_scope = [&]() -> ParseScope* {
        for (auto it = scope_stack.rbegin(); it != scope_stack.rend(); ++it) {
            if (it->type == ScopeType::ClipPath) return &(*it);
        }
        return nullptr;
    };

    auto current_gradient_scope = [&]() -> ParseScope* {
        for (auto it = scope_stack.rbegin(); it != scope_stack.rend(); ++it) {
            if (it->type == ScopeType::LinearGradient || it->type == ScopeType::RadialGradient) {
                return &(*it);
            }
        }
        return nullptr;
    };

    while (pos < xml.size()) {
        size_t tag_start = xml.find('<', pos);
        if (tag_start == std::string_view::npos) break;

        // Skip comments or CDATA
        if (xml.substr(tag_start, 4) == "<!--") {
            size_t c_end = xml.find("-->", tag_start + 4);
            pos = (c_end != std::string_view::npos) ? c_end + 3 : xml.size();
            continue;
        }

        size_t tag_end = xml.find('>', tag_start);
        if (tag_end == std::string_view::npos) break;

        std::string_view full_tag = xml.substr(tag_start + 1, tag_end - tag_start - 1);
        pos = tag_end + 1;
        full_tag = trim(full_tag);

        if (full_tag.empty() || full_tag.front() == '?' || full_tag.front() == '!') continue;

        bool is_closing = (full_tag.front() == '/');
        if (is_closing) {
            std::string_view close_name = trim(full_tag.substr(1));
            if (!scope_stack.empty()) {
                auto& top = scope_stack.back();
                if ((close_name == "linearGradient" || close_name == "radialGradient") && top.gradient) {
                    if (!top.gradient->id.empty()) {
                        doc.add_gradient(std::move(*top.gradient));
                    }
                    scope_stack.pop_back();
                } else if (close_name == "clipPath") {
                    if (!top.id.empty()) {
                        doc.add_clip_path(top.id, std::move(top.clip_path));
                    }
                    scope_stack.pop_back();
                } else if (close_name == "mask") {
                    if (!top.id.empty()) {
                        doc.add_mask(SvgMask{ top.id, std::move(top.mask_elements) });
                    }
                    scope_stack.pop_back();
                } else if (close_name == "defs" || close_name == "g" || close_name == "svg") {
                    if (scope_stack.size() > 1) {
                        scope_stack.pop_back();
                    }
                }
            }
            continue;
        }

        bool is_self_closing = false;
        if (!full_tag.empty() && full_tag.back() == '/') {
            is_self_closing = true;
            full_tag.remove_suffix(1);
            full_tag = trim(full_tag);
        }

        size_t space_pos = full_tag.find_first_of(" \t\r\n/");
        std::string_view tag_name = (space_pos != std::string_view::npos)
            ? full_tag.substr(0, space_pos)
            : full_tag;

        std::string_view attr_str = (space_pos != std::string_view::npos)
            ? full_tag.substr(space_pos)
            : std::string_view();

        auto attrs = parse_attributes(attr_str);
        const auto& cur_scope = scope_stack.back();

        if (tag_name == "svg") {
            if (attrs.count("viewBox")) {
                std::string_view vb = attrs["viewBox"];
                float vx = 0, vy = 0, vw = 100, vh = 100;
                size_t p0 = 0;
                auto next_num = [&](float& out) {
                    while (p0 < vb.size() && (std::isspace(vb[p0]) || vb[p0] == ',')) p0++;
                    if (p0 >= vb.size()) return false;
                    size_t end_idx = vb.find_first_of(" ,\t\r\n", p0);
                    if (end_idx == std::string_view::npos) end_idx = vb.size();
                    out = parse_float(vb.substr(p0, end_idx - p0));
                    p0 = end_idx;
                    return true;
                };
                next_num(vx); next_num(vy); next_num(vw); next_num(vh);
                auto r = Rect::from_xywh(vx, vy, vw, vh);
                if (r) doc.set_view_box(*r);
            }
            float w = attrs.count("width") ? parse_float(attrs["width"], doc.view_box().width()) : doc.view_box().width();
            float h = attrs.count("height") ? parse_float(attrs["height"], doc.view_box().height()) : doc.view_box().height();
            doc.set_dimensions(w, h);
            continue;
        }

        if (tag_name == "defs") {
            if (!is_self_closing) {
                scope_stack.push_back(ParseScope{ .type = ScopeType::Defs, .style = cur_scope.style, .transform = cur_scope.transform });
            }
            continue;
        }

        if (tag_name == "linearGradient" || tag_name == "radialGradient") {
            SvgGradient grad;
            grad.kind = (tag_name == "linearGradient") ? SvgGradient::Kind::Linear : SvgGradient::Kind::Radial;
            if (attrs.count("id")) grad.id = attrs["id"];
            if (attrs.count("href")) grad.href_id = extract_url_id(attrs["href"]);
            else if (attrs.count("xlink:href")) grad.href_id = extract_url_id(attrs["xlink:href"]);

            if (attrs.count("gradientUnits")) {
                if (attrs["gradientUnits"] == "userSpaceOnUse") grad.units = SvgGradientUnits::UserSpaceOnUse;
                else grad.units = SvgGradientUnits::ObjectBoundingBox;
            }
            if (attrs.count("spreadMethod")) {
                auto sm = attrs["spreadMethod"];
                if (sm == "reflect") grad.spread_mode = SpreadMode::Reflect;
                else if (sm == "repeat") grad.spread_mode = SpreadMode::Repeat;
                else grad.spread_mode = SpreadMode::Pad;
            }
            if (attrs.count("gradientTransform")) {
                grad.gradient_transform = parse_transform(attrs["gradientTransform"]);
            }

            if (grad.kind == SvgGradient::Kind::Linear) {
                grad.x1 = attrs.count("x1") ? parse_coord(attrs["x1"], 0.0f) : 0.0f;
                grad.y1 = attrs.count("y1") ? parse_coord(attrs["y1"], 0.0f) : 0.0f;
                grad.x2 = attrs.count("x2") ? parse_coord(attrs["x2"], 1.0f) : 1.0f;
                grad.y2 = attrs.count("y2") ? parse_coord(attrs["y2"], 0.0f) : 0.0f;
            } else {
                grad.cx = attrs.count("cx") ? parse_coord(attrs["cx"], 0.5f) : 0.5f;
                grad.cy = attrs.count("cy") ? parse_coord(attrs["cy"], 0.5f) : 0.5f;
                grad.r = attrs.count("r") ? parse_coord(attrs["r"], 0.5f) : 0.5f;
                grad.fx = attrs.count("fx") ? parse_coord(attrs["fx"], grad.cx) : grad.cx;
                grad.fy = attrs.count("fy") ? parse_coord(attrs["fy"], grad.cy) : grad.cy;
            }

            if (is_self_closing) {
                if (!grad.id.empty()) doc.add_gradient(std::move(grad));
            } else {
                scope_stack.push_back(ParseScope{
                    .type = (grad.kind == SvgGradient::Kind::Linear) ? ScopeType::LinearGradient : ScopeType::RadialGradient,
                    .id = grad.id,
                    .gradient = std::move(grad)
                });
            }
            continue;
        }

        if (tag_name == "stop") {
            auto* g_scope = current_gradient_scope();
            if (g_scope && g_scope->gradient) {
                float offset = 0.0f;
                if (attrs.count("offset")) offset = parse_coord(attrs["offset"], 0.0f);
                Color stop_col = Color::BLACK;
                float stop_opacity = 1.0f;

                if (attrs.count("stop-color")) {
                    auto c = parse_color(attrs["stop-color"]);
                    if (c) stop_col = *c;
                }
                if (attrs.count("stop-opacity")) {
                    stop_opacity = parse_float(attrs["stop-opacity"], 1.0f);
                }

                if (attrs.count("style")) {
                    std::string_view style_str = attrs["style"];
                    size_t s_pos = 0;
                    while (s_pos < style_str.size()) {
                        size_t sc = style_str.find(';', s_pos);
                        if (sc == std::string_view::npos) sc = style_str.size();
                        std::string_view prop = trim(style_str.substr(s_pos, sc - s_pos));
                        s_pos = sc + 1;
                        size_t colon = prop.find(':');
                        if (colon != std::string_view::npos) {
                            std::string_view k = trim(prop.substr(0, colon));
                            std::string_view v = trim(prop.substr(colon + 1));
                            if (k == "stop-color") {
                                auto c = parse_color(v);
                                if (c) stop_col = *c;
                            } else if (k == "stop-opacity") {
                                stop_opacity = parse_float(v, 1.0f);
                            }
                        }
                    }
                }

                stop_col.apply_opacity(stop_opacity);
                g_scope->gradient->stops.push_back(GradientStop::create(offset, stop_col));
            }
            continue;
        }

        if (tag_name == "clipPath") {
            std::string id = attrs.count("id") ? attrs["id"] : "";
            if (!is_self_closing) {
                scope_stack.push_back(ParseScope{
                    .type = ScopeType::ClipPath,
                    .id = std::move(id),
                    .style = cur_scope.style,
                    .transform = cur_scope.transform
                });
            }
            continue;
        }

        if (tag_name == "mask") {
            std::string id = attrs.count("id") ? attrs["id"] : "";
            if (!is_self_closing) {
                scope_stack.push_back(ParseScope{
                    .type = ScopeType::Mask,
                    .id = std::move(id),
                    .style = cur_scope.style,
                    .transform = cur_scope.transform
                });
            }
            continue;
        }

        // Compute cascaded styles & transform for element/group
        SvgPaintStyle elem_style = cur_scope.style;
        if (attrs.count("fill")) {
            std::string_view v = attrs["fill"];
            if (v.starts_with("url(") || v.starts_with("url")) {
                elem_style.fill_url = extract_url_id(v);
                elem_style.fill = std::nullopt;
            } else {
                elem_style.fill = parse_color(v);
                elem_style.fill_url.clear();
            }
        }
        if (attrs.count("stroke")) {
            std::string_view v = attrs["stroke"];
            if (v.starts_with("url(") || v.starts_with("url")) {
                elem_style.stroke_url = extract_url_id(v);
                elem_style.stroke = std::nullopt;
            } else {
                elem_style.stroke = parse_color(v);
                elem_style.stroke_url.clear();
            }
        }
        if (attrs.count("stroke-width")) elem_style.stroke_width = parse_float(attrs["stroke-width"], elem_style.stroke_width);
        if (attrs.count("opacity")) elem_style.opacity = cur_scope.style.opacity * parse_float(attrs["opacity"], 1.0f);
        if (attrs.count("fill-opacity")) elem_style.fill_opacity = parse_float(attrs["fill-opacity"], elem_style.fill_opacity);
        if (attrs.count("stroke-opacity")) elem_style.stroke_opacity = parse_float(attrs["stroke-opacity"], elem_style.stroke_opacity);
        if (attrs.count("stroke-linecap")) {
            auto cap = attrs["stroke-linecap"];
            if (cap == "round") elem_style.line_cap = LineCap::Round;
            else if (cap == "square") elem_style.line_cap = LineCap::Square;
            else elem_style.line_cap = LineCap::Butt;
        }
        if (attrs.count("stroke-linejoin")) {
            auto join = attrs["stroke-linejoin"];
            if (join == "round") elem_style.line_join = LineJoin::Round;
            else if (join == "bevel") elem_style.line_join = LineJoin::Bevel;
            else elem_style.line_join = LineJoin::Miter;
        }
        if (attrs.count("fill-rule")) {
            if (attrs["fill-rule"] == "evenodd") elem_style.fill_rule = FillRule::EvenOdd;
            else elem_style.fill_rule = FillRule::Winding;
        }
        if (attrs.count("clip-path")) elem_style.clip_path_url = extract_url_id(attrs["clip-path"]);
        if (attrs.count("mask")) elem_style.mask_url = extract_url_id(attrs["mask"]);
        if (attrs.count("style")) apply_style_string(elem_style, attrs["style"]);

        Transform elem_ts = cur_scope.transform;
        if (attrs.count("transform")) {
            elem_ts = elem_ts.pre_concat(parse_transform(attrs["transform"]));
        }

        std::string elem_id = attrs.count("id") ? attrs["id"] : "";

        if (tag_name == "g") {
            if (!is_self_closing) {
                scope_stack.push_back(ParseScope{
                    .type = ScopeType::Group,
                    .id = std::move(elem_id),
                    .style = elem_style,
                    .transform = elem_ts
                });
            }
            continue;
        }

        if (tag_name == "use") {
            std::string target_id;
            if (attrs.count("href")) target_id = extract_url_id(attrs["href"]);
            else if (attrs.count("xlink:href")) target_id = extract_url_id(attrs["xlink:href"]);

            float ux = attrs.count("x") ? parse_float(attrs["x"], 0.0f) : 0.0f;
            float uy = attrs.count("y") ? parse_float(attrs["y"], 0.0f) : 0.0f;

            auto* m_scope = current_mask_scope();
            use_instances.push_back(SvgUseInstance{
                .target_id = std::move(target_id),
                .x = ux,
                .y = uy,
                .style = elem_style,
                .transform = elem_ts,
                .in_defs = in_defs(),
                .in_mask = (m_scope != nullptr),
                .group_id = cur_scope.id,
                .mask_id = m_scope ? m_scope->id : ""
            });
            continue;
        }

        if (tag_name == "image") {
            std::string href;
            if (attrs.count("href")) href = attrs["href"];
            else if (attrs.count("xlink:href")) href = attrs["xlink:href"];

            float x = attrs.count("x") ? parse_float(attrs["x"], 0.0f) : 0.0f;
            float y = attrs.count("y") ? parse_float(attrs["y"], 0.0f) : 0.0f;
            float w = attrs.count("width") ? parse_float(attrs["width"], 0.0f) : 0.0f;
            float h = attrs.count("height") ? parse_float(attrs["height"], 0.0f) : 0.0f;

            std::shared_ptr<Pixmap> pixmap_ptr;
            if (!href.empty()) {
                if (href.starts_with("data:")) {
                    size_t comma = href.find(',');
                    if (comma != std::string::npos) {
                        auto b64 = href.substr(comma + 1);
                        auto bytes = base64_decode(b64);
                        auto img_res = nisaba::image::load_image_from_memory(bytes);
                        if (img_res.has_value()) {
                            pixmap_ptr = std::make_shared<Pixmap>(std::move(*img_res));
                        }
                    }
                } else {
                    auto img_res = nisaba::image::load_image_file(href);
                    if (img_res.has_value()) {
                        pixmap_ptr = std::make_shared<Pixmap>(std::move(*img_res));
                    }
                }
            }

            if (pixmap_ptr) {
                if (w <= 0.0f) w = static_cast<float>(pixmap_ptr->width());
                if (h <= 0.0f) h = static_cast<float>(pixmap_ptr->height());
            }

            auto r = Rect::from_xywh(x, y, w, h);
            Rect img_bounds = r ? *r : Rect();

            SvgElement img_elem{
                .type = SvgElementType::Image,
                .path = Path(),
                .style = elem_style,
                .transform = elem_ts,
                .id = elem_id,
                .image = pixmap_ptr,
                .image_bounds = img_bounds,
            };

            auto* m_scope = current_mask_scope();
            if (m_scope) {
                m_scope->mask_elements.push_back(img_elem);
            } else if (in_defs()) {
                if (!elem_id.empty()) doc.add_defined_element(elem_id, img_elem);
            } else {
                if (!elem_id.empty()) doc.add_defined_element(elem_id, img_elem);
                doc.add_element(std::move(img_elem));
            }
            continue;
        }

        // Geometric shape parsing
        std::optional<Path> parsed_path;

        if (tag_name == "path" && attrs.count("d")) {
            parsed_path = PathParser::parse(attrs["d"]);
        } else if (tag_name == "rect") {
            float x = parse_float(attrs["x"], 0.0f);
            float y = parse_float(attrs["y"], 0.0f);
            float w = parse_float(attrs["width"], 0.0f);
            float h = parse_float(attrs["height"], 0.0f);
            float rx = parse_float(attrs["rx"], 0.0f);
            float ry = parse_float(attrs["ry"], rx);

            auto r = Rect::from_xywh(x, y, w, h);
            if (r) {
                if (rx > 0.0f || ry > 0.0f) {
                    parsed_path = PathBuilder::from_rounded_rect(*r, rx, ry);
                } else {
                    parsed_path = PathBuilder::from_rect(*r);
                }
            }
        } else if (tag_name == "circle") {
            float cx = parse_float(attrs["cx"], 0.0f);
            float cy = parse_float(attrs["cy"], 0.0f);
            float r = parse_float(attrs["r"], 0.0f);
            parsed_path = PathBuilder::from_circle(cx, cy, r);
        } else if (tag_name == "ellipse") {
            float cx = parse_float(attrs["cx"], 0.0f);
            float cy = parse_float(attrs["cy"], 0.0f);
            float rx = parse_float(attrs["rx"], 0.0f);
            float ry = parse_float(attrs["ry"], 0.0f);
            auto r = Rect::from_xywh(cx - rx, cy - ry, rx * 2.0f, ry * 2.0f);
            if (r) parsed_path = PathBuilder::from_oval(*r);
        } else if (tag_name == "line") {
            float x1 = parse_float(attrs["x1"], 0.0f);
            float y1 = parse_float(attrs["y1"], 0.0f);
            float x2 = parse_float(attrs["x2"], 0.0f);
            float y2 = parse_float(attrs["y2"], 0.0f);
            PathBuilder pb;
            pb.move_to(x1, y1);
            pb.line_to(x2, y2);
            parsed_path = pb.finish();
        } else if ((tag_name == "polyline" || tag_name == "polygon") && attrs.count("points")) {
            std::string_view pts_str = attrs["points"];
            PathBuilder pb;
            size_t p_idx = 0;
            bool is_first = true;
            while (p_idx < pts_str.size()) {
                while (p_idx < pts_str.size() && (std::isspace(pts_str[p_idx]) || pts_str[p_idx] == ',')) p_idx++;
                if (p_idx >= pts_str.size()) break;
                size_t c_idx = pts_str.find_first_of(" ,\t\r\n", p_idx);
                if (c_idx == std::string_view::npos) break;
                float px = parse_float(pts_str.substr(p_idx, c_idx - p_idx));
                p_idx = c_idx;
                while (p_idx < pts_str.size() && (std::isspace(pts_str[p_idx]) || pts_str[p_idx] == ',')) p_idx++;
                if (p_idx >= pts_str.size()) break;
                size_t end_idx = pts_str.find_first_of(" ,\t\r\n", p_idx);
                if (end_idx == std::string_view::npos) end_idx = pts_str.size();
                float py = parse_float(pts_str.substr(p_idx, end_idx - p_idx));
                p_idx = end_idx;

                if (is_first) {
                    pb.move_to(px, py);
                    is_first = false;
                } else {
                    pb.line_to(px, py);
                }
            }
            if (tag_name == "polygon") pb.close();
            parsed_path = pb.finish();
        }

        if (parsed_path) {
            auto* c_scope = current_clip_scope();
            auto* m_scope = current_mask_scope();

            if (c_scope) {
                Path transformed = *parsed_path;
                transformed.apply_transform(elem_ts);
                c_scope->clip_path.add_path(transformed);
            } else {
                SvgElement elem{
                    .type = SvgElementType::Path,
                    .path = std::move(*parsed_path),
                    .style = elem_style,
                    .transform = elem_ts,
                    .id = elem_id
                };

                if (m_scope) {
                    m_scope->mask_elements.push_back(elem);
                } else if (in_defs()) {
                    if (!elem_id.empty()) doc.add_defined_element(elem_id, elem);
                } else {
                    if (!elem_id.empty()) doc.add_defined_element(elem_id, elem);
                    doc.add_element(std::move(elem));
                }
            }
        }
    }

    // Resolve gradient inheritance (href="#parentGrad")
    for (auto& [id, grad] : doc.gradients_) {
        if (grad.stops.empty() && !grad.href_id.empty()) {
            auto it = doc.gradients_.find(grad.href_id);
            if (it != doc.gradients_.end()) {
                grad.stops = it->second.stops;
            }
        }
    }

    // Resolve <use> references
    for (const auto& use : use_instances) {
        auto it_elem = doc.defined_elements().find(use.target_id);
        if (it_elem != doc.defined_elements().end()) {
            SvgElement clone = it_elem->second;
            clone.transform = use.transform.pre_translate(use.x, use.y).pre_concat(it_elem->second.transform);
            if (!it_elem->second.style.fill.has_value() && it_elem->second.style.fill_url.empty()) {
                clone.style.fill = use.style.fill;
                clone.style.fill_url = use.style.fill_url;
            }
            if (!it_elem->second.style.stroke.has_value() && it_elem->second.style.stroke_url.empty()) {
                clone.style.stroke = use.style.stroke;
                clone.style.stroke_url = use.style.stroke_url;
                clone.style.stroke_width = use.style.stroke_width;
            }
            clone.style.opacity *= use.style.opacity;
            if (clone.style.clip_path_url.empty()) clone.style.clip_path_url = use.style.clip_path_url;
            if (clone.style.mask_url.empty()) clone.style.mask_url = use.style.mask_url;

            if (use.in_mask && !use.mask_id.empty()) {
                // Add to mask
                auto it_m = const_cast<std::map<std::string, SvgMask>&>(doc.masks()).find(use.mask_id);
                if (it_m != doc.masks().end()) {
                    it_m->second.elements.push_back(clone);
                }
            } else if (!use.in_defs) {
                doc.add_element(std::move(clone));
            }
        }
    }

    return doc;
}

std::optional<SvgDocument> SvgDocument::from_file(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) return std::nullopt;
    std::stringstream buffer;
    buffer << file.rdbuf();
    return parse(buffer.str());
}

void SvgDocument::render_element(Canvas& canvas, const SvgElement& elem, std::optional<Color> tint) const {
    canvas.save();
    if (!elem.transform.is_identity()) {
        canvas.concat(elem.transform);
    }

    // 1. Vector ClipPath
    if (!elem.style.clip_path_url.empty()) {
        auto it = clip_paths_.find(elem.style.clip_path_url);
        if (it != clip_paths_.end()) {
            canvas.clip_path(it->second);
        }
    }

    // 2. Alpha/Luminance Mask
    if (!elem.style.mask_url.empty()) {
        auto it = masks_.find(elem.style.mask_url);
        if (it != masks_.end() && !it->second.elements.empty()) {
            auto mask_pix = Pixmap::allocate(canvas.width(), canvas.height());
            if (mask_pix) {
                Canvas mask_canvas(*mask_pix);
                mask_canvas.clear(Color::TRANSPARENT);
                for (const auto& melem : it->second.elements) {
                    render_element(mask_canvas, melem, std::nullopt);
                }
                Mask m = Mask::from_pixmap(mask_pix->as_ref(), MaskType::Luminance);
                canvas.clip_mask(m);
            }
        }
    }

    // 3. Render element
    if (elem.type == SvgElementType::Image) {
        if (elem.image) {
            PixmapPaint pp;
            pp.opacity = elem.style.opacity;
            float iw = static_cast<float>(elem.image->width());
            float ih = static_cast<float>(elem.image->height());
            float bw = elem.image_bounds.width();
            float bh = elem.image_bounds.height();
            if (iw > 0.0f && ih > 0.0f && bw > 0.0f && bh > 0.0f) {
                canvas.save();
                canvas.translate(elem.image_bounds.left(), elem.image_bounds.top());
                canvas.scale(bw / iw, bh / ih);
                canvas.draw_pixmap(0, 0, elem.image->as_ref(), pp);
                canvas.restore();
            }
        }
    } else { // Vector Path
        // Fill pass
        if (!elem.style.fill_url.empty()) {
            auto git = gradients_.find(elem.style.fill_url);
            if (git != gradients_.end()) {
                auto sh = create_shader_from_gradient(
                    git->second,
                    elem.path.bounds(),
                    elem.style.opacity * elem.style.fill_opacity,
                    tint
                );
                Paint fill_paint(std::move(sh));
                canvas.fill_path(elem.path, fill_paint, elem.style.fill_rule);
            } else if (elem.style.fill.has_value()) {
                Color fill_c = tint.has_value() ? *tint : *elem.style.fill;
                fill_c.apply_opacity(elem.style.opacity * elem.style.fill_opacity);
                Paint fill_paint(fill_c);
                canvas.fill_path(elem.path, fill_paint, elem.style.fill_rule);
            }
        } else if (elem.style.fill.has_value()) {
            Color fill_c = tint.has_value() ? *tint : *elem.style.fill;
            fill_c.apply_opacity(elem.style.opacity * elem.style.fill_opacity);
            Paint fill_paint(fill_c);
            canvas.fill_path(elem.path, fill_paint, elem.style.fill_rule);
        }

        // Stroke pass
        if (elem.style.stroke_width > 0.0f) {
            Stroke stroke(elem.style.stroke_width);
            stroke.line_cap = elem.style.line_cap;
            stroke.line_join = elem.style.line_join;

            if (!elem.style.stroke_url.empty()) {
                auto git = gradients_.find(elem.style.stroke_url);
                if (git != gradients_.end()) {
                    auto sh = create_shader_from_gradient(
                        git->second,
                        elem.path.bounds(),
                        elem.style.opacity * elem.style.stroke_opacity,
                        tint
                    );
                    Paint stroke_paint(std::move(sh));
                    canvas.stroke_path(elem.path, stroke_paint, stroke);
                } else if (elem.style.stroke.has_value()) {
                    Color stroke_c = tint.has_value() ? *tint : *elem.style.stroke;
                    stroke_c.apply_opacity(elem.style.opacity * elem.style.stroke_opacity);
                    Paint stroke_paint(stroke_c);
                    canvas.stroke_path(elem.path, stroke_paint, stroke);
                }
            } else if (elem.style.stroke.has_value()) {
                Color stroke_c = tint.has_value() ? *tint : *elem.style.stroke;
                stroke_c.apply_opacity(elem.style.opacity * elem.style.stroke_opacity);
                Paint stroke_paint(stroke_c);
                canvas.stroke_path(elem.path, stroke_paint, stroke);
            }
        }
    }

    canvas.restore();
}

void SvgDocument::render(Canvas& canvas, const Rect* target_bounds, std::optional<Color> tint) const {
    canvas.save();

    if (target_bounds != nullptr) {
        float vb_w = view_box_.width();
        float vb_h = view_box_.height();
        if (vb_w > 0.0f && vb_h > 0.0f) {
            float sx = target_bounds->width() / vb_w;
            float sy = target_bounds->height() / vb_h;
            float s = std::min(sx, sy); // Uniform aspect ratio scaling

            float tx = target_bounds->left() + (target_bounds->width() - vb_w * s) * 0.5f - view_box_.left() * s;
            float ty = target_bounds->top() + (target_bounds->height() - vb_h * s) * 0.5f - view_box_.top() * s;

            canvas.translate(tx, ty);
            canvas.scale(s, s);
        }
    }

    for (const auto& elem : elements_) {
        render_element(canvas, elem, tint);
    }

    canvas.restore();
}

} // namespace nisaba::svg
