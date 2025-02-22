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
                << std::setw(5) << "Index"      // Add column for index
                << std::setw(10) << "Lexeme"
                << std::setw(10) << "Coordinates"
                << "Token Type" << "\n";
            std::cout << std::string(8 + 20 + 16 + 12, '-') << "\n";

            int index = 0; // Initialize index counter
            for (auto&& token : tokens)
            {
                // Construct a coordinate string of the form "(line, column)"
                std::string coord = "(" + std::to_string(token.line) + ", " + std::to_string(token.column) + ")";

                // Print index, lexeme, coordinates, and token type with fixed-width formatting.
                std::cout << std::left
                    << std::setw(5) << index++  // Print and increment index
                    << std::setw(10) << token.lexeme
                    << std::setw(10) << coord
                    << tokenTypeToString(token.type) << "\n";
            }

            Parser parser(tokens);
            auto ast = parser.ParseProgram();
            PrintAST(ast.get(), 0);
        }
    private:
        void PrintIndent(int indent)
        {
            for (int i = 0; i < indent; i++)
                std::cout << "  ";
        }

        void PrintAST(const ASTNode* node, int indent = 0)
        {
            if (!node)
                return;
        
            if (auto program = dynamic_cast<const ProgramNode*>(node))
            {
                PrintIndent(indent);
                std::cout << "ProgramNode\n";
                for (const auto& decl : program->declarations)
                    PrintAST(decl.get(), indent + 1);
            }
            else if (auto varDecl = dynamic_cast<const VarDeclNode*>(node))
            {
                PrintIndent(indent);
                std::cout << "VarDeclNode: " << varDecl->name << "\n";
                PrintIndent(indent + 1);
                std::cout << "Type:\n";
                PrintAST(varDecl->type.get(), indent + 2);
                if (varDecl->initializer)
                {
                    PrintIndent(indent + 1);
                    std::cout << "Initializer:\n";
                    PrintAST(varDecl->initializer.get(), indent + 2);
                }
            }
            else if (auto funDecl = dynamic_cast<const FunDeclNode*>(node))
            {
                PrintIndent(indent);
                std::cout << "FunDeclNode: " << funDecl->name << "\n";
                if (!funDecl->parameters.empty())
                {
                    PrintIndent(indent + 1);
                    std::cout << "Parameters:\n";
                    for (const auto& param : funDecl->parameters)
                    {
                        PrintIndent(indent + 2);
                        std::cout << param.first << ": ";
                        PrintAST(param.second.get(), 0);
                    }
                }
                if (funDecl->returnType)
                {
                    PrintIndent(indent + 1);
                    std::cout << "Return Type:\n";
                    PrintAST(funDecl->returnType.get(), indent + 2);
                }
                PrintIndent(indent + 1);
                std::cout << "Body:\n";
                PrintAST(funDecl->body.get(), indent + 2);
            }
            else if (auto classDecl = dynamic_cast<const ClassDeclNode*>(node))
            {
                PrintIndent(indent);
                std::cout << "ClassDeclNode: " << classDecl->name << "\n";
                PrintIndent(indent + 1);
                std::cout << "Body:\n";
                PrintAST(classDecl->body.get(), indent + 2);
            }
            else if (auto enumDecl = dynamic_cast<const EnumDeclNode*>(node))
            {
                PrintIndent(indent);
                std::cout << "EnumDeclNode: " << enumDecl->name << "\n";
                if (!enumDecl->values.empty())
                {
                    PrintIndent(indent + 1);
                    std::cout << "Values:";
                    for (const auto& val : enumDecl->values)
                        std::cout << " " << val;
                    std::cout << "\n";
                }
            }
            else if (auto block = dynamic_cast<const BlockNode*>(node))
            {
                PrintIndent(indent);
                std::cout << "BlockNode\n";
                for (const auto& stmt : block->statements)
                    PrintAST(stmt.get(), indent + 1);
            }
            else if (auto exprStmt = dynamic_cast<const ExprStmtNode*>(node))
            {
                PrintIndent(indent);
                std::cout << "ExprStmtNode\n";
                PrintAST(exprStmt->expression.get(), indent + 1);
            }
            else if (auto retStmt = dynamic_cast<const ReturnStmtNode*>(node))
            {
                PrintIndent(indent);
                std::cout << "ReturnStmtNode\n";
                if (retStmt->expression)
                    PrintAST(retStmt->expression.get(), indent + 1);
            }
            else if (auto ifStmt = dynamic_cast<const IfStmtNode*>(node))
            {
                PrintIndent(indent);
                std::cout << "IfStmtNode\n";
                PrintIndent(indent + 1);
                std::cout << "Condition:\n";
                PrintAST(ifStmt->condition.get(), indent + 2);
                PrintIndent(indent + 1);
                std::cout << "Then Branch:\n";
                PrintAST(ifStmt->thenBranch.get(), indent + 2);
                if (ifStmt->elseBranch)
                {
                    PrintIndent(indent + 1);
                    std::cout << "Else Branch:\n";
                    PrintAST(ifStmt->elseBranch.get(), indent + 2);
                }
            }
            else if (auto forStmt = dynamic_cast<const ForStmtNode*>(node))
            {
                PrintIndent(indent);
                std::cout << "ForStmtNode\n";
                PrintIndent(indent + 1);
                std::cout << "Initialization:\n";
                PrintAST(forStmt->init.get(), indent + 2);
                PrintIndent(indent + 1);
                std::cout << "Condition:\n";
                PrintAST(forStmt->condition.get(), indent + 2);
                PrintIndent(indent + 1);
                std::cout << "Increment:\n";
                PrintAST(forStmt->increment.get(), indent + 2);
                PrintIndent(indent + 1);
                std::cout << "Body:\n";
                PrintAST(forStmt->body.get(), indent + 2);
            }
            else if (auto whileStmt = dynamic_cast<const WhileStmtNode*>(node))
            {
                PrintIndent(indent);
                std::cout << "WhileStmtNode\n";
                PrintIndent(indent + 1);
                std::cout << "Condition:\n";
                PrintAST(whileStmt->condition.get(), indent + 2);
                PrintIndent(indent + 1);
                std::cout << "Body:\n";
                PrintAST(whileStmt->body.get(), indent + 2);
            }
            else if (auto binaryExpr = dynamic_cast<const BinaryExprNode*>(node))
            {
                PrintIndent(indent);
                std::cout << "BinaryExprNode: ";
                switch (binaryExpr->op)
                {
                    case BinaryOperator::Assign:       std::cout << "="; break;
                    case BinaryOperator::LogicalOr:    std::cout << "||"; break;
                    case BinaryOperator::LogicalAnd:   std::cout << "&&"; break;
                    case BinaryOperator::Equal:        std::cout << "=="; break;
                    case BinaryOperator::NotEqual:     std::cout << "!="; break;
                    case BinaryOperator::Less:         std::cout << "<"; break;
                    case BinaryOperator::Greater:      std::cout << ">"; break;
                    case BinaryOperator::LessEqual:    std::cout << "<="; break;
                    case BinaryOperator::GreaterEqual: std::cout << ">="; break;
                    case BinaryOperator::Add:          std::cout << "+"; break;
                    case BinaryOperator::Subtract:     std::cout << "-"; break;
                    case BinaryOperator::Multiply:     std::cout << "*"; break;
                    case BinaryOperator::Divide:       std::cout << "/"; break;
                    case BinaryOperator::Modulo:       std::cout << "%"; break;
                    default:                           std::cout << "unknown"; break;
                }
                std::cout << "\n";
                PrintIndent(indent + 1);
                std::cout << "Left:\n";
                PrintAST(binaryExpr->left.get(), indent + 2);
                PrintIndent(indent + 1);
                std::cout << "Right:\n";
                PrintAST(binaryExpr->right.get(), indent + 2);
            }
            else if (auto unaryExpr = dynamic_cast<const UnaryExprNode*>(node))
            {
                PrintIndent(indent);
                std::cout << "UnaryExprNode: " << unaryExpr->op << "\n";
                PrintAST(unaryExpr->operand.get(), indent + 1);
            }
            else if (auto literal = dynamic_cast<const LiteralNode*>(node))
            {
                PrintIndent(indent);
                std::cout << "LiteralNode: " << literal->value << "\n";
            }
            else if (auto identifier = dynamic_cast<const IdentifierNode*>(node))
            {
                PrintIndent(indent);
                std::cout << "IdentifierNode: " << identifier->name << "\n";
            }
            else if (dynamic_cast<const BreakStmtNode*>(node))
            {
                PrintIndent(indent);
                std::cout << "BreakStmtNode\n";
            }
            else if (dynamic_cast<const ContinueStmtNode*>(node))
            {
                PrintIndent(indent);
                std::cout << "ContinueStmtNode\n";
            }
            else if (auto arrayLiteral = dynamic_cast<const ArrayLiteralNode*>(node))
            {
                PrintIndent(indent);
                std::cout << "ArrayLiteralNode\n";
                for (const auto& element : arrayLiteral->elements)
                    PrintAST(element.get(), indent + 1);
            }
            else if (auto funcCall = dynamic_cast<const FunctionCallNode*>(node))
            {
                PrintIndent(indent);
                std::cout << "FunctionCallNode: " << funcCall->name << "\n";
                if (!funcCall->arguments.empty())
                {
                    PrintIndent(indent + 1);
                    std::cout << "Arguments:\n";
                    for (const auto& arg : funcCall->arguments)
                        PrintAST(arg.get(), indent + 2);
                }
            }
            else if (auto objInst = dynamic_cast<const ObjectInstantiationNode*>(node))
            {
                PrintIndent(indent);
                std::cout << "ObjectInstantiationNode: " << objInst->name << "\n";
                if (!objInst->arguments.empty())
                {
                    PrintIndent(indent + 1);
                    std::cout << "Arguments:\n";
                    for (const auto& arg : objInst->arguments)
                        PrintAST(arg.get(), indent + 2);
                }
            }
            else
            {
                PrintIndent(indent);
                std::cout << "Unknown AST Node\n";
            }
        }
    };
} // namespace Arcanelab::Mano
