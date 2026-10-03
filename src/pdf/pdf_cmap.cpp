#include "nisaba/pdf/pdf_cmap.hpp"
#include <vector>
#include <string>
#include <string_view>
#include <cctype>
#include <algorithm>

namespace nisaba::pdf {

namespace {

inline bool is_space(char c) {
    return c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '\f' || c == '\0';
}

} // namespace

std::string PdfCMap::unicode_to_utf8(uint32_t cp) {
    std::string s;
    if (cp <= 0x7F) {
        s += static_cast<char>(cp);
    } else if (cp <= 0x7FF) {
        s += static_cast<char>(0xC0 | (cp >> 6));
        s += static_cast<char>(0x80 | (cp & 0x3F));
    } else if (cp <= 0xFFFF) {
        s += static_cast<char>(0xE0 | (cp >> 12));
        s += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
        s += static_cast<char>(0x80 | (cp & 0x3F));
    } else if (cp <= 0x10FFFF) {
        s += static_cast<char>(0xF0 | (cp >> 18));
        s += static_cast<char>(0x80 | ((cp >> 12) & 0x3F));
        s += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
        s += static_cast<char>(0x80 | (cp & 0x3F));
    }
    return s;
}

uint32_t PdfCMap::parse_hex_u32(std::string_view hex) {
    if (!hex.empty() && hex.front() == '<') hex.remove_prefix(1);
    if (!hex.empty() && hex.back() == '>') hex.remove_suffix(1);
    uint32_t val = 0;
    for (char c : hex) {
        val <<= 4;
        if (c >= '0' && c <= '9') val |= (c - '0');
        else if (c >= 'a' && c <= 'f') val |= (c - 'a' + 10);
        else if (c >= 'A' && c <= 'F') val |= (c - 'A' + 10);
    }
    return val;
}

std::string PdfCMap::hex_to_utf8(std::string_view hex) {
    if (!hex.empty() && hex.front() == '<') hex.remove_prefix(1);
    if (!hex.empty() && hex.back() == '>') hex.remove_suffix(1);

    auto hex_nibble = [](char c) -> int {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'a' && c <= 'f') return c - 'a' + 10;
        if (c >= 'A' && c <= 'F') return c - 'A' + 10;
        return 0;
    };

    std::vector<uint16_t> utf16;
    for (size_t i = 0; i + 3 < hex.size(); i += 4) {
        uint16_t val = static_cast<uint16_t>(
            (hex_nibble(hex[i]) << 12) |
            (hex_nibble(hex[i + 1]) << 8) |
            (hex_nibble(hex[i + 2]) << 4) |
            hex_nibble(hex[i + 3])
        );
        utf16.push_back(val);
    }
    if (utf16.empty() && hex.size() == 2) {
        uint16_t val = static_cast<uint16_t>((hex_nibble(hex[0]) << 4) | hex_nibble(hex[1]));
        utf16.push_back(val);
    }

    std::string res;
    for (size_t i = 0; i < utf16.size(); ++i) {
        uint32_t cp = utf16[i];
        if (cp >= 0xD800 && cp <= 0xDBFF && i + 1 < utf16.size()) {
            uint32_t trail = utf16[i + 1];
            if (trail >= 0xDC00 && trail <= 0xDFFF) {
                cp = 0x10000 + ((cp - 0xD800) << 10) + (trail - 0xDC00);
                i++;
            }
        }
        res += unicode_to_utf8(cp);
    }
    return res;
}

std::shared_ptr<PdfCMap> PdfCMap::parse(std::span<const uint8_t> data) {
    auto cmap = std::make_shared<PdfCMap>();
    if (data.empty()) return cmap;

    std::string_view stream(reinterpret_cast<const char*>(data.data()), data.size());
    size_t i = 0;

    auto next_token = [&]() -> std::string {
        while (i < stream.size()) {
            while (i < stream.size() && is_space(stream[i])) i++;
            if (i >= stream.size()) return "";

            // Comment
            if (stream[i] == '%') {
                while (i < stream.size() && stream[i] != '\r' && stream[i] != '\n') i++;
                continue;
            }

            // Hex string <...>
            if (stream[i] == '<') {
                if (i + 1 < stream.size() && stream[i + 1] == '<') {
                    i += 2;
                    return "<<";
                }
                size_t start = i++;
                while (i < stream.size() && stream[i] != '>') i++;
                if (i < stream.size() && stream[i] == '>') i++;
                return std::string(stream.substr(start, i - start));
            }

            // Dict close >>
            if (stream[i] == '>') {
                if (i + 1 < stream.size() && stream[i + 1] == '>') {
                    i += 2;
                    return ">>";
                }
                i++;
                return ">";
            }

            // Array [ or ]
            if (stream[i] == '[' || stream[i] == ']') {
                return std::string(1, stream[i++]);
            }

            // Literal word / operator / number
            size_t start = i;
            while (i < stream.size() && !is_space(stream[i]) &&
                   stream[i] != '<' && stream[i] != '>' &&
                   stream[i] != '[' && stream[i] != ']' &&
                   stream[i] != '/' && stream[i] != '%') {
                i++;
            }
            if (start == i) {
                // Single special char (e.g. '/')
                i++;
                continue;
            }
            return std::string(stream.substr(start, i - start));
        }
        return "";
    };

    while (i < stream.size()) {
        std::string tok = next_token();
        if (tok.empty()) break;

        if (tok == "begincodespacerange") {
            // Read until endcodespacerange
            while (i < stream.size()) {
                std::string t1 = next_token();
                if (t1.empty() || t1 == "endcodespacerange") break;
                std::string t2 = next_token();
                if (t2.empty() || t2 == "endcodespacerange") break;
                // If hex length >= 4 digits without <>: indicates 2-byte code space
                std::string_view h1 = t1;
                if (!h1.empty() && h1.front() == '<') h1.remove_prefix(1);
                if (!h1.empty() && h1.back() == '>') h1.remove_suffix(1);
                if (h1.size() >= 4) {
                    cmap->is_2byte_ = true;
                }
            }
        } else if (tok == "beginbfchar") {
            while (i < stream.size()) {
                std::string src_tok = next_token();
                if (src_tok.empty() || src_tok == "endbfchar") break;
                std::string dst_tok = next_token();
                if (dst_tok.empty() || dst_tok == "endbfchar") break;

                uint32_t src_code = parse_hex_u32(src_tok);
                std::string dst_utf8 = hex_to_utf8(dst_tok);
                cmap->map_[src_code] = dst_utf8;
            }
        } else if (tok == "beginbfrange") {
            while (i < stream.size()) {
                std::string s1_tok = next_token();
                if (s1_tok.empty() || s1_tok == "endbfrange") break;
                std::string s2_tok = next_token();
                if (s2_tok.empty() || s2_tok == "endbfrange") break;
                std::string dst_tok = next_token();
                if (dst_tok.empty() || dst_tok == "endbfrange") break;

                uint32_t s1 = parse_hex_u32(s1_tok);
                uint32_t s2 = parse_hex_u32(s2_tok);

                if (dst_tok == "[") {
                    // Array format: [ <hex1> <hex2> ... ]
                    uint32_t cur = s1;
                    while (i < stream.size()) {
                        std::string arr_elem = next_token();
                        if (arr_elem.empty() || arr_elem == "]") break;
                        if (cur <= s2) {
                            cmap->map_[cur] = hex_to_utf8(arr_elem);
                            cur++;
                        }
                    }
                } else {
                    // Linear range format: <dstStart>
                    uint32_t d = parse_hex_u32(dst_tok);
                    for (uint32_t c = s1; c <= s2; ++c, ++d) {
                        cmap->map_[c] = unicode_to_utf8(d);
                    }
                }
            }
        }
    }

    return cmap;
}

std::string PdfCMap::map_code(uint32_t code) const {
    auto it = map_.find(code);
    if (it != map_.end()) {
        return it->second;
    }
    if (code == 0x0020 || code == 0x0003) {
        return " ";
    }
    if (code >= 32 && code < 127) {
        return std::string(1, static_cast<char>(code));
    }
    return "";
}

bool PdfCMap::has_code(uint32_t code) const noexcept {
    return map_.find(code) != map_.end();
}

std::string PdfCMap::to_utf8(std::string_view raw) const {
    if (raw.empty()) return "";

    std::string result;
    result.reserve(raw.size());

    // Handle 2-byte CID string when configured or length is even with null bytes
    bool use_2byte = is_2byte_;
    if (!use_2byte && raw.size() >= 2 && raw.size() % 2 == 0) {
        // Probe if alternate bytes are 0x00 (typical 2-byte Identity-H encoding)
        size_t zero_count = 0;
        for (size_t k = 0; k < raw.size(); k += 2) {
            if (static_cast<uint8_t>(raw[k]) == 0x00) zero_count++;
        }
        if (zero_count * 2 >= raw.size() / 2) {
            use_2byte = true;
        }
    }

    if (use_2byte && raw.size() >= 2) {
        for (size_t k = 0; k + 1 < raw.size(); k += 2) {
            uint32_t code = (static_cast<uint8_t>(raw[k]) << 8) | static_cast<uint8_t>(raw[k + 1]);
            auto it = map_.find(code);
            if (it != map_.end()) {
                result += it->second;
            } else if (code == 0x0020 || code == 0x0003) {
                result += ' ';
            } else if (code >= 32 && code < 127) {
                result += static_cast<char>(code);
            } else {
                result += ' ';
            }
        }
        if (raw.size() % 2 != 0) {
            uint32_t code = static_cast<uint8_t>(raw.back());
            auto it = map_.find(code);
            if (it != map_.end()) result += it->second;
            else if (code >= 32 && code < 127) result += static_cast<char>(code);
        }
    } else {
        // 1-byte lookup
        for (char ch : raw) {
            uint32_t code = static_cast<uint8_t>(ch);
            auto it = map_.find(code);
            if (it != map_.end()) {
                result += it->second;
            } else if (code >= 32 || code == '\t' || code == '\n' || code == '\r') {
                result += ch;
            } else {
                result += ' ';
            }
        }
    }

    return result;
}

} // namespace nisaba::pdf
