/// @file animation.cpp
/// @brief Implementation of root Lottie & dotLottie (.lottie ZIP) document, assets, and dynamic overrides.

#include "nisaba/lottie/animation.hpp"
#include "nisaba/json/json.hpp"
#include "nisaba/image/deflate.hpp"

#include <fstream>
#include <sstream>
#include <algorithm>
#include <cstring>
#include <filesystem>

namespace nisaba::lottie {

namespace {

// ── Sovereign Base64 Decoder (RFC 4648 / RFC 2397 data URI) ───────────────

std::vector<uint8_t> decode_base64(std::string_view input) {
    static constexpr uint8_t LUT[256] = {
        64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
        64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
        64,64,64,64,64,64,64,64,64,64,64,62,64,62,64,63,
        52,53,54,55,56,57,58,59,60,61,64,64,64,64,64,64,
        64, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,
        15,16,17,18,19,20,21,22,23,24,25,64,64,64,64,63,
        64,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,
        41,42,43,44,45,46,47,48,49,50,51,64,64,64,64,64,
        64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
        64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
        64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
        64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
        64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
        64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
        64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
        64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64
    };

    std::vector<uint8_t> out;
    out.reserve((input.size() * 3) / 4);

    uint32_t accum = 0;
    int bits = 0;
    for (unsigned char c : input) {
        if (c == '=') break;
        uint8_t val = LUT[c];
        if (val >= 64) continue;
        accum = (accum << 6) | val;
        bits += 6;
        if (bits >= 8) {
            bits -= 8;
            out.push_back(static_cast<uint8_t>((accum >> bits) & 0xFF));
        }
    }
    return out;
}

std::optional<Pixmap> decode_data_uri_image(std::string_view uri) {
    auto comma_pos = uri.find(',');
    if (comma_pos == std::string_view::npos) return std::nullopt;
    std::string_view payload = uri.substr(comma_pos + 1);
    auto raw_bytes = decode_base64(payload);
    if (raw_bytes.empty()) return std::nullopt;
    return Pixmap::load_from_memory(raw_bytes);
}

// ── Sovereign ZIP / dotLottie (.lottie) Archive Reader ────────────────────

uint16_t read_u16_le(const uint8_t* p) noexcept {
    return static_cast<uint16_t>(p[0] | (p[1] << 8));
}

uint32_t read_u32_le(const uint8_t* p) noexcept {
    return static_cast<uint32_t>(p[0]) |
           (static_cast<uint32_t>(p[1]) << 8) |
           (static_cast<uint32_t>(p[2]) << 16) |
           (static_cast<uint32_t>(p[3]) << 24);
}

struct ZipArchive {
    std::unordered_map<std::string, std::vector<uint8_t>> files;

    static std::optional<ZipArchive> parse(std::span<const uint8_t> data) {
        if (data.size() < 22) return std::nullopt;
        // Verify PK\x03\x04 or PK\x05\x06 signature
        if (data[0] != 'P' || data[1] != 'K') return std::nullopt;

        ZipArchive archive;

        // Locate End of Central Directory (EOCD) signature 0x06054b50 from the end
        size_t eocd_pos = std::string_view::npos;
        size_t min_search = (data.size() > 65557) ? (data.size() - 65557) : 0;
        for (size_t i = data.size() - 22; ; --i) {
            if (read_u32_le(data.data() + i) == 0x06054b50u) {
                eocd_pos = i;
                break;
            }
            if (i <= min_search) break;
        }

        if (eocd_pos != std::string_view::npos) {
            uint16_t num_entries = read_u16_le(data.data() + eocd_pos + 10);
            uint32_t cd_size = read_u32_le(data.data() + eocd_pos + 12);
            uint32_t cd_offset = read_u32_le(data.data() + eocd_pos + 16);
            (void)cd_size;

            size_t pos = cd_offset;
            for (uint16_t e = 0; e < num_entries && pos + 46 <= data.size(); ++e) {
                if (read_u32_le(data.data() + pos) != 0x02014b50u) break;
                uint16_t method = read_u16_le(data.data() + pos + 10);
                uint32_t comp_size = read_u32_le(data.data() + pos + 20);
                uint32_t uncomp_size = read_u32_le(data.data() + pos + 24);
                uint16_t name_len = read_u16_le(data.data() + pos + 28);
                uint16_t extra_len = read_u16_le(data.data() + pos + 30);
                uint16_t comment_len = read_u16_le(data.data() + pos + 32);
                uint32_t lfh_offset = read_u32_le(data.data() + pos + 42);

                if (pos + 46 + name_len > data.size()) break;
                std::string filename(reinterpret_cast<const char*>(data.data() + pos + 46), name_len);
                // Normalize backslashes
                std::replace(filename.begin(), filename.end(), '\\', '/');
                if (filename.starts_with("./")) filename.erase(0, 2);

                pos += 46 + name_len + extra_len + comment_len;

                if (lfh_offset + 30 <= data.size() && read_u32_le(data.data() + lfh_offset) == 0x04034b50u) {
                    uint16_t lfh_name_len = read_u16_le(data.data() + lfh_offset + 26);
                    uint16_t lfh_extra_len = read_u16_le(data.data() + lfh_offset + 28);
                    size_t data_offset = lfh_offset + 30 + lfh_name_len + lfh_extra_len;
                    if (data_offset + comp_size <= data.size()) {
                        std::span<const uint8_t> payload(data.data() + data_offset, comp_size);
                        if (method == 0) {
                            archive.files[filename] = std::vector<uint8_t>(payload.begin(), payload.end());
                        } else if (method == 8) {
                            auto dec = image::deflate_decompress(payload, uncomp_size);
                            if (dec.has_value()) {
                                archive.files[filename] = std::move(*dec);
                            }
                        }
                    }
                }
            }
        } else {
            // Sequential Local File Header scan fallback
            size_t pos = 0;
            while (pos + 30 <= data.size() && read_u32_le(data.data() + pos) == 0x04034b50u) {
                uint16_t method = read_u16_le(data.data() + pos + 8);
                uint32_t comp_size = read_u32_le(data.data() + pos + 18);
                uint32_t uncomp_size = read_u32_le(data.data() + pos + 22);
                uint16_t name_len = read_u16_le(data.data() + pos + 26);
                uint16_t extra_len = read_u16_le(data.data() + pos + 28);
                size_t data_offset = pos + 30 + name_len + extra_len;
                if (data_offset + comp_size > data.size()) break;

                std::string filename(reinterpret_cast<const char*>(data.data() + pos + 30), name_len);
                std::replace(filename.begin(), filename.end(), '\\', '/');
                std::span<const uint8_t> payload(data.data() + data_offset, comp_size);
                if (method == 0) {
                    archive.files[filename] = std::vector<uint8_t>(payload.begin(), payload.end());
                } else if (method == 8) {
                    auto dec = image::deflate_decompress(payload, uncomp_size);
                    if (dec.has_value()) {
                        archive.files[filename] = std::move(*dec);
                    }
                }
                pos = data_offset + comp_size;
            }
        }

        if (archive.files.empty()) return std::nullopt;
        return archive;
    }

    const std::vector<uint8_t>* find_file(std::string_view path) const {
        std::string norm(path);
        std::replace(norm.begin(), norm.end(), '\\', '/');
        if (norm.starts_with("./")) norm.erase(0, 2);
        if (norm.starts_with("/")) norm.erase(0, 1);

        auto it = files.find(norm);
        if (it != files.end()) return &it->second;

        // Try basename match inside images/ or animations/
        auto slash = norm.find_last_of('/');
        std::string_view base = (slash != std::string::npos) ? std::string_view(norm).substr(slash + 1) : std::string_view(norm);
        for (const auto& [k, v] : files) {
            if (k == base || k.ends_with("/" + std::string(base))) {
                return &v;
            }
        }
        return nullptr;
    }
};

// ── Keypath Matching & Property Override Traversal ────────────────────────

void split_keypath(std::string_view kp, std::vector<std::string_view>& out_parts) {
    out_parts.clear();
    size_t pos = 0;
    while (pos <= kp.size()) {
        size_t next = kp.find('.', pos);
        if (next == std::string_view::npos) {
            if (pos < kp.size()) out_parts.push_back(kp.substr(pos));
            break;
        }
        out_parts.push_back(kp.substr(pos, next - pos));
        pos = next + 1;
    }
}

bool match_segment(std::string_view pattern, std::string_view name) noexcept {
    if (pattern == "*" || pattern == "**") return true;
    return pattern == name;
}

bool match_keypath_prefix(const std::vector<std::string_view>& parts, size_t idx, std::string_view item_name) noexcept {
    if (parts.empty()) return true;
    if (parts.size() == 1 && parts[0] == "*") return true;
    if (idx < parts.size()) {
        return match_segment(parts[idx], item_name);
    }
    return parts.back() == "*" || parts.back() == "**" || parts.back() == item_name;
}

bool apply_color_to_shape_item(LottieShapeItem& item, const std::vector<std::string_view>& parts, size_t depth, Color color) {
    bool matched = false;
    bool seg_ok = (depth >= parts.size()) || match_keypath_prefix(parts, depth, item.name) || (parts.size() == 1 && match_segment(parts[0], item.name));

    if (item.type_name() == "gr") {
        auto& grp = static_cast<ShapeGroup&>(item);
        size_t next_depth = seg_ok ? (depth + 1) : depth;
        for (auto& child : grp.items) {
            if (child && apply_color_to_shape_item(*child, parts, next_depth, color)) {
                matched = true;
            }
        }
    } else if (item.type_name() == "fl" && seg_ok) {
        static_cast<FillShape&>(item).color.set_override(color);
        matched = true;
    } else if (item.type_name() == "st" && seg_ok) {
        static_cast<StrokeShape&>(item).color.set_override(color);
        matched = true;
    }
    return matched;
}

bool apply_scalar_to_shape_item(LottieShapeItem& item, const std::vector<std::string_view>& parts, size_t depth, float value) {
    bool matched = false;
    bool seg_ok = (depth >= parts.size()) || match_keypath_prefix(parts, depth, item.name) || (parts.size() == 1 && match_segment(parts[0], item.name));

    if (item.type_name() == "gr") {
        auto& grp = static_cast<ShapeGroup&>(item);
        if (seg_ok && depth + 1 >= parts.size()) {
            grp.transform.opacity.set_override(value);
            matched = true;
        }
        size_t next_depth = seg_ok ? (depth + 1) : depth;
        for (auto& child : grp.items) {
            if (child && apply_scalar_to_shape_item(*child, parts, next_depth, value)) {
                matched = true;
            }
        }
    } else if (item.type_name() == "fl" && seg_ok) {
        static_cast<FillShape&>(item).opacity.set_override(value);
        matched = true;
    } else if (item.type_name() == "st" && seg_ok) {
        static_cast<StrokeShape&>(item).width.set_override(value);
        matched = true;
    } else if (item.type_name() == "gs" && seg_ok) {
        static_cast<GradientStrokeShape&>(item).width.set_override(value);
        matched = true;
    } else if (item.type_name() == "rd" && seg_ok) {
        static_cast<RoundCornersShape&>(item).radius.set_override(value);
        matched = true;
    }
    return matched;
}

bool apply_point_to_shape_item(LottieShapeItem& item, const std::vector<std::string_view>& parts, size_t depth, Point value) {
    bool matched = false;
    bool seg_ok = (depth >= parts.size()) || match_keypath_prefix(parts, depth, item.name) || (parts.size() == 1 && match_segment(parts[0], item.name));

    if (item.type_name() == "gr") {
        auto& grp = static_cast<ShapeGroup&>(item);
        if (seg_ok && depth + 1 >= parts.size()) {
            grp.transform.position.set_override(value);
            matched = true;
        }
        size_t next_depth = seg_ok ? (depth + 1) : depth;
        for (auto& child : grp.items) {
            if (child && apply_point_to_shape_item(*child, parts, next_depth, value)) {
                matched = true;
            }
        }
    } else if (item.type_name() == "rc" && seg_ok) {
        auto& rc = static_cast<RectShape&>(item);
        rc.size.set_override(value);
        rc.has_cached_static = false;
        matched = true;
    } else if (item.type_name() == "el" && seg_ok) {
        auto& el = static_cast<EllipseShape&>(item);
        el.size.set_override(value);
        el.has_cached_static = false;
        matched = true;
    }
    return matched;
}

bool apply_slot_color_to_shape_item(LottieShapeItem& item, std::string_view slot_id, Color color) {
    bool matched = false;
    if (item.type_name() == "gr") {
        auto& grp = static_cast<ShapeGroup&>(item);
        for (auto& child : grp.items) {
            if (child && apply_slot_color_to_shape_item(*child, slot_id, color)) {
                matched = true;
            }
        }
    } else if (item.type_name() == "fl") {
        auto& fl = static_cast<FillShape&>(item);
        if (fl.color.slot_id() == slot_id) {
            fl.color.set_override(color);
            matched = true;
        }
    } else if (item.type_name() == "st") {
        auto& st = static_cast<StrokeShape&>(item);
        if (st.color.slot_id() == slot_id) {
            st.color.set_override(color);
            matched = true;
        }
    }
    return matched;
}

void clear_shape_item_overrides(LottieShapeItem& item) {
    if (item.type_name() == "gr") {
        auto& grp = static_cast<ShapeGroup&>(item);
        grp.transform.anchor.clear_override();
        grp.transform.position.clear_override();
        grp.transform.scale.clear_override();
        grp.transform.rotation.clear_override();
        grp.transform.opacity.clear_override();
        for (auto& child : grp.items) {
            if (child) clear_shape_item_overrides(*child);
        }
    } else if (item.type_name() == "fl") {
        auto& fl = static_cast<FillShape&>(item);
        fl.color.clear_override();
        fl.opacity.clear_override();
    } else if (item.type_name() == "st") {
        auto& st = static_cast<StrokeShape&>(item);
        st.color.clear_override();
        st.opacity.clear_override();
        st.width.clear_override();
    } else if (item.type_name() == "gs") {
        auto& gs = static_cast<GradientStrokeShape&>(item);
        gs.opacity.clear_override();
        gs.width.clear_override();
    } else if (item.type_name() == "rc") {
        auto& rc = static_cast<RectShape&>(item);
        rc.position.clear_override();
        rc.size.clear_override();
        rc.roundness.clear_override();
    } else if (item.type_name() == "el") {
        auto& el = static_cast<EllipseShape&>(item);
        el.position.clear_override();
        el.size.clear_override();
    } else if (item.type_name() == "rd") {
        static_cast<RoundCornersShape&>(item).radius.clear_override();
    }
}

} // anonymous namespace

std::shared_ptr<Animation> Animation::load_from_bytes(std::span<const uint8_t> bytes, std::string_view base_dir) {
    if (bytes.empty()) return nullptr;

    // Detect if bytes is a .lottie ZIP container (starts with "PK")
    if (bytes.size() >= 4 && bytes[0] == 'P' && bytes[1] == 'K') {
        auto zip = ZipArchive::parse(bytes);
        if (!zip) return nullptr;

        std::string target_json_path;
        if (const auto* manifest_bytes = zip->find_file("manifest.json")) {
            std::string_view manifest_sv(reinterpret_cast<const char*>(manifest_bytes->data()), manifest_bytes->size());
            if (auto m_json = json::JsonValue::parse(manifest_sv); m_json && m_json->is_object()) {
                if (const auto* anims = m_json->get("animations"); anims && anims->is_array() && !anims->as_array().empty()) {
                    std::string id = std::string(anims->as_array()[0].get_string("id", ""));
                    if (!id.empty()) {
                        target_json_path = "animations/" + id + ".json";
                    }
                }
            }
        }

        const std::vector<uint8_t>* anim_json_bytes = nullptr;
        if (!target_json_path.empty()) {
            anim_json_bytes = zip->find_file(target_json_path);
        }
        if (!anim_json_bytes) {
            // Find first .json file in animations/ or root (excluding manifest.json)
            for (const auto& [k, v] : zip->files) {
                if (k != "manifest.json" && k.ends_with(".json")) {
                    anim_json_bytes = &v;
                    break;
                }
            }
        }
        if (!anim_json_bytes) return nullptr;

        std::string_view json_sv(reinterpret_cast<const char*>(anim_json_bytes->data()), anim_json_bytes->size());
        auto anim = load_from_data(json_sv, base_dir);
        if (!anim) return nullptr;

        // Resolve any external images from the ZIP archive
        for (auto& [id, img_asset] : anim->assets_.images) {
            if (!img_asset.pixmap.has_value() && !img_asset.filename.empty()) {
                std::string rel_path = img_asset.directory + img_asset.filename;
                const auto* img_bytes = zip->find_file(rel_path);
                if (!img_bytes) img_bytes = zip->find_file("images/" + img_asset.filename);
                if (!img_bytes) img_bytes = zip->find_file(img_asset.filename);
                if (img_bytes) {
                    img_asset.pixmap = Pixmap::load_from_memory(*img_bytes);
                }
            }
        }
        return anim;
    }

    std::string_view sv(reinterpret_cast<const char*>(bytes.data()), bytes.size());
    return load_from_data(sv, base_dir);
}

std::shared_ptr<Animation> Animation::load_from_data(std::string_view content, std::string_view base_dir) {
    if (content.size() >= 4 && content[0] == 'P' && content[1] == 'K') {
        std::span<const uint8_t> raw(reinterpret_cast<const uint8_t*>(content.data()), content.size());
        return load_from_bytes(raw, base_dir);
    }

    auto opt_root = json::JsonValue::parse(content);
    if (!opt_root || !opt_root->is_object()) {
        return nullptr;
    }
    const auto& root = *opt_root;

    auto anim = std::make_shared<Animation>();
    anim->version_ = std::string(root.get_string("v", ""));
    anim->name_ = std::string(root.get_string("nm", ""));
    anim->width_ = root.get_float("w", 0.0f);
    anim->height_ = root.get_float("h", 0.0f);
    anim->in_point_ = root.get_float("ip", 0.0f);
    anim->out_point_ = root.get_float("op", 0.0f);
    anim->frame_rate_ = root.get_float("fr", 60.0f);
    if (anim->frame_rate_ <= 0.0f) anim->frame_rate_ = 60.0f;

    anim->assets_.comp_width = (anim->width_ > 0.0f) ? anim->width_ : 512.0f;
    anim->assets_.comp_height = (anim->height_ > 0.0f) ? anim->height_ : 512.0f;
    anim->assets_.font_system = std::make_shared<text::FontSystem>();

    // Parse fonts ("fonts": { "list": [ ... ] })
    if (const auto* fonts_val = root.get("fonts"); fonts_val && fonts_val->is_object()) {
        if (const auto* list_val = fonts_val->get("list"); list_val && list_val->is_array()) {
            for (const auto& f_val : list_val->as_array()) {
                if (!f_val.is_object()) continue;
                LottieFontDef fdef;
                fdef.name = std::string(f_val.get_string("fName", ""));
                fdef.family = std::string(f_val.get_string("fFamily", ""));
                fdef.style = std::string(f_val.get_string("fStyle", ""));
                fdef.path = std::string(f_val.get_string("fPath", ""));
                fdef.ascent = f_val.get_float("ascent", 75.0f);
                if (!fdef.path.empty() && anim->assets_.font_system) {
                    if (fdef.path.starts_with("data:")) {
                        auto comma = fdef.path.find(',');
                        if (comma != std::string::npos) {
                            auto ttf_bytes = decode_base64(std::string_view(fdef.path).substr(comma + 1));
                            fdef.loaded_font_id = anim->assets_.font_system->load_font_data(ttf_bytes);
                        }
                    } else if (!base_dir.empty()) {
                        std::filesystem::path fp = std::filesystem::path(base_dir) / fdef.path;
                        fdef.loaded_font_id = anim->assets_.font_system->load_font_file(fp.string());
                    } else {
                        fdef.loaded_font_id = anim->assets_.font_system->load_font_file(fdef.path);
                    }
                }
                if (!fdef.name.empty()) {
                    anim->assets_.fonts[fdef.name] = std::move(fdef);
                }
            }
        }
    }

    // Parse pre-baked glyph characters ("chars": [ ... ])
    if (const auto* chars_val = root.get("chars"); chars_val && chars_val->is_array()) {
        for (const auto& c_val : chars_val->as_array()) {
            if (!c_val.is_object()) continue;
            LottieGlyphChar gc;
            gc.ch = std::string(c_val.get_string("ch", ""));
            gc.style = std::string(c_val.get_string("style", ""));
            gc.family = std::string(c_val.get_string("fFamily", ""));
            gc.size = c_val.get_float("size", 100.0f);
            gc.width = c_val.get_float("w", 0.0f);
            if (const auto* data_obj = c_val.get("data"); data_obj && data_obj->is_object()) {
                if (const auto* shapes_arr = data_obj->get("shapes"); shapes_arr && shapes_arr->is_array()) {
                    for (const auto& s_val : shapes_arr->as_array()) {
                        if (auto sg = ShapeGroup::parse(s_val)) {
                            gc.shapes.push_back(std::move(sg));
                        }
                    }
                }
            }
            anim->assets_.chars.push_back(std::move(gc));
        }
    }

    // Parse precomp and image assets
    if (const auto* assets_val = root.get("assets")) {
        if (assets_val->is_array()) {
            for (const auto& asset : assets_val->as_array()) {
                if (!asset.is_object()) continue;
                std::string id = std::string(asset.get_string("id", ""));
                if (id.empty()) continue;

                if (const auto* layers_val = asset.get("layers")) {
                    if (layers_val->is_array()) {
                        std::vector<std::shared_ptr<LottieLayer>> sub_layers;
                        for (const auto& l_val : layers_val->as_array()) {
                            if (auto l = LottieLayer::parse(l_val)) {
                                sub_layers.push_back(std::move(l));
                            }
                        }
                        link_layer_hierarchy(sub_layers);
                        anim->assets_.precomps[std::move(id)] = std::move(sub_layers);
                    }
                } else if (asset.has("p")) {
                    LottieImageAsset img;
                    img.id = id;
                    img.directory = std::string(asset.get_string("u", ""));
                    img.filename = std::string(asset.get_string("p", ""));
                    img.width = asset.get_float("w", 0.0f);
                    img.height = asset.get_float("h", 0.0f);
                    img.embedded = (asset.get_int("e", 0) == 1) || img.filename.starts_with("data:");

                    if (img.filename.starts_with("data:")) {
                        img.pixmap = decode_data_uri_image(img.filename);
                    } else if (!img.filename.empty()) {
                        std::filesystem::path full_path;
                        if (!base_dir.empty()) {
                            full_path = std::filesystem::path(base_dir) / img.directory / img.filename;
                        } else {
                            full_path = std::filesystem::path(img.directory) / img.filename;
                        }
                        img.pixmap = Pixmap::load_file(full_path.string());
                    }
                    anim->assets_.images[std::move(id)] = std::move(img);
                }
            }
        }
    }

    // Parse root layers
    if (const auto* layers_val = root.get("layers")) {
        if (layers_val->is_array()) {
            for (const auto& l_val : layers_val->as_array()) {
                if (auto l = LottieLayer::parse(l_val)) {
                    anim->layers_.push_back(std::move(l));
                }
            }
        }
    }
    link_layer_hierarchy(anim->layers_);

    // Parse markers if available
    if (const auto* markers_val = root.get("markers")) {
        if (markers_val->is_array()) {
            for (const auto& m_val : markers_val->as_array()) {
                if (!m_val.is_object()) continue;
                Marker m;
                m.name = std::string(m_val.get_string("cm", ""));
                m.time = m_val.get_float("tm", 0.0f);
                m.duration = m_val.get_float("dr", 0.0f);
                anim->markers_.push_back(std::move(m));
            }
        }
    }

    // Parse slots ("slots": { "slot_id": { "p": { ... } } })
    if (const auto* slots_val = root.get("slots"); slots_val && slots_val->is_object()) {
        for (const auto& [sid, s_obj] : slots_val->as_object()) {
            if (!s_obj.is_object()) continue;
            if (const auto* p_val = s_obj.get("p")) {
                ColorProperty cp = parse_color_property(*p_val, Color::WHITE);
                anim->set_slot_color(sid, cp.evaluate(anim->in_point_));
            }
        }
    }

    return anim;
}

std::shared_ptr<Animation> Animation::load_from_file(const std::string& filepath) {
    std::ifstream file(filepath, std::ios::in | std::ios::binary);
    if (!file.is_open()) {
        return nullptr;
    }

    std::ostringstream ss;
    ss << file.rdbuf();
    std::string content = ss.str();

    std::string base_dir;
    std::filesystem::path p(filepath);
    if (p.has_parent_path()) {
        base_dir = p.parent_path().string();
    }
    return load_from_data(content, base_dir);
}

void Animation::set_font_system(std::shared_ptr<text::FontSystem> fs) {
    assets_.font_system = std::move(fs);
}

// ── Dynamic Property Overrides ────────────────────────────────────────────

bool Animation::set_color_override(std::string_view keypath, Color color) {
    std::vector<std::string_view> parts;
    split_keypath(keypath, parts);
    if (parts.empty()) return false;

    bool matched = false;
    auto apply_to_layers = [&](std::vector<std::shared_ptr<LottieLayer>>& list) {
        for (auto& layer : list) {
            if (!layer) continue;
            bool layer_ok = match_segment(parts[0], layer->name) || parts.size() == 1;
            if (!layer_ok) continue;

            if (layer->type == LayerType::Solid) {
                if (match_segment(parts[0], layer->name)) {
                    static_cast<SolidLayer&>(*layer).color_override = color;
                    matched = true;
                }
            } else if (layer->type == LayerType::Shape) {
                auto& sl = static_cast<ShapeLayer&>(*layer);
                size_t next_depth = match_segment(parts[0], layer->name) ? 1 : 0;
                for (auto& item : sl.shapes) {
                    if (item && apply_color_to_shape_item(*item, parts, next_depth, color)) {
                        matched = true;
                    }
                }
            }
        }
    };

    apply_to_layers(layers_);
    for (auto& [id, sub] : assets_.precomps) {
        apply_to_layers(sub);
    }
    return matched;
}

bool Animation::set_scalar_override(std::string_view keypath, float value) {
    std::vector<std::string_view> parts;
    split_keypath(keypath, parts);
    if (parts.empty()) return false;

    bool matched = false;
    auto apply_to_layers = [&](std::vector<std::shared_ptr<LottieLayer>>& list) {
        for (auto& layer : list) {
            if (!layer) continue;
            bool layer_ok = match_segment(parts[0], layer->name);
            if (layer_ok && parts.size() == 1) {
                layer->transform.opacity.set_override(value);
                matched = true;
            }
            if (layer->type == LayerType::Shape && (layer_ok || parts.size() == 1)) {
                auto& sl = static_cast<ShapeLayer&>(*layer);
                size_t next_depth = layer_ok ? 1 : 0;
                for (auto& item : sl.shapes) {
                    if (item && apply_scalar_to_shape_item(*item, parts, next_depth, value)) {
                        matched = true;
                    }
                }
            }
        }
    };

    apply_to_layers(layers_);
    for (auto& [id, sub] : assets_.precomps) {
        apply_to_layers(sub);
    }
    return matched;
}

bool Animation::set_point_override(std::string_view keypath, Point value) {
    std::vector<std::string_view> parts;
    split_keypath(keypath, parts);
    if (parts.empty()) return false;

    bool matched = false;
    auto apply_to_layers = [&](std::vector<std::shared_ptr<LottieLayer>>& list) {
        for (auto& layer : list) {
            if (!layer) continue;
            bool layer_ok = match_segment(parts[0], layer->name);
            if (layer_ok && parts.size() == 1) {
                layer->transform.position.set_override(value);
                matched = true;
            }
            if (layer->type == LayerType::Shape && (layer_ok || parts.size() == 1)) {
                auto& sl = static_cast<ShapeLayer&>(*layer);
                size_t next_depth = layer_ok ? 1 : 0;
                for (auto& item : sl.shapes) {
                    if (item && apply_point_to_shape_item(*item, parts, next_depth, value)) {
                        matched = true;
                    }
                }
            }
        }
    };

    apply_to_layers(layers_);
    for (auto& [id, sub] : assets_.precomps) {
        apply_to_layers(sub);
    }
    return matched;
}

bool Animation::set_text_override(std::string_view layer_name, std::string text) {
    bool matched = false;
    auto apply_to_layers = [&](std::vector<std::shared_ptr<LottieLayer>>& list) {
        for (auto& layer : list) {
            if (!layer || layer->type != LayerType::Text) continue;
            if (layer_name == "*" || layer->name == layer_name) {
                static_cast<TextLayer&>(*layer).text_override = text;
                matched = true;
            }
        }
    };

    apply_to_layers(layers_);
    for (auto& [id, sub] : assets_.precomps) {
        apply_to_layers(sub);
    }
    return matched;
}

bool Animation::set_slot_color(std::string_view slot_id, Color color) {
    if (slot_id.empty()) return false;
    bool matched = false;
    auto apply_to_layers = [&](std::vector<std::shared_ptr<LottieLayer>>& list) {
        for (auto& layer : list) {
            if (!layer || layer->type != LayerType::Shape) continue;
            auto& sl = static_cast<ShapeLayer&>(*layer);
            for (auto& item : sl.shapes) {
                if (item && apply_slot_color_to_shape_item(*item, slot_id, color)) {
                    matched = true;
                }
            }
        }
    };

    apply_to_layers(layers_);
    for (auto& [id, sub] : assets_.precomps) {
        apply_to_layers(sub);
    }
    return matched;
}

void Animation::clear_overrides() {
    auto clear_layers = [&](std::vector<std::shared_ptr<LottieLayer>>& list) {
        for (auto& layer : list) {
            if (!layer) continue;
            layer->transform.anchor.clear_override();
            layer->transform.position.clear_override();
            layer->transform.scale.clear_override();
            layer->transform.rotation.clear_override();
            layer->transform.opacity.clear_override();
            if (layer->type == LayerType::Solid) {
                static_cast<SolidLayer&>(*layer).color_override.reset();
            } else if (layer->type == LayerType::Text) {
                static_cast<TextLayer&>(*layer).text_override.reset();
            } else if (layer->type == LayerType::Shape) {
                for (auto& item : static_cast<ShapeLayer&>(*layer).shapes) {
                    if (item) clear_shape_item_overrides(*item);
                }
            }
        }
    };

    clear_layers(layers_);
    for (auto& [id, sub] : assets_.precomps) {
        clear_layers(sub);
    }
}

std::optional<std::string> Animation::hit_test(float x, float y, float frame) const {
    // Test from top-most layer (index 0) down to bottom-most layer (N-1)
    for (const auto& layer : layers_) {
        if (!layer || !layer->is_active(frame) || layer->is_matte_target) continue;
        std::string hit_name;
        if (layer->hit_test(x, y, frame, assets_, hit_name)) {
            return hit_name.empty() ? layer->name : hit_name;
        }
    }
    return std::nullopt;
}

const Marker* Animation::find_marker(std::string_view marker_name) const noexcept {
    for (const auto& m : markers_) {
        if (m.name == marker_name) return &m;
    }
    return nullptr;
}

void Animation::render(ICanvas& canvas, float frame) const {
    render_layer_tree_with_assets(canvas, layers_, frame, assets_, 1.0f);
}

void Animation::render(ICanvas& canvas, float frame, const Rect& dest_rect, bool preserve_aspect_ratio) const {
    if (width_ <= 0.0f || height_ <= 0.0f || dest_rect.width() <= 0.0f || dest_rect.height() <= 0.0f) {
        return;
    }

    canvas.save();
    float sx = dest_rect.width() / width_;
    float sy = dest_rect.height() / height_;

    if (preserve_aspect_ratio) {
        float s = std::min(sx, sy);
        float ox = dest_rect.x() + (dest_rect.width() - width_ * s) * 0.5f;
        float oy = dest_rect.y() + (dest_rect.height() - height_ * s) * 0.5f;
        canvas.translate(ox, oy);
        canvas.scale(s, s);
    } else {
        canvas.translate(dest_rect.x(), dest_rect.y());
        canvas.scale(sx, sy);
    }

    render(canvas, frame);
    canvas.restore();
}

} // namespace nisaba::lottie
