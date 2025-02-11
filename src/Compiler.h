#include <Lexer.h>
#include <Parser.h>
#include <string>
#include <iostream>

// Helper function to convert TokenType to string.
std::string tokenTypeToString(Arcanelab::Mano::TokenType type) {
    switch (type) {
        case Arcanelab::Mano::TokenType::Identifier:    return "Identifier";
        case Arcanelab::Mano::TokenType::Keyword:       return "Keyword";
        case Arcanelab::Mano::TokenType::Number:        return "Number";
        case Arcanelab::Mano::TokenType::String:        return "String";
        case Arcanelab::Mano::TokenType::Operator:      return "Operator";
        case Arcanelab::Mano::TokenType::Punctuation:   return "Punctuation";
        case Arcanelab::Mano::TokenType::EndOfFile:     return "EndOfFile";
        case Arcanelab::Mano::TokenType::Unknown:       return "Unknown";
        default:                                       return "Unknown";
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

            for (auto&& token : tokens)
            {
                // Here we print the token type using the helper function.
                std::cout << "Token Type: " << tokenTypeToString(token.type)
                    << " | Lexeme: [" << token.lexeme << "]"
                    << " | Line: " << token.line
                    << " | Column: " << token.column << "\n";        
            }

            Parser parser(tokens);
            parser.ParseProgram();
        }
    private:
        // Other members if needed.
    };
} // namespace Arcanelab::Mano
