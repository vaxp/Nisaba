#include "nisaba/text/opentype_tables.hpp"
#include <algorithm>
#include <cstring>

namespace nisaba::text {

namespace {

inline uint32_t make_tag(char a, char b, char c, char d) noexcept {
    return (static_cast<uint32_t>(static_cast<uint8_t>(a)) << 24) |
           (static_cast<uint32_t>(static_cast<uint8_t>(b)) << 16) |
           (static_cast<uint32_t>(static_cast<uint8_t>(c)) << 8) |
            static_cast<uint32_t>(static_cast<uint8_t>(d));
}

class SpanReader {
public:
    explicit SpanReader(std::span<const uint8_t> data) : data_(data), pos_(0) {}

    [[nodiscard]] bool can_read(size_t n) const noexcept {
        return pos_ + n <= data_.size();
    }

    [[nodiscard]] size_t tell() const noexcept { return pos_; }
    void seek(size_t p) noexcept { pos_ = std::min(p, data_.size()); }
    void skip(size_t n) noexcept { seek(pos_ + n); }

    uint8_t read_u8() noexcept {
        return (pos_ < data_.size()) ? data_[pos_++] : 0;
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

    std::span<const uint8_t> subspan(size_t offset) const noexcept {
        return (offset < data_.size()) ? data_.subspan(offset) : std::span<const uint8_t>{};
    }

private:
    std::span<const uint8_t> data_;
    size_t pos_;
};

int32_t read_coverage_index(std::span<const uint8_t> table_data, size_t cov_offset, uint16_t glyph_id) {
    if (cov_offset >= table_data.size()) return -1;
    SpanReader r(table_data.subspan(cov_offset));
    uint16_t format = r.read_u16();

    if (format == 1) {
        uint16_t count = r.read_u16();
        if (!r.can_read(count * 2)) return -1;
        // Binary search since glyphArray is strictly ascending per spec
        int left = 0;
        int right = static_cast<int>(count) - 1;
        while (left <= right) {
            int mid = left + (right - left) / 2;
            size_t off = 4 + mid * 2;
            uint16_t g = (static_cast<uint16_t>(table_data[cov_offset + off]) << 8) |
                          table_data[cov_offset + off + 1];
            if (g == glyph_id) return mid;
            if (g < glyph_id) left = mid + 1;
            else right = mid - 1;
        }
        return -1;
    } else if (format == 2) {
        uint16_t range_count = r.read_u16();
        for (uint16_t i = 0; i < range_count; ++i) {
            if (!r.can_read(6)) break;
            uint16_t start = r.read_u16();
            uint16_t end = r.read_u16();
            uint16_t start_cov = r.read_u16();
            if (glyph_id >= start && glyph_id <= end) {
                return static_cast<int32_t>(start_cov + (glyph_id - start));
            }
        }
        return -1;
    }
    return -1;
}

uint16_t read_class_def(std::span<const uint8_t> table_data, size_t class_def_offset, uint16_t glyph_id) {
    if (class_def_offset >= table_data.size()) return 0;
    SpanReader r(table_data.subspan(class_def_offset));
    uint16_t format = r.read_u16();

    if (format == 1) {
        uint16_t start_glyph = r.read_u16();
        uint16_t count = r.read_u16();
        if (glyph_id >= start_glyph && glyph_id < start_glyph + count) {
            size_t off = 6 + (glyph_id - start_glyph) * 2;
            if (off + 2 <= r.subspan(0).size()) {
                SpanReader cr(r.subspan(off));
                return cr.read_u16();
            }
        }
        return 0;
    } else if (format == 2) {
        uint16_t range_count = r.read_u16();
        for (uint16_t i = 0; i < range_count; ++i) {
            if (!r.can_read(6)) break;
            uint16_t start = r.read_u16();
            uint16_t end = r.read_u16();
            uint16_t cls = r.read_u16();
            if (glyph_id >= start && glyph_id <= end) {
                return cls;
            }
        }
        return 0;
    }
    return 0;
}

size_t get_value_record_size(uint16_t format) noexcept {
    // Each set bit in format 0x00FF consumes 2 bytes
    size_t count = 0;
    for (uint16_t mask = 0x0001; mask <= 0x0080; mask <<= 1) {
        if (format & mask) ++count;
    }
    return count * 2;
}

void parse_value_record(SpanReader& r, uint16_t format, GlyphPlacementAdjustment& adj) {
    if (format & 0x0001) adj.x_placement = r.read_i16();
    if (format & 0x0002) adj.y_placement = r.read_i16();
    if (format & 0x0004) adj.x_advance = r.read_i16();
    if (format & 0x0008) adj.y_advance = r.read_i16();
    if (format & 0x0010) r.skip(2); // XPlaDevice
    if (format & 0x0020) r.skip(2); // YPlaDevice
    if (format & 0x0040) r.skip(2); // XAdvDevice
    if (format & 0x0080) r.skip(2); // YAdvDevice
}

} // namespace

// ============================================================================
// OpenType GSUB Implementation
// ============================================================================

struct OpenTypeGsub::Impl {
    std::span<const uint8_t> data;
    uint16_t script_list_offset{0};
    uint16_t feature_list_offset{0};
    uint16_t lookup_list_offset{0};

    bool parse(std::span<const uint8_t> d) {
        data = d;
        if (data.size() < 10) return false;
        SpanReader r(data);
        uint16_t major = r.read_u16();
        if (major != 1) return false;
        r.read_u16(); // minor
        script_list_offset = r.read_u16();
        feature_list_offset = r.read_u16();
        lookup_list_offset = r.read_u16();
        return (feature_list_offset < data.size() && lookup_list_offset < data.size());
    }

    std::vector<uint16_t> get_lookups_for_features(std::span<const uint32_t> requested_tags) const {
        std::vector<uint16_t> lookup_indices;
        if (feature_list_offset >= data.size()) return lookup_indices;

        SpanReader fr(data.subspan(feature_list_offset));
        uint16_t feature_count = fr.read_u16();

        for (uint16_t i = 0; i < feature_count; ++i) {
            if (!fr.can_read(6)) break;
            uint32_t tag = fr.read_u32();
            uint16_t feat_offset = fr.read_u16();

            bool match = false;
            for (uint32_t req : requested_tags) {
                if (req == tag) { match = true; break; }
            }

            if (match && feature_list_offset + feat_offset < data.size()) {
                SpanReader tr(data.subspan(feature_list_offset + feat_offset));
                tr.skip(2); // featureParams
                uint16_t count = tr.read_u16();
                for (uint16_t k = 0; k < count; ++k) {
                    if (!tr.can_read(2)) break;
                    uint16_t l_idx = tr.read_u16();
                    if (std::find(lookup_indices.begin(), lookup_indices.end(), l_idx) == lookup_indices.end()) {
                        lookup_indices.push_back(l_idx);
                    }
                }
            }
        }
        std::sort(lookup_indices.begin(), lookup_indices.end());
        return lookup_indices;
    }

    void apply_lookup(uint16_t lookup_idx, std::vector<uint16_t>& glyphs) const {
        if (lookup_list_offset >= data.size()) return;
        SpanReader lr(data.subspan(lookup_list_offset));
        uint16_t total_lookups = lr.read_u16();
        if (lookup_idx >= total_lookups) return;

        lr.skip(lookup_idx * 2);
        if (!lr.can_read(2)) return;
        uint16_t l_offset = lr.read_u16();
        size_t abs_lookup_offset = lookup_list_offset + l_offset;
        if (abs_lookup_offset >= data.size()) return;

        SpanReader tr(data.subspan(abs_lookup_offset));
        uint16_t lookup_type = tr.read_u16();
        tr.skip(2); // lookupFlag
        uint16_t subtable_count = tr.read_u16();

        for (uint16_t s = 0; s < subtable_count; ++s) {
            if (!tr.can_read(2)) break;
            uint16_t sub_offset = tr.read_u16();
            size_t abs_sub_offset = abs_lookup_offset + sub_offset;
            if (abs_sub_offset >= data.size()) continue;

            auto sub_span = data.subspan(abs_sub_offset);
            SpanReader sr(sub_span);
            uint16_t effective_type = lookup_type;
            if (lookup_type == 7) { // ExtensionSubst
                uint16_t ext_fmt = sr.read_u16();
                effective_type = sr.read_u16();
                uint32_t ext_offset = sr.read_u32();
                if (ext_fmt != 1 || ext_offset >= sub_span.size()) continue;
                sub_span = sub_span.subspan(ext_offset);
                sr = SpanReader(sub_span);
            }

            if (effective_type == 1) { // Single Substitution
                uint16_t subst_format = sr.read_u16();
                uint16_t cov_offset = sr.read_u16();
                if (subst_format == 1) {
                    int16_t delta = sr.read_i16();
                    for (auto& g : glyphs) {
                        int32_t cov = read_coverage_index(sub_span, cov_offset, g);
                        if (cov >= 0) {
                            g = static_cast<uint16_t>((static_cast<int32_t>(g) + delta) & 0xFFFF);
                        }
                    }
                } else if (subst_format == 2) {
                    uint16_t glyph_count = sr.read_u16();
                    for (auto& g : glyphs) {
                        int32_t cov = read_coverage_index(sub_span, cov_offset, g);
                        if (cov >= 0 && static_cast<uint16_t>(cov) < glyph_count) {
                            SpanReader gr(sub_span.subspan(6 + cov * 2));
                            g = gr.read_u16();
                        }
                    }
                }
            } else if (lookup_type == 4) { // Ligature Substitution
                uint16_t subst_format = sr.read_u16();
                if (subst_format != 1) continue;
                uint16_t cov_offset = sr.read_u16();
                uint16_t lig_set_count = sr.read_u16();

                for (size_t i = 0; i < glyphs.size(); ) {
                    int32_t cov = read_coverage_index(sub_span, cov_offset, glyphs[i]);
                    if (cov < 0 || static_cast<uint16_t>(cov) >= lig_set_count) {
                        ++i;
                        continue;
                    }

                    SpanReader lsr(sub_span.subspan(6 + cov * 2));
                    uint16_t lig_set_offset = lsr.read_u16();
                    if (lig_set_offset >= sub_span.size()) {
                        ++i;
                        continue;
                    }

                    auto lig_set_span = sub_span.subspan(lig_set_offset);
                    SpanReader set_r(lig_set_span);
                    uint16_t lig_count = set_r.read_u16();
                    bool substituted = false;

                    for (uint16_t k = 0; k < lig_count; ++k) {
                        if (!set_r.can_read(2)) break;
                        uint16_t lig_offset = set_r.read_u16();
                        if (lig_offset >= lig_set_span.size()) continue;

                        SpanReader lig_r(lig_set_span.subspan(lig_offset));
                        uint16_t lig_glyph = lig_r.read_u16();
                        uint16_t comp_count = lig_r.read_u16();

                        if (comp_count < 2 || (i + comp_count) > glyphs.size()) {
                            continue;
                        }

                        bool match = true;
                        for (uint16_t c = 1; c < comp_count; ++c) {
                            uint16_t expected_comp = lig_r.read_u16();
                            if (glyphs[i + c] != expected_comp) {
                                match = false;
                                break;
                            }
                        }

                        if (match) {
                            glyphs[i] = lig_glyph;
                            glyphs.erase(glyphs.begin() + static_cast<ptrdiff_t>(i + 1),
                                         glyphs.begin() + static_cast<ptrdiff_t>(i + comp_count));
                            substituted = true;
                            break;
                        }
                    }

                    if (!substituted) {
                        ++i;
                    }
                }
            }
        }
    }
};

OpenTypeGsub::OpenTypeGsub() = default;
OpenTypeGsub::~OpenTypeGsub() = default;
OpenTypeGsub::OpenTypeGsub(OpenTypeGsub&&) noexcept = default;
OpenTypeGsub& OpenTypeGsub::operator=(OpenTypeGsub&&) noexcept = default;

std::unique_ptr<OpenTypeGsub> OpenTypeGsub::from_bytes(std::span<const uint8_t> data) {
    auto gsub = std::make_unique<OpenTypeGsub>();
    gsub->impl_ = std::make_unique<Impl>();
    if (gsub->impl_->parse(data)) {
        gsub->is_valid_ = true;
        return gsub;
    }
    return nullptr;
}

bool OpenTypeGsub::apply_features(std::vector<uint16_t>& glyphs, std::span<const uint32_t> feature_tags) const {
    if (!is_valid_ || !impl_ || glyphs.empty()) return false;
    auto lookups = impl_->get_lookups_for_features(feature_tags);
    if (lookups.empty()) return false;
    for (uint16_t l_idx : lookups) {
        impl_->apply_lookup(l_idx, glyphs);
    }
    return true;
}

bool OpenTypeGsub::apply_ligatures(std::vector<uint16_t>& glyphs) const {
    const uint32_t standard_features[] = {
        make_tag('r', 'l', 'i', 'g'),
        make_tag('c', 'a', 'l', 't'),
        make_tag('l', 'i', 'g', 'a'),
        make_tag('c', 'l', 'i', 'g')
    };
    return apply_features(glyphs, standard_features);
}

// ============================================================================
// OpenType GPOS Implementation
// ============================================================================

struct OpenTypeGpos::Impl {
    std::span<const uint8_t> data;
    uint16_t script_list_offset{0};
    uint16_t feature_list_offset{0};
    uint16_t lookup_list_offset{0};

    bool parse(std::span<const uint8_t> d) {
        data = d;
        if (data.size() < 10) return false;
        SpanReader r(data);
        uint16_t major = r.read_u16();
        if (major != 1) return false;
        r.read_u16(); // minor
        script_list_offset = r.read_u16();
        feature_list_offset = r.read_u16();
        lookup_list_offset = r.read_u16();
        return (feature_list_offset < data.size() && lookup_list_offset < data.size());
    }

    std::vector<uint16_t> get_lookups_for_feature(uint32_t target_tag) const {
        std::vector<uint16_t> lookup_indices;
        if (feature_list_offset >= data.size()) return lookup_indices;

        SpanReader fr(data.subspan(feature_list_offset));
        uint16_t feature_count = fr.read_u16();

        for (uint16_t i = 0; i < feature_count; ++i) {
            if (!fr.can_read(6)) break;
            uint32_t tag = fr.read_u32();
            uint16_t feat_offset = fr.read_u16();

            if (tag == target_tag && feature_list_offset + feat_offset < data.size()) {
                SpanReader tr(data.subspan(feature_list_offset + feat_offset));
                tr.skip(2); // featureParams
                uint16_t count = tr.read_u16();
                for (uint16_t k = 0; k < count; ++k) {
                    if (!tr.can_read(2)) break;
                    uint16_t l_idx = tr.read_u16();
                    if (std::find(lookup_indices.begin(), lookup_indices.end(), l_idx) == lookup_indices.end()) {
                        lookup_indices.push_back(l_idx);
                    }
                }
            }
        }
        return lookup_indices;
    }

    bool get_pair_adjustment_in_lookup(
        uint16_t lookup_idx,
        uint16_t left_glyph,
        uint16_t right_glyph,
        GlyphPlacementAdjustment& out_first,
        GlyphPlacementAdjustment& out_second
    ) const {
        if (lookup_list_offset >= data.size()) return false;
        SpanReader lr(data.subspan(lookup_list_offset));
        uint16_t total_lookups = lr.read_u16();
        if (lookup_idx >= total_lookups) return false;

        lr.skip(lookup_idx * 2);
        if (!lr.can_read(2)) return false;
        uint16_t l_offset = lr.read_u16();
        size_t abs_lookup_offset = lookup_list_offset + l_offset;
        if (abs_lookup_offset >= data.size()) return false;

        SpanReader tr(data.subspan(abs_lookup_offset));
        uint16_t lookup_type = tr.read_u16();
        if (lookup_type != 2 && lookup_type != 9) return false;

        tr.skip(2); // lookupFlag
        uint16_t subtable_count = tr.read_u16();

        for (uint16_t s = 0; s < subtable_count; ++s) {
            if (!tr.can_read(2)) break;
            uint16_t sub_offset = tr.read_u16();
            size_t abs_sub_offset = abs_lookup_offset + sub_offset;
            if (abs_sub_offset >= data.size()) continue;

            auto sub_span = data.subspan(abs_sub_offset);
            SpanReader sr(sub_span);
            uint16_t effective_type = lookup_type;
            if (lookup_type == 9) { // ExtensionPos
                uint16_t ext_fmt = sr.read_u16();
                effective_type = sr.read_u16();
                uint32_t ext_offset = sr.read_u32();
                if (ext_fmt != 1 || ext_offset >= sub_span.size()) continue;
                sub_span = sub_span.subspan(ext_offset);
                sr = SpanReader(sub_span);
            }

            if (effective_type != 2) continue; // Must be Pair Adjustment

            uint16_t pos_format = sr.read_u16();
            uint16_t cov_offset = sr.read_u16();
            uint16_t val_format1 = sr.read_u16();
            uint16_t val_format2 = sr.read_u16();

            int32_t cov = read_coverage_index(sub_span, cov_offset, left_glyph);
            if (cov < 0) continue;

            size_t size1 = get_value_record_size(val_format1);
            size_t size2 = get_value_record_size(val_format2);

            if (pos_format == 1) { // PairSet individual glyphs
                uint16_t pair_set_count = sr.read_u16();
                if (static_cast<uint16_t>(cov) >= pair_set_count) continue;

                SpanReader psr(sub_span.subspan(10 + cov * 2));
                uint16_t pair_set_offset = psr.read_u16();
                if (pair_set_offset >= sub_span.size()) continue;

                auto pair_set_span = sub_span.subspan(pair_set_offset);
                SpanReader vr(pair_set_span);
                uint16_t pair_val_count = vr.read_u16();

                for (uint16_t p = 0; p < pair_val_count; ++p) {
                    if (!vr.can_read(2 + size1 + size2)) break;
                    uint16_t second_g = vr.read_u16();
                    if (second_g == right_glyph) {
                        parse_value_record(vr, val_format1, out_first);
                        parse_value_record(vr, val_format2, out_second);
                        return true;
                    }
                    vr.skip(size1 + size2);
                }
            } else if (pos_format == 2) { // Class-based pair adjustment
                uint16_t class_def1_offset = sr.read_u16();
                uint16_t class_def2_offset = sr.read_u16();
                uint16_t class1_count = sr.read_u16();
                uint16_t class2_count = sr.read_u16();

                uint16_t class1 = read_class_def(sub_span, class_def1_offset, left_glyph);
                uint16_t class2 = read_class_def(sub_span, class_def2_offset, right_glyph);

                if (class1 < class1_count && class2 < class2_count) {
                    size_t pair_size = size1 + size2;
                    size_t record_offset = 16 + (static_cast<size_t>(class1) * class2_count + class2) * pair_size;
                    if (record_offset + pair_size <= sub_span.size()) {
                        SpanReader cr(sub_span.subspan(record_offset));
                        parse_value_record(cr, val_format1, out_first);
                        parse_value_record(cr, val_format2, out_second);
                        return true;
                    }
                }
            }
        }
        return false;
    }

    bool get_mark_to_base_in_lookup(
        uint16_t lookup_idx,
        uint16_t base_glyph,
        uint16_t mark_glyph,
        int16_t& out_dx,
        int16_t& out_dy
    ) const {
        if (lookup_list_offset >= data.size()) return false;
        SpanReader lr(data.subspan(lookup_list_offset));
        uint16_t total_lookups = lr.read_u16();
        if (lookup_idx >= total_lookups) return false;

        lr.skip(lookup_idx * 2);
        if (!lr.can_read(2)) return false;
        uint16_t l_offset = lr.read_u16();
        size_t abs_lookup_offset = lookup_list_offset + l_offset;
        if (abs_lookup_offset >= data.size()) return false;

        SpanReader tr(data.subspan(abs_lookup_offset));
        uint16_t lookup_type = tr.read_u16();
        if (lookup_type != 4 && lookup_type != 9) return false; // Must be Mark-to-Base Attachment

        tr.skip(2); // lookupFlag
        uint16_t subtable_count = tr.read_u16();

        for (uint16_t s = 0; s < subtable_count; ++s) {
            if (!tr.can_read(2)) break;
            uint16_t sub_offset = tr.read_u16();
            size_t abs_sub_offset = abs_lookup_offset + sub_offset;
            if (abs_sub_offset >= data.size()) continue;

            auto sub_span = data.subspan(abs_sub_offset);
            SpanReader sr(sub_span);
            uint16_t effective_type = lookup_type;
            if (lookup_type == 9) { // ExtensionPos
                uint16_t ext_fmt = sr.read_u16();
                effective_type = sr.read_u16();
                uint32_t ext_offset = sr.read_u32();
                if (ext_fmt != 1 || ext_offset >= sub_span.size()) continue;
                sub_span = sub_span.subspan(ext_offset);
                sr = SpanReader(sub_span);
            }

            if (effective_type != 4) continue;

            uint16_t pos_format = sr.read_u16();
            if (pos_format != 1) continue;

            uint16_t mark_cov_offset = sr.read_u16();
            uint16_t base_cov_offset = sr.read_u16();
            uint16_t mark_class_count = sr.read_u16();
            uint16_t mark_array_offset = sr.read_u16();
            uint16_t base_array_offset = sr.read_u16();

            int32_t mark_cov = read_coverage_index(sub_span, mark_cov_offset, mark_glyph);
            int32_t base_cov = read_coverage_index(sub_span, base_cov_offset, base_glyph);

            if (mark_cov < 0 || base_cov < 0) continue;

            // Read mark record from MarkArray
            if (mark_array_offset >= sub_span.size()) continue;
            auto mark_array_span = sub_span.subspan(mark_array_offset);
            SpanReader mar(mark_array_span);
            uint16_t mark_count = mar.read_u16();
            if (static_cast<uint16_t>(mark_cov) >= mark_count) continue;

            mar.skip(mark_cov * 4);
            if (!mar.can_read(4)) continue;
            uint16_t mark_class = mar.read_u16();
            uint16_t mark_anchor_offset = mar.read_u16();

            // Read base record from BaseArray
            if (base_array_offset >= sub_span.size()) continue;
            auto base_array_span = sub_span.subspan(base_array_offset);
            SpanReader bar(base_array_span);
            uint16_t base_count = bar.read_u16();
            if (static_cast<uint16_t>(base_cov) >= base_count) continue;

            size_t base_record_size = static_cast<size_t>(mark_class_count) * 2;
            size_t target_base_record = 2 + static_cast<size_t>(base_cov) * base_record_size;
            if (target_base_record + mark_class * 2 + 2 > base_array_span.size()) continue;

            SpanReader b_rec_r(base_array_span.subspan(target_base_record + mark_class * 2));
            uint16_t base_anchor_offset = b_rec_r.read_u16();
            if (base_anchor_offset == 0) continue;

            // Read Mark Anchor coordinates (relative to MarkArray)
            if (mark_anchor_offset >= mark_array_span.size()) continue;
            SpanReader m_anch_r(mark_array_span.subspan(mark_anchor_offset));
            m_anch_r.skip(2); // anchorFormat
            int16_t mark_x = m_anch_r.read_i16();
            int16_t mark_y = m_anch_r.read_i16();

            // Read Base Anchor coordinates (relative to BaseArray)
            if (base_anchor_offset >= base_array_span.size()) continue;
            SpanReader b_anch_r(base_array_span.subspan(base_anchor_offset));
            b_anch_r.skip(2); // anchorFormat
            int16_t base_x = b_anch_r.read_i16();
            int16_t base_y = b_anch_r.read_i16();

            out_dx = static_cast<int16_t>(base_x - mark_x);
            out_dy = static_cast<int16_t>(base_y - mark_y);
            return true;
        }
        return false;
    }
};

OpenTypeGpos::OpenTypeGpos() = default;
OpenTypeGpos::~OpenTypeGpos() = default;
OpenTypeGpos::OpenTypeGpos(OpenTypeGpos&&) noexcept = default;
OpenTypeGpos& OpenTypeGpos::operator=(OpenTypeGpos&&) noexcept = default;

std::unique_ptr<OpenTypeGpos> OpenTypeGpos::from_bytes(std::span<const uint8_t> data) {
    auto gpos = std::make_unique<OpenTypeGpos>();
    gpos->impl_ = std::make_unique<Impl>();
    if (gpos->impl_->parse(data)) {
        gpos->is_valid_ = true;
        return gpos;
    }
    return nullptr;
}

bool OpenTypeGpos::get_pair_adjustment(
    uint16_t left_glyph,
    uint16_t right_glyph,
    GlyphPlacementAdjustment& out_first,
    GlyphPlacementAdjustment& out_second
) const {
    if (!is_valid_ || !impl_) return false;
    auto lookups = impl_->get_lookups_for_feature(make_tag('k', 'e', 'r', 'n'));
    for (uint16_t l_idx : lookups) {
        if (impl_->get_pair_adjustment_in_lookup(l_idx, left_glyph, right_glyph, out_first, out_second)) {
            return true;
        }
    }
    return false;
}

bool OpenTypeGpos::get_kerning(uint16_t left_glyph, uint16_t right_glyph, int16_t& out_x_advance) const {
    GlyphPlacementAdjustment f{}, s{};
    if (get_pair_adjustment(left_glyph, right_glyph, f, s)) {
        out_x_advance = f.x_advance;
        return true;
    }
    return false;
}

bool OpenTypeGpos::get_mark_to_base_offset(
    uint16_t base_glyph,
    uint16_t mark_glyph,
    int16_t& out_dx,
    int16_t& out_dy
) const {
    if (!is_valid_ || !impl_) return false;
    auto lookups = impl_->get_lookups_for_feature(make_tag('m', 'a', 'r', 'k'));
    for (uint16_t l_idx : lookups) {
        if (impl_->get_mark_to_base_in_lookup(l_idx, base_glyph, mark_glyph, out_dx, out_dy)) {
            return true;
        }
    }
    return false;
}

} // namespace nisaba::text
