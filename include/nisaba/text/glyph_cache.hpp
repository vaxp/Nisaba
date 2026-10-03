#pragma once

#include <cstdint>
#include <utility>
#include <vector>
#include <unordered_map>
#include <memory>
#include <cmath>
#include <cstring>
#include "nisaba/canvas/mask.hpp"
#include "nisaba/path/path.hpp"
#include "nisaba/text/ttf_font.hpp"

namespace nisaba::text {

/// Binning of subpixel position for rasterization caching.
enum class SubpixelBin : uint8_t {
    Zero = 0,
    One = 1,
    Two = 2,
    Three = 3
};

inline std::pair<int32_t, SubpixelBin> compute_subpixel_bin(float pos) noexcept {
    int32_t trunc = static_cast<int32_t>(std::floor(pos));
    float fract = pos - static_cast<float>(trunc);
    if (fract < 0.125f) {
        return {trunc, SubpixelBin::Zero};
    } else if (fract < 0.375f) {
        return {trunc, SubpixelBin::One};
    } else if (fract < 0.625f) {
        return {trunc, SubpixelBin::Two};
    } else if (fract < 0.875f) {
        return {trunc, SubpixelBin::Three};
    } else {
        return {trunc + 1, SubpixelBin::Zero};
    }
}

inline float bin_to_float(SubpixelBin bin) noexcept {
    switch (bin) {
        case SubpixelBin::Zero: return 0.0f;
        case SubpixelBin::One: return 0.25f;
        case SubpixelBin::Two: return 0.5f;
        case SubpixelBin::Three: return 0.75f;
    }
    return 0.0f;
}

/// Cache key uniquely identifying a rasterized glyph instance.
struct CacheKey {
    uint32_t font_id{0};
    uint16_t glyph_id{0};
    uint32_t font_size_bits{0};
    SubpixelBin x_bin{SubpixelBin::Zero};
    SubpixelBin y_bin{SubpixelBin::Zero};

    bool operator==(const CacheKey& other) const noexcept {
        return font_id == other.font_id &&
               glyph_id == other.glyph_id &&
               font_size_bits == other.font_size_bits &&
               x_bin == other.x_bin &&
               y_bin == other.y_bin;
    }
};

struct CacheKeyHash {
    size_t operator()(const CacheKey& k) const noexcept {
        size_t h = std::hash<uint32_t>{}(k.font_id);
        h ^= (std::hash<uint32_t>{}(k.font_size_bits) + 0x9e3779b9 + (h << 6) + (h >> 2));
        h ^= (std::hash<uint16_t>{}(k.glyph_id) + 0x9e3779b9 + (h << 6) + (h >> 2));
        h ^= (static_cast<size_t>(k.x_bin) << 2) ^ static_cast<size_t>(k.y_bin);
        return h;
    }
};

/// A cached 8-bit alpha mask for a glyph with positioning offsets.
struct CachedGlyph {
    int32_t offset_x{0};
    int32_t offset_y{0};
    uint32_t width{0};
    uint32_t height{0};
    std::vector<uint8_t> data{};

    MaskRef as_mask_ref() const noexcept {
        auto m = MaskRef::from_bytes(data.data(), data.size(), width, height);
        return m.value_or(MaskRef{});
    }
};

/// High-performance cache for rasterized glyph alpha masks.
class GlyphCache {
public:
    GlyphCache() = default;
    ~GlyphCache() = default;

    /// Retrieves or rasterizes an 8-bit alpha mask for the glyph at (x, y) subpixel position.
    const CachedGlyph* get_or_render(
        const TtfFont& font,
        uint32_t font_id,
        uint16_t glyph_id,
        float font_size,
        float x,
        float y
    );

    void clear() noexcept {
        cache_.clear();
    }

    size_t size() const noexcept {
        return cache_.size();
    }

private:
    std::unordered_map<CacheKey, CachedGlyph, CacheKeyHash> cache_{};
};

} // namespace nisaba::text
