#pragma once

/// @file stagger.hpp
/// @brief Staggered animation controllers and Interval curves for cascading UI sequences in Nisaba.
///
/// Provides cascading entrance animations for lists, grids, and dashboards.
/// Includes IntervalCurve for mapping sub-windows of an animation progress.

#include "nisaba/animation/curves.hpp"
#include "nisaba/animation/animation_controller.hpp"

#include <chrono>
#include <algorithm>
#include <utility>

namespace nisaba::animation {

// ════════════════════════════════════════════════════════════════
// IntervalCurve — Eases within a window [begin, end] ⊆ [0, 1]
// ════════════════════════════════════════════════════════════════

/// @brief Transforms an animation progress only within a specific interval [begin, end].
class IntervalCurve final : public Curve {
public:
    constexpr IntervalCurve(float begin, float end, const Curve* inner_curve = &Curves::linear) noexcept
        : begin_(std::clamp(begin, 0.0f, 1.0f)),
          end_(std::clamp(end, begin, 1.0f)),
          inner_curve_(inner_curve ? inner_curve : &Curves::linear) {}

    [[nodiscard]] double evaluate(double t) const override {
        if (t <= static_cast<double>(begin_)) return 0.0;
        if (t >= static_cast<double>(end_)) return 1.0;

        double span = static_cast<double>(end_ - begin_);
        double local_t = (span > 0.0) ? (t - static_cast<double>(begin_)) / span : 1.0;
        local_t = std::clamp(local_t, 0.0, 1.0);

        return inner_curve_ ? inner_curve_->evaluate(local_t) : local_t;
    }

private:
    float begin_ = 0.0f;
    float end_   = 1.0f;
    const Curve* inner_curve_ = &Curves::linear;
};

// ════════════════════════════════════════════════════════════════
// StaggerConfig — Staggered sequence configuration
// ════════════════════════════════════════════════════════════════

struct StaggerConfig {
    std::chrono::milliseconds item_duration{250};
    std::chrono::milliseconds delay_between_items{40};
    const Curve*               curve = &Curves::easeOut;
};

// ════════════════════════════════════════════════════════════════
// StaggerHelper — Stagger calculations for collections
// ════════════════════════════════════════════════════════════════

class StaggerHelper {
public:
    /// Computes the total required timeline duration for N items
    static std::chrono::milliseconds total_duration(size_t item_count, const StaggerConfig& config) noexcept {
        if (item_count == 0) return std::chrono::milliseconds{0};
        int64_t total_ms = (static_cast<int64_t>(item_count - 1) * config.delay_between_items.count())
                         + config.item_duration.count();
        return std::chrono::milliseconds{std::max<int64_t>(0, total_ms)};
    }

    static std::chrono::milliseconds totalDuration(size_t item_count, const StaggerConfig& config) noexcept {
        return total_duration(item_count, config);
    }

    /// Computes the normalized interval [begin, end] in [0, 1] for an item index
    static std::pair<float, float> item_interval(size_t index, size_t total_items, const StaggerConfig& config) noexcept {
        if (total_items == 0) return {0.0f, 1.0f};

        auto total_dur = total_duration(total_items, config);
        double total_ms = static_cast<double>(total_dur.count());
        if (total_ms <= 0.0) return {0.0f, 1.0f};

        double start_ms = static_cast<double>(index * config.delay_between_items.count());
        double end_ms   = start_ms + static_cast<double>(config.item_duration.count());

        float begin_frac = static_cast<float>(std::clamp(start_ms / total_ms, 0.0, 1.0));
        float end_frac   = static_cast<float>(std::clamp(end_ms / total_ms, 0.0, 1.0));

        return {begin_frac, end_frac};
    }

    static std::pair<float, float> itemInterval(size_t index, size_t total_items, const StaggerConfig& config) noexcept {
        return item_interval(index, total_items, config);
    }

    /// Evaluates individual progress [0, 1] for item at index given overall master progress t
    static float item_progress(size_t index, size_t total_items, float master_progress, const StaggerConfig& config) {
        auto [begin, end] = item_interval(index, total_items, config);
        IntervalCurve interval(begin, end, config.curve);
        return interval.evaluate_f(master_progress);
    }

    static float itemProgress(size_t index, size_t total_items, float master_progress, const StaggerConfig& config) {
        return item_progress(index, total_items, master_progress, config);
    }
};

} // namespace nisaba::animation
