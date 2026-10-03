#pragma once

#include <cstdint>
#include <cstddef>
#include <algorithm>
#include "nisaba/text/buffer.hpp"

namespace nisaba::text {

/// Affinity specifies whether a cursor associates with the run before or after it at boundaries.
enum class Affinity : uint8_t {
    Before,
    After
};

/// Represents an interactive caret/cursor position within a text buffer.
struct Cursor {
    size_t line{0};
    size_t index{0}; // Byte index in buffer line text
    Affinity affinity{Affinity::Before};

    constexpr Cursor() noexcept = default;
    constexpr Cursor(size_t l, size_t i, Affinity aff = Affinity::Before) noexcept
        : line(l), index(i), affinity(aff) {}

    constexpr bool operator==(const Cursor& o) const noexcept {
        return line == o.line && index == o.index && affinity == o.affinity;
    }

    constexpr bool operator<(const Cursor& o) const noexcept {
        if (line != o.line) return line < o.line;
        return index < o.index;
    }

    constexpr bool operator<=(const Cursor& o) const noexcept {
        return (*this < o) || (*this == o);
    }
};

/// Represents a highlighted text selection span between two cursor positions.
struct Selection {
    Cursor start{};
    Cursor end{};

    constexpr Selection() noexcept = default;
    constexpr Selection(Cursor s, Cursor e) noexcept : start(s), end(e) {}

    bool is_empty() const noexcept {
        return start.line == end.line && start.index == end.index;
    }

    Selection normalized() const noexcept {
        if (end < start) {
            return Selection(end, start);
        }
        return *this;
    }
};

/// Navigation motions for moving the cursor.
enum class Motion : uint8_t {
    Left,
    Right,
    Up,
    Down,
    Home,
    End
};

/// Cursor operations and interactive hit-testing.
class CursorController {
public:
    /// Computes the closest cursor position to physical coordinates (x, y).
    static Cursor hit_test(
        const Buffer& buffer,
        float x,
        float y,
        float origin_x = 0.0f,
        float origin_y = 0.0f
    );

    /// Moves cursor according to Motion navigation rules.
    static Cursor move(
        const Buffer& buffer,
        Cursor current,
        Motion motion
    );
};

} // namespace nisaba::text
