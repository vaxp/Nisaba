#include "nisaba/svg/svg_cache.hpp"
#include "nisaba/canvas/canvas.hpp"

#ifdef NISABA_HAS_GPU
#include "nisaba/gpu/gpu_canvas.hpp"
#endif

#include <algorithm>
#include <cmath>

namespace nisaba::svg {

uint64_t SvgCache::compute_hash(std::string_view s) noexcept {
    // 64-bit FNV-1a
    uint64_t hash = 14695981039346656037ULL;
    for (char c : s) {
        hash ^= static_cast<uint8_t>(c);
        hash *= 1099511628211ULL;
    }
    return hash;
}

uint32_t SvgCache::pack_color(std::optional<Color> c) noexcept {
    if (!c.has_value()) return 0;
    return (static_cast<uint32_t>(c->red() * 255.0f + 0.5f) << 24) |
           (static_cast<uint32_t>(c->green() * 255.0f + 0.5f) << 16) |
           (static_cast<uint32_t>(c->blue() * 255.0f + 0.5f) << 8) |
           static_cast<uint32_t>(c->alpha() * 255.0f + 0.5f);
}

SvgCache::SvgCache(size_t max_entries) noexcept
    : max_entries_(std::max<size_t>(1, max_entries)) {}

void SvgCache::set_max_entries(size_t max_entries) noexcept {
    max_entries_ = std::max<size_t>(1, max_entries);
    while (map_.size() > max_entries_ && !lru_list_.empty()) {
        auto last = lru_list_.end();
        --last;
        map_.erase(*last);
        lru_list_.pop_back();
    }
}

void SvgCache::clear() noexcept {
    map_.clear();
    lru_list_.clear();
    hits_ = 0;
    misses_ = 0;
}

const BakedSvg& SvgCache::get_or_bake(
    std::string_view svg_xml_or_path,
    uint32_t width,
    uint32_t height,
    std::optional<Color> tint
) {
    if (svg_xml_or_path.empty() || width == 0 || height == 0) {
        return empty_fallback_;
    }

    uint64_t hash = compute_hash(svg_xml_or_path);
    uint32_t col_rgba = pack_color(tint);

    SvgCacheKey key{
        .hash = hash,
        .width = width,
        .height = height,
        .tint_rgba = col_rgba
    };

    auto it = map_.find(key);
    if (it != map_.end()) {
        ++hits_;
        lru_list_.splice(lru_list_.begin(), lru_list_, it->second.second);
        return it->second.first;
    }

    // Cache Miss: Parse & Bake
    ++misses_;
    std::optional<SvgDocument> doc;
    if (svg_xml_or_path.find('<') != std::string_view::npos) {
        doc = SvgDocument::parse(svg_xml_or_path);
    } else {
        doc = SvgDocument::from_file(std::string(svg_xml_or_path));
    }

    if (!doc.has_value()) {
        return empty_fallback_;
    }

    BakedSvg baked = BakedSvg::bake(*doc, width, height, tint);
    if (!baked.is_valid()) {
        return empty_fallback_;
    }

    // Evict oldest if full
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

const BakedSvg& SvgCache::get_or_bake(
    const SvgDocument& doc,
    uint32_t width,
    uint32_t height,
    std::optional<Color> tint
) {
    return get_or_bake(doc, reinterpret_cast<uintptr_t>(&doc), width, height, tint);
}

const BakedSvg& SvgCache::get_or_bake(
    const SvgDocument& doc,
    uint64_t doc_id,
    uint32_t width,
    uint32_t height,
    std::optional<Color> tint
) {
    if (width == 0 || height == 0) {
        return empty_fallback_;
    }

    uint32_t col_rgba = pack_color(tint);

    SvgCacheKey key{
        .hash = doc_id,
        .width = width,
        .height = height,
        .tint_rgba = col_rgba
    };

    auto it = map_.find(key);
    if (it != map_.end()) {
        ++hits_;
        lru_list_.splice(lru_list_.begin(), lru_list_, it->second.second);
        return it->second.first;
    }

    // Cache Miss: Bake
    ++misses_;
    BakedSvg baked = BakedSvg::bake(doc, width, height, tint);
    if (!baked.is_valid()) {
        return empty_fallback_;
    }

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

void SvgCache::draw(
    Canvas& canvas,
    std::string_view svg_xml_or_path,
    const Rect& dest_bounds,
    float opacity,
    std::optional<Color> tint
) {
    uint32_t w = static_cast<uint32_t>(std::max(1.0f, std::round(dest_bounds.width())));
    uint32_t h = static_cast<uint32_t>(std::max(1.0f, std::round(dest_bounds.height())));
    const auto& baked = get_or_bake(svg_xml_or_path, w, h, tint);
    if (baked.is_valid()) {
        baked.draw(canvas, dest_bounds, opacity);
    }
}

void SvgCache::draw(
    Canvas& canvas,
    std::string_view svg_xml_or_path,
    float x, float y, float width, float height,
    float opacity,
    std::optional<Color> tint
) {
    auto r = Rect::from_xywh(x, y, width, height);
    if (r) {
        draw(canvas, svg_xml_or_path, *r, opacity, tint);
    }
}

void SvgCache::draw(
    Canvas& canvas,
    const SvgDocument& doc,
    const Rect& dest_bounds,
    float opacity,
    std::optional<Color> tint
) {
    draw(canvas, doc, reinterpret_cast<uintptr_t>(&doc), dest_bounds, opacity, tint);
}

void SvgCache::draw(
    Canvas& canvas,
    const SvgDocument& doc,
    float x, float y, float width, float height,
    float opacity,
    std::optional<Color> tint
) {
    auto r = Rect::from_xywh(x, y, width, height);
    if (r) {
        draw(canvas, doc, reinterpret_cast<uintptr_t>(&doc), *r, opacity, tint);
    }
}

void SvgCache::draw(
    Canvas& canvas,
    const SvgDocument& doc,
    uint64_t doc_id,
    const Rect& dest_bounds,
    float opacity,
    std::optional<Color> tint
) {
    uint32_t w = static_cast<uint32_t>(std::max(1.0f, std::round(dest_bounds.width())));
    uint32_t h = static_cast<uint32_t>(std::max(1.0f, std::round(dest_bounds.height())));
    const auto& baked = get_or_bake(doc, doc_id, w, h, tint);
    if (baked.is_valid()) {
        baked.draw(canvas, dest_bounds, opacity);
    }
}

#ifdef NISABA_HAS_GPU
void SvgCache::draw(
    gpu::GpuCanvas& canvas,
    std::string_view svg_xml_or_path,
    const Rect& dest_bounds,
    float opacity,
    std::optional<Color> tint
) {
    uint32_t w = static_cast<uint32_t>(std::max(1.0f, std::round(dest_bounds.width())));
    uint32_t h = static_cast<uint32_t>(std::max(1.0f, std::round(dest_bounds.height())));
    const auto& baked = get_or_bake(svg_xml_or_path, w, h, tint);
    if (baked.is_valid()) {
        PixmapPaint paint;
        paint.opacity = opacity;
        canvas.draw_pixmap(
            static_cast<int32_t>(std::round(dest_bounds.left())),
            static_cast<int32_t>(std::round(dest_bounds.top())),
            baked.pixmap().as_ref(),
            paint
        );
    }
}

void SvgCache::draw(
    gpu::GpuCanvas& canvas,
    std::string_view svg_xml_or_path,
    float x, float y, float width, float height,
    float opacity,
    std::optional<Color> tint
) {
    auto r = Rect::from_xywh(x, y, width, height);
    if (r) {
        draw(canvas, svg_xml_or_path, *r, opacity, tint);
    }
}

void SvgCache::draw(
    gpu::GpuCanvas& canvas,
    const SvgDocument& doc,
    const Rect& dest_bounds,
    float opacity,
    std::optional<Color> tint
) {
    draw(canvas, doc, reinterpret_cast<uintptr_t>(&doc), dest_bounds, opacity, tint);
}

void SvgCache::draw(
    gpu::GpuCanvas& canvas,
    const SvgDocument& doc,
    float x, float y, float width, float height,
    float opacity,
    std::optional<Color> tint
) {
    auto r = Rect::from_xywh(x, y, width, height);
    if (r) {
        draw(canvas, doc, reinterpret_cast<uintptr_t>(&doc), *r, opacity, tint);
    }
}

void SvgCache::draw(
    gpu::GpuCanvas& canvas,
    const SvgDocument& doc,
    uint64_t doc_id,
    const Rect& dest_bounds,
    float opacity,
    std::optional<Color> tint
) {
    uint32_t w = static_cast<uint32_t>(std::max(1.0f, std::round(dest_bounds.width())));
    uint32_t h = static_cast<uint32_t>(std::max(1.0f, std::round(dest_bounds.height())));
    const auto& baked = get_or_bake(doc, doc_id, w, h, tint);
    if (baked.is_valid()) {
        PixmapPaint paint;
        paint.opacity = opacity;
        canvas.draw_pixmap(
            static_cast<int32_t>(std::round(dest_bounds.left())),
            static_cast<int32_t>(std::round(dest_bounds.top())),
            baked.pixmap().as_ref(),
            paint
        );
    }
}
#endif

} // namespace nisaba::svg
