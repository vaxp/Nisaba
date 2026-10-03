#include "nisaba/text/ttf_font.hpp"
#include "nisaba/path/path_builder.hpp"
#include <fstream>
#include <cmath>
#include <cstring>
#include <algorithm>

namespace nisaba::text {

namespace {

constexpr uint32_t make_tag(char a, char b, char c, char d) noexcept {
    return (static_cast<uint32_t>(static_cast<uint8_t>(a)) << 24) |
           (static_cast<uint32_t>(static_cast<uint8_t>(b)) << 16) |
           (static_cast<uint32_t>(static_cast<uint8_t>(c)) << 8)  |
            static_cast<uint32_t>(static_cast<uint8_t>(d));
}

constexpr uint32_t TAG_HEAD = make_tag('h', 'e', 'a', 'd');
constexpr uint32_t TAG_MAXP = make_tag('m', 'a', 'x', 'p');
constexpr uint32_t TAG_HHEA = make_tag('h', 'h', 'e', 'a');
constexpr uint32_t TAG_HMTX = make_tag('h', 'm', 't', 'x');
constexpr uint32_t TAG_CMAP = make_tag('c', 'm', 'a', 'p');
constexpr uint32_t TAG_LOCA = make_tag('l', 'o', 'c', 'a');
constexpr uint32_t TAG_GLYF = make_tag('g', 'l', 'y', 'f');
constexpr uint32_t TAG_KERN = make_tag('k', 'e', 'r', 'n');
constexpr uint32_t TAG_NAME = make_tag('n', 'a', 'm', 'e');
constexpr uint32_t TAG_OS2  = make_tag('O', 'S', '/', '2');
constexpr uint32_t TAG_CFF  = make_tag('C', 'F', 'F', ' ');
constexpr uint32_t TAG_CFF2 = make_tag('C', 'F', 'F', '2');
constexpr uint32_t TAG_GSUB = make_tag('G', 'S', 'U', 'B');
constexpr uint32_t TAG_GPOS = make_tag('G', 'P', 'O', 'S');

class ByteReader {
public:
    explicit ByteReader(std::span<const uint8_t> buffer) : data_(buffer), pos_(0) {}

    bool can_read(size_t bytes) const noexcept {
        return pos_ + bytes <= data_.size();
    }

    size_t remaining() const noexcept {
        return (pos_ < data_.size()) ? (data_.size() - pos_) : 0;
    }

    size_t tell() const noexcept { return pos_; }

    bool seek(size_t offset) noexcept {
        if (offset <= data_.size()) {
            pos_ = offset;
            return true;
        }
        return false;
    }

    bool skip(size_t bytes) noexcept {
        return seek(pos_ + bytes);
    }

    uint8_t read_u8() noexcept {
        if (pos_ < data_.size()) {
            return data_[pos_++];
        }
        return 0;
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

    int16_t read_i16() noexcept {
        return static_cast<int16_t>(read_u16());
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

    int32_t read_i32() noexcept {
        return static_cast<int32_t>(read_u32());
    }

    float read_f2dot14() noexcept {
        return static_cast<float>(read_i16()) / 16384.0f;
    }

    std::span<const uint8_t> read_bytes(size_t count) noexcept {
        if (pos_ + count <= data_.size()) {
            auto s = data_.subspan(pos_, count);
            pos_ += count;
            return s;
        }
        pos_ = data_.size();
        return {};
    }

private:
    std::span<const uint8_t> data_;
    size_t pos_;
};

} // namespace

TtfFont::TtfFont() = default;
TtfFont::~TtfFont() = default;

TtfFont::TtfFont(TtfFont&&) noexcept = default;
TtfFont& TtfFont::operator=(TtfFont&&) noexcept = default;

std::unique_ptr<TtfFont> TtfFont::from_bytes(std::span<const uint8_t> data, bool copy_data) {
    auto font = std::make_unique<TtfFont>();
    if (copy_data) {
        font->owned_data_.assign(data.begin(), data.end());
        if (!font->parse_internal(font->owned_data_)) {
            return nullptr;
        }
    } else {
        if (!font->parse_internal(data)) {
            return nullptr;
        }
    }
    return font;
}

std::unique_ptr<TtfFont> TtfFont::from_file(std::string_view file_path) {
    std::ifstream file(std::string(file_path), std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        return nullptr;
    }
    std::streamsize size = file.tellg();
    if (size <= 0) {
        return nullptr;
    }
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> buffer(static_cast<size_t>(size));
    if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
        return nullptr;
    }

    auto font = std::make_unique<TtfFont>();
    font->owned_data_ = std::move(buffer);
    if (!font->parse_internal(font->owned_data_)) {
        return nullptr;
    }
    return font;
}

bool TtfFont::parse_internal(std::span<const uint8_t> data) {
    data_ = data;
    hmtx_table_ = nullptr;
    if (data_.size() < 12) {
        return false;
    }

    ByteReader reader(data_);
    uint32_t sfnt_version = reader.read_u32();
    // 0x00010000 for TrueType, 'OTTO' for OpenType, 'true' for Apple TrueType
    if (sfnt_version != 0x00010000 && sfnt_version != make_tag('O', 'T', 'T', 'O') &&
        sfnt_version != make_tag('t', 'r', 'u', 'e')) {
        return false;
    }

    uint16_t num_tables = reader.read_u16();
    reader.skip(6); // searchRange, entrySelector, rangeShift

    tables_.clear();
    for (uint16_t i = 0; i < num_tables; ++i) {
        if (!reader.can_read(16)) break;
        uint32_t tag = reader.read_u32();
        /* uint32_t checksum = */ reader.read_u32();
        uint32_t offset = reader.read_u32();
        uint32_t length = reader.read_u32();
        if (offset + length <= data_.size()) {
            tables_[tag] = TableRecord{offset, length};
        }
    }

    if (!parse_head()) return false;
    if (!parse_maxp()) return false;
    if (!parse_hhea()) return false;
    if (!parse_hmtx()) return false;
    if (!parse_cmap()) return false;

    // Optional tables
    parse_name();
    parse_os2();
    parse_kern();

    // Parse OpenType GSUB table (ligatures & glyph substitutions)
    auto gsub_it = tables_.find(TAG_GSUB);
    if (gsub_it != tables_.end()) {
        gsub_ = OpenTypeGsub::from_bytes(data_.subspan(gsub_it->second.offset, gsub_it->second.length));
    }

    // Parse OpenType GPOS table (pair kerning & mark positioning)
    auto gpos_it = tables_.find(TAG_GPOS);
    if (gpos_it != tables_.end()) {
        gpos_ = OpenTypeGpos::from_bytes(data_.subspan(gpos_it->second.offset, gpos_it->second.length));
    }

    // Parse PostScript CFF or CFF2 table
    auto cff_it = tables_.find(TAG_CFF);
    if (cff_it == tables_.end()) {
        cff_it = tables_.find(TAG_CFF2);
    }
    if (cff_it != tables_.end()) {
        cff_ = CffFont::from_bytes(data_.subspan(cff_it->second.offset, cff_it->second.length), num_glyphs_);
        if (cff_ && cff_->is_valid()) {
            is_cff_ = true;
        }
    }

    // Required tables for TrueType vector glyph outlines
    if (!is_cff_) {
        auto loca_it = tables_.find(TAG_LOCA);
        auto glyf_it = tables_.find(TAG_GLYF);
        if (loca_it != tables_.end()) {
            loca_offset_ = loca_it->second.offset;
            loca_table_ = data_.data() + loca_offset_;
        }
        if (glyf_it != tables_.end()) {
            glyf_offset_ = glyf_it->second.offset;
            glyf_table_ = data_.data() + glyf_offset_;
        }
    }

    is_valid_ = true;
    return true;
}

bool TtfFont::parse_head() {
    auto it = tables_.find(TAG_HEAD);
    if (it == tables_.end() || it->second.length < 54) return false;

    ByteReader reader(data_.subspan(it->second.offset, it->second.length));
    reader.skip(18);
    units_per_em_ = reader.read_u16();
    if (units_per_em_ == 0) units_per_em_ = 1000;

    reader.skip(30); // created, modified, xMin, yMin, xMax, yMax, macStyle, lowestRecPPEM, fontDirectionHint
    index_to_loc_format_ = reader.read_i16();
    return true;
}

bool TtfFont::parse_maxp() {
    auto it = tables_.find(TAG_MAXP);
    if (it == tables_.end() || it->second.length < 6) return false;

    ByteReader reader(data_.subspan(it->second.offset, it->second.length));
    reader.skip(4);
    num_glyphs_ = reader.read_u16();
    return num_glyphs_ > 0;
}

bool TtfFont::parse_hhea() {
    auto it = tables_.find(TAG_HHEA);
    if (it == tables_.end() || it->second.length < 36) return false;

    ByteReader reader(data_.subspan(it->second.offset, it->second.length));
    reader.skip(4);
    ascent_ = reader.read_i16();
    descent_ = reader.read_i16();
    line_gap_ = reader.read_i16();
    reader.skip(24);
    num_h_metrics_ = reader.read_u16();
    return num_h_metrics_ > 0;
}

bool TtfFont::parse_hmtx() {
    auto it = tables_.find(TAG_HMTX);
    if (it == tables_.end()) return false;
    hmtx_offset_ = it->second.offset;
    if (hmtx_offset_ < data_.size()) {
        hmtx_table_ = data_.data() + hmtx_offset_;
    }
    return true;
}

bool TtfFont::parse_cmap() {
    auto it = tables_.find(TAG_CMAP);
    if (it == tables_.end() || it->second.length < 4) return false;

    ByteReader reader(data_.subspan(it->second.offset, it->second.length));
    /* uint16_t version = */ reader.read_u16();
    uint16_t num_subtables = reader.read_u16();

    cmap_format4_offset_ = 0;
    cmap_format12_offset_ = 0;

    for (uint16_t i = 0; i < num_subtables; ++i) {
        if (!reader.can_read(8)) break;
        uint16_t platform_id = reader.read_u16();
        uint16_t encoding_id = reader.read_u16();
        uint32_t subtable_offset = reader.read_u32();

        if (subtable_offset >= it->second.length) continue;
        uint32_t abs_offset = it->second.offset + subtable_offset;

        ByteReader sub_reader(data_.subspan(abs_offset, it->second.length - subtable_offset));
        uint16_t format = sub_reader.read_u16();

        if (format == 4 && cmap_format4_offset_ == 0) {
            // Prioritize Unicode (Platform 0) or Windows Unicode BMP (Platform 3, Encoding 1)
            if (platform_id == 0 || (platform_id == 3 && encoding_id == 1)) {
                cmap_format4_offset_ = abs_offset;
            }
        } else if (format == 12 && cmap_format12_offset_ == 0) {
            // Full 32-bit Unicode coverage (Platform 3, Encoding 10 or Platform 0)
            if (platform_id == 0 || (platform_id == 3 && encoding_id == 10)) {
                cmap_format12_offset_ = abs_offset;
            }
        }
    }

    if (cmap_format4_offset_ != 0 && cmap_format4_offset_ + 14 <= data_.size()) {
        ByteReader f4_reader(data_.subspan(cmap_format4_offset_));
        f4_reader.skip(6);
        uint16_t seg_count_x2 = f4_reader.read_u16();
        cmap_format4_seg_count_ = seg_count_x2 / 2;
        cmap_format4_search_range_ = f4_reader.read_u16() / 2;
        cmap_format4_entry_selector_ = f4_reader.read_u16();
        cmap_format4_range_shift_ = f4_reader.read_u16() / 2;

        size_t end_off = cmap_format4_offset_ + 14;
        size_t start_off = end_off + 2 * cmap_format4_seg_count_ + 2;
        size_t delta_off = start_off + 2 * cmap_format4_seg_count_;
        size_t range_off = delta_off + 2 * cmap_format4_seg_count_;

        if (range_off + 2 * cmap_format4_seg_count_ <= data_.size()) {
            cmap_format4_end_codes_ = data_.data() + end_off;
            cmap_format4_start_codes_ = data_.data() + start_off;
            cmap_format4_id_deltas_ = data_.data() + delta_off;
            cmap_format4_id_range_offsets_ = data_.data() + range_off;
        }
    }

    if (cmap_format12_offset_ != 0 && cmap_format12_offset_ + 16 <= data_.size()) {
        ByteReader f12_reader(data_.subspan(cmap_format12_offset_));
        f12_reader.skip(12);
        cmap_format12_num_groups_ = f12_reader.read_u32();
        if (cmap_format12_offset_ + 16 + static_cast<size_t>(cmap_format12_num_groups_) * 12 <= data_.size()) {
            cmap_format12_groups_ = data_.data() + cmap_format12_offset_ + 16;
        } else {
            cmap_format12_num_groups_ = 0;
            cmap_format12_groups_ = nullptr;
        }
    }

    // Pre-populate ASCII direct lookup table and advance LUT (0 to 127) for O(1) instant queries
    for (char32_t c = 0; c < 128; ++c) {
        uint16_t gid = lookup_glyph_raw(c);
        ascii_lut_[c] = gid;
        ascii_adv_lut_[c] = glyph_advance_units(gid);
    }

    return (cmap_format4_offset_ != 0 || cmap_format12_offset_ != 0);
}

bool TtfFont::parse_name() {
    auto it = tables_.find(TAG_NAME);
    if (it == tables_.end() || it->second.length < 6) return false;

    ByteReader reader(data_.subspan(it->second.offset, it->second.length));
    /* uint16_t format = */ reader.read_u16();
    uint16_t count = reader.read_u16();
    uint16_t string_offset = reader.read_u16();
    uint32_t storage_base = it->second.offset + string_offset;

    for (uint16_t i = 0; i < count; ++i) {
        if (!reader.can_read(12)) break;
        uint16_t platform_id = reader.read_u16();
        uint16_t encoding_id = reader.read_u16();
        /* uint16_t language_id = */ reader.read_u16();
        uint16_t name_id = reader.read_u16();
        uint16_t length = reader.read_u16();
        uint16_t offset = reader.read_u16();

        uint32_t str_abs = storage_base + offset;
        if (str_abs + length > data_.size()) continue;

        auto str_bytes = data_.subspan(str_abs, length);

        auto decode_string = [&](std::span<const uint8_t> bytes, uint16_t pid, uint16_t /*eid*/) -> std::string {
            std::string res;
            if (pid == 0 || pid == 3) {
                // UTF-16BE
                for (size_t k = 0; k + 1 < bytes.size(); k += 2) {
                    char c = static_cast<char>(bytes[k + 1]);
                    if (bytes[k] == 0 && c != 0) res.push_back(c);
                }
            } else {
                // Latin-1 / ASCII
                res.assign(reinterpret_cast<const char*>(bytes.data()), bytes.size());
            }
            return res;
        };

        if (name_id == 1 && family_name_ == "Unknown") {
            family_name_ = decode_string(str_bytes, platform_id, encoding_id);
        } else if (name_id == 2 && style_name_ == "Regular") {
            style_name_ = decode_string(str_bytes, platform_id, encoding_id);
        }
    }
    return true;
}

bool TtfFont::parse_os2() {
    auto it = tables_.find(TAG_OS2);
    if (it == tables_.end() || it->second.length < 4) return false;

    ByteReader reader(data_.subspan(it->second.offset, it->second.length));
    /* uint16_t version = */ reader.read_u16();
    reader.skip(2); // xAvgCharWidth
    uint16_t us_weight_class = reader.read_u16();
    if (us_weight_class >= 100 && us_weight_class <= 900) {
        weight_ = static_cast<Weight>(us_weight_class);
    }
    return true;
}

bool TtfFont::parse_kern() {
    auto it = tables_.find(TAG_KERN);
    if (it == tables_.end() || it->second.length < 4) return false;

    ByteReader reader(data_.subspan(it->second.offset, it->second.length));
    uint16_t version = reader.read_u16();
    uint16_t n_tables = reader.read_u16();

    if (version != 0 || n_tables == 0) return false;

    for (uint16_t t = 0; t < n_tables; ++t) {
        if (!reader.can_read(6)) break;
        reader.skip(2); // subtable version
        uint16_t length = reader.read_u16();
        uint16_t coverage = reader.read_u16();

        // format 0, horizontal, kerning
        if ((coverage & 0xFF00) == 0 && (coverage & 0x0001)) {
            if (!reader.can_read(8)) break;
            uint16_t n_pairs = reader.read_u16();
            reader.skip(6); // searchRange, entrySelector, rangeShift

            for (uint16_t p = 0; p < n_pairs; ++p) {
                if (!reader.can_read(6)) break;
                uint16_t left = reader.read_u16();
                uint16_t right = reader.read_u16();
                int16_t value = reader.read_i16();
                uint32_t key = (static_cast<uint32_t>(left) << 16) | static_cast<uint32_t>(right);
                kerning_pairs_[key] = value;
            }
        } else {
            if (length > 6) reader.skip(length - 6);
        }
    }
    return true;
}

uint16_t TtfFont::lookup_glyph_raw(char32_t codepoint) const noexcept {
    // 1. Fast path for BMP: Try format 4 subtable (BMP up to 0xFFFF)
    if (cmap_format4_end_codes_ != nullptr && codepoint <= 0xFFFF) {
        uint16_t c = static_cast<uint16_t>(codepoint);
        const uint8_t* search = cmap_format4_end_codes_;
        if (c >= static_cast<uint16_t>((search[cmap_format4_range_shift_ * 2] << 8) | search[cmap_format4_range_shift_ * 2 + 1])) {
            search += cmap_format4_range_shift_ * 2;
        }
        search -= 2;
        uint16_t search_range = cmap_format4_search_range_ * 2;
        for (uint16_t es = cmap_format4_entry_selector_; es > 0; --es) {
            search_range >>= 1;
            uint16_t end = static_cast<uint16_t>((search[search_range] << 8) | search[search_range + 1]);
            if (c > end) {
                search += search_range;
            }
        }
        search += 2;
        uint16_t item = static_cast<uint16_t>((search - cmap_format4_end_codes_) >> 1);

        if (item < cmap_format4_seg_count_) {
            uint16_t start = static_cast<uint16_t>((cmap_format4_start_codes_[item * 2] << 8) | cmap_format4_start_codes_[item * 2 + 1]);
            if (c >= start) {
                uint16_t offset = static_cast<uint16_t>((cmap_format4_id_range_offsets_[item * 2] << 8) | cmap_format4_id_range_offsets_[item * 2 + 1]);
                int16_t id_delta = static_cast<int16_t>((cmap_format4_id_deltas_[item * 2] << 8) | cmap_format4_id_deltas_[item * 2 + 1]);

                if (offset == 0) {
                    return static_cast<uint16_t>((c + id_delta) & 0xFFFF);
                } else {
                    const uint8_t* p = cmap_format4_id_range_offsets_ + (item * 2) + offset + (static_cast<size_t>(c - start) * 2);
                    if (p + 2 <= data_.data() + data_.size()) {
                        uint16_t g_id = static_cast<uint16_t>((p[0] << 8) | p[1]);
                        if (g_id != 0) {
                            return static_cast<uint16_t>((g_id + id_delta) & 0xFFFF);
                        }
                    }
                }
            }
        }

        if (cmap_format12_groups_ == nullptr) {
            return 0;
        }
    }

    // 2. Try format 12 subtable (full Unicode / supplementary planes)
    if (cmap_format12_groups_ != nullptr) {
        uint32_t low = 0, high = cmap_format12_num_groups_;
        while (low < high) {
            uint32_t mid = low + (high - low) / 2;
            const uint8_t* p = cmap_format12_groups_ + (static_cast<size_t>(mid) * 12);
            uint32_t start_char = (static_cast<uint32_t>(p[0]) << 24) | (static_cast<uint32_t>(p[1]) << 16) | (static_cast<uint32_t>(p[2]) << 8) | p[3];
            uint32_t end_char = (static_cast<uint32_t>(p[4]) << 24) | (static_cast<uint32_t>(p[5]) << 16) | (static_cast<uint32_t>(p[6]) << 8) | p[7];

            if (codepoint < start_char) {
                high = mid;
            } else if (codepoint > end_char) {
                low = mid + 1;
            } else {
                uint32_t start_glyph = (static_cast<uint32_t>(p[8]) << 24) | (static_cast<uint32_t>(p[9]) << 16) | (static_cast<uint32_t>(p[10]) << 8) | p[11];
                return static_cast<uint16_t>(start_glyph + (codepoint - start_char));
            }
        }
    }

    return 0;
}



int16_t TtfFont::get_kerning(uint16_t left_glyph, uint16_t right_glyph) const noexcept {
    // 1. Try OpenType GPOS PairPos first
    if (gpos_) {
        int16_t gpos_kern = 0;
        if (gpos_->get_kerning(left_glyph, right_glyph, gpos_kern)) {
            return gpos_kern;
        }
    }

    // 2. Fall back to legacy TrueType 'kern' table
    if (kerning_pairs_.empty()) return 0;
    uint32_t key = (static_cast<uint32_t>(left_glyph) << 16) | static_cast<uint32_t>(right_glyph);
    auto it = kerning_pairs_.find(key);
    return (it != kerning_pairs_.end()) ? it->second : 0;
}

bool TtfFont::apply_ligatures(std::vector<uint16_t>& glyphs) const {
    if (gsub_) {
        return gsub_->apply_ligatures(glyphs);
    }
    return false;
}

bool TtfFont::apply_gsub(std::vector<uint16_t>& glyphs, std::span<const uint32_t> feature_tags) const {
    if (gsub_) {
        return gsub_->apply_features(glyphs, feature_tags);
    }
    return false;
}

bool TtfFont::get_glyph_path(uint16_t glyph_id, Path& out_path, float font_size) const {
    if (glyph_id >= num_glyphs_) [[unlikely]] {
        return false;
    }
    if (!is_cff_ && (!loca_table_ || !glyf_table_)) [[unlikely]] {
        return false;
    }

    float scale = scale_for_size(font_size);

    if (glyph_path_cache_.empty()) {
        std::unique_lock<std::recursive_mutex> lock;
        if (cache_mutex_) lock = std::unique_lock<std::recursive_mutex>(*cache_mutex_);
        if (glyph_path_cache_.empty()) {
            glyph_path_cache_.resize(num_glyphs_);
        }
    }

    if (glyph_id < glyph_path_cache_.size()) {
        const auto& entry = glyph_path_cache_[glyph_id];
        if (!entry.valid.load(std::memory_order_acquire)) {
            std::unique_lock<std::recursive_mutex> lock;
            if (cache_mutex_) lock = std::unique_lock<std::recursive_mutex>(*cache_mutex_);
            if (!entry.valid.load(std::memory_order_relaxed)) {
                auto& mut_entry = glyph_path_cache_[glyph_id];

                if (is_cff_ && cff_) {
                    // Extract CFF / CFF2 PostScript cubic Bézier outlines
                    cff_->get_glyph_path(glyph_id, mut_entry.path, 1.0f);
                } else if (loca_table_ && glyf_table_) {
                    // Extract TrueType quadratic outlines
                    uint32_t glyph_offset = 0;
                    uint32_t next_offset = 0;

                    if (index_to_loc_format_ == 0) {
                        const uint8_t* p = loca_table_ + (static_cast<size_t>(glyph_id) * 2);
                        glyph_offset = (static_cast<uint32_t>(p[0]) << 8 | p[1]) * 2;
                        next_offset = (static_cast<uint32_t>(p[2]) << 8 | p[3]) * 2;
                    } else {
                        const uint8_t* p = loca_table_ + (static_cast<size_t>(glyph_id) * 4);
                        glyph_offset = (static_cast<uint32_t>(p[0]) << 24) |
                                       (static_cast<uint32_t>(p[1]) << 16) |
                                       (static_cast<uint32_t>(p[2]) << 8) |
                                        static_cast<uint32_t>(p[3]);
                        next_offset = (static_cast<uint32_t>(p[4]) << 24) |
                                      (static_cast<uint32_t>(p[5]) << 16) |
                                      (static_cast<uint32_t>(p[6]) << 8) |
                                       static_cast<uint32_t>(p[7]);
                    }

                    if (glyph_offset < next_offset) {
                        const uint8_t* g_data = glyf_table_ + glyph_offset;
                        int16_t num_contours = static_cast<int16_t>((g_data[0] << 8) | g_data[1]);
                        if (num_contours >= 0) {
                            extract_simple_glyph(g_data, mut_entry.path, 1.0f);
                        } else {
                            extract_composite_glyph(g_data, mut_entry.path, 1.0f, 0);
                        }
                    } else {
                        mut_entry.path.reset();
                    }
                } else {
                    mut_entry.path.reset();
                }
                mut_entry.valid.store(true, std::memory_order_release);
            }
        }

        const auto& cached = glyph_path_cache_[glyph_id];
        if (cached.path.is_empty()) {
            out_path.reset();
            return true;
        }

        out_path.verbs_ = cached.path.verbs_;
        out_path.points_ = cached.path.points_;
        if (scale != 1.0f) {
            for (auto& pt : out_path.points_) {
                pt.x *= scale;
                pt.y *= scale;
            }
            auto b = Rect::from_ltrb(
                cached.path.bounds_.left() * scale,
                cached.path.bounds_.top() * scale,
                cached.path.bounds_.right() * scale,
                cached.path.bounds_.bottom() * scale
            );
            if (b) out_path.bounds_ = *b;
            else out_path.bounds_ = cached.path.bounds_;
        } else {
            out_path.bounds_ = cached.path.bounds_;
        }
        return true;
    }

    return false;
}

bool TtfFont::extract_simple_glyph(const uint8_t* g_data, Path& path, float scale) const {
    int16_t num_contours = static_cast<int16_t>((g_data[0] << 8) | g_data[1]);
    if (num_contours <= 0) {
        path.reset();
        return true;
    }

    int16_t xMin = static_cast<int16_t>((g_data[2] << 8) | g_data[3]);
    int16_t yMin = static_cast<int16_t>((g_data[4] << 8) | g_data[5]);
    int16_t xMax = static_cast<int16_t>((g_data[6] << 8) | g_data[7]);
    int16_t yMax = static_cast<int16_t>((g_data[8] << 8) | g_data[9]);

    const uint8_t* end_pts_data = g_data + 10;
    uint16_t last_end_idx = (static_cast<uint16_t>(end_pts_data[(num_contours - 1) * 2]) << 8) |
                             end_pts_data[(num_contours - 1) * 2 + 1];
    uint16_t num_points = last_end_idx + 1;
    if (num_points == 0) {
        path.reset();
        return true;
    }

    const uint8_t* p = end_pts_data + (static_cast<size_t>(num_contours) * 2);
    uint16_t instruction_len = (static_cast<uint16_t>(p[0]) << 8) | p[1];
    p += 2 + instruction_len;

    struct RawPt {
        float x;
        float y;
        bool on_curve;
    };

    constexpr size_t STACK_POINTS = 512;
    uint8_t flags_stack[STACK_POINTS];
    RawPt points_stack[STACK_POINTS];
    std::vector<uint8_t> flags_heap;
    std::vector<RawPt> points_heap;

    uint8_t* flags = (num_points <= STACK_POINTS) 
        ? flags_stack 
        : (flags_heap.resize(num_points), flags_heap.data());
    RawPt* points = (num_points <= STACK_POINTS) 
        ? points_stack 
        : (points_heap.resize(num_points), points_heap.data());

    for (uint16_t i = 0; i < num_points; ++i) {
        uint8_t f = *p++;
        flags[i] = f;
        if (f & 0x08) { // REPEAT_FLAG
            uint8_t count = *p++;
            while (count-- > 0 && (i + 1) < num_points) {
                flags[++i] = f;
            }
        }
    }

    // Read X coordinates and pre-scale
    int16_t cur_x = 0;
    for (uint16_t i = 0; i < num_points; ++i) {
        uint8_t f = flags[i];
        points[i].on_curve = (f & 0x01) != 0;
        if (f & 0x02) { // X_SHORT_VECTOR
            uint8_t b = *p++;
            cur_x += (f & 0x10) ? static_cast<int16_t>(b) : -static_cast<int16_t>(b);
        } else {
            if (!(f & 0x10)) { // NOT SAME
                cur_x += static_cast<int16_t>((p[0] << 8) | p[1]);
                p += 2;
            }
        }
        points[i].x = static_cast<float>(cur_x) * scale;
    }

    // Read Y coordinates and pre-scale
    float neg_scale = -scale;
    int16_t cur_y = 0;
    for (uint16_t i = 0; i < num_points; ++i) {
        uint8_t f = flags[i];
        if (f & 0x04) { // Y_SHORT_VECTOR
            uint8_t b = *p++;
            cur_y += (f & 0x20) ? static_cast<int16_t>(b) : -static_cast<int16_t>(b);
        } else {
            if (!(f & 0x20)) { // NOT SAME
                cur_y += static_cast<int16_t>((p[0] << 8) | p[1]);
                p += 2;
            }
        }
        points[i].y = static_cast<float>(cur_y) * neg_scale;
    }

    // Emit contours directly into path without intermediate builder
    size_t max_verbs = static_cast<size_t>(num_points) + static_cast<size_t>(num_contours) * 2;
    size_t max_points = static_cast<size_t>(num_points) * 2 + static_cast<size_t>(num_contours);

    path.verbs_.resize(max_verbs);
    path.points_.resize(max_points);
    PathVerb* v_ptr = path.verbs_.data();
    Point* pt_ptr = path.points_.data();

    uint16_t start_idx = 0;

    for (int16_t c = 0; c < num_contours; ++c) {
        uint16_t end_idx = (static_cast<uint16_t>(end_pts_data[c * 2]) << 8) | end_pts_data[c * 2 + 1];
        if (end_idx < start_idx || end_idx >= num_points) break;

        size_t count = end_idx - start_idx + 1;
        if (count < 2) {
            start_idx = end_idx + 1;
            continue;
        }

        const RawPt* c_pts = &points[start_idx];

        // Determine starting point
        float start_x = 0.0f, start_y = 0.0f;
        size_t curr_idx = 0;

        if (c_pts[0].on_curve) {
            start_x = c_pts[0].x;
            start_y = c_pts[0].y;
            curr_idx = 1;
        } else if (c_pts[count - 1].on_curve) {
            start_x = c_pts[count - 1].x;
            start_y = c_pts[count - 1].y;
            curr_idx = 0;
        } else {
            // Implicit midpoint between first and last off-curve
            start_x = (c_pts[0].x + c_pts[count - 1].x) * 0.5f;
            start_y = (c_pts[0].y + c_pts[count - 1].y) * 0.5f;
            curr_idx = 0;
        }

        *v_ptr++ = PathVerb::Move;
        *pt_ptr++ = Point(start_x, start_y);

        float cx = 0.0f, cy = 0.0f;
        bool has_control = false;

        auto process_pt = [&](const RawPt& pt) {
            float px = pt.x;
            float py = pt.y;

            if (pt.on_curve) {
                if (has_control) {
                    *v_ptr++ = PathVerb::Quad;
                    *pt_ptr++ = Point(cx, cy);
                    *pt_ptr++ = Point(px, py);
                    has_control = false;
                } else {
                    *v_ptr++ = PathVerb::Line;
                    *pt_ptr++ = Point(px, py);
                }
            } else {
                if (has_control) {
                    float mx = (cx + px) * 0.5f;
                    float my = (cy + py) * 0.5f;
                    *v_ptr++ = PathVerb::Quad;
                    *pt_ptr++ = Point(cx, cy);
                    *pt_ptr++ = Point(mx, my);
                }
                cx = px;
                cy = py;
                has_control = true;
            }
        };

        if (curr_idx == 0) {
            for (size_t i = 0; i < count; ++i) {
                process_pt(c_pts[i]);
            }
        } else {
            for (size_t i = 1; i < count; ++i) {
                process_pt(c_pts[i]);
            }
            process_pt(c_pts[0]);
        }

        // Close to start point
        if (has_control) {
            *v_ptr++ = PathVerb::Quad;
            *pt_ptr++ = Point(cx, cy);
            *pt_ptr++ = Point(start_x, start_y);
        }
        *v_ptr++ = PathVerb::Close;

        start_idx = end_idx + 1;
    }

    path.verbs_.resize(v_ptr - path.verbs_.data());
    path.points_.resize(pt_ptr - path.points_.data());

    float bx0 = static_cast<float>(xMin) * scale;
    float bx1 = static_cast<float>(xMax) * scale;
    float by0 = -static_cast<float>(yMax) * scale;
    float by1 = -static_cast<float>(yMin) * scale;
    auto precomputed_bounds = Rect::from_ltrb(bx0, by0, bx1, by1);

    if (precomputed_bounds) {
        path.bounds_ = *precomputed_bounds;
    } else {
        auto b = Rect::from_points(path.points_.data(), path.points_.size());
        if (b) path.bounds_ = *b;
    }

    return true;
}

bool TtfFont::extract_composite_glyph(const uint8_t* g_data, Path& path, float scale, int recursion_depth) const {
    if (recursion_depth > 16) return false;

    const uint8_t* p = g_data + 10; // skip num_contours, xMin, yMin, xMax, yMax

    constexpr uint16_t ARG_1_AND_2_ARE_WORDS = 0x0001;
    constexpr uint16_t ARGS_ARE_XY_VALUES = 0x0002;
    constexpr uint16_t WE_HAVE_A_SCALE = 0x0008;
    constexpr uint16_t MORE_COMPONENTS = 0x0020;
    constexpr uint16_t WE_HAVE_AN_X_AND_Y_SCALE = 0x0040;
    constexpr uint16_t WE_HAVE_A_TWO_BY_TWO = 0x0080;

    PathBuilder combined_builder;

    uint16_t flags = 0;
    do {
        flags = static_cast<uint16_t>((p[0] << 8) | p[1]);
        uint16_t comp_glyph_id = static_cast<uint16_t>((p[2] << 8) | p[3]);
        p += 4;

        float dx = 0.0f, dy = 0.0f;
        if (flags & ARG_1_AND_2_ARE_WORDS) {
            int16_t a1 = static_cast<int16_t>((p[0] << 8) | p[1]);
            int16_t a2 = static_cast<int16_t>((p[2] << 8) | p[3]);
            p += 4;
            if (flags & ARGS_ARE_XY_VALUES) {
                dx = static_cast<float>(a1) * scale;
                dy = -static_cast<float>(a2) * scale;
            }
        } else {
            int8_t a1 = static_cast<int8_t>(*p++);
            int8_t a2 = static_cast<int8_t>(*p++);
            if (flags & ARGS_ARE_XY_VALUES) {
                dx = static_cast<float>(a1) * scale;
                dy = -static_cast<float>(a2) * scale;
            }
        }

        auto read_f2dot14 = [&p]() -> float {
            int16_t val = static_cast<int16_t>((p[0] << 8) | p[1]);
            p += 2;
            return static_cast<float>(val) / 16384.0f;
        };

        float a = 1.0f, b = 0.0f, c = 0.0f, d = 1.0f;
        if (flags & WE_HAVE_A_SCALE) {
            a = d = read_f2dot14();
        } else if (flags & WE_HAVE_AN_X_AND_Y_SCALE) {
            a = read_f2dot14();
            d = read_f2dot14();
        } else if (flags & WE_HAVE_A_TWO_BY_TWO) {
            a = read_f2dot14();
            b = read_f2dot14();
            c = read_f2dot14();
            d = read_f2dot14();
        }

        Path comp_path;
        if (get_glyph_path(comp_glyph_id, comp_path, scale * static_cast<float>(units_per_em_))) {
            // Apply 2x2 affine matrix + translation to component path
            auto transform_pt = [&](Point pt) -> Point {
                float nx = a * pt.x + c * pt.y + dx;
                float ny = b * pt.x + d * pt.y + dy;
                return Point::from_xy(nx, ny);
            };

            auto iter = comp_path.segments();
            while (auto seg_opt = iter.next()) {
                const auto& seg = *seg_opt;
                switch (seg.type) {
                    case PathSegment::Type::MoveTo: {
                        Point p0 = transform_pt(seg.p0);
                        combined_builder.move_to(p0.x, p0.y);
                        break;
                    }
                    case PathSegment::Type::LineTo: {
                        Point p0 = transform_pt(seg.p0);
                        combined_builder.line_to(p0.x, p0.y);
                        break;
                    }
                    case PathSegment::Type::QuadTo: {
                        Point p0 = transform_pt(seg.p0);
                        Point p1 = transform_pt(seg.p1);
                        combined_builder.quad_to(p0.x, p0.y, p1.x, p1.y);
                        break;
                    }
                    case PathSegment::Type::CubicTo: {
                        Point p0 = transform_pt(seg.p0);
                        Point p1 = transform_pt(seg.p1);
                        Point p2 = transform_pt(seg.p2);
                        combined_builder.cubic_to(p0.x, p0.y, p1.x, p1.y, p2.x, p2.y);
                        break;
                    }
                    case PathSegment::Type::Close:
                        combined_builder.close();
                        break;
                }
            }
        }

    } while (flags & MORE_COMPONENTS);

    auto fin = combined_builder.finish();
    if (fin) {
        path = std::move(*fin);
        return true;
    }
    return false;
}

} // namespace nisaba::text
