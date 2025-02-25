#include <fstream> // Include for file stream
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

            PrintTokens(tokens);

            Parser parser(tokens);
            ASTNodePtr ast = parser.ParseProgram();
            PrintASTTree(ast.get());
        }

    private:
        void PrintTokens(const std::vector<Token>& tokens)
        {
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
        }

        void PrintASTTree(const ASTNode* root)
        {
            // Open the file for writing.
            std::ofstream outFile("../test.ast");
            if (!outFile.is_open())
            {
                std::cerr << "Failed to open file: ../test.ast" << std::endl;
                return;
            }

            // Recursive lambda function to print an AST node.
            auto printNode = [&](auto&& self, const ASTNode* node, const std::string& prefix, bool isLast) -> void {
                if (!node)
                    return;

                // Prepare the branch string for the current node.
                std::string branch = prefix;
                branch += (isLast ? "└── " : "├── ");

                // Determine a label based on the concrete node type.
                std::string nodeLabel;
                if (auto prog = dynamic_cast<const ProgramNode*>(node))
                {
                    nodeLabel = "ProgramNode";
                }
                else if (auto typeNode = dynamic_cast<const TypeNode*>(node))
                {
                    nodeLabel = "TypeNode (" + std::string(typeNode->isConst ? "const " : "") + typeNode->name + ")";
                }
                else if (auto varDecl = dynamic_cast<const VariableDeclarationNode*>(node))
                {
                    nodeLabel = "VariableDeclarationNode (" + varDecl->name + ")";
                }
                else if (auto funDecl = dynamic_cast<const FunctionDeclarationNode*>(node))
                {
                    nodeLabel = "FunctionDeclarationNode (" + funDecl->name + ")";
                }
                else if (auto classDecl = dynamic_cast<const ClassDeclarationNode*>(node))
                {
                    nodeLabel = "ClassDeclarationNode (" + classDecl->name + ")";
                }
                else if (auto enumDecl = dynamic_cast<const EnumDeclarationNode*>(node))
                {
                    nodeLabel = "EnumDeclarationNode (" + enumDecl->name + ")";
                }
                else if (auto block = dynamic_cast<const BlockNode*>(node))
                {
                    nodeLabel = "BlockNode";
                }
                else if (auto block = dynamic_cast<const ClassBlockNode*>(node))
                {
                    nodeLabel = "ClassBlockNode";
                }
                else if (auto exprStmt = dynamic_cast<const ExpressionStatementNode*>(node))
                {
                    nodeLabel = "ExpressionStatementNode";
                }
                else if (auto retStmt = dynamic_cast<const ReturnStatementNode*>(node))
                {
                    nodeLabel = "ReturnStatementNode";
                }
                else if (auto ifStmt = dynamic_cast<const IfStatementNode*>(node))
                {
                    nodeLabel = "IfStatementNode";
                }
                else if (auto forStmt = dynamic_cast<const ForStatementNode*>(node))
                {
                    nodeLabel = "ForStatementNode";
                }
                else if (auto whileStmt = dynamic_cast<const WhileStatementNode*>(node))
                {
                    nodeLabel = "WhileStatementNode";
                }
                else if (auto switchStmt = dynamic_cast<const SwitchStatementNode*>(node))
                {
                    nodeLabel = "SwitchStatementNode";
                }
                else if (auto memberAccess = dynamic_cast<const MemberAccessNode*>(node))
                {
                    nodeLabel = "MemberAccessNode (." + memberAccess->memberName + ")";
                }
                else if (auto binaryExpr = dynamic_cast<const BinaryExpressionNode*>(node))
                {
                    std::string opStr;
                    switch (binaryExpr->op)
                    {
                        case BinaryOperator::Assign:         opStr = "="; break;
                        case BinaryOperator::LogicalOr:      opStr = "||"; break;
                        case BinaryOperator::LogicalAnd:     opStr = "&&"; break;
                        case BinaryOperator::Equal:          opStr = "=="; break;
                        case BinaryOperator::NotEqual:       opStr = "!="; break;
                        case BinaryOperator::Less:           opStr = "<"; break;
                        case BinaryOperator::Greater:        opStr = ">"; break;
                        case BinaryOperator::LessEqual:      opStr = "<="; break;
                        case BinaryOperator::GreaterEqual:   opStr = ">="; break;
                        case BinaryOperator::Add:            opStr = "+"; break;
                        case BinaryOperator::Subtract:       opStr = "-"; break;
                        case BinaryOperator::Multiply:       opStr = "*"; break;
                        case BinaryOperator::Divide:         opStr = "/"; break;
                        case BinaryOperator::Modulo:         opStr = "%"; break;
                        case BinaryOperator::BitwiseOr:      opStr = "|"; break;
                        case BinaryOperator::BitwiseXor:     opStr = "^"; break;
                        case BinaryOperator::BitwiseAnd:     opStr = "&"; break;
                        case BinaryOperator::LeftShift:      opStr = "<<"; break;
                        case BinaryOperator::RightShift:     opStr = ">>"; break;
                        default:                             opStr = "op"; break;
                    }
                    nodeLabel = "BinaryExpressionNode (" + opStr + ")";
                }
                else if (auto unaryExpr = dynamic_cast<const UnaryExpressionNode*>(node))
                {
                    nodeLabel = "UnaryExpressionNode (" + unaryExpr->op + ")";
                }
                else if (auto literal = dynamic_cast<const LiteralNode*>(node))
                {
                    nodeLabel = "LiteralNode (" + literal->value + ")";
                }
                else if (auto ident = dynamic_cast<const IdentifierNode*>(node))
                {
                    nodeLabel = "IdentifierNode (" + ident->name + ")";
                }
                else if (dynamic_cast<const BreakStatementNode*>(node))
                {
                    nodeLabel = "BreakStatementNode";
                }
                else if (dynamic_cast<const ContinueStatementNode*>(node))
                {
                    nodeLabel = "ContinueStatementNode";
                }
                else if (auto arrayLiteral = dynamic_cast<const ArrayLiteralNode*>(node))
                {
                    nodeLabel = "ArrayLiteralNode";
                }
                else if (auto funcCall = dynamic_cast<const FunctionCallNode*>(node))
                {
                    nodeLabel = "FunctionCallNode (" + funcCall->name + ")";
                }
                else if (auto objInst = dynamic_cast<const ObjectInstantiationNode*>(node))
                {
                    nodeLabel = "ObjectInstantiationNode (" + objInst->name + ")";
                }
                else
                {
                    nodeLabel = "Unknown ASTNode";
                }
                outFile << branch << nodeLabel << std::endl;

                // Container for child nodes.
                std::vector<const ASTNode*> children;

                if (auto prog = dynamic_cast<const ProgramNode*>(node))
                {
                    for (const auto& decl : prog->declarations)
                        children.push_back(decl.get());
                }
                else if (auto varDecl = dynamic_cast<const VariableDeclarationNode*>(node))
                {
                    if (varDecl->type) children.push_back(varDecl->type.get());
                    if (varDecl->initializer) children.push_back(varDecl->initializer.get());
                }
                else if (auto funDecl = dynamic_cast<const FunctionDeclarationNode*>(node))
                {
                    // Print parameters as pseudo‐nodes.
                    for (size_t i = 0; i < funDecl->parameters.size(); ++i)
                    {
                        bool lastParam = (i == funDecl->parameters.size() - 1);
                        std::string paramLabel = "Param: " + funDecl->parameters[i].first;
                        std::string paramBranch = prefix + (isLast ? "    " : "│   ");
                        paramBranch += (lastParam ? "└── " : "├── ");
                        outFile << paramBranch << paramLabel << std::endl;
                        // Print the parameter’s type node with an extended prefix.
                        if (funDecl->parameters[i].second)
                        {
                            std::string typePrefix = prefix + (isLast ? "    " : "│   ");
                            typePrefix += (lastParam ? "    " : "│   ");
                            self(self, funDecl->parameters[i].second.get(), typePrefix, true);
                        }
                    }
                    if (funDecl->returnType)
                        children.push_back(funDecl->returnType.get());
                    if (funDecl->body)
                        children.push_back(funDecl->body.get());
                }
                else if (auto classDecl = dynamic_cast<const ClassDeclarationNode*>(node))
                {
                    if (classDecl->body) children.push_back(classDecl->body.get());
                }
                else if (auto enumDecl = dynamic_cast<const EnumDeclarationNode*>(node))
                {
                    // Print each enum value as a pseudo‐child.
                    for (size_t i = 0; i < enumDecl->values.size(); i++)
                    {
                        std::string valueLabel = "EnumValue: " + enumDecl->values[i];
                        std::string valueBranch = prefix + (isLast ? "    " : "│   ") + "├── ";
                        outFile << valueBranch << valueLabel << std::endl;
                    }
                }
                else if (auto block = dynamic_cast<const BlockNode*>(node))
                {
                    for (const auto& stmt : block->statements)
                        children.push_back(stmt.get());
                }
                else if (auto block = dynamic_cast<const ClassBlockNode*>(node))
                {
                    for (const auto& stmt : block->declarations)
                        children.push_back(stmt.get());
                }
                else if (auto exprStmt = dynamic_cast<const ExpressionStatementNode*>(node))
                {
                    if (exprStmt->expression) children.push_back(exprStmt->expression.get());
                }
                else if (auto retStmt = dynamic_cast<const ReturnStatementNode*>(node))
                {
                    if (retStmt->expression) children.push_back(retStmt->expression.get());
                }
                else if (auto ifStmt = dynamic_cast<const IfStatementNode*>(node))
                {
                    if (ifStmt->condition) children.push_back(ifStmt->condition.get());
                    if (ifStmt->thenBranch) children.push_back(ifStmt->thenBranch.get());
                    if (ifStmt->elseBranch) children.push_back(ifStmt->elseBranch.get());
                }
                else if (auto forStmt = dynamic_cast<const ForStatementNode*>(node))
                {
                    if (forStmt->init) children.push_back(forStmt->init.get());
                    if (forStmt->condition) children.push_back(forStmt->condition.get());
                    if (forStmt->increment) children.push_back(forStmt->increment.get());
                    if (forStmt->body) children.push_back(forStmt->body.get());
                }
                else if (auto whileStmt = dynamic_cast<const WhileStatementNode*>(node))
                {
                    if (whileStmt->condition) children.push_back(whileStmt->condition.get());
                    if (whileStmt->body) children.push_back(whileStmt->body.get());
                }
                else if (auto switchStmt = dynamic_cast<const SwitchStatementNode*>(node))
                {
                    if (switchStmt->expression)
                        children.push_back(switchStmt->expression.get());
                    // Process each switch case as a pseudo‐node.
                    for (size_t i = 0; i < switchStmt->cases.size(); i++)
                    {
                        bool isLastCase = (i == switchStmt->cases.size() - 1 && switchStmt->defaultCase == nullptr);
                        std::string caseBranch = prefix + (isLast ? "    " : "│   ");
                        caseBranch += (isLastCase ? "└── " : "├── ");
                        outFile << caseBranch << "Case:" << std::endl;
                        std::string casePrefix = prefix + (isLast ? "    " : "│   ");
                        casePrefix += (isLastCase ? "    " : "│   ");
                        // Print the case expression and block.
                        self(self, switchStmt->cases[i].first.get(), casePrefix, false);
                        self(self, switchStmt->cases[i].second.get(), casePrefix, true);
                    }
                    if (switchStmt->defaultCase)
                    {
                        // Print Default: as a pseudo‐node.
                        std::string defaultBranch = prefix + (isLast ? "    " : "│   ") + "└── ";
                        outFile << defaultBranch << "Default:" << std::endl;
                        std::string defPrefix = prefix + (isLast ? "    " : "│   ") + "│   ";
                        self(self, switchStmt->defaultCase.get(), defPrefix, true);
                    }
                }
                else if (auto memberAccess = dynamic_cast<const MemberAccessNode*>(node))
                {
                    if (memberAccess->object) children.push_back(memberAccess->object.get());
                }
                else if (auto binaryExpr = dynamic_cast<const BinaryExpressionNode*>(node))
                {
                    if (binaryExpr->left) children.push_back(binaryExpr->left.get());
                    if (binaryExpr->right) children.push_back(binaryExpr->right.get());
                }
                else if (auto unaryExpr = dynamic_cast<const UnaryExpressionNode*>(node))
                {
                    if (unaryExpr->operand) children.push_back(unaryExpr->operand.get());
                }
                else if (auto arrayLiteral = dynamic_cast<const ArrayLiteralNode*>(node))
                {
                    for (const auto& elem : arrayLiteral->elements)
                        children.push_back(elem.get());
                }
                else if (auto funcCall = dynamic_cast<const FunctionCallNode*>(node))
                {
                    if (funcCall->callTarget) children.push_back(funcCall->callTarget.get());
                    for (const auto& arg : funcCall->arguments)
                        children.push_back(arg.get());
                }
                else if (auto objInst = dynamic_cast<const ObjectInstantiationNode*>(node))
                {
                    for (const auto& arg : objInst->arguments)
                        children.push_back(arg.get());
                }

                // Recursively print any gathered children.
                for (size_t i = 0; i < children.size(); i++)
                {
                    bool lastChild = (i == children.size() - 1);
                    std::string newPrefix = prefix + (isLast ? "    " : "│   ");
                    self(self, children[i], newPrefix, lastChild);
                }
                }; // lambda

            printNode(printNode, root, "", true);

            // Close the file after writing.
            outFile.close();
        }
    };
} // namespace Arcanelab::Mano
