#include "nisaba/gpu/gpu_tessellator.hpp"
#include "nisaba/path/path_geometry.hpp"
#include <cmath>
#include <algorithm>

namespace nisaba::gpu {

namespace {

inline nisaba::Point apply_ts(const nisaba::Transform& ts, nisaba::Point p) noexcept {
    ts.map_point(p);
    return p;
}

inline nisaba::Point apply_ts(const nisaba::Transform& ts, float x, float y) noexcept {
    nisaba::Point p(x, y);
    ts.map_point(p);
    return p;
}

inline nisaba::Point scale_pt(nisaba::Point p, float s) noexcept {
    return nisaba::Point(p.x * s, p.y * s);
}

} // namespace

void GpuTessellator::tessellate_rect(
    std::vector<GpuVertex>& out_verts,
    std::vector<uint32_t>& out_indices,
    const nisaba::Rect& rect,
    nisaba::Color color,
    const nisaba::Transform& transform,
    bool anti_alias
) {
    float x0 = rect.left();
    float y0 = rect.top();
    float x1 = rect.right();
    float y1 = rect.bottom();
    if (x0 > x1) std::swap(x0, x1);
    if (y0 > y1) std::swap(y0, y1);

    uint32_t base_idx = static_cast<uint32_t>(out_verts.size());

    if (!anti_alias) {
        out_verts.emplace_back(apply_ts(transform, x0, y0), color, 1.0f);
        out_verts.emplace_back(apply_ts(transform, x1, y0), color, 1.0f);
        out_verts.emplace_back(apply_ts(transform, x1, y1), color, 1.0f);
        out_verts.emplace_back(apply_ts(transform, x0, y1), color, 1.0f);

        out_indices.push_back(base_idx + 0);
        out_indices.push_back(base_idx + 1);
        out_indices.push_back(base_idx + 2);

        out_indices.push_back(base_idx + 0);
        out_indices.push_back(base_idx + 2);
        out_indices.push_back(base_idx + 3);
        return;
    }

    // Subpixel AA fringe: 4 inner vertices (coverage = 1.0), 4 outer vertices (coverage = 0.0)
    constexpr float fringe = 0.5f;
    float ix0 = x0 + fringe;
    float iy0 = y0 + fringe;
    float ix1 = x1 - fringe;
    float iy1 = y1 - fringe;

    float ox0 = x0 - fringe;
    float oy0 = y0 - fringe;
    float ox1 = x1 + fringe;
    float oy1 = y1 + fringe;

    if (ix1 <= ix0 || iy1 <= iy0) {
        // Fallback for very small rectangles
        tessellate_rect(out_verts, out_indices, rect, color, transform, false);
        return;
    }

    // Inner 4 vertices [0..3]
    out_verts.emplace_back(apply_ts(transform, ix0, iy0), color, 1.0f);
    out_verts.emplace_back(apply_ts(transform, ix1, iy0), color, 1.0f);
    out_verts.emplace_back(apply_ts(transform, ix1, iy1), color, 1.0f);
    out_verts.emplace_back(apply_ts(transform, ix0, iy1), color, 1.0f);

    // Outer 4 vertices [4..7]
    out_verts.emplace_back(apply_ts(transform, ox0, oy0), color, 0.0f);
    out_verts.emplace_back(apply_ts(transform, ox1, oy0), color, 0.0f);
    out_verts.emplace_back(apply_ts(transform, ox1, oy1), color, 0.0f);
    out_verts.emplace_back(apply_ts(transform, ox0, oy1), color, 0.0f);

    // Center fill
    out_indices.push_back(base_idx + 0);
    out_indices.push_back(base_idx + 1);
    out_indices.push_back(base_idx + 2);

    out_indices.push_back(base_idx + 0);
    out_indices.push_back(base_idx + 2);
    out_indices.push_back(base_idx + 3);

    // Fringe quads (Top, Right, Bottom, Left)
    auto add_quad = [&](uint32_t i0, uint32_t i1, uint32_t o0, uint32_t o1) {
        out_indices.push_back(base_idx + i0);
        out_indices.push_back(base_idx + o0);
        out_indices.push_back(base_idx + o1);

        out_indices.push_back(base_idx + i0);
        out_indices.push_back(base_idx + o1);
        out_indices.push_back(base_idx + i1);
    };

    add_quad(0, 1, 4, 5); // Top
    add_quad(1, 2, 5, 6); // Right
    add_quad(2, 3, 6, 7); // Bottom
    add_quad(3, 0, 7, 4); // Left
}

void GpuTessellator::tessellate_circle(
    std::vector<GpuVertex>& out_verts,
    std::vector<uint32_t>& out_indices,
    float cx, float cy, float radius,
    nisaba::Color color,
    const nisaba::Transform& transform,
    bool anti_alias
) {
    if (radius <= 0.0f) return;

    // Determine segment count dynamically based on radius: high density for smooth curves
    uint32_t num_segments = std::clamp(static_cast<uint32_t>(radius * 3.5f), 48u, 256u);
    uint32_t base_idx = static_cast<uint32_t>(out_verts.size());

    // Center vertex
    out_verts.emplace_back(apply_ts(transform, cx, cy), color, 1.0f);

    float angle_step = (2.0f * static_cast<float>(M_PI)) / static_cast<float>(num_segments);
    float inner_r = anti_alias ? (radius - 0.5f) : radius;
    float outer_r = radius + 0.5f;

    // Perimeter vertices [1 .. num_segments]
    for (uint32_t i = 0; i < num_segments; ++i) {
        float theta = static_cast<float>(i) * angle_step;
        float px = cx + inner_r * std::cos(theta);
        float py = cy + inner_r * std::sin(theta);
        out_verts.emplace_back(apply_ts(transform, px, py), color, 1.0f);
    }

    // Inner circle fan
    for (uint32_t i = 0; i < num_segments; ++i) {
        uint32_t next = (i + 1) % num_segments;
        out_indices.push_back(base_idx);
        out_indices.push_back(base_idx + 1 + i);
        out_indices.push_back(base_idx + 1 + next);
    }

    if (anti_alias) {
        // Outer AA fringe ring [1 + num_segments .. 1 + 2 * num_segments - 1]
        uint32_t outer_base = static_cast<uint32_t>(out_verts.size());
        for (uint32_t i = 0; i < num_segments; ++i) {
            float theta = static_cast<float>(i) * angle_step;
            float px = cx + outer_r * std::cos(theta);
            float py = cy + outer_r * std::sin(theta);
            out_verts.emplace_back(apply_ts(transform, px, py), color, 0.0f);
        }

        // Fringe quads connecting inner perimeter to outer ring
        for (uint32_t i = 0; i < num_segments; ++i) {
            uint32_t next = (i + 1) % num_segments;
            uint32_t i0 = base_idx + 1 + i;
            uint32_t i1 = base_idx + 1 + next;
            uint32_t o0 = outer_base + i;
            uint32_t o1 = outer_base + next;

            out_indices.push_back(i0);
            out_indices.push_back(o0);
            out_indices.push_back(o1);

            out_indices.push_back(i0);
            out_indices.push_back(o1);
            out_indices.push_back(i1);
        }
    }
}

void GpuTessellator::tessellate_round_rect(
    std::vector<GpuVertex>& out_verts,
    std::vector<uint32_t>& out_indices,
    const nisaba::Rect& rect,
    float rx, float ry,
    nisaba::Color color,
    const nisaba::Transform& transform,
    bool anti_alias
) {
    float x0 = rect.left();
    float y0 = rect.top();
    float x1 = rect.right();
    float y1 = rect.bottom();
    if (x0 > x1) std::swap(x0, x1);
    if (y0 > y1) std::swap(y0, y1);

    float w = x1 - x0;
    float h = y1 - y0;
    rx = std::min(rx, w * 0.5f);
    ry = std::min(ry, h * 0.5f);

    if (rx <= 0.0f || ry <= 0.0f) {
        tessellate_rect(out_verts, out_indices, rect, color, transform, anti_alias);
        return;
    }

    // Tessellate polygon outline of rounded rectangle with adaptive corner density
    uint32_t arc_steps = std::clamp(static_cast<uint32_t>(std::max(rx, ry) * 1.5f), 12u, 32u);
    std::vector<Point> contour;
    contour.reserve(arc_steps * 4);

    auto add_corner = [&](float center_x, float center_y, float start_angle) {
        float step = (static_cast<float>(M_PI) * 0.5f) / static_cast<float>(arc_steps);
        for (uint32_t i = 0; i <= arc_steps; ++i) {
            float th = start_angle + static_cast<float>(i) * step;
            contour.emplace_back(center_x + rx * std::cos(th), center_y + ry * std::sin(th));
        }
    };

    // Top-Right, Bottom-Right, Bottom-Left, Top-Left
    add_corner(x1 - rx, y0 + ry, -static_cast<float>(M_PI) * 0.5f);
    add_corner(x1 - rx, y1 - ry, 0.0f);
    add_corner(x0 + rx, y1 - ry, static_cast<float>(M_PI) * 0.5f);
    add_corner(x0 + rx, y0 + ry, static_cast<float>(M_PI));

    // Remove duplicate consecutive points
    std::vector<Point> clean_contour;
    for (size_t i = 0; i < contour.size(); ++i) {
        if (clean_contour.empty() || contour[i].distance(clean_contour.back()) > 0.01f) {
            clean_contour.push_back(contour[i]);
        }
    }

    uint32_t n = static_cast<uint32_t>(clean_contour.size());
    if (n < 3) return;

    Point center((x0 + x1) * 0.5f, (y0 + y1) * 0.5f);
    uint32_t base_idx = static_cast<uint32_t>(out_verts.size());

    // Center vertex
    out_verts.emplace_back(apply_ts(transform, center), color, 1.0f);

    // Perimeter vertices
    for (uint32_t i = 0; i < n; ++i) {
        out_verts.emplace_back(apply_ts(transform, clean_contour[i]), color, 1.0f);
    }

    // Fan interior
    for (uint32_t i = 0; i < n; ++i) {
        uint32_t next = (i + 1) % n;
        out_indices.push_back(base_idx);
        out_indices.push_back(base_idx + 1 + i);
        out_indices.push_back(base_idx + 1 + next);
    }

    if (anti_alias) {
        uint32_t outer_base = static_cast<uint32_t>(out_verts.size());
        for (uint32_t i = 0; i < n; ++i) {
            Point p = clean_contour[i];
            Point dir = p - center;
            float len = std::hypot(dir.x, dir.y);
            Point normal = (len > 0.001f) ? Point(dir.x / len, dir.y / len) : Point(0.0f, 0.0f);
            Point outer_p = p + scale_pt(normal, 1.0f);
            out_verts.emplace_back(apply_ts(transform, outer_p), color, 0.0f);
        }

        for (uint32_t i = 0; i < n; ++i) {
            uint32_t next = (i + 1) % n;
            uint32_t i0 = base_idx + 1 + i;
            uint32_t i1 = base_idx + 1 + next;
            uint32_t o0 = outer_base + i;
            uint32_t o1 = outer_base + next;

            out_indices.push_back(i0);
            out_indices.push_back(o0);
            out_indices.push_back(o1);

            out_indices.push_back(i0);
            out_indices.push_back(o1);
            out_indices.push_back(i1);
        }
    }
}

void GpuTessellator::tessellate_line(
    std::vector<GpuVertex>& out_verts,
    std::vector<uint32_t>& out_indices,
    nisaba::Point p0, nisaba::Point p1,
    const nisaba::Stroke& stroke,
    nisaba::Color color,
    const nisaba::Transform& transform,
    bool anti_alias
) {
    Point d = p1 - p0;
    float len = std::hypot(d.x, d.y);
    if (len < 0.001f) return;

    Point normal(-d.y / len, d.x / len);
    float hw = stroke.width * 0.5f;

    Point left0 = p0 + scale_pt(normal, hw);
    Point right0 = p0 - scale_pt(normal, hw);
    Point left1 = p1 + scale_pt(normal, hw);
    Point right1 = p1 - scale_pt(normal, hw);

    uint32_t base_idx = static_cast<uint32_t>(out_verts.size());

    out_verts.emplace_back(apply_ts(transform, left0), color, 1.0f);
    out_verts.emplace_back(apply_ts(transform, right0), color, 1.0f);
    out_verts.emplace_back(apply_ts(transform, right1), color, 1.0f);
    out_verts.emplace_back(apply_ts(transform, left1), color, 1.0f);

    out_indices.push_back(base_idx + 0);
    out_indices.push_back(base_idx + 1);
    out_indices.push_back(base_idx + 2);

    out_indices.push_back(base_idx + 0);
    out_indices.push_back(base_idx + 2);
    out_indices.push_back(base_idx + 3);

    if (anti_alias) {
        float fringe = 0.5f;
        Point o_left0 = p0 + scale_pt(normal, hw + fringe);
        Point o_right0 = p0 - scale_pt(normal, hw + fringe);
        Point o_left1 = p1 + scale_pt(normal, hw + fringe);
        Point o_right1 = p1 - scale_pt(normal, hw + fringe);

        uint32_t o_base = static_cast<uint32_t>(out_verts.size());
        out_verts.emplace_back(apply_ts(transform, o_left0), color, 0.0f);
        out_verts.emplace_back(apply_ts(transform, o_right0), color, 0.0f);
        out_verts.emplace_back(apply_ts(transform, o_right1), color, 0.0f);
        out_verts.emplace_back(apply_ts(transform, o_left1), color, 0.0f);

        // Fringe left
        out_indices.push_back(base_idx + 0);
        out_indices.push_back(o_base + 0);
        out_indices.push_back(o_base + 3);

        out_indices.push_back(base_idx + 0);
        out_indices.push_back(o_base + 3);
        out_indices.push_back(base_idx + 3);

        // Fringe right
        out_indices.push_back(base_idx + 1);
        out_indices.push_back(o_base + 1);
        out_indices.push_back(o_base + 2);

        out_indices.push_back(base_idx + 1);
        out_indices.push_back(o_base + 2);
        out_indices.push_back(base_idx + 2);
    }
}

void GpuTessellator::tessellate_path(
    std::vector<GpuVertex>& out_verts,
    std::vector<uint32_t>& out_indices,
    const nisaba::Path& path,
    nisaba::FillRule fill_rule,
    nisaba::Color color,
    const nisaba::Transform& transform,
    bool anti_alias
) {
    (void)fill_rule;
    // Flatten path segments into polylines
    std::vector<Point> points;
    PathSegmentsIter iter(path);
    iter.set_auto_close(true);

    while (auto seg = iter.next()) {
        switch (seg->type) {
            case PathSegment::Type::MoveTo:
                points.push_back(seg->p0);
                break;
            case PathSegment::Type::LineTo:
                points.push_back(seg->p0);
                break;
            case PathSegment::Type::QuadTo: {
                Point p0 = iter.last_point();
                Point p1 = seg->p0;
                Point p2 = seg->p1;
                float chord = p0.distance(p1) + p1.distance(p2);
                int steps = std::clamp(static_cast<int>(chord / 3.0f), 12, 48);
                for (int i = 1; i <= steps; ++i) {
                    float t = static_cast<float>(i) / static_cast<float>(steps);
                    float u = 1.0f - t;
                    float x = u * u * p0.x + 2.0f * u * t * p1.x + t * t * p2.x;
                    float y = u * u * p0.y + 2.0f * u * t * p1.y + t * t * p2.y;
                    points.emplace_back(x, y);
                }
                break;
            }
            case PathSegment::Type::CubicTo: {
                Point p0 = iter.last_point();
                Point p1 = seg->p0;
                Point p2 = seg->p1;
                Point p3 = seg->p2;
                float chord = p0.distance(p1) + p1.distance(p2) + p2.distance(p3);
                int steps = std::clamp(static_cast<int>(chord / 3.0f), 16, 64);
                for (int i = 1; i <= steps; ++i) {
                    float t = static_cast<float>(i) / static_cast<float>(steps);
                    float u = 1.0f - t;
                    float x = u * u * u * p0.x + 3.0f * u * u * t * p1.x + 3.0f * u * t * t * p2.x + t * t * t * p3.x;
                    float y = u * u * u * p0.y + 3.0f * u * u * t * p1.y + 3.0f * u * t * t * p2.y + t * t * t * p3.y;
                    points.emplace_back(x, y);
                }
                break;
            }
            case PathSegment::Type::Close:
                break;
        }
    }

    if (points.size() < 3) return;

    // Triangle fan tessellation from first point
    uint32_t base_idx = static_cast<uint32_t>(out_verts.size());
    for (const auto& pt : points) {
        out_verts.emplace_back(apply_ts(transform, pt), color, 1.0f);
    }

    uint32_t n = static_cast<uint32_t>(points.size());
    for (uint32_t i = 1; i + 1 < n; ++i) {
        out_indices.push_back(base_idx);
        out_indices.push_back(base_idx + i);
        out_indices.push_back(base_idx + i + 1);
    }

    if (anti_alias) {
        uint32_t outer_base = static_cast<uint32_t>(out_verts.size());
        for (uint32_t i = 0; i < n; ++i) {
            uint32_t prev = (i == 0) ? (n - 1) : (i - 1);
            uint32_t next = (i + 1) % n;
            Point d0 = points[i] - points[prev];
            Point d1 = points[next] - points[i];
            float l0 = std::hypot(d0.x, d0.y);
            float l1 = std::hypot(d1.x, d1.y);
            Point n0 = (l0 > 0.001f) ? Point(-d0.y / l0, d0.x / l0) : Point(0.0f, 0.0f);
            Point n1 = (l1 > 0.001f) ? Point(-d1.y / l1, d1.x / l1) : Point(0.0f, 0.0f);
            Point avg_n = scale_pt(n0 + n1, 0.5f);
            float avg_l = std::hypot(avg_n.x, avg_n.y);
            if (avg_l > 0.001f) avg_n = Point(avg_n.x / avg_l, avg_n.y / avg_l);

            out_verts.emplace_back(apply_ts(transform, points[i] + scale_pt(avg_n, 1.0f)), color, 0.0f);
        }

        for (uint32_t i = 0; i < n; ++i) {
            uint32_t next = (i + 1) % n;
            uint32_t i0 = base_idx + i;
            uint32_t i1 = base_idx + next;
            uint32_t o0 = outer_base + i;
            uint32_t o1 = outer_base + next;

            out_indices.push_back(i0);
            out_indices.push_back(o0);
            out_indices.push_back(o1);

            out_indices.push_back(i0);
            out_indices.push_back(o1);
            out_indices.push_back(i1);
        }
    }
}

void GpuTessellator::tessellate_stroke(
    std::vector<GpuVertex>& out_verts,
    std::vector<uint32_t>& out_indices,
    const nisaba::Path& path,
    const nisaba::Stroke& stroke,
    nisaba::Color color,
    const nisaba::Transform& transform,
    bool anti_alias
) {
    if (stroke.width <= 0.0f) return;

    struct Contour {
        std::vector<Point> pts;
        bool closed{false};
    };

    std::vector<Contour> contours;
    Contour current;

    PathSegmentsIter iter(path);
    Point last_pt(0.0f, 0.0f);

    while (auto seg = iter.next()) {
        switch (seg->type) {
            case PathSegment::Type::MoveTo:
                if (current.pts.size() >= 2) {
                    contours.push_back(std::move(current));
                    current = Contour{};
                } else {
                    current.pts.clear();
                }
                current.pts.push_back(seg->p0);
                last_pt = seg->p0;
                break;

            case PathSegment::Type::LineTo:
                if (current.pts.empty()) current.pts.push_back(last_pt);
                current.pts.push_back(seg->p0);
                last_pt = seg->p0;
                break;

            case PathSegment::Type::QuadTo: {
                if (current.pts.empty()) current.pts.push_back(last_pt);
                Point p0 = last_pt;
                Point p1 = seg->p0;
                Point p2 = seg->p1;
                float chord = p0.distance(p1) + p1.distance(p2);
                int steps = std::clamp(static_cast<int>(chord / 3.0f), 12, 48);
                for (int i = 1; i <= steps; ++i) {
                    float t = static_cast<float>(i) / static_cast<float>(steps);
                    float u = 1.0f - t;
                    Point pt(u * u * p0.x + 2.0f * u * t * p1.x + t * t * p2.x,
                             u * u * p0.y + 2.0f * u * t * p1.y + t * t * p2.y);
                    current.pts.push_back(pt);
                }
                last_pt = p2;
                break;
            }

            case PathSegment::Type::CubicTo: {
                if (current.pts.empty()) current.pts.push_back(last_pt);
                Point p0 = last_pt;
                Point p1 = seg->p0;
                Point p2 = seg->p1;
                Point p3 = seg->p2;
                float chord = p0.distance(p1) + p1.distance(p2) + p2.distance(p3);
                int steps = std::clamp(static_cast<int>(chord / 3.0f), 16, 64);
                for (int i = 1; i <= steps; ++i) {
                    float t = static_cast<float>(i) / static_cast<float>(steps);
                    float u = 1.0f - t;
                    Point pt(u * u * u * p0.x + 3.0f * u * u * t * p1.x + 3.0f * u * t * t * p2.x + t * t * t * p3.x,
                             u * u * u * p0.y + 3.0f * u * u * t * p1.y + 3.0f * u * t * t * p2.y + t * t * t * p3.y);
                    current.pts.push_back(pt);
                }
                last_pt = p3;
                break;
            }

            case PathSegment::Type::Close:
                if (current.pts.size() >= 2) {
                    current.closed = true;
                    contours.push_back(std::move(current));
                    current = Contour{};
                }
                break;
        }
    }

    if (current.pts.size() >= 2) {
        contours.push_back(std::move(current));
    }

    float hw = stroke.width * 0.5f;

    for (auto& contour : contours) {
        std::vector<Point> pts;
        pts.reserve(contour.pts.size());
        for (const auto& p : contour.pts) {
            if (pts.empty() || pts.back().distance(p) > 0.001f) {
                pts.push_back(p);
            }
        }

        if (pts.size() < 2) continue;

        if (contour.closed && pts.size() > 2 && pts.front().distance(pts.back()) <= 0.001f) {
            pts.pop_back();
        }

        if (pts.size() == 2 && !contour.closed) {
            tessellate_line(out_verts, out_indices, pts[0], pts[1], stroke, color, transform, anti_alias);
            continue;
        }

        size_t N = pts.size();
        std::vector<Point> miter_offsets(N);

        for (size_t i = 0; i < N; ++i) {
            if (!contour.closed && i == 0) {
                Point d = pts[1] - pts[0];
                float l = std::hypot(d.x, d.y);
                Point n = (l > 0.0001f) ? Point(-d.y / l, d.x / l) : Point(0.0f, 1.0f);
                miter_offsets[0] = scale_pt(n, hw);
            } else if (!contour.closed && i == N - 1) {
                Point d = pts[N - 1] - pts[N - 2];
                float l = std::hypot(d.x, d.y);
                Point n = (l > 0.0001f) ? Point(-d.y / l, d.x / l) : Point(0.0f, 1.0f);
                miter_offsets[N - 1] = scale_pt(n, hw);
            } else {
                size_t prev_i = (i == 0) ? (N - 1) : (i - 1);
                size_t next_i = (i + 1) % N;
                Point d0 = pts[i] - pts[prev_i];
                Point d1 = pts[next_i] - pts[i];
                float l0 = std::hypot(d0.x, d0.y);
                float l1 = std::hypot(d1.x, d1.y);
                Point n0 = (l0 > 0.0001f) ? Point(-d0.y / l0, d0.x / l0) : Point(0.0f, 1.0f);
                Point n1 = (l1 > 0.0001f) ? Point(-d1.y / l1, d1.x / l1) : Point(0.0f, 1.0f);
                Point avg_n = n0 + n1;
                float avg_l = std::hypot(avg_n.x, avg_n.y);
                if (avg_l > 0.0001f) {
                    avg_n = Point(avg_n.x / avg_l, avg_n.y / avg_l);
                    float dot_val = avg_n.x * n0.x + avg_n.y * n0.y;
                    float miter_len = hw / std::max(dot_val, 0.25f);
                    miter_len = std::min(miter_len, hw * 2.5f);
                    miter_offsets[i] = scale_pt(avg_n, miter_len);
                } else {
                    miter_offsets[i] = scale_pt(n0, hw);
                }
            }
        }

        uint32_t inner_base = static_cast<uint32_t>(out_verts.size());
        for (size_t i = 0; i < N; ++i) {
            Point left_pt = pts[i] + miter_offsets[i];
            Point right_pt = pts[i] - miter_offsets[i];
            out_verts.emplace_back(apply_ts(transform, left_pt), color, 1.0f);
            out_verts.emplace_back(apply_ts(transform, right_pt), color, 1.0f);
        }

        size_t seg_count = contour.closed ? N : (N - 1);
        for (size_t i = 0; i < seg_count; ++i) {
            uint32_t next = static_cast<uint32_t>((i + 1) % N);
            uint32_t l0 = inner_base + 2 * static_cast<uint32_t>(i);
            uint32_t r0 = inner_base + 2 * static_cast<uint32_t>(i) + 1;
            uint32_t l1 = inner_base + 2 * next;
            uint32_t r1 = inner_base + 2 * next + 1;

            out_indices.push_back(l0);
            out_indices.push_back(r0);
            out_indices.push_back(r1);

            out_indices.push_back(l0);
            out_indices.push_back(r1);
            out_indices.push_back(l1);
        }

        if (anti_alias) {
            float fringe = 0.5f;
            float scale = (hw + fringe) / std::max(hw, 0.001f);
            uint32_t outer_base = static_cast<uint32_t>(out_verts.size());

            for (size_t i = 0; i < N; ++i) {
                Point o_left = pts[i] + scale_pt(miter_offsets[i], scale);
                Point o_right = pts[i] - scale_pt(miter_offsets[i], scale);
                out_verts.emplace_back(apply_ts(transform, o_left), color, 0.0f);
                out_verts.emplace_back(apply_ts(transform, o_right), color, 0.0f);
            }

            for (size_t i = 0; i < seg_count; ++i) {
                uint32_t next = static_cast<uint32_t>((i + 1) % N);
                uint32_t l0 = inner_base + 2 * static_cast<uint32_t>(i);
                uint32_t r0 = inner_base + 2 * static_cast<uint32_t>(i) + 1;
                uint32_t l1 = inner_base + 2 * next;
                uint32_t r1 = inner_base + 2 * next + 1;

                uint32_t ol0 = outer_base + 2 * static_cast<uint32_t>(i);
                uint32_t or0 = outer_base + 2 * static_cast<uint32_t>(i) + 1;
                uint32_t ol1 = outer_base + 2 * next;
                uint32_t or1 = outer_base + 2 * next + 1;

                // Left fringe quad
                out_indices.push_back(l0);
                out_indices.push_back(ol0);
                out_indices.push_back(ol1);

                out_indices.push_back(l0);
                out_indices.push_back(ol1);
                out_indices.push_back(l1);

                // Right fringe quad
                out_indices.push_back(r0);
                out_indices.push_back(r1);
                out_indices.push_back(or1);

                out_indices.push_back(r0);
                out_indices.push_back(or1);
                out_indices.push_back(or0);
            }

            if (!contour.closed && N >= 2) {
                // Start cap fringe
                Point d_start = pts[1] - pts[0];
                float l_start = std::hypot(d_start.x, d_start.y);
                Point dir_start = (l_start > 0.0001f) ? Point(-d_start.x / l_start, -d_start.y / l_start) : Point(0.0f, 0.0f);
                Point cap_ol0 = (pts[0] + scale_pt(miter_offsets[0], scale)) + scale_pt(dir_start, fringe);
                Point cap_or0 = (pts[0] - scale_pt(miter_offsets[0], scale)) + scale_pt(dir_start, fringe);
                uint32_t c_idx0 = static_cast<uint32_t>(out_verts.size());
                out_verts.emplace_back(apply_ts(transform, cap_ol0), color, 0.0f);
                out_verts.emplace_back(apply_ts(transform, cap_or0), color, 0.0f);

                out_indices.push_back(inner_base);
                out_indices.push_back(inner_base + 1);
                out_indices.push_back(c_idx0 + 1);

                out_indices.push_back(inner_base);
                out_indices.push_back(c_idx0 + 1);
                out_indices.push_back(c_idx0);

                // End cap fringe
                Point d_end = pts[N - 1] - pts[N - 2];
                float l_end = std::hypot(d_end.x, d_end.y);
                Point dir_end = (l_end > 0.0001f) ? Point(d_end.x / l_end, d_end.y / l_end) : Point(0.0f, 0.0f);
                uint32_t last_inner_l = inner_base + 2 * static_cast<uint32_t>(N - 1);
                uint32_t last_inner_r = last_inner_l + 1;
                Point cap_ol_end = (pts[N - 1] + scale_pt(miter_offsets[N - 1], scale)) + scale_pt(dir_end, fringe);
                Point cap_or_end = (pts[N - 1] - scale_pt(miter_offsets[N - 1], scale)) + scale_pt(dir_end, fringe);
                uint32_t c_idx_end = static_cast<uint32_t>(out_verts.size());
                out_verts.emplace_back(apply_ts(transform, cap_ol_end), color, 0.0f);
                out_verts.emplace_back(apply_ts(transform, cap_or_end), color, 0.0f);

                out_indices.push_back(last_inner_l);
                out_indices.push_back(c_idx_end);
                out_indices.push_back(c_idx_end + 1);

                out_indices.push_back(last_inner_l);
                out_indices.push_back(c_idx_end + 1);
                out_indices.push_back(last_inner_r);
            }
        }
    }
}

void GpuTessellator::tessellate_mesh(
    std::vector<GpuVertex>& out_verts,
    std::vector<uint32_t>& out_indices,
    const nisaba::Vertices& mesh,
    const nisaba::Transform& transform
) {
    if (!mesh.is_valid()) return;

    uint32_t base_idx = static_cast<uint32_t>(out_verts.size());
    size_t num_verts = mesh.positions.size();

    for (size_t i = 0; i < num_verts; ++i) {
        Point p = apply_ts(transform, mesh.positions[i]);
        Color c = (i < mesh.colors.size()) ? mesh.colors[i] : Color::WHITE;
        Point uv = (i < mesh.tex_coords.size()) ? mesh.tex_coords[i] : Point(0.0f, 0.0f);
        out_verts.emplace_back(p, uv, c, 1.0f);
    }

    size_t tri_count = mesh.triangle_count();
    for (size_t t = 0; t < tri_count; ++t) {
        auto [i0, i1, i2] = mesh.get_triangle_indices(t);
        out_indices.push_back(base_idx + static_cast<uint32_t>(i0));
        out_indices.push_back(base_idx + static_cast<uint32_t>(i1));
        out_indices.push_back(base_idx + static_cast<uint32_t>(i2));
    }
}

void GpuTessellator::tessellate_textured_rect(
    std::vector<GpuVertex>& out_verts,
    std::vector<uint32_t>& out_indices,
    const nisaba::Rect& dst_rect,
    const nisaba::Rect& src_uv_rect,
    nisaba::Color tint_color,
    const nisaba::Transform& transform
) {
    uint32_t base_idx = static_cast<uint32_t>(out_verts.size());

    Point p0 = apply_ts(transform, dst_rect.left(), dst_rect.top());
    Point p1 = apply_ts(transform, dst_rect.right(), dst_rect.top());
    Point p2 = apply_ts(transform, dst_rect.right(), dst_rect.bottom());
    Point p3 = apply_ts(transform, dst_rect.left(), dst_rect.bottom());

    Point uv0(src_uv_rect.left(), src_uv_rect.top());
    Point uv1(src_uv_rect.right(), src_uv_rect.top());
    Point uv2(src_uv_rect.right(), src_uv_rect.bottom());
    Point uv3(src_uv_rect.left(), src_uv_rect.bottom());

    out_verts.emplace_back(p0, uv0, tint_color, 1.0f);
    out_verts.emplace_back(p1, uv1, tint_color, 1.0f);
    out_verts.emplace_back(p2, uv2, tint_color, 1.0f);
    out_verts.emplace_back(p3, uv3, tint_color, 1.0f);

    out_indices.push_back(base_idx + 0);
    out_indices.push_back(base_idx + 1);
    out_indices.push_back(base_idx + 2);

    out_indices.push_back(base_idx + 0);
    out_indices.push_back(base_idx + 2);
    out_indices.push_back(base_idx + 3);
}

} // namespace nisaba::gpu
