#include "nisaba/text/cursor.hpp"

namespace nisaba::text {

namespace {

size_t prev_utf8_char(std::string_view text, size_t index) noexcept {
    if (index == 0) return 0;
    size_t i = index - 1;
    while (i > 0 && (static_cast<uint8_t>(text[i]) & 0xC0) == 0x80) {
        --i;
    }
    return i;
}

size_t next_utf8_char(std::string_view text, size_t index) noexcept {
    if (index >= text.size()) return text.size();
    size_t i = index + 1;
    while (i < text.size() && (static_cast<uint8_t>(text[i]) & 0xC0) == 0x80) {
        ++i;
    }
    return i;
}

} // namespace

Cursor CursorController::hit_test(
    const Buffer& buffer,
    float x,
    float y,
    float origin_x,
    float origin_y
) {
    if (buffer.lines().empty()) {
        return Cursor{0, 0};
    }

    float local_x = x - origin_x;
    float local_y = y - origin_y;

    auto runs = buffer.layout_runs();
    if (runs.empty()) {
        return Cursor{0, 0};
    }

    const LayoutRun* best_run = &runs[0];
    for (const auto& run : runs) {
        if (local_y >= run.line_top && local_y < run.line_top + run.line_height) {
            best_run = &run;
            break;
        }
        if (local_y >= run.line_top) {
            best_run = &run;
        }
    }

    if (best_run->glyphs.empty()) {
        return Cursor{best_run->line_i, 0};
    }

    if (local_x <= best_run->glyphs.front().x) {
        return Cursor{best_run->line_i, best_run->glyphs.front().start};
    }

    if (local_x >= best_run->glyphs.back().x + best_run->glyphs.back().w) {
        return Cursor{best_run->line_i, best_run->glyphs.back().end};
    }

    for (const auto& glyph : best_run->glyphs) {
        if (local_x >= glyph.x && local_x <= glyph.x + glyph.w) {
            if (local_x < glyph.x + glyph.w * 0.5f) {
                return Cursor{best_run->line_i, glyph.start};
            } else {
                return Cursor{best_run->line_i, glyph.end};
            }
        }
    }

    return Cursor{best_run->line_i, 0};
}

Cursor CursorController::move(
    const Buffer& buffer,
    Cursor current,
    Motion motion
) {
    const auto& lines = buffer.lines();
    if (lines.empty()) {
        return Cursor{0, 0};
    }

    size_t line = std::min(current.line, lines.size() - 1);
    std::string_view text = lines[line].text();
    size_t index = std::min(current.index, text.size());

    switch (motion) {
        case Motion::Left:
            if (index > 0) {
                index = prev_utf8_char(text, index);
            } else if (line > 0) {
                line--;
                index = lines[line].text().size();
            }
            break;

        case Motion::Right:
            if (index < text.size()) {
                index = next_utf8_char(text, index);
            } else if (line + 1 < lines.size()) {
                line++;
                index = 0;
            }
            break;

        case Motion::Up:
            if (line > 0) {
                line--;
                index = std::min(index, lines[line].text().size());
            }
            break;

        case Motion::Down:
            if (line + 1 < lines.size()) {
                line++;
                index = std::min(index, lines[line].text().size());
            }
            break;

        case Motion::Home:
            index = 0;
            break;

        case Motion::End:
            index = text.size();
            break;
    }

    return Cursor{line, index};
}

} // namespace nisaba::text
