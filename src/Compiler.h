#include <Lexer.h>
#include <Parser.h>

#include <string>
#include <iostream>

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
                std::cout << "Token Type: " << static_cast<int>(token.type)
                    << " | Lexeme: [" << token.lexeme << "]"
                    << " | Line: " << token.line
                    << " | Column: " << token.column << "\n";        
            }
        
            Parser parser(tokens);
            parser.ParseProgram();
        }
    private:
        
    };
} // namespace
