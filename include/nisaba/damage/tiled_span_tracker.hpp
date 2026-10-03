#pragma once

#include <cstdint>
#include <vector>
#include <algorithm>
#include <optional>
#include "nisaba/math/rect.hpp"
#include "nisaba/math/screen_int_rect.hpp"
#include "nisaba/damage/types.hpp"
#include "nisaba/damage/micro_tile_grid.hpp"

namespace nisaba::damage {

/// @brief Coordinates Tiled-Span Hybrid Damage Tracking.
/// Merges 2D Micro-Tile bitmasks with 1D Scanline Spans and multi-buffer age accumulation.
template <size_t TileSize = 16>
class TiledSpanTracker {
public:
    static constexpr size_t TILE_SIZE = TileSize;

    TiledSpanTracker() = default;

    TiledSpanTracker(uint32_t width, uint32_t height)
        : grid_(width, height) {}

    void init(uint32_t width, uint32_t height) {
        grid_.init(width, height);
    }

    /// Mark an invalidated floating-point bounding rectangle.
    void mark_dirty(const Rect& rect) noexcept {
        grid_.mark_dirty(rect);
    }

    /// Mark an invalidated integer screen rectangle.
    void mark_dirty(const ScreenIntRect& rect) noexcept {
        grid_.mark_dirty(rect);
    }

    /// Check if any damage exists in the current frame.
    [[nodiscard]] bool has_damage() const noexcept {
        return grid_.has_damage();
    }

    /// Number of damaged micro-tiles.
    [[nodiscard]] size_t dirty_tile_count() const noexcept {
        return grid_.dirty_tile_count();
    }

    /// Total surface area in pixels covered by damaged micro-tiles.
    [[nodiscard]] uint64_t dirty_pixel_area() const noexcept {
        return static_cast<uint64_t>(grid_.dirty_tile_count()) * (TILE_SIZE * TILE_SIZE);
    }

    /// Computes the single tight bounding rectangle enclosing all dirty tiles.
    [[nodiscard]] std::optional<ScreenIntRect> bounding_damage() const noexcept {
        if (!grid_.has_damage()) return std::nullopt;

        size_t min_tx = grid_.tiles_x();
        size_t max_tx = 0;
        size_t min_ty = grid_.tiles_y();
        size_t max_ty = 0;
        bool found = false;

        for (size_t ty = 0; ty < grid_.tiles_y(); ++ty) {
            for (size_t tx = 0; tx < grid_.tiles_x(); ++tx) {
                if (grid_.is_tile_dirty(tx, ty)) {
                    min_tx = std::min(min_tx, tx);
                    max_tx = std::max(max_tx, tx);
                    min_ty = std::min(min_ty, ty);
                    max_ty = std::max(max_ty, ty);
                    found = true;
                }
            }
        }

        if (!found) return std::nullopt;

        uint32_t x0 = static_cast<uint32_t>(min_tx * TILE_SIZE);
        uint32_t y0 = static_cast<uint32_t>(min_ty * TILE_SIZE);
        uint32_t x1 = std::min(grid_.width(), static_cast<uint32_t>((max_tx + 1) * TILE_SIZE));
        uint32_t y1 = std::min(grid_.height(), static_cast<uint32_t>((max_ty + 1) * TILE_SIZE));

        return ScreenIntRect::from_xywh(x0, y0, x1 - x0, y1 - y0);
    }

    /// Decomposes damaged micro-tiles into contiguous 1D horizontal scanline spans.
    void generate_spans(std::vector<Span>& out_spans) const {
        out_spans.clear();
        if (!grid_.has_damage()) return;

        for (size_t ty = 0; ty < grid_.tiles_y(); ++ty) {
            size_t tx = 0;
            while (tx < grid_.tiles_x()) {
                if (grid_.is_tile_dirty(tx, ty)) {
                    size_t start_tx = tx;
                    while (tx < grid_.tiles_x() && grid_.is_tile_dirty(tx, ty)) {
                        tx++;
                    }
                    size_t end_tx = tx;

                    uint16_t x0 = static_cast<uint16_t>(start_tx * TILE_SIZE);
                    uint16_t x1 = static_cast<uint16_t>(std::min(static_cast<size_t>(grid_.width()), end_tx * TILE_SIZE));

                    uint16_t y_start = static_cast<uint16_t>(ty * TILE_SIZE);
                    uint16_t y_end = static_cast<uint16_t>(std::min(static_cast<size_t>(grid_.height()), (ty + 1) * TILE_SIZE));

                    for (uint16_t y = y_start; y < y_end; ++y) {
                        out_spans.emplace_back(y, x0, x1);
                    }
                } else {
                    tx++;
                }
            }
        }
    }

    /// Coalesces damaged tiles into a compact list of disjoint rectangles for Wayland / DRM.
    void generate_damage_rects(std::vector<ScreenIntRect>& out_rects, size_t max_rects = 16) const {
        out_rects.clear();
        if (!grid_.has_damage()) return;

        // Extract horizontal tile run segments
        struct TileRun {
            size_t ty;
            size_t tx0;
            size_t tx1;
            bool merged = false;
        };

        std::vector<TileRun> runs;
        for (size_t ty = 0; ty < grid_.tiles_y(); ++ty) {
            size_t tx = 0;
            while (tx < grid_.tiles_x()) {
                if (grid_.is_tile_dirty(tx, ty)) {
                    size_t start_tx = tx;
                    while (tx < grid_.tiles_x() && grid_.is_tile_dirty(tx, ty)) {
                        tx++;
                    }
                    runs.push_back({ty, start_tx, tx, false});
                } else {
                    tx++;
                }
            }
        }

        // Merge vertically adjacent runs with identical horizontal extents
        for (size_t i = 0; i < runs.size(); ++i) {
            if (runs[i].merged) continue;

            size_t ty_start = runs[i].ty;
            size_t ty_end = ty_start + 1;
            size_t tx0 = runs[i].tx0;
            size_t tx1 = runs[i].tx1;
            runs[i].merged = true;

            for (size_t j = i + 1; j < runs.size(); ++j) {
                if (runs[j].merged) continue;
                if (runs[j].ty == ty_end && runs[j].tx0 == tx0 && runs[j].tx1 == tx1) {
                    ty_end++;
                    runs[j].merged = true;
                } else if (runs[j].ty > ty_end) {
                    break;
                }
            }

            uint32_t px0 = static_cast<uint32_t>(tx0 * TILE_SIZE);
            uint32_t py0 = static_cast<uint32_t>(ty_start * TILE_SIZE);
            uint32_t px1 = std::min(grid_.width(), static_cast<uint32_t>(tx1 * TILE_SIZE));
            uint32_t py1 = std::min(grid_.height(), static_cast<uint32_t>(ty_end * TILE_SIZE));

            auto r = ScreenIntRect::from_xywh(px0, py0, px1 - px0, py1 - py0);
            if (r) out_rects.push_back(*r);
        }

        // If disjoint rectangles exceed max_rects threshold, consolidate to bounding box
        if (out_rects.size() > max_rects) {
            auto b = bounding_damage();
            out_rects.clear();
            if (b) out_rects.push_back(*b);
        }
    }

    /// Clear all damage for the next frame.
    void clear() noexcept {
        grid_.clear();
    }

    [[nodiscard]] const MicroTileGrid<TileSize>& grid() const noexcept { return grid_; }

private:
    MicroTileGrid<TileSize> grid_;
};

/// @brief Multi-buffer history tracker solving the Double/Triple Buffering Buffer-Age problem.
class BufferAgeTracker {
public:
    explicit BufferAgeTracker(size_t max_history = 4)
        : max_history_(std::max(size_t{2}, max_history)) {}

    /// Records the damage rectangles submitted in the current frame.
    void record_frame_damage(const std::vector<ScreenIntRect>& current_damage) {
        history_.push_back(current_damage);
        if (history_.size() > max_history_) {
            history_.erase(history_.begin());
        }
    }

    /// Computes the accumulated damage needed for a buffer with given age (e.g. age=2 for double-buffer).
    [[nodiscard]] std::vector<ScreenIntRect> get_accumulated_damage(size_t buffer_age) const {
        if (buffer_age == 0 || history_.empty()) {
            return {}; // Full redraw needed if age == 0 (unknown or first frame)
        }

        std::vector<ScreenIntRect> acc;
        size_t frames_to_include = std::min(buffer_age, history_.size());

        for (size_t i = history_.size() - frames_to_include; i < history_.size(); ++i) {
            for (const auto& r : history_[i]) {
                acc.push_back(r);
            }
        }
        return acc;
    }

private:
    size_t max_history_{4};
    std::vector<std::vector<ScreenIntRect>> history_;
};

} // namespace nisaba::damage
