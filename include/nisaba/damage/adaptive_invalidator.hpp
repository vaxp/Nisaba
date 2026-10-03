#pragma once

#include <cstdint>
#include <vector>
#include <algorithm>
#include <optional>

#include "nisaba/math/rect.hpp"
#include "nisaba/math/screen_int_rect.hpp"
#include "nisaba/damage/types.hpp"
#include "nisaba/damage/tiled_span_tracker.hpp"

namespace nisaba::damage {

/// @brief Strategy selected by Adaptive Invalidation.
enum class InvalidationStrategy : uint8_t {
    None,          ///< No damage registered.
    FineGrained,   ///< Scattered small updates (< threshold); fine tiles/spans preserved.
    FullContainer  ///< Extensive updates (>= threshold); collapsed to container bounding box.
};

/// @brief Adaptive Cost-Based Invalidation System (The 60% Threshold Rule).
/// Monitors container regions from nisaba::layout. When internal micro-updates exceed
/// the threshold ratio, collapses fine-grained tracking into a single cohesive bounding box,
/// eliminating tile fragmentation and guaranteeing bounded worst-case performance.
class AdaptiveInvalidator {
public:
    static constexpr float DEFAULT_THRESHOLD = 0.60f; // 60%

    AdaptiveInvalidator() = default;

    explicit AdaptiveInvalidator(const Rect& container_bounds, float threshold = DEFAULT_THRESHOLD)
        : container_rect_(container_bounds), threshold_(threshold) {}

    void set_container(const Rect& container_bounds) noexcept {
        container_rect_ = container_bounds;
        clear();
    }

    void set_threshold(float threshold) noexcept {
        threshold_ = std::clamp(threshold, 0.05f, 0.95f);
    }

    [[nodiscard]] float threshold() const noexcept { return threshold_; }
    [[nodiscard]] const Rect& container() const noexcept { return container_rect_; }

    /// Register a damaged child rectangle.
    void add_damage(const Rect& rect) {
        if (rect.width() <= 0.0f || rect.height() <= 0.0f) return;

        // Clip to container bounds if container is set
        float x0 = std::max(container_rect_.x(), rect.x());
        float y0 = std::max(container_rect_.y(), rect.y());
        float x1 = std::min(container_rect_.x() + container_rect_.width(), rect.x() + rect.width());
        float y1 = std::min(container_rect_.y() + container_rect_.height(), rect.y() + rect.height());

        if (x1 > x0 && y1 > y0) {
            auto clipped = Rect::from_xywh(x0, y0, x1 - x0, y1 - y0);
            if (clipped) {
                child_damages_.push_back(*clipped);
                accumulated_child_area_ += clipped->width() * clipped->height();
            }
        }
    }

    [[nodiscard]] bool has_damage() const noexcept {
        return !child_damages_.empty();
    }

    [[nodiscard]] size_t damage_count() const noexcept {
        return child_damages_.size();
    }

    [[nodiscard]] float container_area() const noexcept {
        return container_rect_.width() * container_rect_.height();
    }

    [[nodiscard]] float accumulated_damage_area() const noexcept {
        return accumulated_child_area_;
    }

    /// Ratio of damaged surface to total container surface (0.0 .. 1.0+).
    [[nodiscard]] float damage_ratio() const noexcept {
        float c_area = container_area();
        if (c_area <= 0.0001f) return 0.0f;
        return accumulated_child_area_ / c_area;
    }

    /// Determines whether the damage exceeds the threshold and should collapse.
    [[nodiscard]] bool should_collapse() const noexcept {
        return damage_ratio() >= threshold_;
    }

    /// @brief Resolves damage directly into a TiledSpanTracker.
    /// If ratio >= threshold, invalidates the single full container box.
    /// Otherwise, invalidates the fine-grained child rects.
    template <size_t TileSize>
    InvalidationStrategy resolve(TiledSpanTracker<TileSize>& tracker) {
        if (child_damages_.empty()) {
            return InvalidationStrategy::None;
        }

        if (should_collapse()) {
            tracker.mark_dirty(container_rect_);
            return InvalidationStrategy::FullContainer;
        }

        for (const auto& r : child_damages_) {
            tracker.mark_dirty(r);
        }
        return InvalidationStrategy::FineGrained;
    }

    /// @brief Resolves damage into a list of bounding boxes.
    InvalidationStrategy resolve(std::vector<Rect>& out_rects) {
        out_rects.clear();
        if (child_damages_.empty()) {
            return InvalidationStrategy::None;
        }

        if (should_collapse()) {
            out_rects.push_back(container_rect_);
            return InvalidationStrategy::FullContainer;
        }

        out_rects = child_damages_;
        return InvalidationStrategy::FineGrained;
    }

    void clear() noexcept {
        child_damages_.clear();
        accumulated_child_area_ = 0.0f;
    }

private:
    Rect container_rect_{};
    float threshold_{DEFAULT_THRESHOLD};
    float accumulated_child_area_{0.0f};
    std::vector<Rect> child_damages_;
};

} // namespace nisaba::damage
