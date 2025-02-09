#include <cctype>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string_view>
#include <string>

// Token types representing our language’s lexical elements.
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
    TokenType Type;
    std::string_view Lexeme;
    size_t Line;
    size_t Column;
};

// Our handwritten lexer encapsulated as a class.
// Function names here (e.g., NextToken, Advance, etc.) all begin with capital letters.
class Lexer
{
public:
    Lexer(std::string_view source)
        : Source(source), Offset(0), Line(1), Column(1)
    {
    }

    Token NextToken()
    {
        SkipWhitespace();
        if (IsAtEnd())
            return MakeToken(TokenType::EndOfFile, "");

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
        return MakeToken(TokenType::Unknown, std::string_view(&current, 1));
    }

private:
    std::string_view Source;
    size_t Offset;
    size_t Line;
    size_t Column;

    // Checks whether we've reached the end of the source text.
    bool IsAtEnd() const
    {
        return Offset >= Source.size();
    }

    // Returns the current character without advancing.
    char Peek() const
    {
        return Source[Offset];
    }

    // Advances one character and updates position (line/column).
    char Advance()
    {
        char c = Source[Offset++];
        if (c == '\n')
        {
            ++Line;
            Column = 1;
        }
        else
        {
            ++Column;
        }
        return c;
    }

    // Skip any whitespaces; our grammar assumes comments are handled here as needed.
    void SkipWhitespace()
    {
        while (!IsAtEnd() && std::isspace(Peek()))
            Advance();
    }

    // Creates a Token instance.
    Token MakeToken(TokenType type, std::string_view lexeme)
    {
        return Token{ type, lexeme, Line, Column };
    }

    // Scans an identifier (or a keyword) token.
    Token ScanIdentifier()
    {
        size_t start = Offset;
        while (!IsAtEnd() && (std::isalnum(Peek()) || Peek() == '_'))
            Advance();
        std::string_view text = Source.substr(start, Offset - start);
        TokenType type = IsKeyword(text) ? TokenType::Keyword : TokenType::Identifier;
        return Token{ type, text, Line, Column };
    }

    // A simple check for language keywords.
    bool IsKeyword(std::string_view text)
    {
        // List keywords per your grammar.
        return (text == "var" || text == "fun" || text == "class" || text == "enum" ||
            text == "if" || text == "else" || text == "for" || text == "while" ||
            text == "break" || text == "continue" || text == "return" ||
            text == "int" || text == "uint" || text == "float" || text == "bool" || text == "string");
    }

    // Scans a number literal (handles both integers and floats).
    Token ScanNumber()
    {
        size_t start = Offset;
        while (!IsAtEnd() && std::isdigit(Peek()))
            Advance();

        // Check for fractional part.
        if (!IsAtEnd() && Peek() == '.')
        {
            Advance();
            while (!IsAtEnd() && std::isdigit(Peek()))
                Advance();
        }
        std::string_view text = Source.substr(start, Offset - start);
        return Token{ TokenType::Number, text, Line, Column };
    }

    // Scans a string literal enclosed in double quotes.
    Token ScanString()
    {
        char quote = Advance(); // Consume the opening "
        size_t start = Offset;
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
        std::string_view text = Source.substr(start, Offset - start - 1);
        return Token{ TokenType::String, text, Line, Column };
    }

    // Determines if a character is part of an operator.
    bool IsOperator(char c)
    {
        // Basic set: you can extend this list to include multi-character operators later.
        return (c == '+' || c == '-' || c == '*' || c == '/' ||
            c == '=' || c == '!' || c == '<' || c == '>' ||
            c == '&' || c == '|' || c == '%');
    }

    // Scans an operator token.
    Token ScanOperator()
    {
        size_t start = Offset;
        Advance(); // Consume the operator character.
        // For a complete implementation, check “peek” for operators like "==", "!=" etc.
        std::string_view text = Source.substr(start, Offset - start);
        return Token{ TokenType::Operator, text, Line, Column };
    }

    // Determines if a character is punctuation per our grammar.
    bool IsPunctuation(char c)
    {
        return (c == '(' || c == ')' || c == '{' || c == '}' ||
            c == '[' || c == ']' || c == ',' || c == ':' || c == ';');
    }

    // Scans a punctuation token.
    Token ScanPunctuation()
    {
        size_t start = Offset;
        Advance();
        std::string_view text = Source.substr(start, Offset - start);
        return Token{ TokenType::Punctuation, text, Line, Column };
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
        std::cout << "Token Type: " << static_cast<int>(token.Type)
            << " | Lexeme: [" << token.Lexeme << "]"
            << " | Line: " << token.Line
            << " | Column: " << token.Column << "\n";
    } while (token.Type != TokenType::EndOfFile);

    return 0;
}
