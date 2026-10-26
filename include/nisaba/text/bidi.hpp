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

/// Contextual joining form for Arabic/cursive scripts.
enum class ArabicForm : uint8_t {
    None = 0,
    Isolated = 1,
    Initial = 2,
    Medial = 3,
    Final = 4
};

/// Represents a contiguous directional run within text.
struct BidiRun {
    size_t start_byte{0};
    size_t end_byte{0};
    uint8_t level{0}; // Even = LTR, Odd = RTL

    bool is_rtl() const noexcept { return (level % 2) != 0; }
};

/// Shaped glyph or codepoint item with positioning and joining context.
struct ShapedChar {
    char32_t codepoint{0};
    size_t byte_index{0};
    uint8_t level{0};
    bool is_rtl{false};
    bool is_mark{false};
    ArabicForm arabic_form{ArabicForm::None};
    char32_t original_codepoint{0};
};

/// Utilities for UTF-8 decoding, Unicode Bidirectional Analysis (UAX #9),
/// Arabic/Persian/Urdu Contextual Shaping, and Grapheme Cluster Segmentation (UAX #29).
class Bidi {
public:
    /// Decodes a UTF-8 string into 32-bit Unicode codepoints with byte offsets.
    static std::vector<std::pair<char32_t, size_t>> decode_utf8(std::string_view text);

    /// Determines character directional classification.
    static CharType classify_char(char32_t c) noexcept;

    /// Checks if a codepoint belongs to the Arabic/Persian/Urdu script blocks.
    static bool is_arabic(char32_t c) noexcept;

    /// Checks if a codepoint is an Arabic diacritic (Tashkeel / Harakah).
    static bool is_arabic_diacritic(char32_t c) noexcept;

    /// Checks if a codepoint is any Unicode combining mark or zero-width grapheme extender.
    static bool is_combining_mark(char32_t c) noexcept;

    /// Returns the mirrored counterpart of a bracket/symbol in RTL context (UAX #9 Bidi_Mirrored).
    static char32_t mirror_char(char32_t c) noexcept;

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

    /// Shapes Arabic/Persian/Urdu text, performing contextual joining (Isolated, Initial, Medial, Final),
    /// mandatory ligatures (e.g. Lam-Alif), Bidi bracket mirroring, and mark clustering.
    static std::vector<ShapedChar> shape_text(std::string_view text, const std::vector<BidiRun>& runs);

    /// Advances to the next Unicode Grapheme Cluster boundary (UAX #29).
    static size_t next_grapheme_cluster(std::string_view text, size_t byte_index) noexcept;

    /// Moves back to the previous Unicode Grapheme Cluster boundary (UAX #29).
    static size_t prev_grapheme_cluster(std::string_view text, size_t byte_index) noexcept;

    /// Advances to the next word boundary.
    static size_t next_word_boundary(std::string_view text, size_t byte_index) noexcept;

    /// Moves back to the previous word boundary.
    static size_t prev_word_boundary(std::string_view text, size_t byte_index) noexcept;
};

} // namespace nisaba::text
