#include <cctype>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string_view>
#include <string>

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

    Token NextToken()
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

private:
    std::string_view source;
    size_t offset;
    size_t line;
    size_t column;

    // Checks whether we've reached the end of the source text.
    bool IsAtEnd() const
    {
        return offset >= source.size();
    }

    // Returns the current character without advancing.
    char Peek() const
    {
        return source[offset];
    }

    // Advances one character and updates position (line/column).
    char Advance()
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
    void SkipWhitespace()
    {
        while (!IsAtEnd() && std::isspace(Peek()))
            Advance();
    }

    // Creates a Token instance.
    Token CreateToken(TokenType tokenType, std::string_view lexeme)
    {
        return Token{ tokenType, lexeme, line, column };
    }

    // Scans an identifier (or a keyword) token.
    Token ScanIdentifier()
    {
        size_t start = offset;
        while (!IsAtEnd() && (std::isalnum(Peek()) || Peek() == '_'))
            Advance();
        std::string_view text = source.substr(start, offset - start);
        TokenType tokenType = IsKeyword(text) ? TokenType::Keyword : TokenType::Identifier;
        return Token{ tokenType, text, line, column };
    }

    bool IsKeyword(std::string_view text)
    {
        return (text == "var" || text == "fun" || text == "class" || text == "enum" ||
                text == "if" || text == "else" || text == "for" || text == "while" ||
                text == "break" || text == "continue" || text == "return" ||
                text == "int" || text == "uint" || text == "float" || text == "bool" || text == "string");
    }

    // Scans a number literal (handles both integers and floats).
    Token ScanNumber()
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
    Token ScanString()
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
    bool IsOperator(char c)
    {
        // Basic set: you can extend this list to include multi-character operators later.
        return (c == '+' || c == '-' || c == '*' || c == '/' ||
                c == '=' || c == '!' || c == '<' || c == '>' ||
                c == '&' || c == '|' || c == '%');
    }

    // Scans an operator token
    Token ScanOperator()
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
    bool IsPunctuation(char c)
    {
        return (c == '(' || c == ')' || c == '{' || c == '}' ||
                c == '[' || c == ']' || c == ',' || c == ':' || c == ';');
    }

    Token ScanPunctuation()
    {
        size_t start = offset;
        Advance();
        std::string_view text = source.substr(start, offset - start);
        return Token{ TokenType::Punctuation, text, line, column };
    }
};

//
// --- Demo usage ---
//
int main()
{
    std::ifstream file("test.mano");
    if (!file)
    {
        std::cerr << "Failed to open test.mano" << "\n";
        return 1;
    }
    std::ostringstream buffer;
    buffer << file.rdbuf();
    std::string source = buffer.str();

    Lexer lex(source);
    Token token;

    // Continuously get tokens until the EndOfFile token is returned.
    do
    {
        token = lex.NextToken();
        std::cout << "Token Type: " << static_cast<int>(token.type)
                  << " | Lexeme: [" << token.lexeme << "]"
                  << " | Line: " << token.line
                  << " | Column: " << token.column << "\n";
    } while (token.type != TokenType::EndOfFile);

    return 0;
}