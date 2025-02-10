#include <Lexer.h>
#include <cctype>
#include <string>

namespace Arcanelab::Mano
{
    Token Lexer::NextToken()
    {
        SkipWhitespace();
        if (IsAtEnd())
            return CreateToken(TokenType::EndOfFile, "");

        char current = Peek();

        if (std::isalpha(current) || current == '_')
            return ScanIdentifier();
        if (std::isdigit(current))
            return ScanNumber();
        if (current == '"')
            return ScanString();
        if (IsOperator(current))
            return ScanOperator();
        if (IsPunctuation(current))
            return ScanPunctuation();

        // Unrecognized character: consume it and return an unknown token.
        Advance();
        return CreateToken(TokenType::Unknown, std::string_view(&current, 1));
    }

    bool Lexer::IsAtEnd() const
    {
        return offset >= source.size();
    }

    char Lexer::Peek() const
    {
        return source[offset];
    }

    char Lexer::Advance()
    {
        char c = source[offset++];
        if (c == '\n')
        {
            ++line;
            column = 1;
        }
        else
        {
            ++column;
        }
        return c;
    }

    void Lexer::SkipWhitespace()
    {
        while (!IsAtEnd())
        {
            char c = Peek();
            if (std::isspace(c))
            {
                Advance();
            }
            // If the current and next characters start a single-line comment
            else if (c == '/' && (offset + 1 < source.size()) && source[offset + 1] == '/')
            {
                // Consume the two slashes
                Advance();
                Advance();
                // Skip until the end of the line or source
                while (!IsAtEnd() && Peek() != '\n')
                    Advance();
            }
            else
            {
                break;
            }
        }
    }

    Token Lexer::CreateToken(TokenType tokenType, std::string_view lexeme)
    {
        return Token{ tokenType, lexeme, line, column };
    }

    Token Lexer::ScanIdentifier()
    {
        size_t start = offset;
        while (!IsAtEnd() && (std::isalnum(Peek()) || Peek() == '_'))
            Advance();
        std::string_view text = source.substr(start, offset - start);
        TokenType tokenType = IsKeyword(text) ? TokenType::Keyword : TokenType::Identifier;
        return Token{ tokenType, text, line, column };
    }

    bool Lexer::IsKeyword(std::string_view text)
    {
        return (text == "var" || text == "fun" || text == "class" || text == "enum" ||
            text == "if" || text == "else" || text == "for" || text == "while" ||
            text == "break" || text == "continue" || text == "return" || text == "let" ||
            text == "int" || text == "uint" || text == "float" || text == "bool" || text == "string");
    }

    Token Lexer::ScanNumber()
    {
        size_t start = offset;
        while (!IsAtEnd() && std::isdigit(Peek()))
            Advance();

        // Check for fractional part.
        if (!IsAtEnd() && Peek() == '.')
        {
            Advance();
            while (!IsAtEnd() && std::isdigit(Peek()))
                Advance();
        }
        std::string_view text = source.substr(start, offset - start);
        return Token{ TokenType::Number, text, line, column };
    }

    Token Lexer::ScanString()
    {
        char quote = Advance(); // Consume the opening "
        size_t start = offset;
        while (!IsAtEnd() && Peek() != quote)
        {
            if (Peek() == '\\')
            {
                // Consume escape sequence.
                Advance();
                if (!IsAtEnd())
                    Advance();
            }
            else
            {
                Advance();
            }
        }
        if (!IsAtEnd())
            Advance(); // Consume the closing "
        std::string_view text = source.substr(start, offset - start - 1);
        return Token{ TokenType::String, text, line, column };
    }

    bool Lexer::IsOperator(char c)
    {
        return (c == '+' || c == '-' || c == '*' || c == '/' ||
            c == '=' || c == '!' || c == '<' || c == '>' ||
            c == '&' || c == '|' || c == '%');
    }

    Token Lexer::ScanOperator()
    {
        size_t start = offset;
        char first = Advance(); // Consume the first operator character.
        // Check for double-character operator.
        if (!IsAtEnd())
        {
            char next = Peek();
            if ((first == '=' && next == '=') ||
                (first == '!' && next == '=') ||
                (first == '<' && next == '>'))
            {
                Advance(); // Consume the second character.
            }
        }
        std::string_view text = source.substr(start, offset - start);
        return Token{ TokenType::Operator, text, line, column };
    }

    bool Lexer::IsPunctuation(char c)
    {
        return (c == '(' || c == ')' || c == '{' || c == '}' ||
            c == '[' || c == ']' || c == ',' || c == ':' || c == ';');
    }

    Token Lexer::ScanPunctuation()
    {
        size_t start = offset;
        Advance();
        std::string_view text = source.substr(start, offset - start);
        return Token{ TokenType::Punctuation, text, line, column };
    }
}
