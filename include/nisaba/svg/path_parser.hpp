#pragma once

#include <string_view>
#include <optional>
#include "nisaba/path/path.hpp"
#include "nisaba/path/path_builder.hpp"

namespace nisaba::svg {

/// Parses W3C SVG path string syntax (the 'd' attribute) into a nisaba::Path.
/// Fully supports M/m, L/l, H/h, V/v, C/c, S/s, Q/q, T/t, A/a, Z/z.
class PathParser {
public:
    /// Parses an SVG path data string into a new nisaba::Path object.
    static std::optional<Path> parse(std::string_view d);

    /// Parses an SVG path data string and appends the resulting commands directly into an existing PathBuilder.
    static bool parse_into(std::string_view d, PathBuilder& builder);
};

} // namespace nisaba::svg
