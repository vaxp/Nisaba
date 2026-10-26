#include "nisaba/text/cursor.hpp"
#include "nisaba/text/bidi.hpp"
#include <cmath>

namespace nisaba::text {

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
        if (glyph.is_mark) continue;
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
                index = Bidi::prev_grapheme_cluster(text, index);
            } else if (line > 0) {
                line--;
                index = lines[line].text().size();
            }
            break;

        case Motion::Right:
            if (index < text.size()) {
                index = Bidi::next_grapheme_cluster(text, index);
            } else if (line + 1 < lines.size()) {
                line++;
                index = 0;
            }
            break;

        case Motion::WordLeft:
            if (index > 0) {
                index = Bidi::prev_word_boundary(text, index);
            } else if (line > 0) {
                line--;
                index = lines[line].text().size();
            }
            break;

        case Motion::WordRight:
            if (index < text.size()) {
                index = Bidi::next_word_boundary(text, index);
            } else if (line + 1 < lines.size()) {
                line++;
                index = 0;
            }
            break;

        case Motion::Up: {
            auto runs = buffer.layout_runs();
            int current_run_idx = -1;
            float cursor_x = 0.0f;
            for (size_t r = 0; r < runs.size(); ++r) {
                if (runs[r].line_i != line) continue;
                if (runs[r].glyphs.empty()) {
                    current_run_idx = static_cast<int>(r);
                    break;
                }
                size_t min_idx = runs[r].glyphs.front().start;
                size_t max_idx = runs[r].glyphs.front().end;
                for (const auto& g : runs[r].glyphs) {
                    min_idx = std::min(min_idx, g.start);
                    max_idx = std::max(max_idx, g.end);
                    if (index == g.start) cursor_x = g.x;
                    else if (index == g.end) cursor_x = g.x + g.w;
                }
                if (index >= min_idx && index <= max_idx) {
                    current_run_idx = static_cast<int>(r);
                    break;
                }
            }
            if (current_run_idx > 0 && runs[current_run_idx - 1].line_i == line) {
                const auto& prev_run = runs[current_run_idx - 1];
                return hit_test(buffer, cursor_x, prev_run.line_top + prev_run.line_height * 0.5f);
            }
            if (line > 0) {
                line--;
                index = std::min(index, lines[line].text().size());
            }
            break;
        }

        case Motion::Down: {
            auto runs = buffer.layout_runs();
            int current_run_idx = -1;
            float cursor_x = 0.0f;
            for (size_t r = 0; r < runs.size(); ++r) {
                if (runs[r].line_i != line) continue;
                if (runs[r].glyphs.empty()) {
                    current_run_idx = static_cast<int>(r);
                    break;
                }
                size_t min_idx = runs[r].glyphs.front().start;
                size_t max_idx = runs[r].glyphs.front().end;
                for (const auto& g : runs[r].glyphs) {
                    min_idx = std::min(min_idx, g.start);
                    max_idx = std::max(max_idx, g.end);
                    if (index == g.start) cursor_x = g.x;
                    else if (index == g.end) cursor_x = g.x + g.w;
                }
                if (index >= min_idx && index <= max_idx) {
                    current_run_idx = static_cast<int>(r);
                    break;
                }
            }
            if (current_run_idx >= 0 && static_cast<size_t>(current_run_idx + 1) < runs.size() &&
                runs[current_run_idx + 1].line_i == line) {
                const auto& next_run = runs[current_run_idx + 1];
                return hit_test(buffer, cursor_x, next_run.line_top + next_run.line_height * 0.5f);
            }
            if (line + 1 < lines.size()) {
                line++;
                index = std::min(index, lines[line].text().size());
            }
            break;
        }

        case Motion::Home:
            index = 0;
            break;

        case Motion::End:
            index = text.size();
            break;

        case Motion::DocumentStart:
            line = 0;
            index = 0;
            break;

        case Motion::DocumentEnd:
            line = lines.size() - 1;
            index = lines[line].text().size();
            break;
    }

    return Cursor{line, index};
}

Rect CursorController::cursor_rect(
    const Buffer& buffer,
    Cursor cursor,
    float origin_x,
    float origin_y,
    float width
) {
    float line_h = buffer.metrics().line_height;
    auto make_rect = [](float x, float y, float w, float h) -> Rect {
        return Rect::from_xywh(x, y, std::max(0.5f, w), std::max(0.5f, h)).value_or(Rect());
    };

    auto runs = buffer.layout_runs();
    if (runs.empty()) {
        return make_rect(origin_x, origin_y, width, line_h);
    }

    const LayoutRun* target_run = nullptr;
    for (const auto& run : runs) {
        if (run.line_i != cursor.line) continue;
        target_run = &run;
        for (const auto& g : run.glyphs) {
            if (g.is_mark) continue;
            bool is_rtl = (g.level & 1) != 0;
            if (cursor.index == g.start) {
                float cx = is_rtl ? (g.x + g.w) : g.x;
                return make_rect(origin_x + cx, origin_y + run.line_top, width, run.line_height);
            }
            if (cursor.index == g.end) {
                float cx = is_rtl ? g.x : (g.x + g.w);
                return make_rect(origin_x + cx, origin_y + run.line_top, width, run.line_height);
            }
            if (cursor.index > g.start && cursor.index < g.end) {
                float ratio = static_cast<float>(cursor.index - g.start) / static_cast<float>(g.end - g.start);
                float cx = is_rtl ? (g.x + g.w * (1.0f - ratio)) : (g.x + g.w * ratio);
                return make_rect(origin_x + cx, origin_y + run.line_top, width, run.line_height);
            }
        }
    }

    if (target_run) {
        float cx = 0.0f;
        if (!target_run->glyphs.empty()) {
            cx = target_run->glyphs.back().x + target_run->glyphs.back().w;
        }
        return make_rect(origin_x + cx, origin_y + target_run->line_top, width, target_run->line_height);
    }

    return make_rect(origin_x, origin_y, width, line_h);
}

std::vector<Rect> CursorController::selection_rects(
    const Buffer& buffer,
    Selection selection,
    float origin_x,
    float origin_y
) {
    std::vector<Rect> rects;
    if (selection.is_empty()) {
        return rects;
    }

    Selection norm = selection.normalized();
    auto runs = buffer.layout_runs();
    if (runs.empty()) {
        return rects;
    }

    auto push_rect = [&](float x, float y, float w, float h) {
        if (auto r = Rect::from_xywh(x, y, std::max(0.5f, w), std::max(0.5f, h))) {
            rects.push_back(*r);
        }
    };

    for (const auto& run : runs) {
        if (run.line_i < norm.start.line || run.line_i > norm.end.line) {
            continue;
        }

        size_t sel_start = (run.line_i == norm.start.line) ? norm.start.index : 0;
        size_t sel_end = (run.line_i == norm.end.line) ? norm.end.index : SIZE_MAX;

        if (run.glyphs.empty()) {
            if (run.line_i >= norm.start.line && run.line_i < norm.end.line) {
                push_rect(origin_x, origin_y + run.line_top, 4.0f, run.line_height);
            }
            continue;
        }

        bool in_span = false;
        float span_x0 = 0.0f;
        float span_x1 = 0.0f;

        for (const auto& g : run.glyphs) {
            if (g.is_mark) continue;
            bool selected = (g.end > sel_start && g.start < sel_end);
            if (selected) {
                float gx0 = g.x;
                float gx1 = g.x + g.w;
                if (!in_span) {
                    in_span = true;
                    span_x0 = gx0;
                    span_x1 = gx1;
                } else {
                    if (std::abs(gx0 - span_x1) <= 1.5f) {
                        span_x1 = std::max(span_x1, gx1);
                    } else if (std::abs(span_x0 - gx1) <= 1.5f) {
                        span_x0 = std::min(span_x0, gx0);
                    } else {
                        push_rect(origin_x + span_x0, origin_y + run.line_top, span_x1 - span_x0, run.line_height);
                        span_x0 = gx0;
                        span_x1 = gx1;
                    }
                }
            } else if (in_span) {
                push_rect(origin_x + span_x0, origin_y + run.line_top, span_x1 - span_x0, run.line_height);
                in_span = false;
            }
        }

        if (in_span) {
            push_rect(origin_x + span_x0, origin_y + run.line_top, span_x1 - span_x0, run.line_height);
        }
    }

    return rects;
}

} // namespace nisaba::text
