#pragma once

#include <cstdint>
#include <optional>
#include <vector>
#include <array>
#include "nisaba/math/point.hpp"
#include "nisaba/math/screen_int_rect.hpp"
#include "nisaba/path/path.hpp"
#include "nisaba/raster/edge.hpp"

namespace nisaba {

class ShiftedIntRect {
public:
    ShiftedIntRect(ScreenIntRect shifted, int32_t shift)
        : shifted_(shifted), shift_(shift) {}

    static std::optional<ShiftedIntRect> create(const ScreenIntRect& rect, int32_t shift) noexcept {
        auto shifted = ScreenIntRect::from_xywh(
            rect.x() << shift,
            rect.y() << shift,
            rect.width() << shift,
            rect.height() << shift
        );
        if (!shifted) return std::nullopt;
        return ShiftedIntRect(*shifted, shift);
    }

    const ScreenIntRect& shifted() const noexcept { return shifted_; }
    int32_t shift() const noexcept { return shift_; }

    ScreenIntRect recover() const noexcept {
        return *ScreenIntRect::from_xywh(
            shifted_.x() >> shift_,
            shifted_.y() >> shift_,
            shifted_.width() >> shift_,
            shifted_.height() >> shift_
        );
    }

private:
    ScreenIntRect shifted_;
    int32_t shift_{0};
};

struct PathEdge {
    enum class Type { LineTo, QuadTo, CubicTo };
    Type type{Type::LineTo};
    std::array<Point, 4> points{};

    static PathEdge line(Point p0, Point p1) noexcept {
        PathEdge e;
        e.type = Type::LineTo;
        e.points[0] = p0;
        e.points[1] = p1;
        return e;
    }

    static PathEdge quad(Point p0, Point p1, Point p2) noexcept {
        PathEdge e;
        e.type = Type::QuadTo;
        e.points[0] = p0;
        e.points[1] = p1;
        e.points[2] = p2;
        return e;
    }

    static PathEdge cubic(Point p0, Point p1, Point p2, Point p3) noexcept {
        PathEdge e;
        e.type = Type::CubicTo;
        e.points[0] = p0;
        e.points[1] = p1;
        e.points[2] = p2;
        e.points[3] = p3;
        return e;
    }
};

class PathEdgeIter {
public:
    explicit PathEdgeIter(const Path& path) noexcept
        : path_(path), verb_index_(0), points_index_(0), move_to_(Point::zero()), needs_close_line_(false) {}

    std::optional<PathEdge> next() noexcept;

private:
    std::optional<PathEdge> close_line() noexcept;

    const Path& path_;
    size_t verb_index_{0};
    size_t points_index_{0};
    Point move_to_{Point::zero()};
    bool needs_close_line_{false};
};

class BasicEdgeBuilder {
public:
    explicit BasicEdgeBuilder(int32_t clip_shift)
        : clip_shift_(clip_shift) {
        edges_.reserve(64);
    }

    static std::optional<std::vector<Edge>*> build_edges_inplace(
        const Path& path,
        const ShiftedIntRect* clip,
        int32_t clip_shift
    );

    static std::optional<std::vector<Edge>> build_edges(
        const Path& path,
        const ShiftedIntRect* clip,
        int32_t clip_shift
    );

    bool build(
        const Path& path,
        const ShiftedIntRect* clip,
        bool can_cull_to_the_right
    );

    const std::vector<Edge>& edges() const noexcept { return edges_; }
    std::vector<Edge>& edges() noexcept { return edges_; }

private:
    void push_line(Point p0, Point p1);
    void push_quad(const Point* points);
    void push_cubic(const Point* points);

    std::vector<Edge> edges_;
    int32_t clip_shift_{0};
};

} // namespace nisaba
