#pragma once

#include <optional>
#include <memory>
#include "nisaba/types.hpp"
#include "nisaba/math/point.hpp"
#include "nisaba/math/transform.hpp"
#include "nisaba/path/path.hpp"
#include "nisaba/path/dash.hpp"

namespace nisaba {

enum class LineCap : uint8_t {
    Butt,
    Round,
    Square,
};

enum class LineJoin : uint8_t {
    Miter,
    MiterClip,
    Round,
    Bevel,
};

struct Stroke {
    float width{1.0f};
    float miter_limit{4.0f};
    LineCap line_cap{LineCap::Butt};
    LineJoin line_join{LineJoin::Miter};
    std::optional<StrokeDash> dash{std::nullopt};

    constexpr Stroke() noexcept = default;
    constexpr explicit Stroke(float w) noexcept : width(w) {}
};

class PathStroker {
public:
    PathStroker();
    ~PathStroker();

    PathStroker(PathStroker&&) noexcept;
    PathStroker& operator=(PathStroker&&) noexcept;

    static float compute_resolution_scale(const Transform& ts) noexcept;

    std::optional<Path> stroke(const Path& path, const Stroke& stroke, float resolution_scale = 1.0f);
    
    // Strokes the path into an internally cached Path object, avoiding allocations.
    // The returned pointer is valid until the next call to stroke_fast on this stroker.
    const Path* stroke_fast(const Path& path, const Stroke& stroke, float resolution_scale = 1.0f);

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

inline std::optional<Path> stroke_path(const Path& path, const Stroke& stroke, float resolution_scale = 1.0f) {
    PathStroker stroker;
    return stroker.stroke(path, stroke, resolution_scale);
}

} // namespace nisaba
