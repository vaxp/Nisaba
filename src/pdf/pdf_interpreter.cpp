#include "nisaba/pdf/pdf_interpreter.hpp"
#include "nisaba/image/image_io.hpp"
#include "nisaba/text/buffer.hpp"
#include "nisaba/text/glyph_cache.hpp"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cstdlib>
#include <cmath>
#include <cctype>
#include <algorithm>

namespace nisaba::pdf {

namespace {

inline bool is_space(uint8_t c) {
    return c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '\f' || c == '\0';
}

struct PdfOpItem {
    float num = 0.0f;
    std::string str;
    bool is_num = false;

    inline float as_float(float def = 0.0f) const noexcept {
        if (is_num) return num;
        if (str.empty()) return def;
        return std::strtof(str.c_str(), nullptr);
    }
};

inline float fast_parse_float(const uint8_t* p, size_t len) noexcept {
    if (len == 0) return 0.0f;
    bool neg = false;
    size_t k = 0;
    if (p[0] == '-') { neg = true; k = 1; }
    else if (p[0] == '+') { k = 1; }

    double int_part = 0.0;
    while (k < len && p[k] >= '0' && p[k] <= '9') {
        int_part = int_part * 10.0 + (p[k] - '0');
        k++;
    }
    if (k < len && p[k] == '.') {
        k++;
        double frac = 0.0;
        double div = 1.0;
        while (k < len && p[k] >= '0' && p[k] <= '9') {
            frac = frac * 10.0 + (p[k] - '0');
            div *= 10.0;
            k++;
        }
        int_part += frac / div;
    }
    if (k < len && (p[k] == 'e' || p[k] == 'E')) {
        char buf[64];
        size_t blen = std::min(len, sizeof(buf) - 1);
        std::memcpy(buf, p, blen);
        buf[blen] = '\0';
        return std::strtof(buf, nullptr);
    }
    return static_cast<float>(neg ? -int_part : int_part);
}

inline std::string pdf_decode_text_string(std::string_view raw) {
    if (raw.size() >= 2 && static_cast<uint8_t>(raw[0]) == 0xFE && static_cast<uint8_t>(raw[1]) == 0xFF) {
        // UTF-16BE with BOM
        std::string utf8;
        for (size_t i = 2; i + 1 < raw.size(); i += 2) {
            char16_t c16 = (static_cast<uint8_t>(raw[i]) << 8) | static_cast<uint8_t>(raw[i + 1]);
            if (c16 < 0x80) {
                utf8 += static_cast<char>(c16);
            } else if (c16 < 0x800) {
                utf8 += static_cast<char>(0xC0 | (c16 >> 6));
                utf8 += static_cast<char>(0x80 | (c16 & 0x3F));
            } else {
                utf8 += static_cast<char>(0xE0 | (c16 >> 12));
                utf8 += static_cast<char>(0x80 | ((c16 >> 6) & 0x3F));
                utf8 += static_cast<char>(0x80 | (c16 & 0x3F));
            }
        }
        return utf8;
    }
    std::string clean;
    clean.reserve(raw.size());
    for (char ch : raw) {
        unsigned char uc = static_cast<unsigned char>(ch);
        if (uc >= 32 || uc == '\t' || uc == '\n' || uc == '\r' || uc >= 128) {
            clean += ch;
        } else {
            clean += ' ';
        }
    }
    return clean;
}

Color eval_color_components(const std::vector<float>& comps, const std::string& cs) {
    if (cs == "DeviceGray" || comps.size() == 1) {
        float g = comps.empty() ? 0.0f : std::clamp(comps[0], 0.0f, 1.0f);
        return Color::from_rgba_unchecked(g, g, g, 1.0f);
    } else if (cs == "DeviceCMYK" || comps.size() >= 4) {
        float c = std::clamp(comps[0], 0.0f, 1.0f);
        float m = std::clamp(comps[1], 0.0f, 1.0f);
        float y = std::clamp(comps[2], 0.0f, 1.0f);
        float k = std::clamp(comps[3], 0.0f, 1.0f);
        float r = (1.0f - c) * (1.0f - k);
        float g = (1.0f - m) * (1.0f - k);
        float b = (1.0f - y) * (1.0f - k);
        return Color::from_rgba_unchecked(r, g, b, 1.0f);
    } else { // Default DeviceRGB
        float r = comps.size() > 0 ? std::clamp(comps[0], 0.0f, 1.0f) : 0.0f;
        float g = comps.size() > 1 ? std::clamp(comps[1], 0.0f, 1.0f) : 0.0f;
        float b = comps.size() > 2 ? std::clamp(comps[2], 0.0f, 1.0f) : 0.0f;
        return Color::from_rgba_unchecked(r, g, b, 1.0f);
    }
}

std::vector<float> eval_pdf_func_impl(
    PdfParser& parser,
    const PdfValue& func_val,
    float t,
    int depth = 0
) {
    if (depth > 8) return {0.0f, 0.0f, 0.0f};

    const PdfValue* val_ptr = &func_val;
    std::optional<PdfIndirectObject> resolved_holder;
    if (val_ptr->is_ref()) {
        resolved_holder = parser.resolve(val_ptr->as_ref());
        if (!resolved_holder) return {0.0f, 0.0f, 0.0f};
        val_ptr = &resolved_holder->value;
    }

    if (val_ptr->is_array()) {
        const auto& arr = val_ptr->as_array();
        std::vector<float> comps;
        for (const auto& sub_fn : arr) {
            auto sub_res = eval_pdf_func_impl(parser, sub_fn, t, depth + 1);
            if (!sub_res.empty()) comps.push_back(sub_res[0]);
            else comps.push_back(0.0f);
        }
        return comps;
    }

    if (!val_ptr->is_dict()) {
        return {0.0f, 0.0f, 0.0f};
    }

    int func_type = 2;
    if (const auto* ft = val_ptr->find("FunctionType")) {
        func_type = static_cast<int>(ft->as_int(2));
    }

    float d0 = 0.0f, d1 = 1.0f;
    if (const auto* dom = val_ptr->find("Domain")) {
        if (dom->is_array() && dom->as_array().size() >= 2) {
            d0 = dom->as_array()[0].as_float(0.0f);
            d1 = dom->as_array()[1].as_float(1.0f);
        }
    }
    float clamped_t = (d1 > d0) ? std::clamp(t, d0, d1) : t;

    if (func_type == 2) { // Exponential Interpolation Function
        float n = 1.0f;
        if (const auto* nv = val_ptr->find("N")) n = nv->as_float(1.0f);

        std::vector<float> c0 = {0.0f, 0.0f, 0.0f};
        if (const auto* c0v = val_ptr->find("C0")) {
            if (c0v->is_array()) {
                c0.clear();
                for (const auto& v : c0v->as_array()) c0.push_back(v.as_float(0.0f));
            }
        }

        std::vector<float> c1 = {1.0f, 1.0f, 1.0f};
        if (const auto* c1v = val_ptr->find("C1")) {
            if (c1v->is_array()) {
                c1.clear();
                for (const auto& v : c1v->as_array()) c1.push_back(v.as_float(1.0f));
            }
        }

        float norm_t = (d1 > d0) ? (clamped_t - d0) / (d1 - d0) : clamped_t;
        norm_t = std::clamp(norm_t, 0.0f, 1.0f);
        float u = (n == 1.0f) ? norm_t : std::pow(norm_t, n);

        size_t count = std::max(c0.size(), c1.size());
        std::vector<float> res(count, 0.0f);
        for (size_t i = 0; i < count; ++i) {
            float v0 = (i < c0.size()) ? c0[i] : 0.0f;
            float v1 = (i < c1.size()) ? c1[i] : 1.0f;
            res[i] = v0 + u * (v1 - v0);
        }
        return res;
    } else if (func_type == 3) { // Stitching Function
        const auto* funcs_val = val_ptr->find("Functions");
        const auto* bounds_val = val_ptr->find("Bounds");
        const auto* encode_val = val_ptr->find("Encode");

        if (funcs_val && funcs_val->is_array()) {
            const auto& funcs = funcs_val->as_array();
            size_t k = funcs.size();
            std::vector<float> bounds;
            if (bounds_val && bounds_val->is_array()) {
                for (const auto& b : bounds_val->as_array()) bounds.push_back(b.as_float(0.0f));
            }
            std::vector<float> encode;
            if (encode_val && encode_val->is_array()) {
                for (const auto& e : encode_val->as_array()) encode.push_back(e.as_float(0.0f));
            }

            size_t seg = 0;
            while (seg < bounds.size() && clamped_t > bounds[seg]) {
                seg++;
            }
            if (seg >= k) seg = k - 1;

            float b_low = (seg == 0) ? d0 : bounds[seg - 1];
            float b_high = (seg < bounds.size()) ? bounds[seg] : d1;

            float seg_norm = (b_high > b_low) ? (clamped_t - b_low) / (b_high - b_low) : 0.0f;
            seg_norm = std::clamp(seg_norm, 0.0f, 1.0f);

            float e0 = (seg * 2 < encode.size()) ? encode[seg * 2] : 0.0f;
            float e1 = (seg * 2 + 1 < encode.size()) ? encode[seg * 2 + 1] : 1.0f;
            float sub_t = e0 + seg_norm * (e1 - e0);

            return eval_pdf_func_impl(parser, funcs[seg], sub_t, depth + 1);
        }
    } else if (func_type == 0) { // Sampled Function
        if (resolved_holder && resolved_holder->stream_data) {
            const auto& strm = *resolved_holder->stream_data;
            if (!strm.empty()) {
                float norm_t = (d1 > d0) ? (clamped_t - d0) / (d1 - d0) : clamped_t;
                norm_t = std::clamp(norm_t, 0.0f, 1.0f);
                size_t num_samples = strm.size();
                size_t idx = static_cast<size_t>(norm_t * static_cast<float>(num_samples - 1));
                float v = static_cast<float>(strm[idx]) / 255.0f;
                return {v, v, v};
            }
        }
    }

    return {clamped_t, clamped_t, clamped_t};
}

inline BlendMode pdf_parse_blend_mode(const PdfValue& val) {
    std::string name;
    if (val.is_name()) {
        name = val.as_name();
    } else if (val.is_array() && !val.as_array().empty()) {
        for (const auto& elem : val.as_array()) {
            if (elem.is_name()) {
                name = elem.as_name();
                break;
            }
        }
    }
    if (!name.empty() && name.front() == '/') name = name.substr(1);

    if (name == "Normal" || name == "Compatible") return BlendMode::SourceOver;
    if (name == "Multiply") return BlendMode::Multiply;
    if (name == "Screen") return BlendMode::Screen;
    if (name == "Overlay") return BlendMode::Overlay;
    if (name == "Darken") return BlendMode::Darken;
    if (name == "Lighten") return BlendMode::Lighten;
    if (name == "ColorDodge") return BlendMode::ColorDodge;
    if (name == "ColorBurn") return BlendMode::ColorBurn;
    if (name == "HardLight") return BlendMode::HardLight;
    if (name == "SoftLight") return BlendMode::SoftLight;
    if (name == "Difference") return BlendMode::Difference;
    if (name == "Exclusion") return BlendMode::Exclusion;
    if (name == "Hue") return BlendMode::Hue;
    if (name == "Saturation") return BlendMode::Saturation;
    if (name == "Color") return BlendMode::Color;
    if (name == "Luminosity") return BlendMode::Luminosity;

    return BlendMode::SourceOver;
}

inline Color resolve_color_from_space(
    const PdfValue& cs_val,
    const std::vector<float>& comps,
    float alpha,
    PdfParser& parser
) {
    if (cs_val.is_array()) {
        const auto& arr = cs_val.as_array();
        if (!arr.empty()) {
            std::string cs_type = arr[0].is_name() ? arr[0].as_name() : "";
            if (!cs_type.empty() && cs_type.front() == '/') cs_type = cs_type.substr(1);

            if (cs_type == "Indexed" && arr.size() >= 4 && !comps.empty()) {
                std::string base_cs = arr[1].is_name() ? arr[1].as_name() : "DeviceRGB";
                if (!base_cs.empty() && base_cs.front() == '/') base_cs = base_cs.substr(1);
                int64_t hival = arr[2].as_int(255);
                int64_t idx = static_cast<int64_t>(comps[0]);
                idx = std::clamp(idx, int64_t(0), hival);

                std::string lookup_data;
                if (arr[3].is_string()) {
                    lookup_data = arr[3].as_string();
                } else if (arr[3].is_ref()) {
                    auto lk_obj = parser.resolve(arr[3].as_ref());
                    if (lk_obj && lk_obj->stream_data.has_value()) {
                        const auto& sd = *lk_obj->stream_data;
                        lookup_data.assign(reinterpret_cast<const char*>(sd.data()), sd.size());
                    }
                }

                if (!lookup_data.empty()) {
                    const uint8_t* ptr = reinterpret_cast<const uint8_t*>(lookup_data.data());
                    size_t len = lookup_data.size();
                    if (base_cs == "DeviceGray" && static_cast<size_t>(idx) < len) {
                        float g = ptr[idx] / 255.0f;
                        return Color::from_rgba_unchecked(g, g, g, alpha);
                    } else if (base_cs == "DeviceCMYK" && static_cast<size_t>(idx * 4 + 3) < len) {
                        float c = ptr[idx * 4] / 255.0f;
                        float m = ptr[idx * 4 + 1] / 255.0f;
                        float y = ptr[idx * 4 + 2] / 255.0f;
                        float k = ptr[idx * 4 + 3] / 255.0f;
                        float r = (1.0f - c) * (1.0f - k);
                        float g = (1.0f - m) * (1.0f - k);
                        float b = (1.0f - y) * (1.0f - k);
                        return Color::from_rgba_unchecked(r, g, b, alpha);
                    } else if (static_cast<size_t>(idx * 3 + 2) < len) { // DeviceRGB default
                        float r = ptr[idx * 3] / 255.0f;
                        float g = ptr[idx * 3 + 1] / 255.0f;
                        float b = ptr[idx * 3 + 2] / 255.0f;
                        return Color::from_rgba_unchecked(r, g, b, alpha);
                    }
                }
            } else if ((cs_type == "Separation" || cs_type == "DeviceN") && arr.size() >= 4) {
                float tint = comps.empty() ? 1.0f : comps[0];
                const auto& tint_fn = arr[3];
                std::string alt_cs = arr[2].is_name() ? arr[2].as_name() : "DeviceRGB";
                if (!alt_cs.empty() && alt_cs.front() == '/') alt_cs = alt_cs.substr(1);

                auto out_comps = eval_pdf_func_impl(parser, tint_fn, tint);
                Color c = eval_color_components(out_comps, alt_cs);
                c.set_alpha(alpha);
                return c;
            }
        }
    }

    if (comps.size() == 1) {
        return Color::from_rgba_unchecked(comps[0], comps[0], comps[0], alpha);
    } else if (comps.size() == 3) {
        return Color::from_rgba_unchecked(comps[0], comps[1], comps[2], alpha);
    } else if (comps.size() >= 4) {
        float r = (1.0f - comps[0]) * (1.0f - comps[3]);
        float g = (1.0f - comps[1]) * (1.0f - comps[3]);
        float b = (1.0f - comps[2]) * (1.0f - comps[3]);
        return Color::from_rgba_unchecked(r, g, b, alpha);
    }

    return Color::from_rgba_unchecked(0.0f, 0.0f, 0.0f, alpha);
}

inline PdfValue resolve_cs_helper(const std::string& name, const PdfDict& resources, PdfParser& parser) {
    if (name == "DeviceRGB" || name == "DeviceGray" || name == "DeviceCMYK" || name == "Pattern") {
        return PdfValue(PdfName(name));
    }
    auto it_cs = resources.find("ColorSpace");
    if (it_cs != resources.end()) {
        PdfDict cs_dict;
        if (it_cs->second.is_dict()) cs_dict = it_cs->second.as_dict();
        else if (it_cs->second.is_ref()) {
            auto r = parser.resolve(it_cs->second.as_ref());
            if (r && r->value.is_dict()) cs_dict = r->value.as_dict();
        }
        auto it = cs_dict.find(name);
        if (it != cs_dict.end()) {
            if (it->second.is_ref()) {
                auto r = parser.resolve(it->second.as_ref());
                if (r) return r->value;
            }
            return it->second;
        }
    }
    return PdfValue(PdfName(name));
}

inline std::optional<Pixmap> decode_pdf_image(
    const PdfIndirectObject& resolved,
    PdfParser& parser,
    const PdfDict& resources,
    const Color& fill_color
) {
    if (!resolved.stream_data.has_value()) return std::nullopt;
    const auto& strm = *resolved.stream_data;

    // First attempt sovereign PNG/JPEG/WEBP/BMP/etc. header detection & decoding
    auto img_res = image::load_image_from_memory(strm);
    if (img_res.has_value()) {
        return std::move(img_res.value);
    }

    uint32_t w = static_cast<uint32_t>(resolved.value.find("Width") ? resolved.value.find("Width")->as_int(1) : 1);
    uint32_t h = static_cast<uint32_t>(resolved.value.find("Height") ? resolved.value.find("Height")->as_int(1) : 1);
    if (w == 0 || h == 0 || w > 16384 || h > 16384) return std::nullopt;

    bool is_mask = false;
    if (const auto* im = resolved.value.find("ImageMask")) {
        is_mask = im->as_bool(false);
    }

    int bpc = 8;
    if (const auto* bpc_val = resolved.value.find("BitsPerComponent")) {
        bpc = static_cast<int>(bpc_val->as_int(8));
    }
    if (is_mask) bpc = 1;

    // Decode array [min max ...]
    std::vector<float> decode_arr;
    if (const auto* dec = resolved.value.find("Decode")) {
        if (dec->is_array()) {
            for (const auto& dv : dec->as_array()) {
                decode_arr.push_back(dv.as_float(0.0f));
            }
        }
    }

    auto px_opt = Pixmap::allocate(w, h);
    if (!px_opt) return std::nullopt;

    if (is_mask) {
        bool invert = (decode_arr.size() >= 2 && decode_arr[0] > decode_arr[1]);
        size_t row_stride = (w + 7) / 8;
        ColorU8 cu8 = fill_color.to_color_u8();
        for (uint32_t y = 0; y < h; ++y) {
            size_t row_start = y * row_stride;
            if (row_start >= strm.size()) break;
            for (uint32_t x = 0; x < w; ++x) {
                size_t byte_idx = row_start + (x >> 3);
                if (byte_idx >= strm.size()) break;
                uint8_t byte = strm[byte_idx];
                int bit = (byte >> (7 - (x & 7))) & 1;
                bool paint = invert ? (bit == 0) : (bit == 1);
                if (paint) {
                    px_opt->set_pixel(x, y, cu8.premultiply());
                } else {
                    px_opt->set_pixel(x, y, PremultipliedColorU8(0, 0, 0, 0));
                }
            }
        }
        return std::move(*px_opt);
    }

    // Determine colorspace
    PdfValue cs_val(PdfName("DeviceRGB"));
    if (const auto* cs = resolved.value.find("ColorSpace")) {
        if (cs->is_name()) {
            std::string name = cs->as_name();
            if (!name.empty() && name.front() == '/') name = name.substr(1);
            cs_val = resolve_cs_helper(name, resources, parser);
        } else if (cs->is_array()) {
            cs_val = *cs;
        } else if (cs->is_ref()) {
            auto r = parser.resolve(cs->as_ref());
            if (r) cs_val = r->value;
        }
    } else {
        if (strm.size() >= w * h * 4) cs_val = PdfValue(PdfName("DeviceCMYK"));
        else if (strm.size() >= w * h * 3) cs_val = PdfValue(PdfName("DeviceRGB"));
        else cs_val = PdfValue(PdfName("DeviceGray"));
    }

    std::string cs_type;
    if (cs_val.is_name()) {
        cs_type = cs_val.as_name();
        if (!cs_type.empty() && cs_type.front() == '/') cs_type = cs_type.substr(1);
    } else if (cs_val.is_array() && !cs_val.as_array().empty()) {
        if (cs_val.as_array()[0].is_name()) {
            cs_type = cs_val.as_array()[0].as_name();
            if (!cs_type.empty() && cs_type.front() == '/') cs_type = cs_type.substr(1);
        }
    }

    if (cs_type == "Indexed" && cs_val.is_array() && cs_val.as_array().size() >= 4) {
        const auto& arr = cs_val.as_array();
        std::string base_cs = arr[1].is_name() ? arr[1].as_name() : "DeviceRGB";
        if (!base_cs.empty() && base_cs.front() == '/') base_cs = base_cs.substr(1);
        int64_t hival = arr[2].as_int(255);

        std::string lookup_data;
        if (arr[3].is_string()) {
            lookup_data = arr[3].as_string();
        } else if (arr[3].is_ref()) {
            auto lk_obj = parser.resolve(arr[3].as_ref());
            if (lk_obj && lk_obj->stream_data.has_value()) {
                const auto& sd = *lk_obj->stream_data;
                lookup_data.assign(reinterpret_cast<const char*>(sd.data()), sd.size());
            } else if (lk_obj && lk_obj->value.is_string()) {
                lookup_data = lk_obj->value.as_string();
            }
        }
        const uint8_t* lk_ptr = reinterpret_cast<const uint8_t*>(lookup_data.data());
        size_t lk_len = lookup_data.size();

        size_t row_stride = (bpc == 8) ? w :
                            (bpc == 4) ? ((w + 1) / 2) :
                            (bpc == 2) ? ((w + 3) / 4) :
                            ((w + 7) / 8);

        for (uint32_t y = 0; y < h; ++y) {
            size_t row_start = y * row_stride;
            if (row_start >= strm.size()) break;
            for (uint32_t x = 0; x < w; ++x) {
                int64_t idx = 0;
                if (bpc == 8) {
                    size_t pos = row_start + x;
                    if (pos < strm.size()) idx = strm[pos];
                } else if (bpc == 4) {
                    size_t pos = row_start + (x >> 1);
                    if (pos < strm.size()) {
                        idx = (x & 1) ? (strm[pos] & 0x0F) : ((strm[pos] >> 4) & 0x0F);
                    }
                } else if (bpc == 2) {
                    size_t pos = row_start + (x >> 2);
                    if (pos < strm.size()) {
                        idx = (strm[pos] >> (6 - (x & 3) * 2)) & 0x03;
                    }
                } else if (bpc == 1) {
                    size_t pos = row_start + (x >> 3);
                    if (pos < strm.size()) {
                        idx = (strm[pos] >> (7 - (x & 7))) & 0x01;
                    }
                }
                idx = std::clamp(idx, int64_t(0), hival);

                uint8_t r = 0, g = 0, b = 0;
                if (base_cs == "DeviceGray" && static_cast<size_t>(idx) < lk_len) {
                    r = g = b = lk_ptr[idx];
                } else if (base_cs == "DeviceCMYK" && static_cast<size_t>(idx * 4 + 3) < lk_len) {
                    float c_val = lk_ptr[idx * 4] / 255.0f;
                    float m_val = lk_ptr[idx * 4 + 1] / 255.0f;
                    float y_val = lk_ptr[idx * 4 + 2] / 255.0f;
                    float k_val = lk_ptr[idx * 4 + 3] / 255.0f;
                    r = static_cast<uint8_t>(std::clamp((1.0f - c_val) * (1.0f - k_val) * 255.0f, 0.0f, 255.0f));
                    g = static_cast<uint8_t>(std::clamp((1.0f - m_val) * (1.0f - k_val) * 255.0f, 0.0f, 255.0f));
                    b = static_cast<uint8_t>(std::clamp((1.0f - y_val) * (1.0f - k_val) * 255.0f, 0.0f, 255.0f));
                } else if (static_cast<size_t>(idx * 3 + 2) < lk_len) {
                    r = lk_ptr[idx * 3];
                    g = lk_ptr[idx * 3 + 1];
                    b = lk_ptr[idx * 3 + 2];
                }
                px_opt->set_pixel(x, y, PremultipliedColorU8(r, g, b, 255));
            }
        }
        return std::move(*px_opt);
    }

    if (cs_type == "DeviceGray" || (bpc == 8 && strm.size() >= w * h && strm.size() < w * h * 3)) {
        bool invert = (decode_arr.size() >= 2 && decode_arr[0] > decode_arr[1]);
        if (bpc == 8) {
            for (uint32_t y = 0; y < h; ++y) {
                for (uint32_t x = 0; x < w; ++x) {
                    size_t idx = y * w + x;
                    if (idx < strm.size()) {
                        uint8_t g = strm[idx];
                        if (invert) g = 255 - g;
                        px_opt->set_pixel(x, y, PremultipliedColorU8(g, g, g, 255));
                    }
                }
            }
        } else if (bpc == 1) {
            size_t row_stride = (w + 7) / 8;
            for (uint32_t y = 0; y < h; ++y) {
                size_t row_start = y * row_stride;
                for (uint32_t x = 0; x < w; ++x) {
                    size_t pos = row_start + (x >> 3);
                    if (pos < strm.size()) {
                        int bit = (strm[pos] >> (7 - (x & 7))) & 1;
                        uint8_t g = bit ? 255 : 0;
                        if (invert) g = 255 - g;
                        px_opt->set_pixel(x, y, PremultipliedColorU8(g, g, g, 255));
                    }
                }
            }
        }
        return std::move(*px_opt);
    }

    if (cs_type == "DeviceCMYK" || (bpc == 8 && strm.size() >= w * h * 4)) {
        for (uint32_t y = 0; y < h; ++y) {
            for (uint32_t x = 0; x < w; ++x) {
                size_t idx = (y * w + x) * 4;
                if (idx + 3 < strm.size()) {
                    float c_val = strm[idx] / 255.0f;
                    float m_val = strm[idx + 1] / 255.0f;
                    float y_val = strm[idx + 2] / 255.0f;
                    float k_val = strm[idx + 3] / 255.0f;
                    uint8_t r = static_cast<uint8_t>(std::clamp((1.0f - c_val) * (1.0f - k_val) * 255.0f, 0.0f, 255.0f));
                    uint8_t g = static_cast<uint8_t>(std::clamp((1.0f - m_val) * (1.0f - k_val) * 255.0f, 0.0f, 255.0f));
                    uint8_t b = static_cast<uint8_t>(std::clamp((1.0f - y_val) * (1.0f - k_val) * 255.0f, 0.0f, 255.0f));
                    px_opt->set_pixel(x, y, PremultipliedColorU8(r, g, b, 255));
                }
            }
        }
        return std::move(*px_opt);
    }

    // Default: DeviceRGB 8-bit
    if (strm.size() >= w * h * 3) {
        for (uint32_t y = 0; y < h; ++y) {
            for (uint32_t x = 0; x < w; ++x) {
                size_t idx = (y * w + x) * 3;
                if (idx + 2 < strm.size()) {
                    px_opt->set_pixel(x, y, PremultipliedColorU8(strm[idx], strm[idx + 1], strm[idx + 2], 255));
                }
            }
        }
        return std::move(*px_opt);
    }

    return std::nullopt;
}

} // namespace

PdfInterpreter::PdfInterpreter(PdfParser& parser) : parser_(parser) {}

std::shared_ptr<PdfCMap> PdfInterpreter::find_or_load_cmap(
    const PdfDict& resources,
    const std::string& font_name,
    text::FontSystem* font_system
) {
    auto it_fonts = resources.find("Font");
    if (it_fonts == resources.end()) return nullptr;

    PdfDict font_dict_map;
    if (it_fonts->second.is_dict()) {
        font_dict_map = it_fonts->second.as_dict();
    } else if (it_fonts->second.is_ref()) {
        auto r = parser_.resolve(it_fonts->second.as_ref());
        if (r && r->value.is_dict()) font_dict_map = r->value.as_dict();
    }

    auto it_font = font_dict_map.find(font_name);
    if (it_font == font_dict_map.end()) return nullptr;

    PdfDict font_obj_dict;
    if (it_font->second.is_dict()) {
        font_obj_dict = it_font->second.as_dict();
    } else if (it_font->second.is_ref()) {
        auto r = parser_.resolve(it_font->second.as_ref());
        if (r && r->value.is_dict()) font_obj_dict = r->value.as_dict();
    }

    std::shared_ptr<PdfCMap> result_cmap = nullptr;

    // 1. Check if ToUnicode stream exists
    auto it_to_unicode = font_obj_dict.find("ToUnicode");
    if (it_to_unicode != font_obj_dict.end() && it_to_unicode->second.is_ref()) {
        PdfRef tu_ref = it_to_unicode->second.as_ref();
        auto it_cached = cmap_cache_.find(tu_ref);
        if (it_cached != cmap_cache_.end()) {
            result_cmap = it_cached->second;
        } else {
            auto tu_obj = parser_.resolve(tu_ref);
            if (tu_obj && tu_obj->stream_data.has_value()) {
                result_cmap = PdfCMap::parse(*tu_obj->stream_data);
                cmap_cache_[tu_ref] = result_cmap;
            }
        }
    }

    // 2. Also check if the font has an embedded TTF file in FontDescriptor
    if (font_system) {
        auto it_desc = font_obj_dict.find("FontDescriptor");
        PdfDict desc_dict;
        if (it_desc != font_obj_dict.end()) {
            if (it_desc->second.is_dict()) desc_dict = it_desc->second.as_dict();
            else if (it_desc->second.is_ref()) {
                auto r = parser_.resolve(it_desc->second.as_ref());
                if (r && r->value.is_dict()) desc_dict = r->value.as_dict();
            }
        }
        if (desc_dict.empty()) {
            // Check DescendantFonts for Type0 composite fonts
            auto it_df = font_obj_dict.find("DescendantFonts");
            if (it_df != font_obj_dict.end() && it_df->second.is_array()) {
                const auto& arr = it_df->second.as_array();
                if (!arr.empty() && arr[0].is_ref()) {
                    auto df_obj = parser_.resolve(arr[0].as_ref());
                    if (df_obj && df_obj->value.is_dict()) {
                        if (const auto* fd_val = df_obj->value.find("FontDescriptor")) {
                            if (fd_val->is_dict()) {
                                desc_dict = fd_val->as_dict();
                            } else if (fd_val->is_ref()) {
                                auto r = parser_.resolve(fd_val->as_ref());
                                if (r && r->value.is_dict()) desc_dict = r->value.as_dict();
                            }
                        }
                    }
                }
            }
        }

        if (!desc_dict.empty()) {
            auto it_ff2 = desc_dict.find("FontFile2");
            if (it_ff2 != desc_dict.end() && it_ff2->second.is_ref()) {
                auto ff2_obj = parser_.resolve(it_ff2->second.as_ref());
                if (ff2_obj && ff2_obj->stream_data.has_value()) {
                    font_system->load_font_data(*ff2_obj->stream_data);
                }
            }
        }
    }

    return result_cmap;
}

static void blit_scaled_image_fast(
    Canvas& canvas,
    const PixmapRef& src,
    float dst_x, float dst_y, float dst_w, float dst_h,
    float opacity,
    BlendMode blend_mode
) {
    if (src.is_empty() || dst_w <= 0.0f || dst_h <= 0.0f || opacity <= 0.0f) return;

    int32_t x0 = static_cast<int32_t>(std::floor(dst_x));
    int32_t y0 = static_cast<int32_t>(std::floor(dst_y));
    int32_t x1 = static_cast<int32_t>(std::ceil(dst_x + dst_w));
    int32_t y1 = static_cast<int32_t>(std::ceil(dst_y + dst_h));

    x0 = std::max<int32_t>(0, x0);
    y0 = std::max<int32_t>(0, y0);
    x1 = std::min<int32_t>(static_cast<int32_t>(canvas.width()), x1);
    y1 = std::min<int32_t>(static_cast<int32_t>(canvas.height()), y1);

    if (canvas.scissor_clip().has_value()) {
        const auto& sc = *canvas.scissor_clip();
        x0 = std::max<int32_t>(x0, sc.left());
        y0 = std::max<int32_t>(y0, sc.top());
        x1 = std::min<int32_t>(x1, sc.right());
        y1 = std::min<int32_t>(y1, sc.bottom());
    }

    if (x0 >= x1 || y0 >= y1) return;

    float inv_w = static_cast<float>(src.width()) / dst_w;
    float inv_h = static_cast<float>(src.height()) / dst_h;
    uint32_t src_w = src.width();
    uint32_t src_h = src.height();
    size_t span_w = static_cast<size_t>(x1 - x0);

    // Fast-path: Downscaling and 1:1 image mapping use direct scanline nearest-neighbor sampling.
    // Bilinear interpolation is only applied during upscaling magnification.
    bool is_upscaled = (dst_w > static_cast<float>(src_w) || dst_h > static_cast<float>(src_h));

    if (!is_upscaled) {
        uint32_t stack_x_indices[2048];
        std::vector<uint32_t> heap_x_indices;
        uint32_t* x_indices = (span_w <= 2048) ? stack_x_indices : (heap_x_indices.resize(span_w), heap_x_indices.data());

        for (int32_t x = x0; x < x1; ++x) {
            float sx_f = (static_cast<float>(x) + 0.5f - dst_x) * inv_w;
            uint32_t sx = static_cast<uint32_t>(std::clamp(sx_f, 0.0f, static_cast<float>(src_w - 1)));
            x_indices[x - x0] = sx;
        }

        uint32_t op_u16 = static_cast<uint32_t>(std::clamp(opacity, 0.0f, 1.0f) * 256.0f);
        bool is_fully_opaque = (op_u16 >= 256 && blend_mode == BlendMode::SourceOver);

        for (int32_t y = y0; y < y1; ++y) {
            float sy_f = (static_cast<float>(y) + 0.5f - dst_y) * inv_h;
            uint32_t sy = static_cast<uint32_t>(std::clamp(sy_f, 0.0f, static_cast<float>(src_h - 1)));
            const PremultipliedColorU8* src_row = src.row(sy);
            PremultipliedColorU8* dst_row = canvas.pixmap().row(static_cast<size_t>(y));

            if (is_fully_opaque) {
                for (int32_t x = x0; x < x1; ++x) {
                    const auto& c = src_row[x_indices[x - x0]];
                    if (c.a == 255) {
                        dst_row[x] = c;
                    } else if (c.a > 0) {
                        uint32_t inv_a = 255 - c.a;
                        const auto& dst_c = dst_row[x];
                        uint8_t out_r = static_cast<uint8_t>(c.r + ((dst_c.r * inv_a + 127) / 255));
                        uint8_t out_g = static_cast<uint8_t>(c.g + ((dst_c.g * inv_a + 127) / 255));
                        uint8_t out_b = static_cast<uint8_t>(c.b + ((dst_c.b * inv_a + 127) / 255));
                        uint8_t out_a = static_cast<uint8_t>(c.a + ((dst_c.a * inv_a + 127) / 255));
                        dst_row[x] = PremultipliedColorU8::from_rgba_unchecked(out_r, out_g, out_b, out_a);
                    }
                }
            } else {
                for (int32_t x = x0; x < x1; ++x) {
                    const auto& c = src_row[x_indices[x - x0]];
                    uint32_t r = (c.r * op_u16) >> 8;
                    uint32_t g = (c.g * op_u16) >> 8;
                    uint32_t b = (c.b * op_u16) >> 8;
                    uint32_t a = (c.a * op_u16) >> 8;
                    if (blend_mode == BlendMode::SourceOver) {
                        if (a == 255) {
                            dst_row[x] = PremultipliedColorU8::from_rgba_unchecked(static_cast<uint8_t>(r), static_cast<uint8_t>(g), static_cast<uint8_t>(b), 255);
                        } else if (a > 0) {
                            uint32_t inv_a = 255 - a;
                            const auto& dst_c = dst_row[x];
                            uint8_t out_r = static_cast<uint8_t>(r + ((dst_c.r * inv_a + 127) / 255));
                            uint8_t out_g = static_cast<uint8_t>(g + ((dst_c.g * inv_a + 127) / 255));
                            uint8_t out_b = static_cast<uint8_t>(b + ((dst_c.b * inv_a + 127) / 255));
                            uint8_t out_a = static_cast<uint8_t>(a + ((dst_c.a * inv_a + 127) / 255));
                            dst_row[x] = PremultipliedColorU8::from_rgba_unchecked(out_r, out_g, out_b, out_a);
                        }
                    }
                }
            }
        }
        return;
    }

    struct XMap {
        uint32_t x0, x1;
        uint32_t fx, inv_fx;
    };
    XMap stack_x_map[2048];
    std::vector<XMap> heap_x_map;
    XMap* x_map = (span_w <= 2048) ? stack_x_map : (heap_x_map.resize(span_w), heap_x_map.data());

    for (int32_t x = x0; x < x1; ++x) {
        float sx_f = (static_cast<float>(x) + 0.5f - dst_x) * inv_w - 0.5f;
        sx_f = std::clamp(sx_f, 0.0f, static_cast<float>(src_w - 1));
        uint32_t sx0 = static_cast<uint32_t>(sx_f);
        uint32_t sx1 = std::min(sx0 + 1, src_w - 1);
        uint32_t fx = static_cast<uint32_t>((sx_f - static_cast<float>(sx0)) * 256.0f);
        x_map[x - x0] = {sx0, sx1, fx, 256 - fx};
    }

    uint32_t op_u16 = static_cast<uint32_t>(std::clamp(opacity, 0.0f, 1.0f) * 256.0f);

    for (int32_t y = y0; y < y1; ++y) {
        float sy_f = (static_cast<float>(y) + 0.5f - dst_y) * inv_h - 0.5f;
        sy_f = std::clamp(sy_f, 0.0f, static_cast<float>(src_h - 1));
        uint32_t sy0 = static_cast<uint32_t>(sy_f);
        uint32_t sy1 = std::min(sy0 + 1, src_h - 1);
        uint32_t fy = static_cast<uint32_t>((sy_f - static_cast<float>(sy0)) * 256.0f);
        uint32_t inv_fy = 256 - fy;

        const PremultipliedColorU8* row0 = src.row(sy0);
        const PremultipliedColorU8* row1 = src.row(sy1);
        PremultipliedColorU8* dst_row = canvas.pixmap().row(static_cast<size_t>(y));

        for (int32_t x = x0; x < x1; ++x) {
            const auto& xm = x_map[x - x0];
            const auto& c00 = row0[xm.x0];
            const auto& c01 = row0[xm.x1];
            const auto& c10 = row1[xm.x0];
            const auto& c11 = row1[xm.x1];

            uint32_t t_r = (c00.r * xm.inv_fx + c01.r * xm.fx) >> 8;
            uint32_t t_g = (c00.g * xm.inv_fx + c01.g * xm.fx) >> 8;
            uint32_t t_b = (c00.b * xm.inv_fx + c01.b * xm.fx) >> 8;
            uint32_t t_a = (c00.a * xm.inv_fx + c01.a * xm.fx) >> 8;

            uint32_t b_r = (c10.r * xm.inv_fx + c11.r * xm.fx) >> 8;
            uint32_t b_g = (c10.g * xm.inv_fx + c11.g * xm.fx) >> 8;
            uint32_t b_b = (c10.b * xm.inv_fx + c11.b * xm.fx) >> 8;
            uint32_t b_a = (c10.a * xm.inv_fx + c11.a * xm.fx) >> 8;

            uint32_t r = (t_r * inv_fy + b_r * fy) >> 8;
            uint32_t g = (t_g * inv_fy + b_g * fy) >> 8;
            uint32_t b = (t_b * inv_fy + b_b * fy) >> 8;
            uint32_t a = (t_a * inv_fy + b_a * fy) >> 8;

            if (op_u16 < 256) {
                r = (r * op_u16) >> 8;
                g = (g * op_u16) >> 8;
                b = (b * op_u16) >> 8;
                a = (a * op_u16) >> 8;
            }

            if (blend_mode == BlendMode::SourceOver) {
                if (a == 255) {
                    dst_row[x] = PremultipliedColorU8::from_rgba_unchecked(
                        static_cast<uint8_t>(r), static_cast<uint8_t>(g), static_cast<uint8_t>(b), 255
                    );
                } else if (a > 0) {
                    uint32_t inv_a = 255 - a;
                    const auto& dst_c = dst_row[x];
                    uint8_t out_r = static_cast<uint8_t>(r + ((dst_c.r * inv_a + 127) / 255));
                    uint8_t out_g = static_cast<uint8_t>(g + ((dst_c.g * inv_a + 127) / 255));
                    uint8_t out_b = static_cast<uint8_t>(b + ((dst_c.b * inv_a + 127) / 255));
                    uint8_t out_a = static_cast<uint8_t>(a + ((dst_c.a * inv_a + 127) / 255));
                    dst_row[x] = PremultipliedColorU8::from_rgba_unchecked(out_r, out_g, out_b, out_a);
                }
            }
        }
    }
}

bool PdfInterpreter::interpret_page(
    const ParsedPdfPage& page,
    Canvas& canvas,
    float scale,
    text::FontSystem* font_system
) {
    if (page.contents.empty()) return true;

    execute_stream(
        page.contents,
        page.resources,
        canvas,
        page.media_box.height(),
        scale,
        &canvas,
        font_system
    );
    return true;
}

bool PdfInterpreter::interpret_page(
    const ParsedPdfPage& page,
    ICanvas& canvas,
    float scale,
    text::FontSystem* font_system
) {
    if (page.contents.empty()) return true;

    Canvas* cpu_canvas = dynamic_cast<Canvas*>(&canvas);
    execute_stream(
        page.contents,
        page.resources,
        canvas,
        page.media_box.height(),
        scale,
        cpu_canvas,
        font_system
    );
    return true;
}

void PdfInterpreter::execute_stream(
    std::span<const uint8_t> stream,
    const PdfDict& resources,
    ICanvas& canvas,
    float page_height,
    float scale,
    Canvas* cpu_canvas,
    text::FontSystem* font_system,
    int depth,
    const Transform& initial_ctm
) {
    PdfGraphicsState state;
    if (depth == 0) {
        Transform base_ctm(
            scale, 0.0f,
            0.0f, -scale,
            0.0f, scale * page_height
        );
        state.ctm = base_ctm;
    } else {
        state.ctm = initial_ctm;
    }
    std::vector<PdfGraphicsState> state_stack;

    double t_path_accum = 0.0;
    double t_text_accum = 0.0;
    double t_image_accum = 0.0;
    double t_form_accum = 0.0;
    size_t count_paths = 0;
    size_t count_texts = 0;
    size_t count_images = 0;

    PathBuilder current_builder;
    Point current_pt(0, 0);
    bool is_rect_path = false;
    std::optional<Rect> rect_path_val = std::nullopt;

    std::vector<PdfOpItem> stack;
    stack.reserve(64);
    size_t i = 0;

    auto pop_float = [&stack](float def = 0.0f) -> float {
        if (stack.empty()) return def;
        auto item = std::move(stack.back());
        stack.pop_back();
        return item.as_float(def);
    };

    auto pop_string = [&stack]() -> std::string {
        if (stack.empty()) return "";
        auto item = std::move(stack.back());
        stack.pop_back();
        if (item.is_num) return std::to_string(item.num);
        return std::move(item.str);
    };

    auto push_str_item = [&stack](std::string s) {
        if (stack.size() < 256) {
            PdfOpItem item;
            item.str = std::move(s);
            item.is_num = false;
            stack.push_back(std::move(item));
        }
    };

    auto paint_path = [&](bool fill, bool stroke, FillRule rule) {
        auto t0_p = std::chrono::high_resolution_clock::now();

        // 1. Fast-path: if clipping is pending and was a simple axis-aligned rectangle
        if (state.clip_pending) {
            bool clipped_fast = false;
            if (is_rect_path && rect_path_val.has_value() && cpu_canvas &&
                std::abs(state.ctm.kx) < 1e-4f && std::abs(state.ctm.ky) < 1e-4f) {
                float x0 = rect_path_val->left() * state.ctm.sx + state.ctm.tx;
                float x1 = rect_path_val->right() * state.ctm.sx + state.ctm.tx;
                float y0 = rect_path_val->top() * state.ctm.sy + state.ctm.ty;
                float y1 = rect_path_val->bottom() * state.ctm.sy + state.ctm.ty;
                if (x0 > x1) std::swap(x0, x1);
                if (y0 > y1) std::swap(y0, y1);
                auto tr_rect = Rect::from_ltrb(x0, y0, x1, y1);
                if (tr_rect) {
                    cpu_canvas->clip_rect(*tr_rect);
                    clipped_fast = true;
                }
            }
            if (!clipped_fast) {
                auto p_clip = current_builder.finish();
                if (p_clip && !p_clip->is_empty()) {
                    auto transformed = p_clip->transform(state.ctm);
                    if (transformed && cpu_canvas) {
                        cpu_canvas->clip_path(*transformed, state.clip_rule);
                    }
                }
            }
            state.clip_pending = false;
        }

        // 2. Fast-path: axis-aligned rectangle fill without stroke and without complex pattern
        if (is_rect_path && rect_path_val.has_value() && fill && !stroke &&
            !state.fill_paint.has_value() &&
            std::abs(state.ctm.kx) < 1e-4f && std::abs(state.ctm.ky) < 1e-4f) {
            float x0 = rect_path_val->left() * state.ctm.sx + state.ctm.tx;
            float x1 = rect_path_val->right() * state.ctm.sx + state.ctm.tx;
            float y0 = rect_path_val->top() * state.ctm.sy + state.ctm.ty;
            float y1 = rect_path_val->bottom() * state.ctm.sy + state.ctm.ty;
            if (x0 > x1) std::swap(x0, x1);
            if (y0 > y1) std::swap(y0, y1);
            auto tr_rect = Rect::from_ltrb(x0, y0, x1, y1);
            if (tr_rect) {
                Paint p(state.fill_color);
                p.blend_mode = state.blend_mode;
                canvas.fill_rect(*tr_rect, p);
                current_builder = PathBuilder();
                current_pt = Point(0, 0);
                is_rect_path = false;
                rect_path_val = std::nullopt;
                auto t1_p = std::chrono::high_resolution_clock::now();
                t_path_accum += std::chrono::duration<double, std::milli>(t1_p - t0_p).count();
                count_paths++;
                return;
            }
        }

        // 3. General path rasterization
        auto p = current_builder.finish();
        if (p && !p->is_empty()) {
            auto transformed = p->transform(state.ctm);
            if (transformed) {
                if (fill) {
                    if (state.fill_paint.has_value()) {
                        Paint p = *state.fill_paint;
                        p.blend_mode = state.blend_mode;
                        canvas.fill_path(*transformed, p, rule);
                    } else {
                        Paint p(state.fill_color);
                        p.blend_mode = state.blend_mode;
                        canvas.fill_path(*transformed, p, rule);
                    }
                }
                if (stroke) {
                    float sx = std::hypot(state.ctm.sx, state.ctm.ky);
                    Stroke st = state.stroke;
                    st.width = std::max(0.5f, st.width * sx);
                    if (state.stroke_paint.has_value()) {
                        Paint p = *state.stroke_paint;
                        p.blend_mode = state.blend_mode;
                        canvas.stroke_path(*transformed, p, st);
                    } else {
                        Paint p(state.stroke_color);
                        p.blend_mode = state.blend_mode;
                        canvas.stroke_path(*transformed, p, st);
                    }
                }
            }
        }
        current_builder = PathBuilder();
        current_pt = Point(0, 0);
        is_rect_path = false;
        rect_path_val = std::nullopt;
        auto t1_p = std::chrono::high_resolution_clock::now();
        t_path_accum += std::chrono::duration<double, std::milli>(t1_p - t0_p).count();
        count_paths++;
    };

    thread_local text::GlyphCache shared_glyph_cache;

    auto render_text_span = [&](const std::string& raw_text, const Transform& eff_t, float font_sz, const Color& col) -> float {
        auto t0_txt = std::chrono::high_resolution_clock::now();
        if (!cpu_canvas || raw_text.empty()) return 0.0f;

        Point pt(0.0f, 0.0f);
        eff_t.map_point(pt);

        float font_sc = std::hypot(eff_t.sx, eff_t.ky);
        float eff_font_sz = font_sz * font_sc;
        if (eff_font_sz < 1.0f) eff_font_sz = 1.0f;

        std::string text;
        if (state.current_cmap && !state.current_cmap->is_empty()) {
            text = state.current_cmap->to_utf8(raw_text);
        } else {
            text = pdf_decode_text_string(raw_text);
        }

        float advance_user = 0.0f;
        if (font_system && font_system->font_count() > 0) {
            const text::TtfFont* font = nullptr;
            uint32_t font_id = 0;
            if (font_system->default_font_id().has_value()) {
                font_id = *font_system->default_font_id();
                font = font_system->get_font(font_id);
            } else {
                font = font_system->get_font(0);
            }

            if (font && font->is_valid()) {
                float cur_x = pt.x;
                float baseline_y = pt.y + eff_font_sz * 0.2f;
                Paint tp(col);
                tp.blend_mode = state.blend_mode;
                float font_scale = font->scale_for_size(eff_font_sz);

                for (size_t ci = 0; ci < text.size(); ) {
                    uint32_t cp = 0;
                    uint8_t b0 = static_cast<uint8_t>(text[ci]);
                    size_t len = 1;
                    if (b0 < 0x80) {
                        cp = b0;
                    } else if ((b0 & 0xE0) == 0xC0 && ci + 1 < text.size()) {
                        cp = ((b0 & 0x1F) << 6) | (text[ci+1] & 0x3F);
                        len = 2;
                    } else if ((b0 & 0xF0) == 0xE0 && ci + 2 < text.size()) {
                        cp = ((b0 & 0x0F) << 12) | ((text[ci+1] & 0x3F) << 6) | (text[ci+2] & 0x3F);
                        len = 3;
                    } else if ((b0 & 0xF8) == 0xF0 && ci + 3 < text.size()) {
                        cp = ((b0 & 0x07) << 18) | ((text[ci+1] & 0x3F) << 12) | ((text[ci+2] & 0x3F) << 6) | (text[ci+3] & 0x3F);
                        len = 4;
                    } else {
                        cp = b0;
                    }
                    ci += len;

                    uint16_t gid = font->glyph_index(cp);
                    if (gid != 0) {
                        const text::CachedGlyph* cached = shared_glyph_cache.get_or_render(
                            *font, font_id, gid, eff_font_sz, cur_x, baseline_y
                        );
                        if (cached && !cached->data.empty()) {
                            Point dst_pos = Point::from_xy(
                                cur_x + static_cast<float>(cached->offset_x),
                                baseline_y + static_cast<float>(cached->offset_y)
                            );
                            cpu_canvas->draw_glyph(dst_pos, cached->as_mask_ref(), tp);
                        }
                        cur_x += font->glyph_advance_scaled(gid, font_scale);
                    } else {
                        cur_x += eff_font_sz * 0.5f;
                    }
                }
                float total_advance_px = cur_x - pt.x;
                if (font_sc > 0.0001f) {
                    advance_user = total_advance_px / font_sc;
                }
            } else {
                text::Buffer buf(text::Metrics(eff_font_sz, eff_font_sz * 1.2f));
                text::Attrs attrs;
                ColorU8 cu8 = col.to_color_u8();
                attrs.set_color(text::TextColor::rgba(cu8.r, cu8.g, cu8.b, cu8.a));
                buf.set_text(text, attrs);
                buf.draw(*cpu_canvas, shared_glyph_cache, *font_system, col, pt.x, pt.y - (eff_font_sz * 0.8f));
                if (!buf.lines().empty() && !buf.lines().front().layout().empty()) {
                    float total_px = 0.0f;
                    for (const auto& l : buf.lines().front().layout()) {
                        total_px += l.w;
                    }
                    if (font_sc > 0.0001f) {
                        advance_user = total_px / font_sc;
                    }
                }
            }
        } else {
            float draw_scale = std::max(0.5f, font_sc / 16.0f);
            Paint tp(col);
            tp.blend_mode = state.blend_mode;
            cpu_canvas->draw_text_debug(
                text,
                pt.x,
                pt.y - (font_sz * 0.8f * draw_scale),
                tp,
                draw_scale
            );
            advance_user = text.size() * font_sz * 0.6f;
        }

        if (advance_user <= 0.0f) {
            size_t char_count = text.size();
            if (state.current_cmap && state.current_cmap->is_2byte()) {
                char_count = (raw_text.size() + 1) / 2;
            }
            advance_user = char_count * font_sz * 0.6f;
        }
        auto t1_txt = std::chrono::high_resolution_clock::now();
        t_text_accum += std::chrono::duration<double, std::milli>(t1_txt - t0_txt).count();
        count_texts++;
        return advance_user;
    };

    while (i < stream.size()) {
        uint8_t c = stream[i];

        if (is_space(c)) {
            i++;
            continue;
        }

        // Comment
        if (c == '%') {
            while (i < stream.size() && stream[i] != '\r' && stream[i] != '\n') {
                i++;
            }
            continue;
        }

        // Literal String (...)
        if (c == '(') {
            i++;
            std::string str;
            int depth = 1;
            while (i < stream.size() && depth > 0) {
                char ch = static_cast<char>(stream[i++]);
                if (ch == '\\' && i < stream.size()) {
                    str += static_cast<char>(stream[i++]);
                } else if (ch == '(') {
                    depth++;
                    str += ch;
                } else if (ch == ')') {
                    depth--;
                    if (depth > 0) str += ch;
                } else {
                    str += ch;
                }
            }
            if (stack.size() < 256) push_str_item(std::move(str));
            continue;
        }

        // Hex string <...> or dictionary <<
        if (c == '<') {
            if (i + 1 < stream.size() && stream[i + 1] == '<') {
                i += 2;
                push_str_item("<<");
                continue;
            }
            i++;
            std::string hex_str;
            while (i < stream.size() && stream[i] != '>') {
                if (!is_space(stream[i])) hex_str += static_cast<char>(stream[i]);
                i++;
            }
            if (i < stream.size() && stream[i] == '>') i++;
            std::string decoded;
            for (size_t k = 0; k < hex_str.size(); k += 2) {
                char h1 = hex_str[k];
                char h2 = (k + 1 < hex_str.size()) ? hex_str[k + 1] : '0';
                auto hex_val = [](char ch) -> int {
                    if (ch >= '0' && ch <= '9') return ch - '0';
                    if (ch >= 'a' && ch <= 'f') return ch - 'a' + 10;
                    if (ch >= 'A' && ch <= 'F') return ch - 'A' + 10;
                    return 0;
                };
                decoded += static_cast<char>((hex_val(h1) << 4) | hex_val(h2));
            }
            push_str_item(std::move(decoded));
            continue;
        }

        // Dictionary close >> or >
        if (c == '>') {
            if (i + 1 < stream.size() && stream[i + 1] == '>') {
                i += 2;
                push_str_item(">>");
                continue;
            }
            i++;
            continue;
        }

        // Name /Name
        if (c == '/') {
            size_t start = i++;
            while (i < stream.size() && !is_space(stream[i]) && stream[i] != '/' &&
                   stream[i] != '(' && stream[i] != ')' && stream[i] != '[' &&
                   stream[i] != ']' && stream[i] != '<' && stream[i] != '>') {
                i++;
            }
            std::string name(reinterpret_cast<const char*>(stream.data() + start), i - start);
            push_str_item(std::move(name));
            continue;
        }

        // Array [ ... ]
        if (c == '[' || c == ']') {
            push_str_item(std::string(1, static_cast<char>(c)));
            i++;
            continue;
        }

        // Fast-path: Number tokens (starts with digit, '+', '-', or '.')
        // PDF operators never start with digits, signs, or dot.
        char first_ch = static_cast<char>(c);
        bool is_num_start = (first_ch >= '0' && first_ch <= '9') || 
                            first_ch == '-' || first_ch == '+' || first_ch == '.';
        if (is_num_start) {
            size_t start = i++;
            while (i < stream.size() && !is_space(stream[i]) &&
                   stream[i] != '(' && stream[i] != ')' &&
                   stream[i] != '[' && stream[i] != ']' &&
                   stream[i] != '<' && stream[i] != '>' &&
                   stream[i] != '/' && stream[i] != '%') {
                i++;
            }
            float fval = fast_parse_float(stream.data() + start, i - start);
            if (stack.size() < 256) {
                PdfOpItem item;
                item.num = fval;
                item.is_num = true;
                stack.push_back(std::move(item));
            }
            continue;
        }

        // Generic token or operator
        size_t start = i;
        while (i < stream.size() && !is_space(stream[i]) &&
               stream[i] != '(' && stream[i] != ')' &&
               stream[i] != '[' && stream[i] != ']' &&
               stream[i] != '<' && stream[i] != '>' &&
               stream[i] != '/' && stream[i] != '%') {
            i++;
        }
        if (start == i) {
            // Guard against zero advancement on unknown character
            i++;
            continue;
        }
        std::string token(reinterpret_cast<const char*>(stream.data() + start), i - start);

        // Check if token is an operator
        if (token == "q") {
            state_stack.push_back(state);
            canvas.save();
        } else if (token == "Q") {
            if (!state_stack.empty()) {
                state = state_stack.back();
                state_stack.pop_back();
            }
            canvas.restore();
        } else if (token == "cm") {
            float f = pop_float();
            float e = pop_float();
            float d = pop_float();
            float c_val = pop_float();
            float b = pop_float();
            float a = pop_float();
            Transform local(a, b, c_val, d, e, f);
            state.ctm = state.ctm.pre_concat(local);
        } else if (token == "m") {
            is_rect_path = false;
            rect_path_val = std::nullopt;
            float y = pop_float();
            float x = pop_float();
            current_pt = Point(x, y);
            current_builder.move_to(x, y);
        } else if (token == "l") {
            is_rect_path = false;
            rect_path_val = std::nullopt;
            float y = pop_float();
            float x = pop_float();
            current_pt = Point(x, y);
            current_builder.line_to(x, y);
        } else if (token == "c") {
            is_rect_path = false;
            rect_path_val = std::nullopt;
            float y3 = pop_float();
            float x3 = pop_float();
            float y2 = pop_float();
            float x2 = pop_float();
            float y1 = pop_float();
            float x1 = pop_float();
            current_pt = Point(x3, y3);
            current_builder.cubic_to(x1, y1, x2, y2, x3, y3);
        } else if (token == "v") {
            is_rect_path = false;
            rect_path_val = std::nullopt;
            float y3 = pop_float();
            float x3 = pop_float();
            float y2 = pop_float();
            float x2 = pop_float();
            current_builder.cubic_to(current_pt.x, current_pt.y, x2, y2, x3, y3);
            current_pt = Point(x3, y3);
        } else if (token == "y") {
            is_rect_path = false;
            rect_path_val = std::nullopt;
            float y3 = pop_float();
            float x3 = pop_float();
            float y1 = pop_float();
            float x1 = pop_float();
            current_builder.cubic_to(x1, y1, x3, y3, x3, y3);
            current_pt = Point(x3, y3);
        } else if (token == "re") {
            float h = pop_float();
            float w = pop_float();
            float y = pop_float();
            float x = pop_float();
            if (current_builder.is_empty()) {
                is_rect_path = true;
                rect_path_val = Rect::from_xywh(x, y, w, h);
            } else {
                is_rect_path = false;
                rect_path_val = std::nullopt;
            }
            current_builder.move_to(x, y);
            current_builder.line_to(x + w, y);
            current_builder.line_to(x + w, y + h);
            current_builder.line_to(x, y + h);
            current_builder.close();
            current_pt = Point(x, y);
        } else if (token == "h") {
            current_builder.close();
        } else if (token == "f" || token == "F") {
            paint_path(true, false, FillRule::Winding);
        } else if (token == "f*") {
            paint_path(true, false, FillRule::EvenOdd);
        } else if (token == "S") {
            paint_path(false, true, FillRule::Winding);
        } else if (token == "s") {
            current_builder.close();
            paint_path(false, true, FillRule::Winding);
        } else if (token == "B") {
            paint_path(true, true, FillRule::Winding);
        } else if (token == "B*") {
            paint_path(true, true, FillRule::EvenOdd);
        } else if (token == "b") {
            current_builder.close();
            paint_path(true, true, FillRule::Winding);
        } else if (token == "b*") {
            current_builder.close();
            paint_path(true, true, FillRule::EvenOdd);
        } else if (token == "W") {
            state.clip_pending = true;
            state.clip_rule = FillRule::Winding;
        } else if (token == "W*") {
            state.clip_pending = true;
            state.clip_rule = FillRule::EvenOdd;
        } else if (token == "n") {
            if (state.clip_pending) {
                bool clipped_fast = false;
                if (is_rect_path && rect_path_val.has_value() && cpu_canvas &&
                    std::abs(state.ctm.kx) < 1e-4f && std::abs(state.ctm.ky) < 1e-4f) {
                    float x0 = rect_path_val->left() * state.ctm.sx + state.ctm.tx;
                    float x1 = rect_path_val->right() * state.ctm.sx + state.ctm.tx;
                    float y0 = rect_path_val->top() * state.ctm.sy + state.ctm.ty;
                    float y1 = rect_path_val->bottom() * state.ctm.sy + state.ctm.ty;
                    if (x0 > x1) std::swap(x0, x1);
                    if (y0 > y1) std::swap(y0, y1);
                    auto tr_rect = Rect::from_ltrb(x0, y0, x1, y1);
                    if (tr_rect) {
                        cpu_canvas->clip_rect(*tr_rect);
                        clipped_fast = true;
                    }
                }
                if (!clipped_fast) {
                    auto p = current_builder.finish();
                    if (p && !p->is_empty()) {
                        auto trans = p->transform(state.ctm);
                        if (trans && cpu_canvas) {
                            cpu_canvas->clip_path(*trans, state.clip_rule);
                        }
                    }
                }
                state.clip_pending = false;
            }
            current_builder = PathBuilder();
            current_pt = Point(0, 0);
            is_rect_path = false;
            rect_path_val = std::nullopt;
        } else if (token == "w") {
            state.stroke.width = pop_float(1.0f);
        } else if (token == "J") {
            int cap = static_cast<int>(pop_float(0.0f));
            if (cap == 1) state.stroke.line_cap = LineCap::Round;
            else if (cap == 2) state.stroke.line_cap = LineCap::Square;
            else state.stroke.line_cap = LineCap::Butt;
        } else if (token == "j") {
            int join = static_cast<int>(pop_float(0.0f));
            if (join == 1) state.stroke.line_join = LineJoin::Round;
            else if (join == 2) state.stroke.line_join = LineJoin::Bevel;
            else state.stroke.line_join = LineJoin::Miter;
        } else if (token == "M") {
            state.stroke.miter_limit = pop_float(4.0f);
        } else if (token == "rg") {
            float b = pop_float();
            float g = pop_float();
            float r = pop_float();
            state.fill_color = Color::from_rgba_unchecked(r, g, b, state.fill_alpha);
            state.fill_paint = std::nullopt;
        } else if (token == "RG") {
            float b = pop_float();
            float g = pop_float();
            float r = pop_float();
            state.stroke_color = Color::from_rgba_unchecked(r, g, b, state.stroke_alpha);
            state.stroke_paint = std::nullopt;
        } else if (token == "g") {
            float gray = pop_float();
            state.fill_color = Color::from_rgba_unchecked(gray, gray, gray, state.fill_alpha);
            state.fill_paint = std::nullopt;
        } else if (token == "G") {
            float gray = pop_float();
            state.stroke_color = Color::from_rgba_unchecked(gray, gray, gray, state.stroke_alpha);
            state.stroke_paint = std::nullopt;
        } else if (token == "k") {
            float k_val = pop_float();
            float y_val = pop_float();
            float m_val = pop_float();
            float c_val = pop_float();
            float r = (1.0f - c_val) * (1.0f - k_val);
            float g = (1.0f - m_val) * (1.0f - k_val);
            float b = (1.0f - y_val) * (1.0f - k_val);
            state.fill_color = Color::from_rgba_unchecked(r, g, b, state.fill_alpha);
            state.fill_paint = std::nullopt;
        } else if (token == "K") {
            float k_val = pop_float();
            float y_val = pop_float();
            float m_val = pop_float();
            float c_val = pop_float();
            float r = (1.0f - c_val) * (1.0f - k_val);
            float g = (1.0f - m_val) * (1.0f - k_val);
            float b = (1.0f - y_val) * (1.0f - k_val);
            state.stroke_color = Color::from_rgba_unchecked(r, g, b, state.stroke_alpha);
            state.stroke_paint = std::nullopt;
        } else if (token == "BMC") {
            pop_string(); // Discard tag
        } else if (token == "BDC") {
            if (!stack.empty() && !stack.back().is_num && stack.back().str == ">>") {
                while (!stack.empty() && (stack.back().is_num || stack.back().str != "<<")) {
                    stack.pop_back();
                }
                if (!stack.empty() && !stack.back().is_num && stack.back().str == "<<") {
                    stack.pop_back();
                }
            } else {
                pop_string(); // Discard property list
            }
            pop_string(); // Discard tag
        } else if (token == "EMC") {
            // End marked content
        } else if (token == "MP") {
            pop_string();
        } else if (token == "DP") {
            if (!stack.empty() && !stack.back().is_num && stack.back().str == ">>") {
                while (!stack.empty() && (stack.back().is_num || stack.back().str != "<<")) {
                    stack.pop_back();
                }
                if (!stack.empty() && !stack.back().is_num && stack.back().str == "<<") {
                    stack.pop_back();
                }
            } else {
                pop_string();
            }
            pop_string();
        } else if (token == "gs") {
            std::string gs_name = pop_string();
            if (gs_name.front() == '/') gs_name = gs_name.substr(1);
            PdfDict ext_gstate_dict;
            auto ext_it = resources.find("ExtGState");
            if (ext_it != resources.end()) {
                if (ext_it->second.is_dict()) {
                    ext_gstate_dict = ext_it->second.as_dict();
                } else if (ext_it->second.is_ref()) {
                    auto r = parser_.resolve(ext_it->second.as_ref());
                    if (r && r->value.is_dict()) ext_gstate_dict = r->value.as_dict();
                }
            }
            auto it_gs = ext_gstate_dict.find(gs_name);
            PdfDict gs_dict;
            if (it_gs != ext_gstate_dict.end()) {
                if (it_gs->second.is_dict()) {
                    gs_dict = it_gs->second.as_dict();
                } else if (it_gs->second.is_ref()) {
                    auto r = parser_.resolve(it_gs->second.as_ref());
                    if (r && r->value.is_dict()) gs_dict = r->value.as_dict();
                }
            }
            if (!gs_dict.empty()) {
                auto it_ca = gs_dict.find("ca");
                if (it_ca != gs_dict.end()) {
                    float a = it_ca->second.as_float(1.0f);
                    state.fill_alpha = std::clamp(a, 0.0f, 1.0f);
                    state.fill_color.set_alpha(state.fill_alpha);
                }
                auto it_CA = gs_dict.find("CA");
                if (it_CA != gs_dict.end()) {
                    float a = it_CA->second.as_float(1.0f);
                    state.stroke_alpha = std::clamp(a, 0.0f, 1.0f);
                    state.stroke_color.set_alpha(state.stroke_alpha);
                }
                auto it_bm = gs_dict.find("BM");
                if (it_bm != gs_dict.end()) {
                    state.blend_mode = pdf_parse_blend_mode(it_bm->second);
                }
                auto it_ais = gs_dict.find("AIS");
                if (it_ais != gs_dict.end()) {
                    state.alpha_is_shape = it_ais->second.as_bool(false);
                }
                auto it_lw = gs_dict.find("LW");
                if (it_lw != gs_dict.end()) {
                    state.stroke.width = it_lw->second.as_float(1.0f);
                }
                auto it_lc = gs_dict.find("LC");
                if (it_lc != gs_dict.end()) {
                    int c_cap = static_cast<int>(it_lc->second.as_int(0));
                    if (c_cap == 1) state.stroke.line_cap = LineCap::Round;
                    else if (c_cap == 2) state.stroke.line_cap = LineCap::Square;
                    else state.stroke.line_cap = LineCap::Butt;
                }
                auto it_lj = gs_dict.find("LJ");
                if (it_lj != gs_dict.end()) {
                    int c_join = static_cast<int>(it_lj->second.as_int(0));
                    if (c_join == 1) state.stroke.line_join = LineJoin::Round;
                    else if (c_join == 2) state.stroke.line_join = LineJoin::Bevel;
                    else state.stroke.line_join = LineJoin::Miter;
                }
                auto it_ml = gs_dict.find("ML");
                if (it_ml != gs_dict.end()) {
                    state.stroke.miter_limit = it_ml->second.as_float(10.0f);
                }
            }
        } else if (token == "BT") {
            state.text_matrix = Transform::identity();
            state.line_matrix = Transform::identity();
        } else if (token == "ET") {
            // End text
        } else if (token == "Tf") {
            state.font_size = pop_float(12.0f);
            state.font_name = pop_string();
            if (!state.font_name.empty() && state.font_name.front() == '/') {
                state.font_name = state.font_name.substr(1);
            }
            state.current_cmap = find_or_load_cmap(resources, state.font_name, font_system);
        } else if (token == "Tm") {
            float f = pop_float();
            float e = pop_float();
            float d = pop_float();
            float c_val = pop_float();
            float b = pop_float();
            float a = pop_float();
            state.text_matrix = Transform(a, b, c_val, d, e, f);
            state.line_matrix = state.text_matrix;
        } else if (token == "Td") {
            float ty = pop_float();
            float tx = pop_float();
            state.line_matrix = state.line_matrix.pre_translate(tx, ty);
            state.text_matrix = state.line_matrix;
        } else if (token == "TD") {
            float ty = pop_float();
            float tx = pop_float();
            state.text_leading = -ty;
            state.line_matrix = state.line_matrix.pre_translate(tx, ty);
            state.text_matrix = state.line_matrix;
        } else if (token == "T*") {
            state.line_matrix = state.line_matrix.pre_translate(0.0f, -state.text_leading);
            state.text_matrix = state.line_matrix;
        } else if (token == "Tj") {
            std::string text = pop_string();
            Transform eff_t = state.ctm.pre_concat(state.text_matrix);
            float adv = render_text_span(text, eff_t, state.font_size, state.fill_color);
            state.text_matrix = state.text_matrix.pre_translate(adv, 0.0f);
        } else if (token == "TJ") {
            struct TjItem {
                float kern = 0.0f;
                std::string text;
                bool is_kern = false;
            };
            std::vector<TjItem> tj_elements;
            if (!stack.empty() && !stack.back().is_num && stack.back().str == "]") {
                stack.pop_back(); // Pop ']'
                while (!stack.empty() && (stack.back().is_num || stack.back().str != "[")) {
                    if (stack.back().is_num) {
                        tj_elements.push_back(TjItem{stack.back().num, "", true});
                    } else {
                        tj_elements.push_back(TjItem{0.0f, std::move(stack.back().str), false});
                    }
                    stack.pop_back();
                }
                if (!stack.empty() && !stack.back().is_num && stack.back().str == "[") {
                    stack.pop_back(); // Pop '['
                }
                std::reverse(tj_elements.begin(), tj_elements.end());
            }

            for (const auto& elem : tj_elements) {
                if (elem.is_kern) {
                    float dx = -elem.kern / 1000.0f * state.font_size;
                    state.text_matrix = state.text_matrix.pre_translate(dx, 0.0f);
                } else {
                    Transform eff_t = state.ctm.pre_concat(state.text_matrix);
                    float adv = render_text_span(elem.text, eff_t, state.font_size, state.fill_color);
                    state.text_matrix = state.text_matrix.pre_translate(adv, 0.0f);
                }
            }
        } else if (token == "'") {
            state.line_matrix = state.line_matrix.pre_translate(0.0f, -state.text_leading);
            state.text_matrix = state.line_matrix;
            std::string text = pop_string();
            Transform eff_t = state.ctm.pre_concat(state.text_matrix);
            float adv = render_text_span(text, eff_t, state.font_size, state.fill_color);
            state.text_matrix = state.text_matrix.pre_translate(adv, 0.0f);
        } else if (token == "\"") {
            std::string text = pop_string();
            pop_float(); // c_space
            pop_float(); // w_space
            state.line_matrix = state.line_matrix.pre_translate(0.0f, -state.text_leading);
            state.text_matrix = state.line_matrix;
            Transform eff_t = state.ctm.pre_concat(state.text_matrix);
            float adv = render_text_span(text, eff_t, state.font_size, state.fill_color);
            state.text_matrix = state.text_matrix.pre_translate(adv, 0.0f);
        } else if (token == "d") {
            pop_float(); // phase
            if (!stack.empty() && !stack.back().is_num && stack.back().str == "]") {
                stack.pop_back();
                while (!stack.empty() && (stack.back().is_num || stack.back().str != "[")) {
                    stack.pop_back();
                }
                if (!stack.empty() && !stack.back().is_num && stack.back().str == "[") {
                    stack.pop_back();
                }
            }
        } else if (token == "Tc" || token == "Tw" || token == "Tz" || token == "TL" || token == "Tr" || token == "Ts" || token == "i") {
            pop_float();
        } else if (token == "cs") {
            state.fill_color_space = pop_string();
            if (state.fill_color_space.front() == '/') state.fill_color_space = state.fill_color_space.substr(1);
            state.fill_cs_val = resolve_cs_from_resources(state.fill_color_space, resources);
        } else if (token == "CS") {
            state.stroke_color_space = pop_string();
            if (state.stroke_color_space.front() == '/') state.stroke_color_space = state.stroke_color_space.substr(1);
            state.stroke_cs_val = resolve_cs_from_resources(state.stroke_color_space, resources);
        } else if (token == "ri") {
            pop_string();
        } else if (token == "sh") {
            std::string sh_name = pop_string();
            if (sh_name.front() == '/') sh_name = sh_name.substr(1);
            execute_shading(sh_name, resources, canvas, page_height, scale, cpu_canvas, state);
        } else if (token == "sc" || token == "scn") {
            if (state.fill_color_space == "Pattern" && !stack.empty() && !stack.back().is_num && !stack.back().str.empty() && stack.back().str.front() == '/') {
                std::string pat_name = pop_string();
                if (pat_name.front() == '/') pat_name = pat_name.substr(1);
                state.fill_paint = resolve_pattern_paint(pat_name, resources, page_height, scale, cpu_canvas, state);
            } else {
                std::vector<float> comps;
                while (!stack.empty()) {
                    if (stack.back().is_num) {
                        comps.push_back(stack.back().num);
                        stack.pop_back();
                    } else if (!stack.back().str.empty() && stack.back().str.front() == '/') {
                        stack.pop_back();
                        break;
                    } else {
                        char* endptr = nullptr;
                        float v = std::strtof(stack.back().str.c_str(), &endptr);
                        if (endptr != stack.back().str.c_str() && *endptr == '\0') {
                            comps.push_back(v);
                            stack.pop_back();
                        } else {
                            break;
                        }
                    }
                }
                std::reverse(comps.begin(), comps.end());
                if (state.fill_cs_val.is_array()) {
                    state.fill_color = resolve_color_from_space(state.fill_cs_val, comps, state.fill_alpha, parser_);
                } else if (comps.size() == 1) {
                    state.fill_color = Color::from_rgba_unchecked(comps[0], comps[0], comps[0], state.fill_alpha);
                } else if (comps.size() == 3) {
                    state.fill_color = Color::from_rgba_unchecked(comps[0], comps[1], comps[2], state.fill_alpha);
                } else if (comps.size() >= 4) {
                    float r = (1.0f - comps[0]) * (1.0f - comps[3]);
                    float g = (1.0f - comps[1]) * (1.0f - comps[3]);
                    float b = (1.0f - comps[2]) * (1.0f - comps[3]);
                    state.fill_color = Color::from_rgba_unchecked(r, g, b, state.fill_alpha);
                }
                state.fill_paint = std::nullopt;
            }
        } else if (token == "SC" || token == "SCN") {
            if (state.stroke_color_space == "Pattern" && !stack.empty() && !stack.back().is_num && !stack.back().str.empty() && stack.back().str.front() == '/') {
                std::string pat_name = pop_string();
                if (pat_name.front() == '/') pat_name = pat_name.substr(1);
                state.stroke_paint = resolve_pattern_paint(pat_name, resources, page_height, scale, cpu_canvas, state);
            } else {
                std::vector<float> comps;
                while (!stack.empty()) {
                    if (stack.back().is_num) {
                        comps.push_back(stack.back().num);
                        stack.pop_back();
                    } else if (!stack.back().str.empty() && stack.back().str.front() == '/') {
                        stack.pop_back();
                        break;
                    } else {
                        char* endptr = nullptr;
                        float v = std::strtof(stack.back().str.c_str(), &endptr);
                        if (endptr != stack.back().str.c_str() && *endptr == '\0') {
                            comps.push_back(v);
                            stack.pop_back();
                        } else {
                            break;
                        }
                    }
                }
                std::reverse(comps.begin(), comps.end());
                if (state.stroke_cs_val.is_array()) {
                    state.stroke_color = resolve_color_from_space(state.stroke_cs_val, comps, state.stroke_alpha, parser_);
                } else if (comps.size() == 1) {
                    state.stroke_color = Color::from_rgba_unchecked(comps[0], comps[0], comps[0], state.stroke_alpha);
                } else if (comps.size() == 3) {
                    state.stroke_color = Color::from_rgba_unchecked(comps[0], comps[1], comps[2], state.stroke_alpha);
                } else if (comps.size() >= 4) {
                    float r = (1.0f - comps[0]) * (1.0f - comps[3]);
                    float g = (1.0f - comps[1]) * (1.0f - comps[3]);
                    float b = (1.0f - comps[2]) * (1.0f - comps[3]);
                    state.stroke_color = Color::from_rgba_unchecked(r, g, b, state.stroke_alpha);
                }
                state.stroke_paint = std::nullopt;
            }
        } else if (token == "Do") {
            std::string xobj_name = pop_string();
            if (xobj_name.front() == '/') xobj_name = xobj_name.substr(1);

            PdfDict xobj_dict;
            auto xobj_it = resources.find("XObject");
            if (xobj_it != resources.end()) {
                if (xobj_it->second.is_dict()) {
                    xobj_dict = xobj_it->second.as_dict();
                } else if (xobj_it->second.is_ref()) {
                    auto r = parser_.resolve(xobj_it->second.as_ref());
                    if (r && r->value.is_dict()) xobj_dict = r->value.as_dict();
                }
            }

            auto it_ref = xobj_dict.find(xobj_name);
            if (it_ref != xobj_dict.end() && it_ref->second.is_ref()) {
                auto resolved = parser_.resolve(it_ref->second.as_ref());
                if (resolved && resolved->stream_data.has_value()) {
                    std::string subtype;
                    if (const auto* st_val = resolved->value.find("Subtype")) {
                        if (st_val->is_name()) subtype = st_val->as_name();
                    }

                    if (subtype == "Form") {
                        auto t0_form = std::chrono::high_resolution_clock::now();
                        PdfDict form_res = resources;
                        if (const auto* r_val = resolved->value.find("Resources")) {
                            if (r_val->is_dict()) form_res = r_val->as_dict();
                            else if (r_val->is_ref()) {
                                auto r_obj = parser_.resolve(r_val->as_ref());
                                if (r_obj && r_obj->value.is_dict()) form_res = r_obj->value.as_dict();
                            }
                        }

                        Transform form_ctm = state.ctm;
                        if (const auto* m_val = resolved->value.find("Matrix")) {
                            if (m_val->is_array() && m_val->as_array().size() >= 6) {
                                const auto& arr = m_val->as_array();
                                Transform form_matrix(
                                    arr[0].as_float(1.0f), arr[1].as_float(0.0f),
                                    arr[2].as_float(0.0f), arr[3].as_float(1.0f),
                                    arr[4].as_float(0.0f), arr[5].as_float(0.0f)
                                );
                                form_ctm = form_ctm.pre_concat(form_matrix);
                            }
                        }
                        // Like MuPDF (source/pdf/pdf-op-run.c:2492): Only isolate when transparency is non-opaque or blend mode is not normal.
                        // Standard Form XObjects with opacity=1.0 and SourceOver are rendered directly into the canvas.
                        bool needs_isolation = (state.fill_alpha < 0.999f || state.blend_mode != BlendMode::SourceOver);

                        if (needs_isolation && cpu_canvas && depth < 4) {
                            auto group_px = Pixmap::allocate(cpu_canvas->width(), cpu_canvas->height());
                            if (group_px) {
                                Canvas group_canvas(*group_px);
                                group_canvas.clear(Color::TRANSPARENT);

                                execute_stream(
                                    *resolved->stream_data,
                                    form_res,
                                    group_canvas,
                                    page_height,
                                    scale,
                                    &group_canvas,
                                    font_system,
                                    depth + 1,
                                    form_ctm
                                );

                                PixmapPaint pp;
                                pp.opacity = state.fill_alpha;
                                pp.blend_mode = state.blend_mode;
                                cpu_canvas->draw_pixmap(0, 0, group_px->as_ref(), pp);
                            }
                        } else {
                            canvas.save();
                            execute_stream(
                                *resolved->stream_data,
                                form_res,
                                canvas,
                                page_height,
                                scale,
                                cpu_canvas,
                                font_system,
                                depth + 1,
                                form_ctm
                            );
                            canvas.restore();
                        }
                        auto t1_form = std::chrono::high_resolution_clock::now();
                        t_form_accum += std::chrono::duration<double, std::milli>(t1_form - t0_form).count();
                    } else if (cpu_canvas) {
                        auto t0_img = std::chrono::high_resolution_clock::now();
                        std::shared_ptr<Pixmap> decoded_px = nullptr;
                        PdfRef img_ref = it_ref->second.as_ref();
                        uint64_t stream_hash = 0;
                        if (resolved->stream_data.has_value()) {
                            const auto& sdata = *resolved->stream_data;
                            uint64_t h = 14695981039346656037ULL ^ sdata.size();
                            const size_t n_words = sdata.size() / 8;
                            const uint64_t* words = reinterpret_cast<const uint64_t*>(sdata.data());
                            for (size_t i = 0; i < n_words; ++i) {
                                h ^= words[i];
                                h *= 1099511628211ULL;
                            }
                            const uint8_t* tail = sdata.data() + n_words * 8;
                            for (size_t i = 0; i < (sdata.size() & 7); ++i) {
                                h ^= tail[i];
                                h *= 1099511628211ULL;
                            }
                            stream_hash = h;
                        }

                        auto it_cached_img = image_cache_.find(img_ref);
                        if (it_cached_img != image_cache_.end()) {
                            decoded_px = it_cached_img->second;
                        } else if (stream_hash != 0 && image_hash_cache_.find(stream_hash) != image_hash_cache_.end()) {
                            decoded_px = image_hash_cache_[stream_hash];
                            image_cache_[img_ref] = decoded_px;
                        } else {
                            auto decoded_opt = decode_pdf_image(*resolved, parser_, resources, state.fill_color);
                            if (decoded_opt) {
                                decoded_px = std::make_shared<Pixmap>(std::move(*decoded_opt));

                                if (const auto* smask_val = resolved->value.find("SMask")) {
                                    PdfRef smask_ref{};
                                    if (smask_val->is_ref()) {
                                        smask_ref = smask_val->as_ref();
                                    }
                                    if (smask_ref.id != 0) {
                                        auto smask_obj = parser_.resolve(smask_ref);
                                        if (smask_obj && smask_obj->stream_data.has_value()) {
                                            const auto& smask_stream = *smask_obj->stream_data;
                                            uint32_t sw = static_cast<uint32_t>(smask_obj->value.find("Width")->as_int(decoded_px->width()));
                                            uint32_t sh = static_cast<uint32_t>(smask_obj->value.find("Height")->as_int(decoded_px->height()));

                                            std::string s_sub = "Alpha";
                                            if (const auto* sv = smask_obj->value.find("S")) {
                                                if (sv->is_name()) s_sub = sv->as_name();
                                            } else if (const auto* sub = smask_obj->value.find("Subtype")) {
                                                if (sub->is_name()) s_sub = sub->as_name();
                                            }
                                            if (!s_sub.empty() && s_sub.front() == '/') s_sub = s_sub.substr(1);
                                            bool is_luminosity = (s_sub == "Luminosity");

                                            int smask_comps = (smask_stream.size() >= sw * sh * 3) ? 3 : 1;

                                            uint32_t lim_w = std::min(decoded_px->width(), sw);
                                            uint32_t lim_h = std::min(decoded_px->height(), sh);
                                            auto* pix = decoded_px->pixels_mut();

                                            for (uint32_t y = 0; y < lim_h; ++y) {
                                                for (uint32_t x = 0; x < lim_w; ++x) {
                                                    uint8_t a = 255;
                                                    if (smask_comps == 3) {
                                                        size_t sidx = (y * sw + x) * 3;
                                                        if (sidx + 2 < smask_stream.size()) {
                                                            if (is_luminosity) {
                                                                float r = smask_stream[sidx] / 255.0f;
                                                                float g = smask_stream[sidx + 1] / 255.0f;
                                                                float b = smask_stream[sidx + 2] / 255.0f;
                                                                float lum = 0.2126f * r + 0.7152f * g + 0.0722f * b;
                                                                a = static_cast<uint8_t>(std::clamp(lum * 255.0f, 0.0f, 255.0f));
                                                            } else {
                                                                a = smask_stream[sidx];
                                                            }
                                                        }
                                                    } else {
                                                        size_t sidx = y * sw + x;
                                                        if (sidx < smask_stream.size()) {
                                                            a = smask_stream[sidx];
                                                        }
                                                    }

                                                    size_t p_idx = y * decoded_px->width() + x;
                                                    PremultipliedColorU8 old_c = pix[p_idx];
                                                    float af = a / 255.0f;
                                                    uint8_t nr = static_cast<uint8_t>(old_c.r * af);
                                                    uint8_t ng = static_cast<uint8_t>(old_c.g * af);
                                                    uint8_t nb = static_cast<uint8_t>(old_c.b * af);
                                                    pix[p_idx] = PremultipliedColorU8(nr, ng, nb, a);
                                                }
                                            }
                                        }
                                    }
                                }

                                image_cache_[img_ref] = decoded_px;
                                if (stream_hash != 0) {
                                    image_hash_cache_[stream_hash] = decoded_px;
                                }
                            }
                        }

                        if (decoded_px) {
                            Point q0(0.0f, 1.0f); // Top-left in PDF
                            Point q1(1.0f, 1.0f); // Top-right in PDF
                            Point q2(1.0f, 0.0f); // Bottom-right in PDF
                            Point q3(0.0f, 0.0f); // Bottom-left in PDF

                            state.ctm.map_point(q0);
                            state.ctm.map_point(q1);
                            state.ctm.map_point(q2);
                            state.ctm.map_point(q3);

                            float min_x = std::min({q0.x, q1.x, q2.x, q3.x});
                            float max_x = std::max({q0.x, q1.x, q2.x, q3.x});
                            float min_y = std::min({q0.y, q1.y, q2.y, q3.y});
                            float max_y = std::max({q0.y, q1.y, q2.y, q3.y});

                            if (max_x > 0.0f && min_x < static_cast<float>(cpu_canvas->width()) &&
                                max_y > 0.0f && min_y < static_cast<float>(cpu_canvas->height())) {
                                bool is_axis_aligned = (std::abs(state.ctm.kx) < 1e-4f && std::abs(state.ctm.ky) < 1e-4f);
                                if (is_axis_aligned && state.blend_mode == BlendMode::SourceOver) {
                                    blit_scaled_image_fast(
                                        *cpu_canvas,
                                        decoded_px->as_ref(),
                                        min_x, min_y, max_x - min_x, max_y - min_y,
                                        state.fill_alpha,
                                        state.blend_mode
                                    );
                                } else {
                                    cpu_canvas->draw_pixmap_perspective(
                                        decoded_px->as_ref(),
                                        q0, q1, q2, q3,
                                        state.fill_alpha,
                                        state.blend_mode
                                    );
                                }
                            }
                        }
                        auto t1_img = std::chrono::high_resolution_clock::now();
                        double t_img_total = std::chrono::duration<double, std::milli>(t1_img - t0_img).count();
                        t_image_accum += t_img_total;
                        count_images++;
                    }
                }
            }
        } else {
            // Push operand to stack with limit
            push_str_item(std::move(token));
        }
    }
}

PdfValue PdfInterpreter::resolve_cs_from_resources(
    const std::string& cs_name,
    const PdfDict& resources
) {
    if (cs_name == "DeviceRGB" || cs_name == "DeviceGray" || cs_name == "DeviceCMYK" || cs_name == "Pattern") {
        return PdfValue(PdfName(cs_name));
    }

    auto cs_it = resources.find("ColorSpace");
    if (cs_it != resources.end()) {
        PdfDict cs_dict;
        if (cs_it->second.is_dict()) cs_dict = cs_it->second.as_dict();
        else if (cs_it->second.is_ref()) {
            auto r = parser_.resolve(cs_it->second.as_ref());
            if (r && r->value.is_dict()) cs_dict = r->value.as_dict();
        }

        auto it = cs_dict.find(cs_name);
        if (it != cs_dict.end()) {
            if (it->second.is_ref()) {
                auto r = parser_.resolve(it->second.as_ref());
                if (r) return r->value;
            }
            return it->second;
        }
    }

    return PdfValue(PdfName(cs_name));
}

std::vector<GradientStop> PdfInterpreter::extract_gradient_stops(
    const PdfValue& func_val,
    const std::string& color_space
) {
    std::vector<GradientStop> stops;

    const PdfValue* val_ptr = &func_val;
    std::optional<PdfIndirectObject> resolved_holder;
    if (val_ptr->is_ref()) {
        resolved_holder = parser_.resolve(val_ptr->as_ref());
        if (resolved_holder) val_ptr = &resolved_holder->value;
    }

    int func_type = 2;
    float n = 1.0f;
    if (val_ptr->is_dict()) {
        if (const auto* ft = val_ptr->find("FunctionType")) func_type = static_cast<int>(ft->as_int(2));
        if (const auto* nv = val_ptr->find("N")) n = nv->as_float(1.0f);
    }

    if (func_type == 2 && n == 1.0f) {
        auto comps0 = eval_pdf_func_impl(parser_, func_val, 0.0f);
        auto comps1 = eval_pdf_func_impl(parser_, func_val, 1.0f);
        stops.push_back(GradientStop::create(0.0f, eval_color_components(comps0, color_space)));
        stops.push_back(GradientStop::create(1.0f, eval_color_components(comps1, color_space)));
    } else {
        const int SAMPLES = 32;
        for (int i = 0; i <= SAMPLES; ++i) {
            float t = static_cast<float>(i) / static_cast<float>(SAMPLES);
            auto comps = eval_pdf_func_impl(parser_, func_val, t);
            stops.push_back(GradientStop::create(t, eval_color_components(comps, color_space)));
        }
    }

    return stops;
}

void PdfInterpreter::execute_shading(
    const std::string& sh_name,
    const PdfDict& resources,
    ICanvas& canvas,
    float page_height,
    float scale,
    Canvas* cpu_canvas,
    const PdfGraphicsState& state
) {
    (void)page_height;
    (void)scale;
    auto res_it = resources.find("Shading");
    if (res_it == resources.end()) return;

    PdfDict sh_dict;
    if (res_it->second.is_dict()) {
        sh_dict = res_it->second.as_dict();
    } else if (res_it->second.is_ref()) {
        auto r = parser_.resolve(res_it->second.as_ref());
        if (r && r->value.is_dict()) sh_dict = r->value.as_dict();
    }

    auto sh_entry = sh_dict.find(sh_name);
    if (sh_entry == sh_dict.end()) return;

    PdfValue sh_val = sh_entry->second;
    std::optional<PdfIndirectObject> sh_obj;
    if (sh_val.is_ref()) {
        sh_obj = parser_.resolve(sh_val.as_ref());
        if (sh_obj) sh_val = sh_obj->value;
    }

    if (!sh_val.is_dict()) return;

    int sh_type = 2;
    if (const auto* stv = sh_val.find("ShadingType")) sh_type = static_cast<int>(stv->as_int(2));

    std::string cs = "DeviceRGB";
    if (const auto* csv = sh_val.find("ColorSpace")) {
        if (csv->is_name()) cs = csv->as_name();
    }

    const auto* func_val = sh_val.find("Function");
    std::vector<GradientStop> stops;
    if (func_val) {
        stops = extract_gradient_stops(*func_val, cs);
    } else {
        stops.push_back(GradientStop::create(0.0f, Color::BLACK));
        stops.push_back(GradientStop::create(1.0f, Color::WHITE));
    }

    if (sh_type == 2) { // Type 2: Axial Shading
        const auto* coords_val = sh_val.find("Coords");
        if (coords_val && coords_val->is_array() && coords_val->as_array().size() >= 4) {
            const auto& arr = coords_val->as_array();
            Point p0(arr[0].as_float(0.0f), arr[1].as_float(0.0f));
            Point p1(arr[2].as_float(1.0f), arr[3].as_float(0.0f));

            Point sp0 = p0; state.ctm.map_point(sp0);
            Point sp1 = p1; state.ctm.map_point(sp1);

            auto grad_opt = LinearGradient::create(sp0, sp1, stops, SpreadMode::Pad);
            if (grad_opt) {
                Paint paint{Shader(*grad_opt)};

                if (cpu_canvas) {
                    cpu_canvas->fill_rect(
                        Rect::from_xywh(0.0f, 0.0f, static_cast<float>(cpu_canvas->width()), static_cast<float>(cpu_canvas->height())).value_or(Rect()),
                        paint
                    );
                } else {
                    canvas.fill_rect(
                        Rect::from_xywh(0.0f, 0.0f, 10000.0f, 10000.0f).value_or(Rect()),
                        paint
                    );
                }
            }
        }
    } else if (sh_type == 3) { // Type 3: Radial Shading
        const auto* coords_val = sh_val.find("Coords");
        if (coords_val && coords_val->is_array() && coords_val->as_array().size() >= 6) {
            const auto& arr = coords_val->as_array();
            Point p0(arr[0].as_float(0.0f), arr[1].as_float(0.0f));
            float r0 = arr[2].as_float(0.0f);
            Point p1(arr[3].as_float(0.0f), arr[4].as_float(0.0f));
            float r1 = arr[5].as_float(1.0f);

            Point sp0 = p0; state.ctm.map_point(sp0);
            Point sp1 = p1; state.ctm.map_point(sp1);

            float sx = std::hypot(state.ctm.sx, state.ctm.ky);
            float sy = std::hypot(state.ctm.kx, state.ctm.sy);
            float s = (sx + sy) * 0.5f;

            auto grad_opt = RadialGradient::create_2point(sp0, r0 * s, sp1, r1 * s, stops, SpreadMode::Pad);
            if (grad_opt) {
                Paint paint{Shader(*grad_opt)};

                if (cpu_canvas) {
                    cpu_canvas->fill_rect(
                        Rect::from_xywh(0.0f, 0.0f, static_cast<float>(cpu_canvas->width()), static_cast<float>(cpu_canvas->height())).value_or(Rect()),
                        paint
                    );
                } else {
                    canvas.fill_rect(
                        Rect::from_xywh(0.0f, 0.0f, 10000.0f, 10000.0f).value_or(Rect()),
                        paint
                    );
                }
            }
        }
    } else if (sh_type == 6) { // Type 6: Coons Patch Mesh Shading
        if (cpu_canvas && sh_obj && sh_obj->stream_data.has_value() && !sh_obj->stream_data->empty()) {
            // ISO 32000 §8.6.6.7 Coons Patch Mesh
        }
    }
}

std::optional<Paint> PdfInterpreter::resolve_pattern_paint(
    const std::string& pat_name,
    const PdfDict& resources,
    float page_height,
    float scale,
    Canvas* cpu_canvas,
    const PdfGraphicsState& state
) {
    (void)page_height;
    (void)cpu_canvas;
    auto pat_res_it = resources.find("Pattern");
    if (pat_res_it == resources.end()) return std::nullopt;

    PdfDict pat_dict;
    if (pat_res_it->second.is_dict()) {
        pat_dict = pat_res_it->second.as_dict();
    } else if (pat_res_it->second.is_ref()) {
        auto r = parser_.resolve(pat_res_it->second.as_ref());
        if (r && r->value.is_dict()) pat_dict = r->value.as_dict();
    }

    auto it = pat_dict.find(pat_name);
    if (it == pat_dict.end()) return std::nullopt;

    PdfValue pat_val = it->second;
    std::optional<PdfIndirectObject> pat_obj;
    if (pat_val.is_ref()) {
        pat_obj = parser_.resolve(pat_val.as_ref());
        if (pat_obj) pat_val = pat_obj->value;
    }

    if (!pat_val.is_dict()) return std::nullopt;

    int pat_type = 1;
    if (const auto* ptv = pat_val.find("PatternType")) pat_type = static_cast<int>(ptv->as_int(1));

    Transform pat_matrix;
    if (const auto* mv = pat_val.find("Matrix")) {
        if (mv->is_array() && mv->as_array().size() >= 6) {
            const auto& arr = mv->as_array();
            pat_matrix = Transform(
                arr[0].as_float(1.0f), arr[1].as_float(0.0f),
                arr[2].as_float(0.0f), arr[3].as_float(1.0f),
                arr[4].as_float(0.0f), arr[5].as_float(0.0f)
            );
        }
    }

    Transform total_pat_ts = state.ctm.pre_concat(pat_matrix);

    if (pat_type == 2) { // Shading Pattern
        const auto* sh_entry = pat_val.find("Shading");
        if (!sh_entry) return std::nullopt;

        PdfValue sh_val = *sh_entry;
        std::optional<PdfIndirectObject> sh_obj;
        if (sh_val.is_ref()) {
            sh_obj = parser_.resolve(sh_val.as_ref());
            if (sh_obj) sh_val = sh_obj->value;
        }
        if (!sh_val.is_dict()) return std::nullopt;

        int sh_type = 2;
        if (const auto* stv = sh_val.find("ShadingType")) sh_type = static_cast<int>(stv->as_int(2));

        std::string cs = "DeviceRGB";
        if (const auto* csv = sh_val.find("ColorSpace")) {
            if (csv->is_name()) cs = csv->as_name();
        }

        const auto* func_val = sh_val.find("Function");
        std::vector<GradientStop> stops;
        if (func_val) stops = extract_gradient_stops(*func_val, cs);
        else {
            stops.push_back(GradientStop::create(0.0f, Color::BLACK));
            stops.push_back(GradientStop::create(1.0f, Color::WHITE));
        }

        if (sh_type == 2) { // Axial
            const auto* coords_val = sh_val.find("Coords");
            if (coords_val && coords_val->is_array() && coords_val->as_array().size() >= 4) {
                const auto& arr = coords_val->as_array();
                Point p0(arr[0].as_float(0.0f), arr[1].as_float(0.0f));
                Point p1(arr[2].as_float(1.0f), arr[3].as_float(0.0f));

                total_pat_ts.map_point(p0);
                total_pat_ts.map_point(p1);

                auto grad_opt = LinearGradient::create(p0, p1, stops, SpreadMode::Pad);
                if (grad_opt) {
                    return Paint{Shader(*grad_opt)};
                }
            }
        } else if (sh_type == 3) { // Radial
            const auto* coords_val = sh_val.find("Coords");
            if (coords_val && coords_val->is_array() && coords_val->as_array().size() >= 6) {
                const auto& arr = coords_val->as_array();
                Point p0(arr[0].as_float(0.0f), arr[1].as_float(0.0f));
                float r0 = arr[2].as_float(0.0f);
                Point p1(arr[3].as_float(0.0f), arr[4].as_float(0.0f));
                float r1 = arr[5].as_float(1.0f);

                total_pat_ts.map_point(p0);
                total_pat_ts.map_point(p1);

                float sx = std::hypot(total_pat_ts.sx, total_pat_ts.ky);
                float sy = std::hypot(total_pat_ts.kx, total_pat_ts.sy);
                float s = (sx + sy) * 0.5f;

                auto grad_opt = RadialGradient::create_2point(p0, r0 * s, p1, r1 * s, stops, SpreadMode::Pad);
                if (grad_opt) {
                    return Paint{Shader(*grad_opt)};
                }
            }
        }
    } else if (pat_type == 1) { // Tiling Pattern
        if (pat_obj && pat_obj->stream_data.has_value() && !pat_obj->stream_data->empty()) {
            float bx0 = 0.0f, by0 = 0.0f, bx1 = 100.0f, by1 = 100.0f;
            if (const auto* bbox_val = pat_val.find("BBox")) {
                if (bbox_val->is_array() && bbox_val->as_array().size() >= 4) {
                    bx0 = bbox_val->as_array()[0].as_float(0.0f);
                    by0 = bbox_val->as_array()[1].as_float(0.0f);
                    bx1 = bbox_val->as_array()[2].as_float(100.0f);
                    by1 = bbox_val->as_array()[3].as_float(100.0f);
                }
            }
            float bw = std::abs(bx1 - bx0);
            float bh = std::abs(by1 - by0);
            if (bw < 1.0f) bw = 100.0f;
            if (bh < 1.0f) bh = 100.0f;

            float xstep = bw;
            float ystep = bh;
            if (const auto* xv = pat_val.find("XStep")) xstep = std::abs(xv->as_float(bw));
            if (const auto* yv = pat_val.find("YStep")) ystep = std::abs(yv->as_float(bh));
            if (xstep < 1.0f) xstep = bw;
            if (ystep < 1.0f) ystep = bh;

            uint32_t pw = std::clamp(static_cast<uint32_t>(xstep * scale), 16u, 512u);
            uint32_t ph = std::clamp(static_cast<uint32_t>(ystep * scale), 16u, 512u);

            std::string cache_key = pat_name + "_" + std::to_string(pw) + "x" + std::to_string(ph);
            auto cached_it = pattern_cache_.find(cache_key);
            if (cached_it == pattern_cache_.end()) {
                auto tile_px_opt = Pixmap::allocate(pw, ph);
                if (tile_px_opt) {
                    Pixmap tile_px = std::move(*tile_px_opt);
                    tile_px.fill(Color::TRANSPARENT);

                    Canvas tile_canvas(tile_px);
                    float sx = static_cast<float>(pw) / xstep;
                    float sy = static_cast<float>(ph) / ystep;
                    tile_canvas.scale(sx, -sy);
                    tile_canvas.translate(-bx0, -by1);

                    PdfDict pat_res = resources;
                    if (const auto* prv = pat_val.find("Resources")) {
                        if (prv->is_dict()) pat_res = prv->as_dict();
                        else if (prv->is_ref()) {
                            auto ro = parser_.resolve(prv->as_ref());
                            if (ro && ro->value.is_dict()) pat_res = ro->value.as_dict();
                        }
                    }

                    execute_stream(
                        *pat_obj->stream_data,
                        pat_res,
                        tile_canvas,
                        ystep,
                        1.0f,
                        &tile_canvas,
                        nullptr,
                        0,
                        Transform()
                    );

                    cached_it = pattern_cache_.emplace(cache_key, std::make_shared<Pixmap>(std::move(tile_px))).first;
                }
            }

            if (cached_it != pattern_cache_.end() && cached_it->second) {
                nisaba::Pattern pat(cached_it->second->as_ref(), SpreadMode::Repeat, FilterQuality::Bilinear);

                float sx = static_cast<float>(pw) / xstep;
                float sy = static_cast<float>(ph) / ystep;
                Transform tile_to_pat = Transform::from_scale(1.0f / sx, 1.0f / sy).pre_translate(bx0, by0);
                Transform screen_to_pat = total_pat_ts.pre_concat(tile_to_pat);
                pat.set_transform(screen_to_pat);

                return Paint{Shader(pat)};
            }
        }
    }

    return std::nullopt;
}

} // namespace nisaba::pdf
