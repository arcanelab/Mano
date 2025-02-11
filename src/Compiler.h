#include <Lexer.h>
#include <Parser.h>
#include <string>
#include <iostream>
#include <iomanip>

// Helper function to convert TokenType to string.
std::string tokenTypeToString(Arcanelab::Mano::TokenType type)
{
    switch (type)
    {
        case Arcanelab::Mano::TokenType::Identifier:    return "Identifier";
        case Arcanelab::Mano::TokenType::Keyword:       return "Keyword";
        case Arcanelab::Mano::TokenType::Number:        return "Number";
        case Arcanelab::Mano::TokenType::String:        return "String";
        case Arcanelab::Mano::TokenType::Operator:      return "Operator";
        case Arcanelab::Mano::TokenType::Punctuation:   return "Punctuation";
        case Arcanelab::Mano::TokenType::EndOfFile:     return "EndOfFile";
        case Arcanelab::Mano::TokenType::Unknown:       return "Unknown";
        default:                                        return "Unknown";
    }
}

namespace Arcanelab::Mano
{
    class Compiler
    {
    public:
        void Run(const std::string& source)
        {
            Lexer lexer(source);
            auto tokens = lexer.Tokenize();

            if (tokens.empty())
                return;

            // Print header for clarity.
            std::cout << std::left
                << std::setw(20) << "Lexeme"
                << std::setw(16) << "Coordinates"
                << "Token Type" << "\n";
            std::cout << std::string(20 + 16 + 12, '-') << "\n";

            for (auto&& token : tokens)
            {
                // Construct a coordinate string of the form "(line, column)"
                std::string coord = "(" + std::to_string(token.line) + ", " + std::to_string(token.column) + ")";

                // Print lexeme, coordinates, and token type with fixed-width formatting.
                std::cout << std::left
                    << std::setw(20) << token.lexeme
                    << std::setw(16) << coord
                    << tokenTypeToString(token.type) << "\n";
            }

            Parser parser(tokens);
            parser.ParseProgram();
        }
    private:
        // Other members if needed.
    };
} // namespace Arcanelab::Mano
