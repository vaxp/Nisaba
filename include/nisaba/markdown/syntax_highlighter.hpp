#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <cstdint>
#include "nisaba/color/color.hpp"
#include "nisaba/markdown/style.hpp"

namespace nisaba::markdown {

/// Classification of a syntax token.
enum class TokenType : uint8_t {
    PlainText,
    Keyword,
    Type,
    StringLiteral,
    NumberLiteral,
    Comment,
    Preprocessor,
    Operator,
    Punctuation
};

/// A single lexical token in a code block.
struct SyntaxToken {
    TokenType type{TokenType::PlainText};
    std::string text{};

    SyntaxToken() = default;
    SyntaxToken(TokenType t, std::string txt) : type(t), text(std::move(txt)) {}
};

/// High-speed sovereign lexical analyzer and syntax highlighter.
class SyntaxHighlighter {
public:
    /// Tokenizes a single line of code according to the target programming language.
    /// `in_multi_comment` tracks multi-line comment state across consecutive lines.
    static std::vector<SyntaxToken> tokenize_line(
        std::string_view line,
        std::string_view language,
        bool& in_multi_comment
    );

    /// Maps a token type to its designated theme color in MarkdownStyle.
    static Color token_color(TokenType type, const MarkdownStyle& style);
};

} // namespace nisaba::markdown
