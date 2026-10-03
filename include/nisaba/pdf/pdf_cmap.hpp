#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>
#include <map>
#include <memory>
#include <span>

namespace nisaba::pdf {

/// Sovereign CMap Parser and Character Code Translator (ISO 32000-1 §9.10)
/// Maps raw PDF character codes / CID values to Unicode UTF-8 strings.
class PdfCMap {
public:
    PdfCMap() = default;

    /// Parse a PostScript-based CMap stream (e.g. from /ToUnicode)
    static std::shared_ptr<PdfCMap> parse(std::span<const uint8_t> data);

    /// Convert a raw PDF character string (1-byte or 2-byte CIDs) to UTF-8
    std::string to_utf8(std::string_view raw) const;

    /// Map a single character or CID code to its UTF-8 representation
    std::string map_code(uint32_t code) const;

    /// Check if a code mapping exists
    bool has_code(uint32_t code) const noexcept;

    /// Returns true if CMap is configured for 2-byte character codes
    bool is_2byte() const noexcept { return is_2byte_; }

    /// Returns true if no mappings were parsed
    bool is_empty() const noexcept { return map_.empty(); }

    /// Number of mapped character entries
    size_t size() const noexcept { return map_.size(); }

    // --- Static Unicode / Hex Helpers ---
    static std::string unicode_to_utf8(uint32_t cp);
    static std::string hex_to_utf8(std::string_view hex);
    static uint32_t parse_hex_u32(std::string_view hex);

private:
    std::map<uint32_t, std::string> map_;
    bool is_2byte_{false};
};

} // namespace nisaba::pdf
