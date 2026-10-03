#pragma once

#include <vector>
#include <optional>
#include "nisaba/math/point.hpp"
#include "nisaba/math/rect.hpp"
#include "nisaba/path/path.hpp"
#include "nisaba/raster/edge_builder.hpp"

namespace nisaba {

class EdgeClipper {
public:
    EdgeClipper(Rect clip, bool can_cull_to_the_right)
        : clip_(clip), can_cull_to_the_right_(can_cull_to_the_right) {}

    bool clip_line(Point p0, Point p1, std::vector<PathEdge>& out);
    bool clip_quad(Point p0, Point p1, Point p2, std::vector<PathEdge>& out);
    bool clip_cubic(Point p0, Point p1, Point p2, Point p3, std::vector<PathEdge>& out);

private:
    void push_vline(float x, float y0, float y1, bool reverse, std::vector<PathEdge>& out);
    void push_quad(const Point* pts, bool reverse, std::vector<PathEdge>& out);
    void push_cubic(const Point* pts, bool reverse, std::vector<PathEdge>& out);

    void clip_mono_quad(const Point* src, std::vector<PathEdge>& out);
    void clip_mono_cubic(const Point* src, std::vector<PathEdge>& out);

    Rect clip_;
    bool can_cull_to_the_right_{false};
};

class EdgeClipperIter {
public:
    EdgeClipperIter(const Path& path, Rect clip, bool can_cull_to_the_right)
        : edge_iter_(path), clipper_(clip, can_cull_to_the_right) {}

    std::optional<PathEdge> next();

private:
    PathEdgeIter edge_iter_;
    EdgeClipper clipper_;
    std::vector<PathEdge> current_edges_;
    size_t current_idx_{0};
};

} // namespace nisaba
