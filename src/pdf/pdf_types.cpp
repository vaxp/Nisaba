#include "nisaba/pdf/pdf_types.hpp"
#include <sstream>
#include <iomanip>

namespace nisaba::pdf {

const PdfValue* PdfValue::find(std::string_view key) const noexcept {
    if (!is_dict()) return nullptr;
    const auto& dict = as_dict();
    auto it = dict.find(std::string(key));
    if (it != dict.end()) {
        return &it->second;
    }
    return nullptr;
}

std::string escape_pdf_string(std::string_view str) {
    std::string out;
    out.reserve(str.size() + 16);
    for (char c : str) {
        switch (c) {
            case '(':
                out += "\\(";
                break;
            case ')':
                out += "\\)";
                break;
            case '\\':
                out += "\\\\";
                break;
            case '\r':
                out += "\\r";
                break;
            case '\n':
                out += "\\n";
                break;
            case '\t':
                out += "\\t";
                break;
            default:
                out += c;
                break;
        }
    }
    return out;
}

void serialize_pdf_value(const PdfValue& value, std::string& out) {
    std::visit([&out](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, std::monostate>) {
            out += "null";
        } else if constexpr (std::is_same_v<T, bool>) {
            out += arg ? "true" : "false";
        } else if constexpr (std::is_same_v<T, int64_t>) {
            out += std::to_string(arg);
        } else if constexpr (std::is_same_v<T, double>) {
            // Format float cleanly
            char buf[64];
            snprintf(buf, sizeof(buf), "%.4f", arg);
            // Trim unnecessary trailing zeros
            std::string_view s(buf);
            while (s.size() > 1 && s.back() == '0') {
                s.remove_suffix(1);
            }
            if (s.size() > 1 && s.back() == '.') {
                s.remove_suffix(1);
            }
            out.append(s);
        } else if constexpr (std::is_same_v<T, std::string>) {
            out += '(';
            out += escape_pdf_string(arg);
            out += ')';
        } else if constexpr (std::is_same_v<T, PdfName>) {
            out += '/';
            out += arg.name;
        } else if constexpr (std::is_same_v<T, PdfRef>) {
            out += std::to_string(arg.id);
            out += ' ';
            out += std::to_string(arg.gen);
            out += " R";
        } else if constexpr (std::is_same_v<T, PdfArray>) {
            out += '[';
            for (size_t i = 0; i < arg.size(); ++i) {
                if (i > 0) out += ' ';
                serialize_pdf_value(arg[i], out);
            }
            out += ']';
        } else if constexpr (std::is_same_v<T, PdfDict>) {
            out += "<<\n";
            for (const auto& [k, v] : arg) {
                out += "  /";
                out += k;
                out += ' ';
                serialize_pdf_value(v, out);
                out += '\n';
            }
            out += ">>";
        }
    }, value.data);
}

} // namespace nisaba::pdf
