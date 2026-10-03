#pragma once

/// @file text_cache.hpp
/// @brief Transparent LRU Text Surface Cache (TextCache) for Nisaba.
///
/// Automatically caches rasterized BakedText instances, reducing repeated
/// per-frame text rendering to an ultra-fast O(1) SIMD block blit.

#include <string>
#include <string_view>
#include <unordered_map>
#include <list>
#include <memory>
#include <cstdint>
#include <cstring>

#include "nisaba/text/baked_text.hpp"
#include "nisaba/text/font_system.hpp"
#include "nisaba/text/glyph_cache.hpp"

namespace nisaba {
class Canvas;
}

namespace nisaba::text {

struct TextCacheKey {
    std::string text;
    uint32_t font_size_bits{0};
    uint32_t color_rgba{0};
    uint16_t weight{400};
    bool monospace{false};

    bool operator==(const TextCacheKey& other) const noexcept {
        return font_size_bits == other.font_size_bits &&
               color_rgba == other.color_rgba &&
               weight == other.weight &&
               monospace == other.monospace &&
               text == other.text;
    }
};

struct TextCacheKeyHash {
    size_t operator()(const TextCacheKey& k) const noexcept {
        uint64_t hash = 14695981039346656037ULL;
        for (char c : k.text) {
            hash ^= static_cast<uint8_t>(c);
            hash *= 1099511628211ULL;
        }
        hash ^= (static_cast<uint64_t>(k.font_size_bits) + 0x9e3779b97f4a7c15ULL + (hash << 6) + (hash >> 2));
        hash ^= (static_cast<uint64_t>(k.color_rgba) + 0x9e3779b97f4a7c15ULL + (hash << 6) + (hash >> 2));
        hash ^= (static_cast<uint64_t>(k.weight) << 1) ^ (k.monospace ? 1 : 0);
        return static_cast<size_t>(hash);
    }
};

class TextCache {
public:
    explicit TextCache(size_t max_entries = 512) noexcept;
    ~TextCache() = default;

    TextCache(const TextCache&) = delete;
    TextCache& operator=(const TextCache&) = delete;
    TextCache(TextCache&&) noexcept = default;
    TextCache& operator=(TextCache&&) noexcept = default;

    /// Retrieves pre-rendered BakedText from cache or bakes it on miss.
    const BakedText& get_or_bake(
        FontSystem& font_system,
        GlyphCache& glyph_cache,
        std::string_view text,
        float font_size,
        Color color = Color::WHITE,
        Weight weight = Weight::Normal,
        bool monospace = false
    );

    /// High-level cached draw: gets or bakes text and draws it onto Canvas instantly.
    void draw(
        Canvas& canvas,
        FontSystem& font_system,
        GlyphCache& glyph_cache,
        std::string_view text,
        float x,
        float y,
        float font_size,
        Color color = Color::WHITE,
        Weight weight = Weight::Normal,
        bool monospace = false,
        float opacity = 1.0f
    );

    /// Clears all cached surfaces and resets counters.
    void clear() noexcept;

    /// Cache telemetry and diagnostics
    [[nodiscard]] size_t size() const noexcept { return map_.size(); }
    [[nodiscard]] size_t max_entries() const noexcept { return max_entries_; }
    [[nodiscard]] size_t hits() const noexcept { return hits_; }
    [[nodiscard]] size_t misses() const noexcept { return misses_; }
    [[nodiscard]] double hit_ratio() const noexcept {
        size_t total = hits_ + misses_;
        return total > 0 ? (static_cast<double>(hits_) / static_cast<double>(total)) : 0.0;
    }

private:
    size_t max_entries_{512};
    size_t hits_{0};
    size_t misses_{0};

    using ListType = std::list<TextCacheKey>;
    ListType lru_list_;
    std::unordered_map<TextCacheKey, std::pair<BakedText, ListType::iterator>, TextCacheKeyHash> map_;
    BakedText empty_fallback_{};
};

} // namespace nisaba::text
