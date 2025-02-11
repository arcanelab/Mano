#pragma once

#include <string_view>
#include <vector>

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

    class Lexer
    {
    public:
        Lexer(std::string_view source)
        : source(source), offset(0), line(1), column(1)
        {
        }

        std::vector<Token> Tokenize()
        {
            Token token;
            std::vector<Token> tokens;

            do
            {
                token = NextToken();
                tokens.push_back(token);
            } while (token.type != TokenType::EndOfFile);

            return tokens;
        }
        
    private:
        std::string_view source;
        size_t offset;
        size_t line;
        size_t column;
        
        Token NextToken();
        bool IsAtEnd() const;
        char Peek() const;
        char Advance();
        void SkipWhitespace();
        Token CreateToken(TokenType tokenType, std::string_view lexeme);
        Token ScanIdentifier();
        bool IsKeyword(std::string_view text);
        Token ScanNumber();
        Token ScanString();
        bool IsOperator(char c);
        Token ScanOperator();
        bool IsPunctuation(char c);
        Token ScanPunctuation();
    };
}
