#pragma once

#include <cstdint>
#include <span>
#include <vector>
#include <memory>
#include <string_view>
#include <optional>

namespace nisaba::text {

/// Represents 2D glyph positioning adjustments according to OpenType GPOS ValueRecord.
struct GlyphPlacementAdjustment {
    int16_t x_placement{0};
    int16_t y_placement{0};
    int16_t x_advance{0};
    int16_t y_advance{0};
};

/// Sovereign OpenType GSUB (Glyph Substitution Table) parser and executor.
/// Supports Single Substitution (LookupType 1) and Ligature Substitution (LookupType 4)
/// with zero external dependencies.
class OpenTypeGsub {
public:
    OpenTypeGsub();
    ~OpenTypeGsub();

    OpenTypeGsub(const OpenTypeGsub&) = delete;
    OpenTypeGsub& operator=(const OpenTypeGsub&) = delete;
    OpenTypeGsub(OpenTypeGsub&&) noexcept;
    OpenTypeGsub& operator=(OpenTypeGsub&&) noexcept;

    static std::unique_ptr<OpenTypeGsub> from_bytes(std::span<const uint8_t> data);

    [[nodiscard]] bool is_valid() const noexcept { return is_valid_; }

    /// Applies requested substitution features (e.g., 'liga', 'clig', 'dlig', 'rlig')
    /// on a sequence of glyph IDs in-place.
    bool apply_features(std::vector<uint16_t>& glyphs, std::span<const uint32_t> feature_tags) const;

    /// Convenience wrapper applying standard ligature features ('liga', 'clig').
    bool apply_ligatures(std::vector<uint16_t>& glyphs) const;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
    bool is_valid_{false};
};

/// Sovereign OpenType GPOS (Glyph Positioning Table) parser and executor.
/// Supports Pair Adjustment (LookupType 2 Formats 1 & 2) and Mark-to-Base (LookupType 4)
/// with zero external dependencies.
class OpenTypeGpos {
public:
    OpenTypeGpos();
    ~OpenTypeGpos();

    OpenTypeGpos(const OpenTypeGpos&) = delete;
    OpenTypeGpos& operator=(const OpenTypeGpos&) = delete;
    OpenTypeGpos(OpenTypeGpos&&) noexcept;
    OpenTypeGpos& operator=(OpenTypeGpos&&) noexcept;

    static std::unique_ptr<OpenTypeGpos> from_bytes(std::span<const uint8_t> data);

    [[nodiscard]] bool is_valid() const noexcept { return is_valid_; }

    /// Retrieves horizontal kerning advance adjustment in font design units between two glyphs.
    /// Returns true if a kerning pair adjustment was found in GPOS.
    bool get_kerning(uint16_t left_glyph, uint16_t right_glyph, int16_t& out_x_advance) const;

    /// Retrieves full pair positioning adjustments for both glyphs.
    bool get_pair_adjustment(
        uint16_t left_glyph,
        uint16_t right_glyph,
        GlyphPlacementAdjustment& out_first,
        GlyphPlacementAdjustment& out_second
    ) const;

    /// Retrieves mark-to-base anchor displacement in font design units.
    /// Returns true if a mark attachment anchor pair was found.
    bool get_mark_to_base_offset(
        uint16_t base_glyph,
        uint16_t mark_glyph,
        int16_t& out_dx,
        int16_t& out_dy
    ) const;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
    bool is_valid_{false};
};

} // namespace nisaba::text
