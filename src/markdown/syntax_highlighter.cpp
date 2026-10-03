#include "nisaba/markdown/syntax_highlighter.hpp"
#include <cctype>
#include <unordered_set>
#include <algorithm>

namespace nisaba::markdown {

namespace {

inline std::string to_lower(std::string_view s) {
    std::string res;
    res.reserve(s.size());
    for (char c : s) {
        res.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
    }
    return res;
}

// C/C++ Keywords
static const std::unordered_set<std::string_view> cpp_keywords = {
    "auto", "break", "case", "catch", "class", "const", "constexpr", "consteval",
    "continue", "default", "delete", "do", "else", "enum", "explicit", "export",
    "extern", "false", "for", "friend", "goto", "if", "inline", "mutable",
    "namespace", "new", "noexcept", "nullptr", "operator", "override", "final",
    "private", "protected", "public", "return", "sizeof", "static", "static_assert",
    "struct", "switch", "template", "this", "throw", "true", "try", "typedef",
    "typename", "union", "using", "virtual", "volatile", "while", "co_await",
    "co_return", "co_yield", "concept", "requires"
};

// C/C++ Types
static const std::unordered_set<std::string_view> cpp_types = {
    "bool", "char", "char8_t", "char16_t", "char32_t", "double", "float",
    "int", "long", "short", "signed", "unsigned", "void", "size_t", "uint8_t",
    "uint16_t", "uint32_t", "uint64_t", "int8_t", "int16_t", "int32_t", "int64_t",
    "string", "string_view", "vector", "shared_ptr", "unique_ptr", "optional",
    "array", "pair", "tuple", "map", "unordered_map", "set", "unordered_set"
};

// Python Keywords
static const std::unordered_set<std::string_view> py_keywords = {
    "def", "class", "import", "from", "as", "return", "if", "elif", "else",
    "for", "while", "try", "except", "finally", "with", "yield", "lambda",
    "pass", "break", "continue", "in", "is", "not", "and", "or", "True",
    "False", "None", "async", "await", "global", "nonlocal", "assert", "raise"
};

// Rust Keywords
static const std::unordered_set<std::string_view> rust_keywords = {
    "as", "break", "const", "continue", "crate", "else", "enum", "extern",
    "false", "fn", "for", "if", "impl", "in", "let", "loop", "match", "mod",
    "move", "mut", "pub", "ref", "return", "self", "Self", "static", "struct",
    "super", "trait", "true", "type", "unsafe", "use", "where", "while",
    "async", "await", "dyn"
};

// JS/TS Keywords
static const std::unordered_set<std::string_view> js_keywords = {
    "break", "case", "catch", "class", "const", "continue", "debugger",
    "default", "delete", "do", "else", "export", "extends", "finally", "for",
    "function", "if", "import", "in", "instanceof", "new", "return", "super",
    "switch", "this", "throw", "try", "typeof", "var", "void", "while", "with",
    "yield", "let", "static", "enum", "await", "async", "null", "undefined",
    "true", "false", "type", "interface", "from", "of"
};

// Bash Keywords
static const std::unordered_set<std::string_view> bash_keywords = {
    "if", "then", "else", "elif", "fi", "case", "esac", "for", "while",
    "until", "do", "done", "in", "function", "select", "time", "return",
    "exit", "echo", "export", "local", "source", "alias"
};

// CMake Keywords
static const std::unordered_set<std::string_view> cmake_keywords = {
    "project", "cmake_minimum_required", "add_executable", "add_library",
    "target_link_libraries", "target_include_directories", "target_compile_features",
    "find_package", "set", "option", "if", "endif", "else", "elseif",
    "foreach", "endforeach", "while", "endwhile", "function", "endfunction",
    "macro", "endmacro", "message", "include", "install"
};

// Meson Keywords
static const std::unordered_set<std::string_view> meson_keywords = {
    "project", "executable", "library", "shared_library", "static_library",
    "dependency", "declare_dependency", "include_directories", "files",
    "subdir", "subproject", "configure_file", "custom_target", "test",
    "benchmark", "install_headers", "install_data", "set_variable",
    "get_variable", "is_variable", "import", "foreach", "endforeach",
    "if", "elif", "else", "endif"
};

// HTML/XML Tags & Attributes
static const std::unordered_set<std::string_view> html_tags = {
    "html", "head", "body", "title", "div", "span", "p", "a", "img",
    "ul", "ol", "li", "table", "tr", "th", "td", "thead", "tbody",
    "h1", "h2", "h3", "h4", "h5", "h6", "pre", "code", "blockquote",
    "form", "input", "button", "section", "article", "header", "footer",
    "nav", "details", "summary", "kbd", "sub", "sup", "mark", "br", "hr"
};

static const std::unordered_set<std::string_view> html_attrs = {
    "class", "id", "style", "src", "href", "alt", "width", "height",
    "rel", "target", "type", "value", "name", "placeholder", "disabled"
};

// CSS Properties
static const std::unordered_set<std::string_view> css_keywords = {
    "display", "color", "background", "margin", "padding", "border",
    "width", "height", "font-size", "font-family", "position", "top",
    "bottom", "left", "right", "flex", "grid", "opacity", "overflow",
    "none", "block", "inline", "auto", "important", "media", "keyframes"
};

} // namespace

std::vector<SyntaxToken> SyntaxHighlighter::tokenize_line(
    std::string_view line,
    std::string_view language,
    bool& in_multi_comment
) {
    std::vector<SyntaxToken> tokens;
    if (line.empty()) return tokens;

    std::string lang = to_lower(language);
    size_t i = 0;
    size_t len = line.size();

    // 1. If currently inside multi-line comment /* ... */ or <!-- ... -->
    if (in_multi_comment) {
        std::string_view cmt_end_token = (lang == "html" || lang == "xml" || lang == "svg") ? "-->" : "*/";
        size_t end_cmt = line.find(cmt_end_token);
        if (end_cmt == std::string_view::npos) {
            tokens.emplace_back(TokenType::Comment, std::string(line));
            return tokens;
        } else {
            tokens.emplace_back(TokenType::Comment, std::string(line.substr(0, end_cmt + cmt_end_token.size())));
            in_multi_comment = false;
            i = end_cmt + cmt_end_token.size();
        }
    }

    // 2. Preprocessor directive for C/C++ (#include, #define...)
    if (lang == "cpp" || lang == "c" || lang == "cxx" || lang == "hpp" || lang == "h") {
        size_t first_non_space = line.find_first_not_of(" \t");
        if (first_non_space != std::string_view::npos && line[first_non_space] == '#') {
            tokens.emplace_back(TokenType::Preprocessor, std::string(line));
            return tokens;
        }
    }

    while (i < len) {
        char c = line[i];

        // Whitespace
        if (std::isspace(static_cast<unsigned char>(c))) {
            size_t start = i;
            while (i < len && std::isspace(static_cast<unsigned char>(line[i]))) i++;
            tokens.emplace_back(TokenType::PlainText, std::string(line.substr(start, i - start)));
            continue;
        }

        // HTML/XML comment <!-- ... -->
        if ((lang == "html" || lang == "xml" || lang == "svg") && i + 3 < len && line.substr(i, 4) == "<!--") {
            size_t end_cmt = line.find("-->", i + 4);
            if (end_cmt == std::string_view::npos) {
                in_multi_comment = true;
                tokens.emplace_back(TokenType::Comment, std::string(line.substr(i)));
                break;
            } else {
                tokens.emplace_back(TokenType::Comment, std::string(line.substr(i, end_cmt + 3 - i)));
                i = end_cmt + 3;
                continue;
            }
        }

        // Multi-line comment start /*
        if (c == '/' && i + 1 < len && line[i + 1] == '*') {
            size_t end_cmt = line.find("*/", i + 2);
            if (end_cmt == std::string_view::npos) {
                in_multi_comment = true;
                tokens.emplace_back(TokenType::Comment, std::string(line.substr(i)));
                break;
            } else {
                tokens.emplace_back(TokenType::Comment, std::string(line.substr(i, end_cmt + 2 - i)));
                i = end_cmt + 2;
                continue;
            }
        }

        // Single-line comment: // (C/C++, Rust, JS, GLSL)
        if (c == '/' && i + 1 < len && line[i + 1] == '/') {
            tokens.emplace_back(TokenType::Comment, std::string(line.substr(i)));
            break;
        }

        // Single-line comment: # (Python, Bash, CMake, Meson)
        if (c == '#' && (lang == "python" || lang == "py" || lang == "bash" || lang == "sh" || lang == "cmake" || lang == "meson")) {
            tokens.emplace_back(TokenType::Comment, std::string(line.substr(i)));
            break;
        }

        // String literals: "..." or '...'
        if (c == '"' || c == '\'') {
            char quote = c;
            size_t start = i++;
            while (i < len) {
                if (line[i] == '\\' && i + 1 < len) {
                    i += 2;
                } else if (line[i] == quote) {
                    i++;
                    break;
                } else {
                    i++;
                }
            }
            tokens.emplace_back(TokenType::StringLiteral, std::string(line.substr(start, i - start)));
            continue;
        }

        // Numbers: 123, 0x1A, 3.14f...
        if (std::isdigit(static_cast<unsigned char>(c))) {
            size_t start = i++;
            while (i < len && (std::isalnum(static_cast<unsigned char>(line[i])) || line[i] == '.' || line[i] == '_')) {
                i++;
            }
            tokens.emplace_back(TokenType::NumberLiteral, std::string(line.substr(start, i - start)));
            continue;
        }

        // Identifiers: keywords, types, variables
        if (std::isalpha(static_cast<unsigned char>(c)) || c == '_') {
            size_t start = i++;
            while (i < len && (std::isalnum(static_cast<unsigned char>(line[i])) || line[i] == '_')) {
                i++;
            }
            std::string_view word = line.substr(start, i - start);

            TokenType type = TokenType::PlainText;
            if (lang == "cpp" || lang == "c" || lang == "cxx" || lang == "hpp" || lang == "h") {
                if (cpp_keywords.count(word)) type = TokenType::Keyword;
                else if (cpp_types.count(word)) type = TokenType::Type;
            } else if (lang == "python" || lang == "py") {
                if (py_keywords.count(word)) type = TokenType::Keyword;
            } else if (lang == "rust" || lang == "rs") {
                if (rust_keywords.count(word)) type = TokenType::Keyword;
            } else if (lang == "javascript" || lang == "js" || lang == "typescript" || lang == "ts") {
                if (js_keywords.count(word)) type = TokenType::Keyword;
            } else if (lang == "bash" || lang == "sh" || lang == "shell") {
                if (bash_keywords.count(word)) type = TokenType::Keyword;
            } else if (lang == "cmake") {
                if (cmake_keywords.count(word)) type = TokenType::Keyword;
            } else if (lang == "meson") {
                if (meson_keywords.count(word)) type = TokenType::Keyword;
            } else if (lang == "html" || lang == "xml" || lang == "svg") {
                if (html_tags.count(word)) type = TokenType::Keyword;
                else if (html_attrs.count(word)) type = TokenType::Type;
            } else if (lang == "css") {
                if (css_keywords.count(word)) type = TokenType::Keyword;
            } else if (lang == "json") {
                if (word == "true" || word == "false" || word == "null") type = TokenType::Keyword;
            }

            tokens.emplace_back(type, std::string(word));
            continue;
        }

        // Operators & Punctuation
        if (c == '+' || c == '-' || c == '*' || c == '/' || c == '%' || c == '=' ||
            c == '<' || c == '>' || c == '!' || c == '&' || c == '|' || c == '^' || c == '~') {
            tokens.emplace_back(TokenType::Operator, std::string(1, c));
            i++;
            continue;
        }

        if (c == '(' || c == ')' || c == '{' || c == '}' || c == '[' || c == ']' ||
            c == ';' || c == ':' || c == ',' || c == '.') {
            tokens.emplace_back(TokenType::Punctuation, std::string(1, c));
            i++;
            continue;
        }

        // Fallback single character
        tokens.emplace_back(TokenType::PlainText, std::string(1, c));
        i++;
    }

    return tokens;
}

Color SyntaxHighlighter::token_color(TokenType type, const MarkdownStyle& style) {
    switch (type) {
        case TokenType::Keyword:       return style.syn_keyword;
        case TokenType::Type:          return style.syn_type;
        case TokenType::StringLiteral: return style.syn_string;
        case TokenType::NumberLiteral: return style.syn_number;
        case TokenType::Comment:       return style.syn_comment;
        case TokenType::Preprocessor:  return style.syn_preprocessor;
        case TokenType::Operator:      return style.syn_operator;
        case TokenType::Punctuation:   return style.syn_punctuation;
        case TokenType::PlainText:
        default:
            return style.code_text_color;
    }
}

} // namespace nisaba::markdown
