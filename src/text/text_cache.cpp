#include "nisaba/text/text_cache.hpp"
#include "nisaba/canvas/canvas.hpp"

#include <bit>

namespace nisaba::text {

TextCache::TextCache(size_t max_entries) noexcept
    : max_entries_(std::max<size_t>(1, max_entries)) {}

const BakedText& TextCache::get_or_bake(
    FontSystem& font_system,
    GlyphCache& glyph_cache,
    std::string_view text,
    float font_size,
    Color color,
    Weight weight,
    bool monospace
) {
    if (text.empty()) {
        return empty_fallback_;
    }

    uint32_t fs_bits = std::bit_cast<uint32_t>(font_size);
    uint32_t col_rgba = (static_cast<uint32_t>(color.red() * 255.0f + 0.5f) << 24) |
                        (static_cast<uint32_t>(color.green() * 255.0f + 0.5f) << 16) |
                        (static_cast<uint32_t>(color.blue() * 255.0f + 0.5f) << 8) |
                        static_cast<uint32_t>(color.alpha() * 255.0f + 0.5f);

    TextCacheKey key{
        .text = std::string(text),
        .font_size_bits = fs_bits,
        .color_rgba = col_rgba,
        .weight = static_cast<uint16_t>(weight),
        .monospace = monospace
    };

    auto it = map_.find(key);
    if (it != map_.end()) {
        ++hits_;
        // Move to front of LRU list
        lru_list_.splice(lru_list_.begin(), lru_list_, it->second.second);
        return it->second.first;
    }

    // Cache Miss: Bake new surface
    ++misses_;
    BakedText baked = BakedText::bake(
        font_system,
        glyph_cache,
        text,
        font_size,
        color,
        weight,
        monospace
    );

    if (!baked) {
        return empty_fallback_;
    }

    // Evict oldest if capacity exceeded
    if (map_.size() >= max_entries_ && !lru_list_.empty()) {
        auto last = lru_list_.end();
        --last;
        map_.erase(*last);
        lru_list_.pop_back();
    }

    lru_list_.push_front(key);
    auto inserted = map_.emplace(std::move(key), std::make_pair(std::move(baked), lru_list_.begin()));
    return inserted.first->second.first;
}

void TextCache::draw(
    Canvas& canvas,
    FontSystem& font_system,
    GlyphCache& glyph_cache,
    std::string_view text,
    float x,
    float y,
    float font_size,
    Color color,
    Weight weight,
    bool monospace,
    float opacity
) {
    const auto& baked = get_or_bake(font_system, glyph_cache, text, font_size, color, weight, monospace);
    if (baked) {
        canvas.draw_baked_text(baked, x, y, opacity);
    }
}

void TextCache::clear() noexcept {
    map_.clear();
    lru_list_.clear();
    hits_ = 0;
    misses_ = 0;
}

} // namespace nisaba::text
