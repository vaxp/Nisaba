#include "nisaba/text/cff_font.hpp"
#include "nisaba/path/path_builder.hpp"
#include <cmath>
#include <cstring>
#include <algorithm>

namespace nisaba::text {

namespace {

class CffReader {
public:
    explicit CffReader(std::span<const uint8_t> data) : data_(data), pos_(0) {}

    [[nodiscard]] bool can_read(size_t bytes) const noexcept {
        return pos_ + bytes <= data_.size();
    }

    [[nodiscard]] size_t tell() const noexcept { return pos_; }
    void seek(size_t pos) noexcept { pos_ = std::min(pos, data_.size()); }
    void skip(size_t bytes) noexcept { seek(pos_ + bytes); }

    uint8_t read_u8() noexcept {
        return (pos_ < data_.size()) ? data_[pos_++] : 0;
    }

    int8_t read_i8() noexcept {
        return static_cast<int8_t>(read_u8());
    }

    uint16_t read_u16() noexcept {
        if (pos_ + 2 <= data_.size()) {
            uint16_t v = (static_cast<uint16_t>(data_[pos_]) << 8) |
                         static_cast<uint16_t>(data_[pos_ + 1]);
            pos_ += 2;
            return v;
        }
        pos_ = data_.size();
        return 0;
    }

    uint32_t read_u24() noexcept {
        if (pos_ + 3 <= data_.size()) {
            uint32_t v = (static_cast<uint32_t>(data_[pos_]) << 16) |
                         (static_cast<uint32_t>(data_[pos_ + 1]) << 8) |
                          static_cast<uint32_t>(data_[pos_ + 2]);
            pos_ += 3;
            return v;
        }
        pos_ = data_.size();
        return 0;
    }

    uint32_t read_u32() noexcept {
        if (pos_ + 4 <= data_.size()) {
            uint32_t v = (static_cast<uint32_t>(data_[pos_]) << 24) |
                         (static_cast<uint32_t>(data_[pos_ + 1]) << 16) |
                         (static_cast<uint32_t>(data_[pos_ + 2]) << 8) |
                          static_cast<uint32_t>(data_[pos_ + 3]);
            pos_ += 4;
            return v;
        }
        pos_ = data_.size();
        return 0;
    }

    int16_t read_i16() noexcept {
        return static_cast<int16_t>(read_u16());
    }

    int32_t read_i32() noexcept {
        return static_cast<int32_t>(read_u32());
    }

    uint32_t read_offset(uint8_t off_size) noexcept {
        switch (off_size) {
            case 1: return read_u8();
            case 2: return read_u16();
            case 3: return read_u24();
            case 4: return read_u32();
            default: return 0;
        }
    }

    std::span<const uint8_t> remaining_span() const noexcept {
        return (pos_ < data_.size()) ? data_.subspan(pos_) : std::span<const uint8_t>{};
    }

private:
    std::span<const uint8_t> data_;
    size_t pos_;
};

int32_t calculate_subr_bias(uint32_t count) noexcept {
    if (count < 1240) return 107;
    if (count < 33900) return 1131;
    return 32768;
}

} // namespace

CffFont::CffFont() = default;
CffFont::~CffFont() = default;

CffFont::CffFont(CffFont&&) noexcept = default;
CffFont& CffFont::operator=(CffFont&&) noexcept = default;

std::unique_ptr<CffFont> CffFont::from_bytes(std::span<const uint8_t> data, uint16_t expected_glyphs) {
    auto font = std::make_unique<CffFont>();
    if (!font->parse(data, expected_glyphs)) {
        return nullptr;
    }
    return font;
}

CffFont::CffIndex CffFont::read_index(std::span<const uint8_t> data, size_t& offset, bool is_cff2) {
    CffIndex idx;
    if (offset >= data.size()) return idx;

    CffReader reader(data);
    reader.seek(offset);

    uint32_t count = is_cff2 ? reader.read_u32() : reader.read_u16();
    if (count == 0) {
        offset = reader.tell();
        return idx;
    }

    uint8_t off_size = reader.read_u8();
    if (off_size < 1 || off_size > 4) return idx;

    idx.count = count;
    idx.off_size = off_size;
    idx.offsets.reserve(count + 1);

    for (size_t i = 0; i <= count; ++i) {
        idx.offsets.push_back(reader.read_offset(off_size));
    }

    if (idx.offsets.empty() || idx.offsets[0] != 1) return idx;

    uint32_t data_bytes = idx.offsets.back() - 1;
    size_t data_start = reader.tell();
    if (data_start + data_bytes <= data.size()) {
        idx.data = data.subspan(data_start, data_bytes);
        reader.skip(data_bytes);
    }

    offset = reader.tell();
    return idx;
}

bool CffFont::parse(std::span<const uint8_t> data, uint16_t expected_glyphs) {
    data_ = data;
    if (data_.size() < 4) return false;

    CffReader reader(data_);
    uint8_t major = reader.read_u8();
    uint8_t minor = reader.read_u8();
    (void)minor;
    uint8_t hdr_size = reader.read_u8();

    is_cff2_ = (major == 2);
    if (major != 1 && major != 2) return false;

    size_t pos = hdr_size;

    uint32_t charstrings_offset = 0;
    uint32_t private_dict_size = 0;
    uint32_t private_dict_offset = 0;

    if (!is_cff2_) {
        // CFF 1.0: Header -> Name INDEX -> Top DICT INDEX -> String INDEX -> Global Subrs INDEX
        reader.seek(pos);
        /* auto name_idx = */ read_index(data_, pos, false);
        auto top_dict_idx = read_index(data_, pos, false);
        /* auto string_idx = */ read_index(data_, pos, false);
        global_subrs_ = read_index(data_, pos, false);
        global_subrs_bias_ = calculate_subr_bias(global_subrs_.count);

        if (top_dict_idx.count > 0) {
            auto dict_bytes = top_dict_idx.get(0);
            CffReader d_reader(dict_bytes);
            std::vector<float> num_stack;

            while (d_reader.can_read(1)) {
                uint8_t b0 = d_reader.read_u8();
                if (b0 <= 21) { // Operator
                    uint16_t op = b0;
                    if (b0 == 12) {
                        op = (12 << 8) | d_reader.read_u8();
                    }

                    if (op == 17 && !num_stack.empty()) { // CharStrings
                        charstrings_offset = static_cast<uint32_t>(num_stack.back());
                    } else if (op == 18 && num_stack.size() >= 2) { // Private (size, offset)
                        private_dict_size = static_cast<uint32_t>(num_stack[num_stack.size() - 2]);
                        private_dict_offset = static_cast<uint32_t>(num_stack.back());
                    }
                    num_stack.clear();
                } else if (b0 >= 32 && b0 <= 246) {
                    num_stack.push_back(static_cast<float>(b0 - 139));
                } else if (b0 >= 247 && b0 <= 250) {
                    uint8_t b1 = d_reader.read_u8();
                    num_stack.push_back(static_cast<float>((b0 - 247) * 256 + b1 + 108));
                } else if (b0 >= 251 && b0 <= 254) {
                    uint8_t b1 = d_reader.read_u8();
                    num_stack.push_back(static_cast<float>(-(b0 - 251) * 256 - b1 - 108));
                } else if (b0 == 28) {
                    num_stack.push_back(static_cast<float>(d_reader.read_i16()));
                } else if (b0 == 29) {
                    num_stack.push_back(static_cast<float>(d_reader.read_i32()));
                } else if (b0 == 30) { // Real number (nibbles)
                    while (d_reader.can_read(1)) {
                        uint8_t nibbles = d_reader.read_u8();
                        if ((nibbles & 0x0F) == 0x0F || ((nibbles >> 4) & 0x0F) == 0x0F) break;
                    }
                    num_stack.push_back(0.0f);
                }
            }
        }
    } else {
        // CFF2: Top DICT length follows header
        reader.seek(3);
        /* uint8_t off_size = */ reader.read_u8();
        uint16_t top_dict_len = reader.read_u16();
        pos = hdr_size;

        if (pos + top_dict_len <= data_.size()) {
            auto dict_bytes = data_.subspan(pos, top_dict_len);
            pos += top_dict_len;
            CffReader d_reader(dict_bytes);
            std::vector<float> num_stack;

            while (d_reader.can_read(1)) {
                uint8_t b0 = d_reader.read_u8();
                if (b0 <= 21) {
                    uint16_t op = b0;
                    if (b0 == 12) op = (12 << 8) | d_reader.read_u8();

                    if (op == 17 && !num_stack.empty()) {
                        charstrings_offset = static_cast<uint32_t>(num_stack.back());
                    }
                    num_stack.clear();
                } else if (b0 >= 32 && b0 <= 246) {
                    num_stack.push_back(static_cast<float>(b0 - 139));
                } else if (b0 >= 247 && b0 <= 250) {
                    uint8_t b1 = d_reader.read_u8();
                    num_stack.push_back(static_cast<float>((b0 - 247) * 256 + b1 + 108));
                } else if (b0 >= 251 && b0 <= 254) {
                    uint8_t b1 = d_reader.read_u8();
                    num_stack.push_back(static_cast<float>(-(b0 - 251) * 256 - b1 - 108));
                } else if (b0 == 28) {
                    num_stack.push_back(static_cast<float>(d_reader.read_i16()));
                }
            }
        }
        global_subrs_ = read_index(data_, pos, true);
        global_subrs_bias_ = calculate_subr_bias(global_subrs_.count);
    }

    // Read CharStrings INDEX
    if (charstrings_offset > 0 && charstrings_offset < data_.size()) {
        size_t cs_pos = charstrings_offset;
        charstrings_index_ = read_index(data_, cs_pos, is_cff2_);
        num_glyphs_ = static_cast<uint16_t>(charstrings_index_.count);
    }

    if (num_glyphs_ == 0 && expected_glyphs > 0) {
        num_glyphs_ = expected_glyphs;
    }

    // Read Local Subrs if Private DICT was found
    if (private_dict_size > 0 && private_dict_offset < data_.size()) {
        auto priv_bytes = data_.subspan(private_dict_offset, std::min(static_cast<size_t>(private_dict_size), data_.size() - private_dict_offset));
        CffReader p_reader(priv_bytes);
        std::vector<float> num_stack;
        uint32_t local_subrs_offset = 0;

        while (p_reader.can_read(1)) {
            uint8_t b0 = p_reader.read_u8();
            if (b0 <= 21) {
                if (b0 == 19 && !num_stack.empty()) { // Subrs operator
                    local_subrs_offset = static_cast<uint32_t>(num_stack.back());
                }
                num_stack.clear();
            } else if (b0 >= 32 && b0 <= 246) {
                num_stack.push_back(static_cast<float>(b0 - 139));
            } else if (b0 >= 247 && b0 <= 250) {
                uint8_t b1 = p_reader.read_u8();
                num_stack.push_back(static_cast<float>((b0 - 247) * 256 + b1 + 108));
            } else if (b0 >= 251 && b0 <= 254) {
                uint8_t b1 = p_reader.read_u8();
                num_stack.push_back(static_cast<float>(-(b0 - 251) * 256 - b1 - 108));
            } else if (b0 == 28) {
                num_stack.push_back(static_cast<float>(p_reader.read_i16()));
            } else if (b0 == 29) {
                num_stack.push_back(static_cast<float>(p_reader.read_i32()));
            }
        }

        if (local_subrs_offset > 0) {
            size_t ls_pos = private_dict_offset + local_subrs_offset;
            local_subrs_ = read_index(data_, ls_pos, is_cff2_);
            local_subrs_bias_ = calculate_subr_bias(local_subrs_.count);
        }
    }

    is_valid_ = (charstrings_index_.count > 0);
    return is_valid_;
}

bool CffFont::execute_charstring_internal(
    std::span<const uint8_t> charstring,
    PathBuilder& builder,
    float& cur_x,
    float& cur_y,
    bool& in_subpath,
    size_t& num_stems,
    bool& width_checked,
    std::vector<float>& stack,
    int recursion_depth
) const {
    if (recursion_depth > 16 || charstring.empty()) return false;

    size_t pos = 0;
    while (pos < charstring.size()) {
        uint8_t b0 = charstring[pos++];

        if (b0 >= 32 && b0 <= 246) {
            stack.push_back(static_cast<float>(b0 - 139));
        } else if (b0 >= 247 && b0 <= 250) {
            if (pos >= charstring.size()) break;
            uint8_t b1 = charstring[pos++];
            stack.push_back(static_cast<float>((b0 - 247) * 256 + b1 + 108));
        } else if (b0 >= 251 && b0 <= 254) {
            if (pos >= charstring.size()) break;
            uint8_t b1 = charstring[pos++];
            stack.push_back(static_cast<float>(-(b0 - 251) * 256 - b1 - 108));
        } else if (b0 == 28) {
            if (pos + 1 >= charstring.size()) break;
            int16_t v = static_cast<int16_t>((charstring[pos] << 8) | charstring[pos + 1]);
            pos += 2;
            stack.push_back(static_cast<float>(v));
        } else if (b0 == 255) { // 32-bit 16.16 fixed-point number
            if (pos + 3 >= charstring.size()) break;
            int32_t int_part = static_cast<int16_t>((charstring[pos] << 8) | charstring[pos + 1]);
            uint16_t frac_part = (static_cast<uint16_t>(charstring[pos + 2]) << 8) | charstring[pos + 3];
            pos += 4;
            stack.push_back(static_cast<float>(int_part) + static_cast<float>(frac_part) / 65536.0f);
        } else {
            // Operator byte
            uint16_t op = b0;
            if (b0 == 12) {
                if (pos >= charstring.size()) break;
                op = (12 << 8) | charstring[pos++];
            }

            switch (op) {
                case 21: { // rmoveto: [width] dx dy
                    if (!width_checked && stack.size() > 2) {
                        width_checked = true;
                    }
                    if (stack.size() >= 2) {
                        float dx = stack[stack.size() - 2];
                        float dy = stack[stack.size() - 1];
                        if (in_subpath) builder.close();
                        cur_x += dx;
                        cur_y += dy;
                        builder.move_to(cur_x, -cur_y);
                        in_subpath = true;
                    }
                    stack.clear();
                    break;
                }
                case 22: { // hmoveto: [width] dx
                    if (!width_checked && stack.size() > 1) {
                        width_checked = true;
                    }
                    if (!stack.empty()) {
                        float dx = stack.back();
                        if (in_subpath) builder.close();
                        cur_x += dx;
                        builder.move_to(cur_x, -cur_y);
                        in_subpath = true;
                    }
                    stack.clear();
                    break;
                }
                case 4: { // vmoveto: [width] dy
                    if (!width_checked && stack.size() > 1) {
                        width_checked = true;
                    }
                    if (!stack.empty()) {
                        float dy = stack.back();
                        if (in_subpath) builder.close();
                        cur_y += dy;
                        builder.move_to(cur_x, -cur_y);
                        in_subpath = true;
                    }
                    stack.clear();
                    break;
                }
                case 5: { // rlineto: dx1 dy1 {dx_i dy_i}*
                    width_checked = true;
                    for (size_t i = 0; i + 1 < stack.size(); i += 2) {
                        cur_x += stack[i];
                        cur_y += stack[i + 1];
                        builder.line_to(cur_x, -cur_y);
                    }
                    stack.clear();
                    break;
                }
                case 6: { // hlineto: dx1 {dy2 dx3 dy4 ...}*
                    width_checked = true;
                    bool horiz = true;
                    for (float d : stack) {
                        if (horiz) cur_x += d;
                        else cur_y += d;
                        builder.line_to(cur_x, -cur_y);
                        horiz = !horiz;
                    }
                    stack.clear();
                    break;
                }
                case 7: { // vlineto: dy1 {dx2 dy3 dx4 ...}*
                    width_checked = true;
                    bool vert = true;
                    for (float d : stack) {
                        if (vert) cur_y += d;
                        else cur_x += d;
                        builder.line_to(cur_x, -cur_y);
                        vert = !vert;
                    }
                    stack.clear();
                    break;
                }
                case 8: { // rrcurveto: {dxa dya dxb dyb dxc dyc}+
                    width_checked = true;
                    for (size_t i = 0; i + 5 < stack.size(); i += 6) {
                        float cp1x = cur_x + stack[i];
                        float cp1y = cur_y + stack[i + 1];
                        float cp2x = cp1x + stack[i + 2];
                        float cp2y = cp1y + stack[i + 3];
                        cur_x = cp2x + stack[i + 4];
                        cur_y = cp2y + stack[i + 5];
                        builder.cubic_to(cp1x, -cp1y, cp2x, -cp2y, cur_x, -cur_y);
                    }
                    stack.clear();
                    break;
                }
                case 27: { // hhcurveto: [dy1] {dxa dxb dyb dxc}+
                    width_checked = true;
                    size_t i = 0;
                    if (stack.size() % 2 != 0) {
                        cur_y += stack[i++];
                    }
                    while (i + 3 < stack.size()) {
                        float cp1x = cur_x + stack[i];
                        float cp1y = cur_y;
                        float cp2x = cp1x + stack[i + 1];
                        float cp2y = cp1y + stack[i + 2];
                        cur_x = cp2x + stack[i + 3];
                        cur_y = cp2y;
                        builder.cubic_to(cp1x, -cp1y, cp2x, -cp2y, cur_x, -cur_y);
                        i += 4;
                    }
                    stack.clear();
                    break;
                }
                case 26: { // vvcurveto: [dx1] {dya dxb dyb dyc}+
                    width_checked = true;
                    size_t i = 0;
                    if (stack.size() % 2 != 0) {
                        cur_x += stack[i++];
                    }
                    while (i + 3 < stack.size()) {
                        float cp1x = cur_x;
                        float cp1y = cur_y + stack[i];
                        float cp2x = cp1x + stack[i + 1];
                        float cp2y = cp1y + stack[i + 2];
                        cur_x = cp2x;
                        cur_y = cp2y + stack[i + 3];
                        builder.cubic_to(cp1x, -cp1y, cp2x, -cp2y, cur_x, -cur_y);
                        i += 4;
                    }
                    stack.clear();
                    break;
                }
                case 31: { // hvcurveto: {dxa dxb dyb dyc} or {dya dxb dyb dxc}
                    width_checked = true;
                    size_t i = 0;
                    bool horiz = true;
                    while (i + 3 < stack.size()) {
                        if (horiz) {
                            float cp1x = cur_x + stack[i];
                            float cp1y = cur_y;
                            float cp2x = cp1x + stack[i + 1];
                            float cp2y = cp1y + stack[i + 2];
                            cur_x = cp2x + (i + 5 == stack.size() ? stack[i + 4] : 0.0f);
                            cur_y = cp2y + stack[i + 3];
                            if (i + 5 == stack.size()) i++;
                            builder.cubic_to(cp1x, -cp1y, cp2x, -cp2y, cur_x, -cur_y);
                        } else {
                            float cp1x = cur_x;
                            float cp1y = cur_y + stack[i];
                            float cp2x = cp1x + stack[i + 1];
                            float cp2y = cp1y + stack[i + 2];
                            cur_x = cp2x + stack[i + 3];
                            cur_y = cp2y + (i + 5 == stack.size() ? stack[i + 4] : 0.0f);
                            if (i + 5 == stack.size()) i++;
                            builder.cubic_to(cp1x, -cp1y, cp2x, -cp2y, cur_x, -cur_y);
                        }
                        horiz = !horiz;
                        i += 4;
                    }
                    stack.clear();
                    break;
                }
                case 30: { // vhcurveto
                    width_checked = true;
                    size_t i = 0;
                    bool vert = true;
                    while (i + 3 < stack.size()) {
                        if (vert) {
                            float cp1x = cur_x;
                            float cp1y = cur_y + stack[i];
                            float cp2x = cp1x + stack[i + 1];
                            float cp2y = cp1y + stack[i + 2];
                            cur_x = cp2x + stack[i + 3];
                            cur_y = cp2y + (i + 5 == stack.size() ? stack[i + 4] : 0.0f);
                            if (i + 5 == stack.size()) i++;
                            builder.cubic_to(cp1x, -cp1y, cp2x, -cp2y, cur_x, -cur_y);
                        } else {
                            float cp1x = cur_x + stack[i];
                            float cp1y = cur_y;
                            float cp2x = cp1x + stack[i + 1];
                            float cp2y = cp1y + stack[i + 2];
                            cur_x = cp2x + (i + 5 == stack.size() ? stack[i + 4] : 0.0f);
                            cur_y = cp2y + stack[i + 3];
                            if (i + 5 == stack.size()) i++;
                            builder.cubic_to(cp1x, -cp1y, cp2x, -cp2y, cur_x, -cur_y);
                        }
                        vert = !vert;
                        i += 4;
                    }
                    stack.clear();
                    break;
                }
                case 10: { // callsubr
                    if (!stack.empty()) {
                        int32_t subr_num = static_cast<int32_t>(stack.back()) + local_subrs_bias_;
                        stack.pop_back();
                        if (subr_num >= 0 && static_cast<size_t>(subr_num) < local_subrs_.count) {
                            auto subr_bytes = local_subrs_.get(static_cast<size_t>(subr_num));
                            execute_charstring_internal(subr_bytes, builder, cur_x, cur_y, in_subpath, num_stems, width_checked, stack, recursion_depth + 1);
                        }
                    }
                    break;
                }
                case 29: { // callgsubr
                    if (!stack.empty()) {
                        int32_t gsubr_num = static_cast<int32_t>(stack.back()) + global_subrs_bias_;
                        stack.pop_back();
                        if (gsubr_num >= 0 && static_cast<size_t>(gsubr_num) < global_subrs_.count) {
                            auto subr_bytes = global_subrs_.get(static_cast<size_t>(gsubr_num));
                            execute_charstring_internal(subr_bytes, builder, cur_x, cur_y, in_subpath, num_stems, width_checked, stack, recursion_depth + 1);
                        }
                    }
                    break;
                }
                case 11: { // return
                    return true;
                }
                case 14: { // endchar
                    if (in_subpath) {
                        builder.close();
                        in_subpath = false;
                    }
                    stack.clear();
                    return true;
                }
                case 1:  // hstem
                case 3:  // vstem
                case 18: // hstemhm
                case 23: // vstemhm
                    num_stems += stack.size() / 2;
                    stack.clear();
                    break;
                case 19: // hintmask
                case 20: // cntrmask
                    num_stems += stack.size() / 2;
                    stack.clear();
                    pos += (num_stems + 7) / 8;
                    break;
                default:
                    // Drop other unhandled or hinting operators
                    stack.clear();
                    break;
            }
        }
    }

    return true;
}

bool CffFont::execute_charstring(
    std::span<const uint8_t> charstring,
    Path& out_path,
    float scale
) const {
    PathBuilder builder;
    std::vector<float> stack;
    stack.reserve(64);

    float cur_x = 0.0f;
    float cur_y = 0.0f;
    bool in_subpath = false;
    size_t num_stems = 0;
    bool width_checked = false;

    execute_charstring_internal(
        charstring,
        builder,
        cur_x,
        cur_y,
        in_subpath,
        num_stems,
        width_checked,
        stack,
        0
    );

    if (in_subpath) {
        builder.close();
    }

    auto raw_path = builder.finish();
    if (raw_path && !raw_path->is_empty()) {
        if (scale != 1.0f) {
            raw_path->apply_transform(Transform::from_scale(scale, scale));
        }
        out_path.add_path(*raw_path);
    }

    return true;
}

bool CffFont::get_glyph_path(uint16_t glyph_id, Path& out_path, float scale) const {
    if (!is_valid_ || glyph_id >= charstrings_index_.count) {
        out_path.reset();
        return false;
    }

    auto cs_bytes = charstrings_index_.get(glyph_id);
    if (cs_bytes.empty()) {
        out_path.reset();
        return true;
    }

    out_path.reset();
    return execute_charstring(cs_bytes, out_path, scale);
}

} // namespace nisaba::text
