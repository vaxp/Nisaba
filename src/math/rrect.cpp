#include "nisaba/math/rrect.hpp"
#include "nisaba/path/path_builder.hpp"
#include <algorithm>

namespace nisaba {

Path RRect::to_path() const {
    auto p = PathBuilder::from_rrect(*this);
    return p.value_or(Path{});
}

bool RRect::contains(Point p) const noexcept {
    if (rect.is_empty()) return false;
    if (p.x < rect.left() || p.x > rect.right() || p.y < rect.top() || p.y > rect.bottom()) {
        return false;
    }

    BorderRadius cr = radii.constrained_for_size(rect.width(), rect.height());
    if (cr.is_zero()) return true;

    // Top-Left corner test
    if (cr.top_left > 0.0f && p.x < rect.left() + cr.top_left && p.y < rect.top() + cr.top_left) {
        float dx = p.x - (rect.left() + cr.top_left);
        float dy = p.y - (rect.top() + cr.top_left);
        if (dx * dx + dy * dy > cr.top_left * cr.top_left) return false;
    }

    // Top-Right corner test
    if (cr.top_right > 0.0f && p.x > rect.right() - cr.top_right && p.y < rect.top() + cr.top_right) {
        float dx = p.x - (rect.right() - cr.top_right);
        float dy = p.y - (rect.top() + cr.top_right);
        if (dx * dx + dy * dy > cr.top_right * cr.top_right) return false;
    }

    // Bottom-Right corner test
    if (cr.bottom_right > 0.0f && p.x > rect.right() - cr.bottom_right && p.y > rect.bottom() - cr.bottom_right) {
        float dx = p.x - (rect.right() - cr.bottom_right);
        float dy = p.y - (rect.bottom() - cr.bottom_right);
        if (dx * dx + dy * dy > cr.bottom_right * cr.bottom_right) return false;
    }

    // Bottom-Left corner test
    if (cr.bottom_left > 0.0f && p.x < rect.left() + cr.bottom_left && p.y > rect.bottom() - cr.bottom_left) {
        float dx = p.x - (rect.left() + cr.bottom_left);
        float dy = p.y - (rect.bottom() - cr.bottom_left);
        if (dx * dx + dy * dy > cr.bottom_left * cr.bottom_left) return false;
    }

    return true;
}

RRect RRect::deflate(float dx, float dy) const noexcept {
    float nl = rect.left() + dx;
    float nt = rect.top() + dy;
    float nr = rect.right() - dx;
    float nb = rect.bottom() - dy;
    if (nl >= nr || nt >= nb) {
        return RRect(Rect{}, BorderRadius{});
    }
    auto nr_opt = Rect::from_ltrb(nl, nt, nr, nb);
    if (!nr_opt) return RRect(Rect{}, BorderRadius{});

    // Scale radii proportionally with dimension reduction
    float sx = (rect.width() > 1e-4f) ? std::max(0.0f, 1.0f - (2.0f * dx) / rect.width()) : 1.0f;
    float sy = (rect.height() > 1e-4f) ? std::max(0.0f, 1.0f - (2.0f * dy) / rect.height()) : 1.0f;
    float s = std::min(sx, sy);
    return RRect(*nr_opt, radii.scaled_by(s).non_negative());
}

RRect RRect::inset(float d_left, float d_top, float d_right, float d_bottom) const noexcept {
    float nl = rect.left() + d_left;
    float nt = rect.top() + d_top;
    float nr = rect.right() - d_right;
    float nb = rect.bottom() - d_bottom;
    if (nl >= nr || nt >= nb) {
        return RRect(Rect{}, BorderRadius{});
    }
    auto nr_opt = Rect::from_ltrb(nl, nt, nr, nb);
    if (!nr_opt) return RRect(Rect{}, BorderRadius{});
    return RRect(*nr_opt, radii);
}

} // namespace nisaba
