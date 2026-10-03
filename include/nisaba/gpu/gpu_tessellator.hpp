#pragma once

#include <vector>
#include <cstdint>
#include "nisaba/gpu/gpu_types.hpp"
#include "nisaba/math/rect.hpp"
#include "nisaba/math/point.hpp"
#include "nisaba/math/transform.hpp"
#include "nisaba/path/path.hpp"
#include "nisaba/path/stroker.hpp"
#include "nisaba/mesh/mesh.hpp"

namespace nisaba::gpu {

class GpuTessellator {
public:
    GpuTessellator() = default;

    /// Tessellates an axis-aligned or transformed rectangle with subpixel AA fringe.
    static void tessellate_rect(
        std::vector<GpuVertex>& out_verts,
        std::vector<uint32_t>& out_indices,
        const nisaba::Rect& rect,
        nisaba::Color color,
        const nisaba::Transform& transform = nisaba::Transform(),
        bool anti_alias = true
    );

    /// Tessellates a rounded rectangle with corner arcs and AA fringe.
    static void tessellate_round_rect(
        std::vector<GpuVertex>& out_verts,
        std::vector<uint32_t>& out_indices,
        const nisaba::Rect& rect,
        float rx, float ry,
        nisaba::Color color,
        const nisaba::Transform& transform = nisaba::Transform(),
        bool anti_alias = true
    );

    /// Tessellates a circle with radial fan and AA fringe ring.
    static void tessellate_circle(
        std::vector<GpuVertex>& out_verts,
        std::vector<uint32_t>& out_indices,
        float cx, float cy, float radius,
        nisaba::Color color,
        const nisaba::Transform& transform = nisaba::Transform(),
        bool anti_alias = true
    );

    /// Tessellates a stroked line between p0 and p1.
    static void tessellate_line(
        std::vector<GpuVertex>& out_verts,
        std::vector<uint32_t>& out_indices,
        nisaba::Point p0, nisaba::Point p1,
        const nisaba::Stroke& stroke,
        nisaba::Color color,
        const nisaba::Transform& transform = nisaba::Transform(),
        bool anti_alias = true
    );

    /// Tessellates an arbitrary vector path into triangles with fill rule and AA.
    static void tessellate_path(
        std::vector<GpuVertex>& out_verts,
        std::vector<uint32_t>& out_indices,
        const nisaba::Path& path,
        nisaba::FillRule fill_rule,
        nisaba::Color color,
        const nisaba::Transform& transform = nisaba::Transform(),
        bool anti_alias = true
    );

    /// Tessellates a stroked path into wide ribbon triangle strips.
    static void tessellate_stroke(
        std::vector<GpuVertex>& out_verts,
        std::vector<uint32_t>& out_indices,
        const nisaba::Path& path,
        const nisaba::Stroke& stroke,
        nisaba::Color color,
        const nisaba::Transform& transform = nisaba::Transform(),
        bool anti_alias = true
    );

    /// Directly streams a Nisaba 2D vertex mesh into GPU geometry.
    static void tessellate_mesh(
        std::vector<GpuVertex>& out_verts,
        std::vector<uint32_t>& out_indices,
        const nisaba::Vertices& mesh,
        const nisaba::Transform& transform = nisaba::Transform()
    );

    /// Tessellates a textured rectangle with UV coordinates.
    static void tessellate_textured_rect(
        std::vector<GpuVertex>& out_verts,
        std::vector<uint32_t>& out_indices,
        const nisaba::Rect& dst_rect,
        const nisaba::Rect& src_uv_rect,
        nisaba::Color tint_color = nisaba::Color::WHITE,
        const nisaba::Transform& transform = nisaba::Transform()
    );
};

} // namespace nisaba::gpu
