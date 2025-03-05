#pragma once

#include <ErrorReporter.h>
#include <Token.h>

#include <string_view>
#include <vector>

namespace Arcanelab::Mano
{
    class Lexer
    {
    public:
        Lexer(std::string_view source, ErrorReporter& errorReporter)
            : source(source), errorReporter(errorReporter),
            offset(0), line(1), column(1)
        {
        }

        std::vector<Token> Tokenize();

    private:
        std::string_view source;
        ErrorReporter& errorReporter;
        size_t offset;
        size_t line;
        size_t column;

        bool IsAtEnd() const;
        bool IsKeyword(std::string_view text) const;
        bool IsOperator(char c) const;
        bool IsPunctuation(char c) const;
        char Advance();
        char Peek() const;
        Token NextToken();
        Token ScanIdentifier();
        Token ScanNumber();
        Token ScanOperator();
        Token ScanPunctuation();
        Token ScanString();
        void SkipWhitespace();
    };
} // namespace Arcanelab::Mano
