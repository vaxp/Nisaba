#pragma once

#include <cstdint>
#include <optional>
#include <variant>
#include <vector>
#include "nisaba/math/point.hpp"
#include "nisaba/raster/fixed_point.hpp"

namespace nisaba {

inline constexpr int32_t MAX_COEFF_SHIFT = 6;

struct LineEdge {
    uint32_t prev{0};
    uint32_t next{0};

    FDot16 x{0};
    FDot16 dx{0};
    int32_t first_y{0};
    int32_t last_y{0};
    int8_t winding{1}; // 1 or -1
    int8_t pad[7]{0};

    static std::optional<LineEdge> create(Point p0, Point p1, int32_t shift) noexcept;

    bool is_vertical() const noexcept {
        return dx == 0;
    }

    bool update(FDot16 x0, FDot16 y0, FDot16 x1, FDot16 y1) noexcept;
};

struct QuadraticEdge {
    LineEdge line;
    int8_t curve_count{0};
    uint8_t curve_shift{0};
    FDot16 qx{0};
    FDot16 qy{0};
    FDot16 qdx{0};
    FDot16 qdy{0};
    FDot16 qddx{0};
    FDot16 qddy{0};
    FDot16 q_last_x{0};
    FDot16 q_last_y{0};

    static std::optional<QuadraticEdge> create(const Point* points, int32_t shift) noexcept;
    bool update() noexcept;
};

struct CubicEdge {
    LineEdge line;
    int8_t curve_count{0};
    uint8_t curve_shift{0};
    uint8_t dshift{0};
    FDot16 cx{0};
    FDot16 cy{0};
    FDot16 cdx{0};
    FDot16 cdy{0};
    FDot16 cddx{0};
    FDot16 cddy{0};
    FDot16 cdddx{0};
    FDot16 cdddy{0};
    FDot16 c_last_x{0};
    FDot16 c_last_y{0};

    static std::optional<CubicEdge> create(const Point* points, int32_t shift) noexcept;
    bool update() noexcept;
};

class Edge {
public:
    enum class Type { Line, Quadratic, Cubic };

    Edge(LineEdge le) : line_(le), type_(Type::Line) {}
    Edge(QuadraticEdge qe) : line_(qe.line), type_(Type::Quadratic) {}
    Edge(CubicEdge ce) : line_(ce.line), type_(Type::Cubic) {}

    Type type() const noexcept {
        return type_;
    }

    const LineEdge& as_line() const noexcept {
        return line_;
    }

    LineEdge& as_line_mut() noexcept {
        return line_;
    }

    QuadraticEdge* as_quadratic() noexcept {
        return nullptr;
    }

    CubicEdge* as_cubic() noexcept {
        return nullptr;
    }

    const LineEdge* operator->() const noexcept { return &line_; }
    LineEdge* operator->() noexcept { return &line_; }

private:
    LineEdge line_{};
    Type type_{Type::Line};
};

} // namespace nisaba
