/// @file json.cpp
/// @brief Sovereign, zero-dependency RFC 8259 JSON parser implementation in C++20.
/// Part of the Nisaba Graphics & Animation Engine.

#include "nisaba/json/json.hpp"
#include <fstream>
#include <sstream>
#include <cctype>
#include <cstdlib>
#include <cstring>

namespace nisaba::json {

namespace {

class JsonParser {
public:
    explicit JsonParser(std::string_view input) noexcept
        : src_(input), pos_(0) {}

    std::optional<JsonValue> parse() {
        skip_whitespace();
        if (pos_ >= src_.size()) return std::nullopt;

        auto val = parse_value();
        skip_whitespace();
        return val;
    }

private:
    void skip_whitespace() noexcept {
        while (pos_ < src_.size()) {
            char c = src_[pos_];
            if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
                ++pos_;
            } else if (c == '/' && pos_ + 1 < src_.size()) {
                // Support C-style line and block comments (often in test JSON files)
                if (src_[pos_ + 1] == '/') {
                    pos_ += 2;
                    while (pos_ < src_.size() && src_[pos_] != '\n') ++pos_;
                } else if (src_[pos_ + 1] == '*') {
                    pos_ += 2;
                    while (pos_ + 1 < src_.size() && !(src_[pos_] == '*' && src_[pos_ + 1] == '/')) ++pos_;
                    if (pos_ + 1 < src_.size()) pos_ += 2;
                } else {
                    break;
                }
            } else {
                break;
            }
        }
    }

    char peek() const noexcept {
        return (pos_ < src_.size()) ? src_[pos_] : '\0';
    }

    char get() noexcept {
        return (pos_ < src_.size()) ? src_[pos_++] : '\0';
    }

    bool match(char expected) noexcept {
        if (peek() == expected) {
            ++pos_;
            return true;
        }
        return false;
    }

    std::optional<JsonValue> parse_value() {
        skip_whitespace();
        if (pos_ >= src_.size()) return std::nullopt;

        char c = peek();
        if (c == '{') return parse_object();
        if (c == '[') return parse_array();
        if (c == '"') return parse_string();
        if (c == 't' || c == 'f') return parse_boolean();
        if (c == 'n') return parse_null();
        if (c == '-' || (c >= '0' && c <= '9')) return parse_number();

        return std::nullopt;
    }

    std::optional<JsonValue> parse_null() {
        if (src_.substr(pos_, 4) == "null") {
            pos_ += 4;
            return JsonValue(nullptr);
        }
        return std::nullopt;
    }

    std::optional<JsonValue> parse_boolean() {
        if (src_.substr(pos_, 4) == "true") {
            pos_ += 4;
            return JsonValue(true);
        }
        if (src_.substr(pos_, 5) == "false") {
            pos_ += 5;
            return JsonValue(false);
        }
        return std::nullopt;
    }

    std::optional<JsonValue> parse_number() {
        size_t start = pos_;
        if (peek() == '-') ++pos_;

        if (peek() == '0') {
            ++pos_;
        } else if (peek() >= '1' && peek() <= '9') {
            while (peek() >= '0' && peek() <= '9') ++pos_;
        } else {
            return std::nullopt;
        }

        if (peek() == '.') {
            ++pos_;
            while (peek() >= '0' && peek() <= '9') ++pos_;
        }

        if (peek() == 'e' || peek() == 'E') {
            ++pos_;
            if (peek() == '+' || peek() == '-') ++pos_;
            while (peek() >= '0' && peek() <= '9') ++pos_;
        }

        std::string num_str(src_.substr(start, pos_ - start));
        char* end = nullptr;
        double val = std::strtod(num_str.c_str(), &end);
        if (end == num_str.c_str()) return std::nullopt;
        return JsonValue(val);
    }

    static void append_utf8(std::string& out, uint32_t codepoint) {
        if (codepoint <= 0x7F) {
            out.push_back(static_cast<char>(codepoint));
        } else if (codepoint <= 0x7FF) {
            out.push_back(static_cast<char>(0xC0 | ((codepoint >> 6) & 0x1F)));
            out.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
        } else if (codepoint <= 0xFFFF) {
            out.push_back(static_cast<char>(0xE0 | ((codepoint >> 12) & 0x0F)));
            out.push_back(static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F)));
            out.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
        } else if (codepoint <= 0x10FFFF) {
            out.push_back(static_cast<char>(0xF0 | ((codepoint >> 18) & 0x07)));
            out.push_back(static_cast<char>(0x80 | ((codepoint >> 12) & 0x3F)));
            out.push_back(static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F)));
            out.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
        }
    }

    std::optional<std::string> parse_string_raw() {
        if (get() != '"') return std::nullopt;

        std::string res;
        res.reserve(32);

        while (pos_ < src_.size()) {
            char c = get();
            if (c == '"') {
                return res;
            }
            if (c == '\\') {
                if (pos_ >= src_.size()) return std::nullopt;
                char esc = get();
                switch (esc) {
                    case '"':  res.push_back('"'); break;
                    case '\\': res.push_back('\\'); break;
                    case '/':  res.push_back('/'); break;
                    case 'b':  res.push_back('\b'); break;
                    case 'f':  res.push_back('\f'); break;
                    case 'n':  res.push_back('\n'); break;
                    case 'r':  res.push_back('\r'); break;
                    case 't':  res.push_back('\t'); break;
                    case 'u': {
                        if (pos_ + 4 > src_.size()) return std::nullopt;
                        uint32_t code = 0;
                        for (int i = 0; i < 4; ++i) {
                            char h = get();
                            code <<= 4;
                            if (h >= '0' && h <= '9') code |= (h - '0');
                            else if (h >= 'a' && h <= 'f') code |= (h - 'a' + 10);
                            else if (h >= 'A' && h <= 'F') code |= (h - 'A' + 10);
                            else return std::nullopt;
                        }
                        append_utf8(res, code);
                        break;
                    }
                    default:
                        res.push_back(esc);
                        break;
                }
            } else {
                res.push_back(c);
            }
        }
        return std::nullopt;
    }

    std::optional<JsonValue> parse_string() {
        auto str = parse_string_raw();
        if (!str) return std::nullopt;
        return JsonValue(std::move(*str));
    }

    std::optional<JsonValue> parse_array() {
        if (get() != '[') return std::nullopt;

        JsonArray arr;
        skip_whitespace();
        if (match(']')) return JsonValue(std::move(arr));

        while (true) {
            auto elem = parse_value();
            if (!elem) return std::nullopt;
            arr.push_back(std::move(*elem));

            skip_whitespace();
            if (match(']')) break;
            if (!match(',')) return std::nullopt;
            skip_whitespace();
            if (match(']')) break; // Tolerate trailing comma
        }

        return JsonValue(std::move(arr));
    }

    std::optional<JsonValue> parse_object() {
        if (get() != '{') return std::nullopt;

        JsonObject obj;
        skip_whitespace();
        if (match('}')) return JsonValue(std::move(obj));

        while (true) {
            skip_whitespace();
            if (peek() != '"') return std::nullopt;

            auto key = parse_string_raw();
            if (!key) return std::nullopt;

            skip_whitespace();
            if (!match(':')) return std::nullopt;

            auto val = parse_value();
            if (!val) return std::nullopt;

            obj.emplace(std::move(*key), std::move(*val));

            skip_whitespace();
            if (match('}')) break;
            if (!match(',')) return std::nullopt;
            skip_whitespace();
            if (match('}')) break; // Tolerate trailing comma
        }

        return JsonValue(std::move(obj));
    }

    std::string_view src_;
    size_t pos_;
};

} // anonymous namespace

std::optional<JsonValue> JsonValue::parse(std::string_view json_str) {
    JsonParser parser(json_str);
    return parser.parse();
}

std::optional<JsonValue> JsonValue::parse_file(const std::string& filepath) {
    std::ifstream file(filepath, std::ios::in | std::ios::binary);
    if (!file.is_open()) return std::nullopt;

    std::ostringstream ss;
    ss << file.rdbuf();
    std::string content = ss.str();
    return parse(content);
}

} // namespace nisaba::json
