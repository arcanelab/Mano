#include <Lexer.h>

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

    // Checks whether we've reached the end of the source text.
    bool Lexer::IsAtEnd() const
    {
        return offset >= source.size();
    }

    // Returns the current character without advancing.
    char Lexer::Peek() const
    {
        return source[offset];
    }

    // Advances one character and updates position (line/column).
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

    // Skip any whitespaces; the grammar assumes comments are handled here as needed.
    void Lexer::SkipWhitespace()
    {
        while (!IsAtEnd() && std::isspace(Peek()))
            Advance();
    }

    // Creates a Token instance.
    Token Lexer::CreateToken(TokenType tokenType, std::string_view lexeme)
    {
        return Token{ tokenType, lexeme, line, column };
    }

    // Scans an identifier (or a keyword) token.
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
            text == "break" || text == "continue" || text == "return" ||
            text == "int" || text == "uint" || text == "float" || text == "bool" || text == "string");
    }

    // Scans a number literal (handles both integers and floats).
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

    // Scans a string literal enclosed in double quotes.
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

    // Determines if a character is part of an operator.
    bool Lexer::IsOperator(char c)
    {
        // Basic set: you can extend this list to include multi-character operators later.
        return (c == '+' || c == '-' || c == '*' || c == '/' ||
            c == '=' || c == '!' || c == '<' || c == '>' ||
            c == '&' || c == '|' || c == '%');
    }

    // Scans an operator token
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

    // Determines if a character is punctuation.
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
} // namespace
