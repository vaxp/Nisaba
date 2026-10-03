#pragma once

#include <string>
#include <string_view>
#include <unordered_map>
#include <list>
#include <optional>
#include <cstdint>
#include <cstddef>
#include "nisaba/svg/baked_svg.hpp"
#include "nisaba/svg/svg_document.hpp"

namespace nisaba {
    class Canvas;
    namespace gpu {
        class GpuCanvas;
    }
}

namespace nisaba::svg {

/// @brief Cache key for looking up baked SVG elements.
struct SvgCacheKey {
    uint64_t hash{0};           ///< 64-bit hash of SVG XML or file path or document ID
    uint32_t width{0};          ///< Target raster width in pixels
    uint32_t height{0};         ///< Target raster height in pixels
    uint32_t tint_rgba{0};      ///< Packed RGBA tint color (0 if no tint)

    bool operator==(const SvgCacheKey& other) const noexcept = default;
};

struct SvgCacheKeyHasher {
    size_t operator()(const SvgCacheKey& k) const noexcept {
        size_t h = static_cast<size_t>(k.hash);
        h ^= static_cast<size_t>(k.width) + 0x9e3779b9 + (h << 6) + (h >> 2);
        h ^= static_cast<size_t>(k.height) + 0x9e3779b9 + (h << 6) + (h >> 2);
        h ^= static_cast<size_t>(k.tint_rgba) + 0x9e3779b9 + (h << 6) + (h >> 2);
        return h;
    }
};

/// @brief LRU-driven high performance cache for vector SVG documents and icons.
/// Eliminates repeated bezier parsing, tessellation and rasterization by baking to O(1) SIMD blits.
class SvgCache {
public:
    explicit SvgCache(size_t max_entries = 128) noexcept;

    /// @brief Retrieve or bake an SVG from a raw XML string or file path.
    const BakedSvg& get_or_bake(
        std::string_view svg_xml_or_path,
        uint32_t width,
        uint32_t height,
        std::optional<Color> tint = std::nullopt
    );

    /// @brief Retrieve or bake a pre-parsed SvgDocument using its memory address as identifier.
    const BakedSvg& get_or_bake(
        const SvgDocument& doc,
        uint32_t width,
        uint32_t height,
        std::optional<Color> tint = std::nullopt
    );

    /// @brief Retrieve or bake a pre-parsed SvgDocument using a unique document identifier.
    const BakedSvg& get_or_bake(
        const SvgDocument& doc,
        uint64_t doc_id,
        uint32_t width,
        uint32_t height,
        std::optional<Color> tint = std::nullopt
    );

    /// @brief Draw cached SVG to a software CPU Canvas with O(1) SIMD blit.
    void draw(
        Canvas& canvas,
        std::string_view svg_xml_or_path,
        const Rect& dest_bounds,
        float opacity = 1.0f,
        std::optional<Color> tint = std::nullopt
    );

    /// @brief Draw cached SVG using explicit coordinates.
    void draw(
        Canvas& canvas,
        std::string_view svg_xml_or_path,
        float x, float y, float width, float height,
        float opacity = 1.0f,
        std::optional<Color> tint = std::nullopt
    );

    /// @brief Draw pre-parsed cached SVG to a software CPU Canvas.
    void draw(
        Canvas& canvas,
        const SvgDocument& doc,
        const Rect& dest_bounds,
        float opacity = 1.0f,
        std::optional<Color> tint = std::nullopt
    );

    /// @brief Draw pre-parsed cached SVG using explicit coordinates.
    void draw(
        Canvas& canvas,
        const SvgDocument& doc,
        float x, float y, float width, float height,
        float opacity = 1.0f,
        std::optional<Color> tint = std::nullopt
    );

    /// @brief Draw pre-parsed cached SVG to a software CPU Canvas with explicit doc_id.
    void draw(
        Canvas& canvas,
        const SvgDocument& doc,
        uint64_t doc_id,
        const Rect& dest_bounds,
        float opacity = 1.0f,
        std::optional<Color> tint = std::nullopt
    );

#ifdef NISABA_HAS_GPU
    /// @brief Draw cached SVG to a GpuCanvas via texture/pixmap pattern.
    void draw(
        gpu::GpuCanvas& canvas,
        std::string_view svg_xml_or_path,
        const Rect& dest_bounds,
        float opacity = 1.0f,
        std::optional<Color> tint = std::nullopt
    );

    /// @brief Draw cached SVG to a GpuCanvas using explicit coordinates.
    void draw(
        gpu::GpuCanvas& canvas,
        std::string_view svg_xml_or_path,
        float x, float y, float width, float height,
        float opacity = 1.0f,
        std::optional<Color> tint = std::nullopt
    );

    /// @brief Draw pre-parsed cached SVG to a GpuCanvas.
    void draw(
        gpu::GpuCanvas& canvas,
        const SvgDocument& doc,
        const Rect& dest_bounds,
        float opacity = 1.0f,
        std::optional<Color> tint = std::nullopt
    );

    /// @brief Draw pre-parsed cached SVG to a GpuCanvas using explicit coordinates.
    void draw(
        gpu::GpuCanvas& canvas,
        const SvgDocument& doc,
        float x, float y, float width, float height,
        float opacity = 1.0f,
        std::optional<Color> tint = std::nullopt
    );

    /// @brief Draw pre-parsed cached SVG to a GpuCanvas with explicit doc_id.
    void draw(
        gpu::GpuCanvas& canvas,
        const SvgDocument& doc,
        uint64_t doc_id,
        const Rect& dest_bounds,
        float opacity = 1.0f,
        std::optional<Color> tint = std::nullopt
    );
#endif

    /// @brief Cache hit statistics
    [[nodiscard]] size_t hits() const noexcept { return hits_; }
    [[nodiscard]] size_t misses() const noexcept { return misses_; }
    [[nodiscard]] float hit_ratio() const noexcept {
        size_t total = hits_ + misses_;
        return total > 0 ? static_cast<float>(hits_) / static_cast<float>(total) : 0.0f;
    }
    [[nodiscard]] size_t size() const noexcept { return map_.size(); }
    [[nodiscard]] size_t entries_count() const noexcept { return map_.size(); }
    [[nodiscard]] size_t max_entries() const noexcept { return max_entries_; }
    [[nodiscard]] size_t capacity() const noexcept { return max_entries_; }

    void set_max_entries(size_t max_entries) noexcept;
    void clear() noexcept;

    /// @brief Fast 64-bit FNV-1a hash utility
    static uint64_t compute_hash(std::string_view s) noexcept;
    static uint32_t pack_color(std::optional<Color> c) noexcept;

private:
    size_t max_entries_{128};
    size_t hits_{0};
    size_t misses_{0};

    using LRUList = std::list<SvgCacheKey>;
    LRUList lru_list_;
    std::unordered_map<SvgCacheKey, std::pair<BakedSvg, LRUList::iterator>, SvgCacheKeyHasher> map_;
    BakedSvg empty_fallback_{};
};

} // namespace nisaba::svg
