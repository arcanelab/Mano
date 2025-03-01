#pragma once

#include <Token.h>

#include <string_view>
#include <vector>

namespace Arcanelab::Mano
{
    class Lexer
    {
    public:
        Lexer(std::string_view source)
            : source(source), offset(0), line(1), column(1)
        {
        }

        std::vector<Token> Tokenize();

    private:
        std::string_view source;
        size_t offset;
        size_t line;
        size_t column;

        bool IsAtEnd() const;
        bool IsKeyword(std::string_view text);
        bool IsOperator(char c);
        bool IsPunctuation(char c);
        char Advance();
        char Peek() const;
        Token CreateToken(TokenType tokenType, std::string_view lexeme);
        Token NextToken();
        Token ScanIdentifier();
        Token ScanNumber();
        Token ScanOperator();
        Token ScanPunctuation();
        Token ScanString();
        void SkipWhitespace();
    }; // Lexer
} // namespace
