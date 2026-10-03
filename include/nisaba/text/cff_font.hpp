#pragma once

#include <cstdint>
#include <span>
#include <vector>
#include <memory>
#include <optional>
#include "nisaba/path/path.hpp"

namespace nisaba {
class PathBuilder;
}

namespace nisaba::text {

/// Sovereign Compact Font Format (CFF / CFF2) parser and Type 2 Charstring decompiler.
/// Extracts PostScript cubic Bézier outlines (.otf) with zero external dependencies.
class CffFont {
public:
    CffFont();
    ~CffFont();

    CffFont(const CffFont&) = delete;
    CffFont& operator=(const CffFont&) = delete;
    CffFont(CffFont&&) noexcept;
    CffFont& operator=(CffFont&&) noexcept;

    /// Parses a CFF or CFF2 table from memory.
    static std::unique_ptr<CffFont> from_bytes(std::span<const uint8_t> data, uint16_t expected_glyphs = 0);

    [[nodiscard]] bool is_valid() const noexcept { return is_valid_; }
    [[nodiscard]] uint16_t num_glyphs() const noexcept { return num_glyphs_; }
    [[nodiscard]] bool is_cff2() const noexcept { return is_cff2_; }

    /// Extracts the vector outline for a glyph, scaled to the target font scale factor.
    /// Inverts the PostScript Y-axis so positive Y points downwards (screen coordinate system).
    bool get_glyph_path(uint16_t glyph_id, Path& out_path, float scale) const;

private:
    struct CffIndex {
        uint32_t count{0};
        uint8_t off_size{0};
        std::vector<uint32_t> offsets{};
        std::span<const uint8_t> data{};

        [[nodiscard]] std::span<const uint8_t> get(size_t index) const noexcept {
            if (index >= count || index + 1 >= offsets.size()) return {};
            uint32_t start = offsets[index];
            uint32_t end = offsets[index + 1];
            if (start > end || start == 0 || (start - 1) > data.size() || (end - 1) > data.size()) {
                return {};
            }
            return data.subspan(start - 1, end - start);
        }
    };

    bool parse(std::span<const uint8_t> data, uint16_t expected_glyphs);
    static CffIndex read_index(std::span<const uint8_t> data, size_t& offset, bool is_cff2);

    bool execute_charstring(
        std::span<const uint8_t> charstring,
        Path& out_path,
        float scale
    ) const;

    bool execute_charstring_internal(
        std::span<const uint8_t> charstring,
        PathBuilder& builder,
        float& cur_x,
        float& cur_y,
        bool& in_subpath,
        size_t& num_stems,
        bool& width_checked,
        std::vector<float>& stack,
        int recursion_depth
    ) const;

    std::span<const uint8_t> data_{};
    bool is_valid_{false};
    bool is_cff2_{false};
    uint16_t num_glyphs_{0};

    // Subroutine biases
    int32_t global_subrs_bias_{0};
    int32_t local_subrs_bias_{0};

    // CFF Index structures
    CffIndex charstrings_index_{};
    CffIndex global_subrs_{};
    CffIndex local_subrs_{};
};

} // namespace nisaba::text
