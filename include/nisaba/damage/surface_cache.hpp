#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <functional>
#include <string>
#include <unordered_map>
#include <algorithm>

#include "nisaba/canvas/pixmap.hpp"
#include "nisaba/canvas/canvas.hpp"
#include "nisaba/math/rect.hpp"
#include "nisaba/math/screen_int_rect.hpp"
#include "nisaba/damage/types.hpp"

namespace nisaba::damage {

/// @brief Retained Surface / Sub-Tree / Page Backing-Store Cache.
/// Provides zero-cost blitting for complex static views, pages, and dialogs.
class SurfaceCache {
public:
    SurfaceCache() = default;

    SurfaceCache(uint32_t width, uint32_t height) {
        init(width, height);
    }

    bool init(uint32_t width, uint32_t height) {
        width_ = width;
        height_ = height;
        dirty_ = true;
        damage_rect_ = std::nullopt;
        auto p = Pixmap::create(width, height);
        if (!p) {
            pixmap_ = std::nullopt;
            return false;
        }
        pixmap_ = std::move(*p);
        return true;
    }

    [[nodiscard]] bool is_valid() const noexcept {
        return !dirty_ && pixmap_.has_value();
    }

    [[nodiscard]] bool is_dirty() const noexcept {
        return dirty_ || !pixmap_.has_value();
    }

    void invalidate() noexcept {
        dirty_ = true;
        if (width_ > 0 && height_ > 0) {
            damage_rect_ = ScreenIntRect::from_xywh(0, 0, width_, height_);
        }
    }

    void invalidate_rect(const Rect& rect) noexcept {
        dirty_ = true;
        auto irect = ScreenIntRect::from_xywh(
            static_cast<uint32_t>(std::max(0.0f, rect.x())),
            static_cast<uint32_t>(std::max(0.0f, rect.y())),
            static_cast<uint32_t>(std::max(1.0f, rect.width() + 0.5f)),
            static_cast<uint32_t>(std::max(1.0f, rect.height() + 0.5f))
        );
        if (!irect) return;

        if (!damage_rect_) {
            damage_rect_ = *irect;
        } else {
            damage_rect_ = unite(*damage_rect_, *irect);
        }
    }

    void invalidate_rect(const ScreenIntRect& rect) noexcept {
        dirty_ = true;
        if (!damage_rect_) {
            damage_rect_ = rect;
        } else {
            damage_rect_ = unite(*damage_rect_, rect);
        }
    }

    /// @brief Renders content using draw_fn ONLY if the surface is dirty.
    /// Returns true if redrawn, false if cache was reused.
    bool render_if_dirty(const std::function<void(Canvas&)>& draw_fn) {
        if (!pixmap_) {
            if (!init(width_, height_)) return false;
        }

        if (!dirty_) {
            return false; // Reused without drawing
        }

        Canvas canvas(*pixmap_);
        draw_fn(canvas);
        dirty_ = false;
        damage_rect_ = std::nullopt;
        return true;
    }

    /// @brief Blits cached surface to target canvas at specified coordinates.
    bool draw(Canvas& target, float x = 0.0f, float y = 0.0f) const {
        if (!pixmap_) return false;

        // Fast Path: Direct Scanline memcpy when canvas transform is identity
        if (target.transform().is_identity()) {
            const auto& clip_opt = target.scissor_clip();
            if (!clip_opt.has_value()) return false;
            const auto& clip = *clip_opt;

            int32_t dst_x = static_cast<int32_t>(x);
            int32_t dst_y = static_cast<int32_t>(y);

            int32_t x0 = std::max(dst_x, static_cast<int32_t>(clip.x()));
            int32_t y0 = std::max(dst_y, static_cast<int32_t>(clip.y()));
            int32_t x1 = std::min(dst_x + static_cast<int32_t>(width_), static_cast<int32_t>(clip.right()));
            int32_t y1 = std::min(dst_y + static_cast<int32_t>(height_), static_cast<int32_t>(clip.bottom()));

            if (x1 <= x0 || y1 <= y0) return true;

            uint32_t copy_width = static_cast<uint32_t>(x1 - x0);
            size_t copy_bytes = copy_width * sizeof(PremultipliedColorU8);

            auto src_ref = pixmap_->as_ref();
            auto& dst = target.pixmap();
            for (int32_t cy = y0; cy < y1; ++cy) {
                int32_t sy = cy - dst_y;
                int32_t sx = x0 - dst_x;
                const auto* src_row = src_ref.row(static_cast<size_t>(sy)) + sx;
                auto* dst_row = dst.row(static_cast<size_t>(cy)) + x0;
                std::memcpy(dst_row, src_row, copy_bytes);
            }
            return true;
        }

        target.draw_pixmap(static_cast<int32_t>(x), static_cast<int32_t>(y), pixmap_->as_ref());
        return true;
    }

    [[nodiscard]] uint32_t width() const noexcept { return width_; }
    [[nodiscard]] uint32_t height() const noexcept { return height_; }
    [[nodiscard]] const Pixmap* pixmap() const noexcept { return pixmap_ ? &(*pixmap_) : nullptr; }
    [[nodiscard]] Pixmap* pixmap() noexcept { return pixmap_ ? &(*pixmap_) : nullptr; }
    [[nodiscard]] const std::optional<ScreenIntRect>& damage_rect() const noexcept { return damage_rect_; }

private:
    static ScreenIntRect unite(const ScreenIntRect& a, const ScreenIntRect& b) noexcept {
        uint32_t x0 = std::min(a.left(), b.left());
        uint32_t y0 = std::min(a.top(), b.top());
        uint32_t x1 = std::max(a.right(), b.right());
        uint32_t y1 = std::max(a.bottom(), b.bottom());
        auto u = ScreenIntRect::from_xywh(x0, y0, x1 - x0, y1 - y0);
        return u ? *u : a;
    }

    uint32_t width_{0};
    uint32_t height_{0};
    bool dirty_{true};
    std::optional<ScreenIntRect> damage_rect_{std::nullopt};
    std::optional<Pixmap> pixmap_{std::nullopt};
};

/// @brief Multi-Page / Multi-View Retained Surface Manager.
/// Manages transitions (e.g. Home <-> Settings) with zero re-render overhead.
class SurfaceCacheManager {
public:
    SurfaceCacheManager() = default;

    SurfaceCache& get_or_create(const std::string& name, uint32_t width, uint32_t height) {
        auto it = caches_.find(name);
        if (it != caches_.end()) {
            if (it->second.width() != width || it->second.height() != height) {
                it->second.init(width, height);
            }
            return it->second;
        }
        auto [inserted_it, _] = caches_.emplace(name, SurfaceCache(width, height));
        return inserted_it->second;
    }

    [[nodiscard]] SurfaceCache* get(const std::string& name) noexcept {
        auto it = caches_.find(name);
        return (it != caches_.end()) ? &it->second : nullptr;
    }

    void invalidate(const std::string& name) {
        auto it = caches_.find(name);
        if (it != caches_.end()) {
            it->second.invalidate();
        }
    }

    void invalidate_all() {
        for (auto& [_, cache] : caches_) {
            cache.invalidate();
        }
    }

    bool draw_cached(const std::string& name, Canvas& target, float x, float y,
                     uint32_t width, uint32_t height,
                     const std::function<void(Canvas&)>& draw_fn) {
        auto& cache = get_or_create(name, width, height);
        cache.render_if_dirty(draw_fn);
        return cache.draw(target, x, y);
    }

    void clear() {
        caches_.clear();
    }

private:
    std::unordered_map<std::string, SurfaceCache> caches_;
};

} // namespace nisaba::damage
