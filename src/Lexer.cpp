#include <Lexer.h>
#include <cctype>
#include <string>

namespace Arcanelab::Mano
{
    Token Lexer::NextToken()
    {
        SkipWhitespace();
        if (IsAtEnd())
            return Token{ TokenType::EndOfFile, "", line, column };

        // Record token start position.
        size_t tokenLine = line;
        size_t tokenColumn = column;
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

        // Unrecognized character: record its starting position before consuming.
        tokenLine = line;
        tokenColumn = column;
        char unknown = Advance();
        std::string_view lexeme = source.substr(offset - 1, 1);
        return Token{ TokenType::Unknown, lexeme, tokenLine, tokenColumn };
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
            // Check for single-line comment starting with "//"
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

    // The helper function is now local to each scan routine (using token start positions).
    Token Lexer::ScanIdentifier()
    {
        size_t tokenLine = line;
        size_t tokenColumn = column;
        size_t start = offset;
        while (!IsAtEnd() && (std::isalnum(Peek()) || Peek() == '_'))
            Advance();
        std::string_view text = source.substr(start, offset - start);
        TokenType tokenType = IsKeyword(text) ? TokenType::Keyword : TokenType::Identifier;
        return Token{ tokenType, text, tokenLine, tokenColumn };
    }

    bool Lexer::IsKeyword(std::string_view text)
    {
        return (text == "var" || text == "fun" || text == "class" || text == "enum" ||
            text == "if" || text == "else" || text == "for" || text == "while" ||
            text == "break" || text == "continue" || text == "return" || text == "let" ||
            text == "int" || text == "uint" || text == "float" || text == "bool" ||
            text == "string" || text == "switch" || text == "case" || text == "default" ||
            text == "const");
    }

    Token Lexer::ScanNumber()
    {
        size_t tokenLine = line;
        size_t tokenColumn = column;
        size_t start = offset;
        while (!IsAtEnd() && std::isdigit(Peek()))
            Advance();

        // Check for a fractional part.
        if (!IsAtEnd() && Peek() == '.')
        {
            Advance();
            while (!IsAtEnd() && std::isdigit(Peek()))
                Advance();
        }
        std::string_view text = source.substr(start, offset - start);
        return Token{ TokenType::Number, text, tokenLine, tokenColumn };
    }

    Token Lexer::ScanString()
    {
        size_t tokenLine = line;
        size_t tokenColumn = column;
        char quote = Advance(); // Consume the opening "
        size_t start = offset;
        while (!IsAtEnd() && Peek() != quote)
        {
            if (Peek() == '\\')
            {
                // Consume the escape character and the escaped character.
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
        // Exclude the quotes from the lexeme.
        std::string_view text = source.substr(start, offset - start - 1);
        return Token{ TokenType::String, text, tokenLine, tokenColumn };
    }

    bool Lexer::IsOperator(char c)
    {
        return (c == '+' || c == '-' || c == '*' || c == '/' ||
            c == '=' || c == '!' || c == '<' || c == '>' ||
            c == '&' || c == '|' || c == '^' || c == '%');
    }

    Token Lexer::ScanOperator()
    {
        size_t tokenLine = line;
        size_t tokenColumn = column;
        size_t start = offset;
        char first = Advance(); // Consume the first operator character.
        if (!IsAtEnd())
        {
            char next = Peek();
            // Handle double-character operators.
            if ((first == '=' && next == '=') ||
                (first == '!' && next == '=') ||
                (first == '<' && next == '=') ||
                (first == '>' && next == '=') ||
                (first == '&' && next == '&') ||
                (first == '|' && next == '|') ||
                (first == '<' && next == '<') ||
                (first == '>' && next == '>'))
            {
                Advance();
            }
        }
        std::string_view text = source.substr(start, offset - start);
        return Token{ TokenType::Operator, text, tokenLine, tokenColumn };
    }

    bool Lexer::IsPunctuation(char c)
    {
        return (c == '(' || c == ')' || c == '{' || c == '}' ||
            c == '[' || c == ']' || c == ',' || c == ':' ||
            c == ';' || c == '.');
    }

    Token Lexer::ScanPunctuation()
    {
        size_t tokenLine = line;
        size_t tokenColumn = column;
        size_t start = offset;
        Advance();
        std::string_view text = source.substr(start, offset - start);
        return Token{ TokenType::Punctuation, text, tokenLine, tokenColumn };
    }
}
