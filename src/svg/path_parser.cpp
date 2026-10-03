#include "nisaba/svg/path_parser.hpp"
#include <string>
#include <cmath>
#include <cstdlib>
#include <cctype>
#include <algorithm>

namespace nisaba::svg {

namespace {

constexpr float PI = 3.14159265358979323846f;
constexpr float TWO_PI = 2.0f * PI;

class PathLexer {
public:
    explicit PathLexer(std::string_view str) : str_(str), pos_(0) {}

    bool has_more() noexcept {
        skip_spaces_and_commas();
        return pos_ < str_.size();
    }

    char peek() noexcept {
        skip_spaces_and_commas();
        return (pos_ < str_.size()) ? str_[pos_] : '\0';
    }

    static bool is_command_char(char c) noexcept {
        switch (c) {
            case 'M': case 'm':
            case 'L': case 'l':
            case 'H': case 'h':
            case 'V': case 'v':
            case 'C': case 'c':
            case 'S': case 's':
            case 'Q': case 'q':
            case 'T': case 't':
            case 'A': case 'a':
            case 'Z': case 'z':
                return true;
            default:
                return false;
        }
    }

    char next_cmd() noexcept {
        skip_spaces_and_commas();
        if (pos_ < str_.size() && is_command_char(str_[pos_])) {
            return str_[pos_++];
        }
        return '\0';
    }

    bool next_number(float& out) noexcept {
        skip_spaces_and_commas();
        if (pos_ >= str_.size()) return false;

        size_t start = pos_;
        if (str_[pos_] == '+' || str_[pos_] == '-') {
            pos_++;
        }
        bool has_digits = false;
        while (pos_ < str_.size() && (str_[pos_] >= '0' && str_[pos_] <= '9')) {
            has_digits = true;
            pos_++;
        }
        if (pos_ < str_.size() && str_[pos_] == '.') {
            pos_++;
            while (pos_ < str_.size() && (str_[pos_] >= '0' && str_[pos_] <= '9')) {
                has_digits = true;
                pos_++;
            }
        }
        if (!has_digits) {
            pos_ = start;
            return false;
        }
        if (pos_ < str_.size() && (str_[pos_] == 'e' || str_[pos_] == 'E')) {
            size_t exp_start = pos_;
            pos_++;
            if (pos_ < str_.size() && (str_[pos_] == '+' || str_[pos_] == '-')) {
                pos_++;
            }
            bool exp_digits = false;
            while (pos_ < str_.size() && (str_[pos_] >= '0' && str_[pos_] <= '9')) {
                exp_digits = true;
                pos_++;
            }
            if (!exp_digits) {
                pos_ = exp_start;
            }
        }

        std::string_view num_str = str_.substr(start, pos_ - start);
        std::string s(num_str);
        char* end = nullptr;
        out = std::strtof(s.c_str(), &end);
        return true;
    }

    bool next_flag(bool& out) noexcept {
        skip_spaces_and_commas();
        if (pos_ < str_.size() && (str_[pos_] == '0' || str_[pos_] == '1')) {
            out = (str_[pos_++] == '1');
            return true;
        }
        return false;
    }

private:
    void skip_spaces_and_commas() noexcept {
        while (pos_ < str_.size()) {
            char c = str_[pos_];
            if (c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == ',') {
                pos_++;
            } else {
                break;
            }
        }
    }

    std::string_view str_;
    size_t pos_{0};
};

float vector_angle(float ux, float uy, float vx, float vy) noexcept {
    float dot = ux * vx + uy * vy;
    float len = std::sqrt(ux * ux + uy * uy) * std::sqrt(vx * vx + vy * vy);
    if (len <= 0.0f) return 0.0f;
    float cos_val = std::clamp(dot / len, -1.0f, 1.0f);
    float ang = std::acos(cos_val);
    if (ux * vy - uy * vx < 0.0f) ang = -ang;
    return ang;
}

void emit_arc_as_cubics(
    PathBuilder& builder,
    float x1, float y1,
    float rx, float ry,
    float phi_deg,
    bool large_arc,
    bool sweep,
    float x2, float y2
) {
    if (std::abs(x1 - x2) < 1e-6f && std::abs(y1 - y2) < 1e-6f) return;
    rx = std::abs(rx);
    ry = std::abs(ry);
    if (rx < 1e-6f || ry < 1e-6f) {
        builder.line_to(x2, y2);
        return;
    }

    float phi = phi_deg * (PI / 180.0f);
    float cos_phi = std::cos(phi);
    float sin_phi = std::sin(phi);

    // Step 1: Compute (x1', y1')
    float dx = (x1 - x2) * 0.5f;
    float dy = (y1 - y2) * 0.5f;
    float x1_p = cos_phi * dx + sin_phi * dy;
    float y1_p = -sin_phi * dx + cos_phi * dy;

    // Check radii scaling
    float lambda = (x1_p * x1_p) / (rx * rx) + (y1_p * y1_p) / (ry * ry);
    if (lambda > 1.0f) {
        float sqrt_l = std::sqrt(lambda);
        rx *= sqrt_l;
        ry *= sqrt_l;
    }

    // Step 2: Compute (cx', cy')
    float rx_sq = rx * rx;
    float ry_sq = ry * ry;
    float x1_p_sq = x1_p * x1_p;
    float y1_p_sq = y1_p * y1_p;

    float denom = rx_sq * y1_p_sq + ry_sq * x1_p_sq;
    float num = rx_sq * ry_sq - denom;
    float sq = (num < 0.0f || denom <= 0.0f) ? 0.0f : (num / denom);
    float coef = (large_arc == sweep ? -1.0f : 1.0f) * std::sqrt(sq);

    float cx_p = coef * ((rx * y1_p) / ry);
    float cy_p = -coef * ((ry * x1_p) / rx);

    // Step 3: Compute (cx, cy)
    float cx = cos_phi * cx_p - sin_phi * cy_p + (x1 + x2) * 0.5f;
    float cy = sin_phi * cx_p + cos_phi * cy_p + (y1 + y2) * 0.5f;

    // Step 4: Compute theta1 and delta_theta
    float vx1 = (x1_p - cx_p) / rx;
    float vy1 = (y1_p - cy_p) / ry;
    float vx2 = (-x1_p - cx_p) / rx;
    float vy2 = (-y1_p - cy_p) / ry;

    float theta1 = vector_angle(1.0f, 0.0f, vx1, vy1);
    float delta_theta = vector_angle(vx1, vy1, vx2, vy2);

    if (!sweep && delta_theta > 0.0f) {
        delta_theta -= TWO_PI;
    } else if (sweep && delta_theta < 0.0f) {
        delta_theta += TWO_PI;
    }

    // Step 5: Subdivide into segments <= PI/2
    int segments = static_cast<int>(std::ceil(std::abs(delta_theta) / (PI * 0.5f)));
    if (segments < 1) segments = 1;
    float d_theta = delta_theta / static_cast<float>(segments);
    float alpha = (4.0f / 3.0f) * std::tan(d_theta * 0.25f);

    auto map_pt = [cx, cy, rx, ry, cos_phi, sin_phi](float u, float v) -> Point {
        return Point::from_xy(
            cx + rx * cos_phi * u - ry * sin_phi * v,
            cy + rx * sin_phi * u + ry * cos_phi * v
        );
    };

    for (int i = 0; i < segments; ++i) {
        float t1 = theta1 + static_cast<float>(i) * d_theta;
        float t2 = t1 + d_theta;

        float cos_t1 = std::cos(t1);
        float sin_t1 = std::sin(t1);
        float cos_t2 = std::cos(t2);
        float sin_t2 = std::sin(t2);

        // Unit circle points
        float p1_u = cos_t1;
        float p1_v = sin_t1;
        float p2_u = cos_t2;
        float p2_v = sin_t2;

        // Tangent derivatives
        float cp1_u = p1_u - alpha * sin_t1;
        float cp1_v = p1_v + alpha * cos_t1;
        float cp2_u = p2_u + alpha * sin_t2;
        float cp2_v = p2_v - alpha * cos_t2;

        Point p_cp1 = map_pt(cp1_u, cp1_v);
        Point p_cp2 = map_pt(cp2_u, cp2_v);
        Point p_end = (i == segments - 1) ? Point::from_xy(x2, y2) : map_pt(p2_u, p2_v);

        builder.cubic_to(p_cp1.x, p_cp1.y, p_cp2.x, p_cp2.y, p_end.x, p_end.y);
    }
}

} // namespace

bool PathParser::parse_into(std::string_view d, PathBuilder& builder) {
    PathLexer lex(d);

    float cur_x = 0.0f;
    float cur_y = 0.0f;
    float start_x = 0.0f;
    float start_y = 0.0f;
    float last_ctrl_x = 0.0f;
    float last_ctrl_y = 0.0f;

    char cmd = '\0';
    char prev_cmd = '\0';

    while (lex.has_more()) {
        char next_c = lex.peek();
        if (PathLexer::is_command_char(next_c)) {
            cmd = lex.next_cmd();
        } else if (cmd == '\0') {
            return false;
        } else if (cmd == 'M') {
            cmd = 'L'; // Subsequent coordinates to M become L
        } else if (cmd == 'm') {
            cmd = 'l'; // Subsequent coordinates to m become l
        }

        switch (cmd) {
            case 'M': case 'm': {
                float x = 0.0f, y = 0.0f;
                if (!lex.next_number(x) || !lex.next_number(y)) return false;
                if (cmd == 'm') {
                    x += cur_x;
                    y += cur_y;
                }
                builder.move_to(x, y);
                start_x = cur_x = x;
                start_y = cur_y = y;
                last_ctrl_x = x;
                last_ctrl_y = y;
                prev_cmd = cmd;
                break;
            }

            case 'L': case 'l': {
                float x = 0.0f, y = 0.0f;
                if (!lex.next_number(x) || !lex.next_number(y)) return false;
                if (cmd == 'l') {
                    x += cur_x;
                    y += cur_y;
                }
                builder.line_to(x, y);
                cur_x = x;
                cur_y = y;
                last_ctrl_x = x;
                last_ctrl_y = y;
                prev_cmd = cmd;
                break;
            }

            case 'H': case 'h': {
                float x = 0.0f;
                if (!lex.next_number(x)) return false;
                if (cmd == 'h') x += cur_x;
                builder.line_to(x, cur_y);
                cur_x = x;
                last_ctrl_x = cur_x;
                last_ctrl_y = cur_y;
                prev_cmd = cmd;
                break;
            }

            case 'V': case 'v': {
                float y = 0.0f;
                if (!lex.next_number(y)) return false;
                if (cmd == 'v') y += cur_y;
                builder.line_to(cur_x, y);
                cur_y = y;
                last_ctrl_x = cur_x;
                last_ctrl_y = cur_y;
                prev_cmd = cmd;
                break;
            }

            case 'C': case 'c': {
                float x1 = 0.0f, y1 = 0.0f, x2 = 0.0f, y2 = 0.0f, x = 0.0f, y = 0.0f;
                if (!lex.next_number(x1) || !lex.next_number(y1) ||
                    !lex.next_number(x2) || !lex.next_number(y2) ||
                    !lex.next_number(x) || !lex.next_number(y)) return false;

                if (cmd == 'c') {
                    x1 += cur_x; y1 += cur_y;
                    x2 += cur_x; y2 += cur_y;
                    x += cur_x;  y += cur_y;
                }
                builder.cubic_to(x1, y1, x2, y2, x, y);
                last_ctrl_x = x2;
                last_ctrl_y = y2;
                cur_x = x;
                cur_y = y;
                prev_cmd = cmd;
                break;
            }

            case 'S': case 's': {
                float x2 = 0.0f, y2 = 0.0f, x = 0.0f, y = 0.0f;
                if (!lex.next_number(x2) || !lex.next_number(y2) ||
                    !lex.next_number(x) || !lex.next_number(y)) return false;

                if (cmd == 's') {
                    x2 += cur_x; y2 += cur_y;
                    x += cur_x;  y += cur_y;
                }

                float x1 = cur_x;
                float y1 = cur_y;
                if (prev_cmd == 'C' || prev_cmd == 'c' || prev_cmd == 'S' || prev_cmd == 's') {
                    x1 = 2.0f * cur_x - last_ctrl_x;
                    y1 = 2.0f * cur_y - last_ctrl_y;
                }

                builder.cubic_to(x1, y1, x2, y2, x, y);
                last_ctrl_x = x2;
                last_ctrl_y = y2;
                cur_x = x;
                cur_y = y;
                prev_cmd = cmd;
                break;
            }

            case 'Q': case 'q': {
                float x1 = 0.0f, y1 = 0.0f, x = 0.0f, y = 0.0f;
                if (!lex.next_number(x1) || !lex.next_number(y1) ||
                    !lex.next_number(x) || !lex.next_number(y)) return false;

                if (cmd == 'q') {
                    x1 += cur_x; y1 += cur_y;
                    x += cur_x;  y += cur_y;
                }
                builder.quad_to(x1, y1, x, y);
                last_ctrl_x = x1;
                last_ctrl_y = y1;
                cur_x = x;
                cur_y = y;
                prev_cmd = cmd;
                break;
            }

            case 'T': case 't': {
                float x = 0.0f, y = 0.0f;
                if (!lex.next_number(x) || !lex.next_number(y)) return false;
                if (cmd == 't') {
                    x += cur_x;
                    y += cur_y;
                }

                float x1 = cur_x;
                float y1 = cur_y;
                if (prev_cmd == 'Q' || prev_cmd == 'q' || prev_cmd == 'T' || prev_cmd == 't') {
                    x1 = 2.0f * cur_x - last_ctrl_x;
                    y1 = 2.0f * cur_y - last_ctrl_y;
                }

                builder.quad_to(x1, y1, x, y);
                last_ctrl_x = x1;
                last_ctrl_y = y1;
                cur_x = x;
                cur_y = y;
                prev_cmd = cmd;
                break;
            }

            case 'A': case 'a': {
                float rx = 0.0f, ry = 0.0f, angle = 0.0f;
                bool large_arc = false, sweep = false;
                float x = 0.0f, y = 0.0f;

                if (!lex.next_number(rx) || !lex.next_number(ry) || !lex.next_number(angle) ||
                    !lex.next_flag(large_arc) || !lex.next_flag(sweep) ||
                    !lex.next_number(x) || !lex.next_number(y)) return false;

                if (cmd == 'a') {
                    x += cur_x;
                    y += cur_y;
                }

                emit_arc_as_cubics(builder, cur_x, cur_y, rx, ry, angle, large_arc, sweep, x, y);
                cur_x = x;
                cur_y = y;
                last_ctrl_x = cur_x;
                last_ctrl_y = cur_y;
                prev_cmd = cmd;
                break;
            }

            case 'Z': case 'z': {
                builder.close();
                cur_x = start_x;
                cur_y = start_y;
                last_ctrl_x = cur_x;
                last_ctrl_y = cur_y;
                prev_cmd = cmd;
                break;
            }

            default:
                return false;
        }
    }

    return true;
}

std::optional<Path> PathParser::parse(std::string_view d) {
    PathBuilder builder;
    if (!parse_into(d, builder)) {
        return std::nullopt;
    }
    return builder.finish();
}

} // namespace nisaba::svg
