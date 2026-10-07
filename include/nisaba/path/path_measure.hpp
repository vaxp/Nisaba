#pragma once

#include <vector>
#include <optional>
#include <cstdint>
#include <utility>
#include "nisaba/types.hpp"
#include "nisaba/math/point.hpp"
#include "nisaba/path/path.hpp"

namespace nisaba {

/// \brief Arc-length path parameterization, contour measurement, and sub-curve extraction.
///
/// Fully sovereign C++20 implementation providing 1:1 behavioral parity with SkPathMeasure.
/// Essential for animated spinners, progress rings, stroke trimming, and path morphing.
class PathMeasure {
public:
    enum class SegmentType : uint8_t {
        Line,
        Quad,
        Cubic,
    };

    struct Segment {
        float distance{0.0f};
        size_t point_index{0};
        uint32_t t_value{0};
        SegmentType kind{SegmentType::Line};

        [[nodiscard]] float scalar_t() const noexcept;
    };

    struct Contour {
        std::vector<Segment> segments;
        std::vector<Point> points;
        float length{0.0f};
        bool is_closed{false};

        [[nodiscard]] std::optional<std::pair<size_t, NormalizedF32>> distance_to_segment(float distance) const noexcept;
    };

    PathMeasure() noexcept = default;
    explicit PathMeasure(const Path& path, bool force_closed = false, float res_scale = 1.0f);
    PathMeasure(const Path* path, bool force_closed = false, float res_scale = 1.0f);

    /// Associates a new path and initializes measurement from the first contour.
    void set_path(const Path& path, bool force_closed = false, float res_scale = 1.0f);
    void set_path(const Path* path, bool force_closed = false, float res_scale = 1.0f);

    /// Total arc-length of the current contour. Returns 0.0f if no contour is active.
    [[nodiscard]] float length() const noexcept;

    /// Evaluates physical position and unit tangent vector at the given distance along the current contour.
    /// \param distance Distance from the start of the contour (pinned to [0, length]).
    /// \param position Optional output pointer for physical 2D coordinates.
    /// \param tangent Optional output pointer for normalized unit tangent vector.
    /// \return True on success, false if the contour is empty or distance is invalid.
    bool get_pos_tan(float distance, Point* position, Point* tangent) const noexcept;

    /// Extracts a sub-path segment between start_d and stop_d and appends it to dst.
    /// \param start_d Starting arc distance.
    /// \param stop_d Ending arc distance.
    /// \param dst Destination path to which the segment verbs and points are appended.
    /// \param start_with_move_to Whether to emit a MoveTo at the start of the extracted segment.
    /// \return True if a non-empty segment was extracted and appended, false otherwise.
    bool get_segment(float start_d, float stop_d, Path* dst, bool start_with_move_to = true) const;

    /// Returns true if the current contour is closed.
    [[nodiscard]] bool is_closed() const noexcept;

    /// Advances to the next non-empty contour in a multi-contour path.
    /// \return True if a subsequent contour exists, false if finished.
    bool next_contour();

    /// Returns the total count of non-empty contours in the measured path.
    [[nodiscard]] size_t contour_count() const noexcept { return contours_.size(); }

    /// Returns the current 0-based contour index.
    [[nodiscard]] size_t current_contour_index() const noexcept { return current_contour_idx_; }

    /// Clears all contours and resets state.
    void reset() noexcept;

    // --- Skia 1:1 Parity Aliases (CamelCase) ---
    [[nodiscard]] float getLength() const noexcept { return length(); }
    bool getPosTan(float distance, Point* position, Point* tangent) const noexcept {
        return get_pos_tan(distance, position, tangent);
    }
    bool getSegment(float start_d, float stop_d, Path* dst, bool start_with_move_to = true) const {
        return get_segment(start_d, stop_d, dst, start_with_move_to);
    }
    [[nodiscard]] bool isClosed() const noexcept { return is_closed(); }
    bool nextContour() { return next_contour(); }
    void setPath(const Path* path, bool force_closed = false) { set_path(path, force_closed); }
    void setPath(const Path& path, bool force_closed = false) { set_path(path, force_closed); }

private:
    void build_contours(const Path& path, bool force_closed, float res_scale);
    static void segment_to_path(
        const Point* points,
        SegmentType seg_kind,
        NormalizedF32 start_t,
        NormalizedF32 stop_t,
        Path& dst
    ) noexcept;

    std::vector<Contour> contours_;
    size_t current_contour_idx_{0};
};

} // namespace nisaba
