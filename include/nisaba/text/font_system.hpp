#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>
#include <memory>
#include <optional>
#include <span>
#include "nisaba/text/attrs.hpp"
#include "nisaba/text/ttf_font.hpp"

namespace nisaba::text {

/// Sovereign Font Management & Fallback System.
/// Discovers, caches, matches, and falls back across loaded TrueType fonts.
class FontSystem {
public:
    FontSystem();
    ~FontSystem();

    FontSystem(const FontSystem&) = delete;
    FontSystem& operator=(const FontSystem&) = delete;
    FontSystem(FontSystem&&) noexcept;
    FontSystem& operator=(FontSystem&&) noexcept;

    /// Loads a TrueType font from a file path. Returns unique font ID if successful.
    std::optional<uint32_t> load_font_file(std::string_view path);

    /// Loads a TrueType font from a memory buffer. Returns unique font ID if successful.
    std::optional<uint32_t> load_font_data(std::span<const uint8_t> data);

    /// Retrieves a font by its unique ID.
    const TtfFont* get_font(uint32_t font_id) const noexcept;

    /// Matches the closest font based on Family, Weight, and Style.
    std::pair<uint32_t, const TtfFont*> match_font(
        const Family& family,
        Weight weight = Weight::Normal,
        Style style = Style::Normal
    ) const noexcept;

    /// Searches for a glyph representing the codepoint, starting with the primary font,
    /// and falling back to any other loaded font that contains the glyph.
    std::pair<uint32_t, uint16_t> find_glyph_or_fallback(
        uint32_t primary_font_id,
        char32_t codepoint
    ) const noexcept;

    /// Returns the number of loaded fonts.
    size_t font_count() const noexcept { return fonts_.size(); }

    /// Sets the default font ID to use when no family is specified.
    void set_default_font(uint32_t font_id) noexcept { default_font_id_ = font_id; }
    std::optional<uint32_t> default_font_id() const noexcept { return default_font_id_; }

private:
    struct FontRecord {
        uint32_t id{0};
        std::unique_ptr<TtfFont> font{nullptr};
    };

    std::vector<FontRecord> fonts_{};
    uint32_t next_id_{1};
    std::optional<uint32_t> default_font_id_{std::nullopt};
};

} // namespace nisaba::text
