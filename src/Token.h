#pragma once

#include <string_view>

namespace Arcanelab::Mano
{
    enum class TokenType
    {
        Identifier,    // e.g., foo
        Keyword,       // e.g., var, fun, class, enum, if, etc.
        Number,        // integer and float literals
        String,        // string literal
        Operator,      // +, -, *, /, etc.
        Punctuation,   // punctuation such as (, ), {, }, :, ;, etc.
        EndOfFile,     // end-of-input marker
        Unknown        // any token that does not match known types
    };

    struct Token
    {
        TokenType type;
        std::string_view lexeme;
        size_t line;
        size_t column;
    };
} // namespace
