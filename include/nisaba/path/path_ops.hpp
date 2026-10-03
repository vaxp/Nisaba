#pragma once

#include <optional>
#include "nisaba/path/path.hpp"

namespace nisaba {

/// Geometric boolean operation types between two vector paths.
enum class PathOp : uint8_t {
    /// Result is the geometric union of both paths (A ∪ B).
    Union,
    /// Result is the geometric difference: path A minus path B (A \ B).
    Difference,
    /// Result is the geometric intersection of both paths (A ∩ B).
    Intersect,
    /// Result is the symmetric difference (exclusive OR) of both paths (A ⊕ B).
    Xor,
};

/// Computes the boolean operation between two paths.
///
/// Both paths are interpreted according to their respective fill rules.
/// Returns a new Path representing the boundary of the boolean combination,
/// or std::nullopt if the operation fails or produces an invalid state.
std::optional<Path> path_op(
    const Path& path_a,
    const Path& path_b,
    PathOp op,
    FillRule fill_rule_a = FillRule::Winding,
    FillRule fill_rule_b = FillRule::Winding
);

/// Computes the geometric union of two paths (path_a ∪ path_b).
inline std::optional<Path> path_union(const Path& path_a, const Path& path_b) {
    return path_op(path_a, path_b, PathOp::Union);
}

/// Computes the geometric difference (path_a \ path_b).
inline std::optional<Path> path_difference(const Path& path_a, const Path& path_b) {
    return path_op(path_a, path_b, PathOp::Difference);
}

/// Computes the geometric intersection (path_a ∩ path_b).
inline std::optional<Path> path_intersect(const Path& path_a, const Path& path_b) {
    return path_op(path_a, path_b, PathOp::Intersect);
}

/// Computes the symmetric difference (path_a ⊕ path_b).
inline std::optional<Path> path_xor(const Path& path_a, const Path& path_b) {
    return path_op(path_a, path_b, PathOp::Xor);
}

} // namespace nisaba
