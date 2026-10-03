#pragma once

#include <cstdint>
#include <string_view>
#include <vector>
#include <optional>

namespace nisaba::text {

/// Text direction level (Even = LTR, Odd = RTL).
enum class Direction : uint8_t {
    LeftToRight = 0,
    RightToLeft = 1
};

/// Directional character class.
enum class CharType : uint8_t {
    LTR,
    RTL,
    ArabicLetter,
    EuropeanNumber,
    ArabicNumber,
    Whitespace,
    Neutral
};

/// Represents a contiguous directional run within text.
struct BidiRun {
    size_t start_byte{0};
    size_t end_byte{0};
    uint8_t level{0}; // 0 = LTR, 1 = RTL

    bool is_rtl() const noexcept { return (level % 2) != 0; }
};

/// Shaped glyph or codepoint item with positioning context.
struct ShapedChar {
    char32_t codepoint{0};
    size_t byte_index{0};
    uint8_t level{0};
    bool is_rtl{false};
};

/// Utilities for UTF-8 decoding, Unicode Bidirectional Analysis, and Arabic Shaping.
class Bidi {
public:
    /// Decodes a UTF-8 string into 32-bit Unicode codepoints with byte offsets.
    static std::vector<std::pair<char32_t, size_t>> decode_utf8(std::string_view text);

    /// Determines character directional classification.
    static CharType classify_char(char32_t c) noexcept;

    /// Checks if a codepoint belongs to the Arabic script block.
    static bool is_arabic(char32_t c) noexcept;

    /// Checks if a codepoint is an Arabic diacritic (Tashkeel / Harakah).
    static bool is_arabic_diacritic(char32_t c) noexcept;

    /// Checks if a codepoint is CJK (Chinese, Japanese, Korean) ideograph or syllabary.
    static bool is_cjk(char32_t c) noexcept;

    /// Checks if a CJK character must not appear at the start of a line (Kinsoku Shori closing punctuation).
    static bool is_cjk_no_break_before(char32_t c) noexcept;

    /// Checks if a CJK character must not appear at the end of a line (opening punctuation).
    static bool is_cjk_no_break_after(char32_t c) noexcept;

    /// Detects the paragraph base direction (LTR or RTL) based on the first strong character.
    static Direction detect_base_direction(std::string_view text) noexcept;

    /// Segments text into directional runs according to Unicode BiDi algorithm principles.
    static std::vector<BidiRun> segment_runs(std::string_view text, std::optional<Direction> base_dir = std::nullopt);

    /// Shapes Arabic text, performing contextual joining (Isolated, Initial, Medial, Final)
    /// and mandatory ligatures (e.g. Lam-Alif).
    static std::vector<ShapedChar> shape_text(std::string_view text, const std::vector<BidiRun>& runs);
};

} // namespace nisaba::text
