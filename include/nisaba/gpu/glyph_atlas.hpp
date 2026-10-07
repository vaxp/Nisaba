#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>
#include <optional>
#include <algorithm>
#include <cstring>
#include "nisaba/gpu/renderer.hpp"
#include "nisaba/text/glyph_cache.hpp"

namespace nisaba::gpu {

struct AtlasGlyphEntry {
    float u0{0.0f};
    float v0{0.0f};
    float u1{0.0f};
    float v1{0.0f};
    int offsetX{0};
    int offsetY{0};
    int width{0};
    int height{0};
};

/// High-performance dynamic GPU texture atlas for rasterized glyph masks.
/// Uses a Skyline Bin Packer to pack 8-bit alpha glyphs with 1-pixel padding to prevent texture bleeding.
class SovereignGlyphAtlas {
public:
    explicit SovereignGlyphAtlas(int width = 1024, int height = 1024);
    ~SovereignGlyphAtlas() = default;

    SovereignGlyphAtlas(const SovereignGlyphAtlas&) = delete;
    SovereignGlyphAtlas& operator=(const SovereignGlyphAtlas&) = delete;

    int width() const noexcept { return m_width; }
    int height() const noexcept { return m_height; }
    const uint8_t* data() const noexcept { return m_data.data(); }

    /// Clears the atlas allocations and reset the skyline.
    void reset();

    /// Looks up an existing entry in the atlas.
    const AtlasGlyphEntry* find(const text::CacheKey& key) const noexcept;

    /// Packs and stores a rasterized glyph mask into the atlas.
    /// Returns the allocated AtlasGlyphEntry, or std::nullopt if the atlas is full.
    std::optional<AtlasGlyphEntry> insert(const text::CacheKey& key, const text::CachedGlyph* glyph);

    /// Resizes the atlas buffer and resets allocations.
    bool resize(int newWidth, int newHeight);

    /// Checks if any new glyphs were inserted since last flush.
    bool isDirty() const noexcept { return m_dirty; }

    /// Uploads all dirty regions to the GPU texture via Renderer::updateTexture.
    void flushToGpu(Renderer* renderer, int textureId);

private:
    struct SkylineNode {
        int x{0};
        int y{0};
        int width{0};
    };

    int rectFits(size_t idx, int w, int h) const noexcept;
    void addSkylineLevel(size_t idx, int x, int y, int w, int h);

    int m_width{1024};
    int m_height{1024};
    std::vector<uint8_t> m_data;
    std::vector<SkylineNode> m_nodes;
    std::unordered_map<text::CacheKey, AtlasGlyphEntry, text::CacheKeyHash> m_entries;

    bool m_dirty{false};
    int m_dirtyMinX{0};
    int m_dirtyMinY{0};
    int m_dirtyMaxX{0};
    int m_dirtyMaxY{0};
};

/// Alias matching ENKI specification naming.
using GpuGlyphAtlas = SovereignGlyphAtlas;

} // namespace nisaba::gpu
