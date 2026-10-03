#pragma once

#include <cstdint>
#include <vector>
#include <cmath>
#include <algorithm>
#include <bit>
#include "nisaba/math/rect.hpp"
#include "nisaba/math/screen_int_rect.hpp"
#include "nisaba/damage/types.hpp"

namespace nisaba::damage {

/// @brief High-performance spatial 2D micro-tile bitmask grid.
/// Decomposes surface geometry into discrete Micro-Tiles (default 16x16 pixels)
/// where each tile's dirty state is represented by a single bit in a 64-bit word.
template <size_t TileSize = 16>
class MicroTileGrid {
    static_assert((TileSize & (TileSize - 1)) == 0, "TileSize must be a power of two (e.g. 8, 16, 32)");

public:
    static constexpr size_t TILE_SIZE = TileSize;
    static constexpr size_t TILE_SHIFT = std::countr_zero(TileSize);
    static constexpr size_t TILE_MASK = TileSize - 1;

    MicroTileGrid() = default;

    MicroTileGrid(uint32_t surface_width, uint32_t surface_height) {
        init(surface_width, surface_height);
    }

    void init(uint32_t surface_width, uint32_t surface_height) {
        width_ = surface_width;
        height_ = surface_height;
        tiles_x_ = (width_ + TILE_SIZE - 1) >> TILE_SHIFT;
        tiles_y_ = (height_ + TILE_SIZE - 1) >> TILE_SHIFT;
        total_tiles_ = tiles_x_ * tiles_y_;

        size_t word_count = (total_tiles_ + 63) / 64;
        words_.assign(word_count, 0);
    }

    /// Mark a floating-point bounding rectangle as dirty across intersecting micro-tiles.
    void mark_dirty(const Rect& rect) noexcept {
        if (words_.empty() || rect.is_empty()) return;

        int32_t rx0 = std::max(0, static_cast<int32_t>(std::floor(rect.x())));
        int32_t ry0 = std::max(0, static_cast<int32_t>(std::floor(rect.y())));
        int32_t rx1 = std::min(static_cast<int32_t>(width_), static_cast<int32_t>(std::ceil(rect.x() + rect.width())));
        int32_t ry1 = std::min(static_cast<int32_t>(height_), static_cast<int32_t>(std::ceil(rect.y() + rect.height())));

        if (rx1 <= rx0 || ry1 <= ry0) return;

        size_t tx0 = static_cast<size_t>(rx0) >> TILE_SHIFT;
        size_t ty0 = static_cast<size_t>(ry0) >> TILE_SHIFT;
        size_t tx1 = static_cast<size_t>(rx1 + TILE_MASK) >> TILE_SHIFT;
        size_t ty1 = static_cast<size_t>(ry1 + TILE_MASK) >> TILE_SHIFT;

        tx1 = std::min(tx1, tiles_x_);
        ty1 = std::min(ty1, tiles_y_);

        for (size_t ty = ty0; ty < ty1; ++ty) {
            size_t row_start_tile = ty * tiles_x_;
            for (size_t tx = tx0; tx < tx1; ++tx) {
                size_t tile_idx = row_start_tile + tx;
                size_t word_idx = tile_idx >> 6;
                size_t bit_idx = tile_idx & 63;
                words_[word_idx] |= (1ULL << bit_idx);
            }
        }
    }

    /// Mark an integer pixel rectangle as dirty.
    void mark_dirty(const ScreenIntRect& rect) noexcept {
        if (words_.empty() || rect.width() == 0 || rect.height() == 0) return;

        size_t rx0 = rect.x();
        size_t ry0 = rect.y();
        size_t rx1 = std::min(static_cast<size_t>(width_), static_cast<size_t>(rect.x() + rect.width()));
        size_t ry1 = std::min(static_cast<size_t>(height_), static_cast<size_t>(rect.y() + rect.height()));

        if (rx1 <= rx0 || ry1 <= ry0) return;

        size_t tx0 = rx0 >> TILE_SHIFT;
        size_t ty0 = ry0 >> TILE_SHIFT;
        size_t tx1 = (rx1 + TILE_MASK) >> TILE_SHIFT;
        size_t ty1 = (ry1 + TILE_MASK) >> TILE_SHIFT;

        tx1 = std::min(tx1, tiles_x_);
        ty1 = std::min(ty1, tiles_y_);

        for (size_t ty = ty0; ty < ty1; ++ty) {
            size_t row_start_tile = ty * tiles_x_;
            for (size_t tx = tx0; tx < tx1; ++tx) {
                size_t tile_idx = row_start_tile + tx;
                size_t word_idx = tile_idx >> 6;
                size_t bit_idx = tile_idx & 63;
                words_[word_idx] |= (1ULL << bit_idx);
            }
        }
    }

    /// Query if a specific micro-tile is dirty.
    [[nodiscard]] bool is_tile_dirty(size_t tx, size_t ty) const noexcept {
        if (tx >= tiles_x_ || ty >= tiles_y_) return false;
        size_t idx = ty * tiles_x_ + tx;
        return (words_[idx >> 6] & (1ULL << (idx & 63))) != 0;
    }

    /// Check if any tile in the entire grid is dirty.
    [[nodiscard]] bool has_damage() const noexcept {
        for (uint64_t w : words_) {
            if (w != 0) return true;
        }
        return false;
    }

    /// Total number of damaged micro-tiles.
    [[nodiscard]] size_t dirty_tile_count() const noexcept {
        size_t count = 0;
        for (uint64_t w : words_) {
            count += std::popcount(w);
        }
        return count;
    }

    /// Clears all dirty bits in the grid.
    void clear() noexcept {
        std::fill(words_.begin(), words_.end(), 0ULL);
    }

    [[nodiscard]] uint32_t width() const noexcept { return width_; }
    [[nodiscard]] uint32_t height() const noexcept { return height_; }
    [[nodiscard]] size_t tiles_x() const noexcept { return tiles_x_; }
    [[nodiscard]] size_t tiles_y() const noexcept { return tiles_y_; }
    [[nodiscard]] size_t total_tiles() const noexcept { return total_tiles_; }
    [[nodiscard]] const std::vector<uint64_t>& words() const noexcept { return words_; }

private:
    uint32_t width_{0};
    uint32_t height_{0};
    size_t tiles_x_{0};
    size_t tiles_y_{0};
    size_t total_tiles_{0};
    std::vector<uint64_t> words_;
};

} // namespace nisaba::damage
