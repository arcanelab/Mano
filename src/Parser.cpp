#include "Parser.h"
#include <cassert>
#include <iostream>
#include <cstdlib>

namespace Arcanelab::Mano
{
    Parser::Parser(const std::vector<Token>& tokens)
        : m_tokens(tokens), m_current(0)
    {
    }

    bool Parser::IsAtEnd() const
    {
        return m_tokens[m_current].type == TokenType::EndOfFile;
    }

    const Token& Parser::Peek() const
    {
        return m_tokens[m_current];
    }

    const Token& Parser::Previous() const
    {
        return m_tokens[m_current - 1];
    }

    const Token& Parser::Advance()
    {
        if (!IsAtEnd())
            m_current++;
        return Previous();
    }

    bool Parser::CheckType(TokenType type) const
    {
        if (IsAtEnd())
            return false;
        return Peek().type == type;
    }

    bool Parser::Match(const std::initializer_list<TokenType>& types)
    {
        for (TokenType type : types)
        {
            if (CheckType(type))
            {
                Advance();
                return true;
            }
        }
        return false;
    }

    // Only consumes the token if it is a Keyword and its lexeme equals expected.
    bool Parser::MatchKeyword(const std::string& expected)
    {
        if (CheckType(TokenType::Keyword) && std::string(Peek().lexeme) == expected)
        {
            Advance();
            return true;
        }
        return false;
    }

    const Token& Parser::Consume(TokenType type, const std::string& message)
    {
        if (CheckType(type))
            return Advance();
        ErrorAtCurrent(message);
        return Peek(); // Unreachable, but required for compilation.
    }

    // NEW helper: consumes a punctuation token with an expected lexeme.
    const Token& Parser::ConsumePunctuation(const std::string& expected, const std::string& message)
    {
        if (!IsAtEnd() && Peek().type == TokenType::Punctuation && std::string(Peek().lexeme) == expected)
            return Advance();
        ErrorAtCurrent(message);
        return Peek(); // Unreachable.
    }

    void Parser::ErrorAtCurrent(const std::string& message)
    {
        const Token& token = Peek();
        std::cerr << "[Line " << token.line << ", Column " << token.column
            << "] Error: " << message << "\n";
        std::exit(1);
    }

    ASTNodePtr Parser::ParseProgram()
    {
        auto program = std::make_unique<ProgramNode>();
        while (!IsAtEnd())
        {
            auto decl = ParseDeclaration();
            if (decl)
                program->declarations.push_back(std::move(decl));
        }
        return program;
    }

    ASTNodePtr Parser::ParseDeclaration()
    {
        if (MatchKeyword("let"))
            return ParseConstantDeclaration();
        if (MatchKeyword("var"))
            return ParseVariableDeclaration();
        if (MatchKeyword("fun"))
            return ParseFunctionDeclaration();
        if (MatchKeyword("class"))
            return ParseClassDeclaration();
        if (MatchKeyword("enum"))
            return ParseEnumDeclaration();

        ErrorAtCurrent("Expected declaration.");
        return nullptr;
    }

    TypeNodePtr Parser::ParseType(bool isConst)
    {
        if (Match({ TokenType::Keyword }))
        {
            // Handle primitive types (int, uint, float, bool, string).
            auto typeNode = std::make_unique<TypeNode>();
            typeNode->name = std::string(Previous().lexeme);
            typeNode->isConst = isConst;
            return typeNode;
        }
        else if (Match({ TokenType::Identifier }))
        {
            // Handle user defined types (Identifier)
            auto typeNode = std::make_unique<TypeNode>();
            typeNode->name = std::string(Previous().lexeme);
            typeNode->isConst = isConst;
            return typeNode;
        }
        else if (Match({ TokenType::Punctuation }) && std::string(Previous().lexeme) == "[") //Check for '['
        {
            //Handle array type.
            auto arrayType = ParseType(false); // Recursively parse element type.
            ConsumePunctuation("]", "Expected ']' after array element type.");
            //Construct the type name to be, for instance, "[int]".
            auto typeNode = std::make_unique<TypeNode>();
            typeNode->name = "[" + arrayType->name + "]";
            typeNode->isConst = isConst;

            return typeNode;

        }
        ErrorAtCurrent("Expected type name.");
        return nullptr;
    }

    ASTNodePtr Parser::ParseConstantDeclaration()
    {
        // "let" has already been consumed.
        auto constDecl = std::make_unique<VarDeclNode>(); // Also used for constants.
        constDecl->name = std::string(Consume(TokenType::Identifier, "Expected constant name.").lexeme);
        ConsumePunctuation(":", "Expected ':' after constant name.");
        auto typeNode = ParseType(true); // Pass 'true' for const.
        constDecl->type = std::move(typeNode);

        // Match an operator token with "=".
        if (!(Match({ TokenType::Operator }) && std::string(Previous().lexeme) == "="))
        {
            ErrorAtCurrent("Expected '=' after type.");
        }
        constDecl->initializer = ParseExpression();
        ConsumePunctuation(";", "Expected ';' after constant declaration.");
        return constDecl;
    }

    ASTNodePtr Parser::ParseVariableDeclaration()
    {
        auto varDecl = std::make_unique<VarDeclNode>();
        varDecl->name = std::string(Consume(TokenType::Identifier, "Expected variable name.").lexeme);
        ConsumePunctuation(":", "Expected ':' after variable name.");
        auto typeNode = ParseType(false); // Pass 'false' for non-const.
        varDecl->type = std::move(typeNode);

        if (Match({ TokenType::Operator }) && std::string(Previous().lexeme) == "=")
        {
            varDecl->initializer = ParseExpression();
        }
        ConsumePunctuation(";", "Expected ';' after variable declaration.");
        return varDecl;
    }

    ASTNodePtr Parser::ParseFunctionDeclaration()
    {
        auto funDecl = std::make_unique<FunDeclNode>();
        funDecl->name = std::string(Consume(TokenType::Identifier, "Expected function name.").lexeme);
        ConsumePunctuation("(", "Expected '(' after function name.");
        // Only parse parameters if the next token is not a closing parenthesis.
        if (CheckType(TokenType::Identifier)) //we now check if it's an identifier
        {
            ParseParameterList(funDecl->parameters);
        }
        ConsumePunctuation(")", "Expected ')' after parameters.");
        // Check for an optional return type.
        if (CheckType(TokenType::Punctuation) && std::string(Peek().lexeme) == ":")
        {
            Advance(); // Consume the colon.
            auto returnType = ParseType(false); // Return types are not const by default
            funDecl->returnType = std::move(returnType);
        }
        funDecl->body = ParseBlock();
        return funDecl;
    }

    void Parser::ParseParameterList(std::vector<std::pair<std::string, TypeNodePtr>>& parameters)
    {
        // At least one parameter is expected.
        if (CheckType(TokenType::Identifier))
        {
            std::string paramName = std::string(Consume(TokenType::Identifier, "Expected parameter name.").lexeme);
            ConsumePunctuation(":", "Expected ':' after parameter name.");
            TypeNodePtr paramType = ParseType(false); // Parameters are not const by default
            parameters.push_back({ paramName, std::move(paramType) });
        }

        while (CheckType(TokenType::Punctuation) && std::string(Peek().lexeme) == ",")
        {
            Advance(); // Consume the comma.
            std::string paramName = std::string(Consume(TokenType::Identifier, "Expected parameter name after comma.").lexeme);
            ConsumePunctuation(":", "Expected ':' after parameter name.");
            TypeNodePtr paramType = ParseType(false); // Parameters are not const by default
            parameters.push_back({ paramName, std::move(paramType) });
        }
    }

    ASTNodePtr Parser::ParseClassDeclaration()
    {
        auto classDecl = std::make_unique<ClassDeclNode>();
        classDecl->name = std::string(Consume(TokenType::Identifier, "Expected class name.").lexeme);
        classDecl->body = ParseBlock();
        return classDecl;
    }

    ASTNodePtr Parser::ParseEnumDeclaration()
    {
        auto enumDecl = std::make_unique<EnumDeclNode>();
        enumDecl->name = std::string(Consume(TokenType::Identifier, "Expected enum name.").lexeme);
        enumDecl->values = ParseEnumBody();
        return enumDecl;
    }

    std::vector<std::string> Parser::ParseEnumBody()
    {
        std::vector<std::string> values;
        ConsumePunctuation("{", "Expected '{' to start enum body.");

        // Empty enum.
        if (CheckType(TokenType::Punctuation) && std::string(Peek().lexeme) == "}")
        {
            Advance(); // consume "}"
            return values;
        }

        do
        {
            // Each enum case should be an identifier.
            std::string enumName = std::string(Consume(TokenType::Identifier, "Expected enum name.").lexeme);
            values.push_back(enumName);

            // If there's a comma, consume it and continue.
            if (CheckType(TokenType::Punctuation) && std::string(Peek().lexeme) == ",")
            {
                Advance(); // consume comma
            }
            else
            {
                break; // no comma
            }
        } while (true);

        // Expect a closing brace.
        ConsumePunctuation("}", "Expected '}' to close enum body.");
        return values;
    }

    ASTNodePtr Parser::ParseBlock()
    {
        ConsumePunctuation("{", "Expected '{' to start a block.");
        auto block = std::make_unique<BlockNode>();
        while (!CheckType(TokenType::Punctuation) || std::string(Peek().lexeme) != "}")
        {
            //Check for declaration keywords first.
            if (CheckType(TokenType::Keyword) &&
                (std::string(Peek().lexeme) == "let" ||
                    std::string(Peek().lexeme) == "var" ||
                    std::string(Peek().lexeme) == "fun" ||
                    std::string(Peek().lexeme) == "class" ||
                    std::string(Peek().lexeme) == "enum"))
            {
                block->statements.push_back(ParseDeclaration());
            }
            else //It must be a statement
            {
                block->statements.push_back(ParseStatement());
            }
        }
        ConsumePunctuation("}", "Expected '}' to close block.");
        return block;
    }

    ASTNodePtr Parser::ParseStatement()
    {
        if (MatchKeyword("if")) return ParseIfStatement();
        if (MatchKeyword("for")) return ParseForStatement();
        if (MatchKeyword("while")) return ParseWhileStatement();
        if (MatchKeyword("return")) return ParseReturnStatement();
        if (MatchKeyword("break")) return ParseBreakStatement();
        if (MatchKeyword("continue")) return ParseContinueStatement();

        // If none of the above, it must be an expression statement.
        auto expr = ParseExpression();
        ConsumePunctuation(";", "Expected ';' after expression statement.");
        auto expressionNode = std::make_unique<ExprStmtNode>(); // CREATE THE NODE
        expressionNode->expression = std::move(expr);           // STORE THE EXPRESSION
        return expressionNode;                                  // RETURN THE NODE
    }

    ASTNodePtr Parser::ParseBreakStatement()
    {
        ConsumePunctuation(";", "Expected ';' after 'break'.");
        return std::make_unique<BreakStmtNode>();
    }

    ASTNodePtr Parser::ParseContinueStatement()
    {
        ConsumePunctuation(";", "Expected ';' after 'break'.");
        return std::make_unique<ContinueStmtNode>();
    }

    ASTNodePtr Parser::ParseIfStatement()
    {
        ConsumePunctuation("(", "Expected '(' after 'if'.");
        auto condition = ParseExpression();
        ConsumePunctuation(")", "Expected ')' after if condition.");
        auto thenBranch = ParseBlock();
        ASTNodePtr elseBranch = nullptr;
        if (Match({ TokenType::Keyword }) && std::string(Previous().lexeme) == "else")
        {
            elseBranch = ParseBlock();
        }
        auto ifStmt = std::make_unique<IfStmtNode>();
        ifStmt->condition = std::move(condition);
        ifStmt->thenBranch = std::move(thenBranch);
        ifStmt->elseBranch = std::move(elseBranch);
        return ifStmt;
    }

    ASTNodePtr Parser::ParseForStatement()
    {
        ConsumePunctuation("(", "Expected '(' after 'for'.");
        ASTNodePtr init = nullptr;

        // The grammar now enforces that the initialization part *must* be a VariableDeclaration.
        init = ParseVariableDeclaration();

        ConsumePunctuation(";", "Expected ';' after for initializer.");
        auto condition = ParseExpression();
        ConsumePunctuation(";", "Expected ';' after for condition.");
        auto increment = ParseExpression();
        ConsumePunctuation(")", "Expected ')' after for clauses.");
        auto body = ParseBlock();

        auto forStmt = std::make_unique<ForStmtNode>();
        forStmt->init = std::move(init);
        forStmt->condition = std::move(condition);
        forStmt->increment = std::move(increment);
        forStmt->body = std::move(body);
        return forStmt;
    }

    ASTNodePtr Parser::ParseWhileStatement()
    {
        ConsumePunctuation("(", "Expected '(' after 'while'.");
        auto condition = ParseExpression();
        ConsumePunctuation(")", "Expected ')' after while condition.");
        auto body = ParseBlock();
        auto whileStmt = std::make_unique<WhileStmtNode>();
        whileStmt->condition = std::move(condition);
        whileStmt->body = std::move(body);
        return whileStmt;
    }

    ASTNodePtr Parser::ParseReturnStatement()
    {
        auto retStmt = std::make_unique<ReturnStmtNode>();
        if (!CheckType(TokenType::Punctuation) || std::string(Peek().lexeme) != ";")
        {
            retStmt->expression = ParseExpression();
        }
        ConsumePunctuation(";", "Expected ';' after return statement.");
        return retStmt;
    }

    ASTNodePtr Parser::ParseExpression()
    {
        return ParseAssignmentExpression();
    }

    ASTNodePtr Parser::ParseAssignmentExpression()
    {
        auto left = ParseLogicalOrExpression();
        // Use lookahead instead of Match to check for the assignment operator.
        if (CheckType(TokenType::Operator) && std::string(Peek().lexeme) == "=")
        {
            Advance(); // consume the "=" operator.
            auto binary = std::make_unique<BinaryExprNode>();
            binary->left = std::move(left);
            binary->op = BinaryOperator::Assign;
            binary->right = ParseAssignmentExpression();
            return binary;
        }
        return left;
    }

    ASTNodePtr Parser::ParseLogicalOrExpression()
    {
        auto expr = ParseLogicalAndExpression();
        // Only consume "||" operators.
        while (CheckType(TokenType::Operator) && std::string(Peek().lexeme) == "||")
        {
            Advance(); // consume the "||" token
            auto binary = std::make_unique<BinaryExprNode>();
            binary->left = std::move(expr);
            binary->op = BinaryOperator::LogicalOr;
            binary->right = ParseLogicalAndExpression();
            expr = std::move(binary);
        }
        return expr;
    }

    ASTNodePtr Parser::ParseLogicalAndExpression()
    {
        auto expr = ParseEqualityExpression();
        // Only consume "&&" operators.
        while (CheckType(TokenType::Operator) && std::string(Peek().lexeme) == "&&")
        {
            Advance(); // consume the "&&" token
            auto binary = std::make_unique<BinaryExprNode>();
            binary->left = std::move(expr);
            binary->op = BinaryOperator::LogicalAnd;
            binary->right = ParseEqualityExpression();
            expr = std::move(binary);
        }
        return expr;
    }

    ASTNodePtr Parser::ParseEqualityExpression()
    {
        auto expr = ParseRelationalExpression();
        while (CheckType(TokenType::Operator) &&
            (std::string(Peek().lexeme) == "==" || std::string(Peek().lexeme) == "!="))
        {
            // Now that we know the operator is one we want, consume it.
            std::string op = std::string(Advance().lexeme);
            auto binary = std::make_unique<BinaryExprNode>();
            binary->left = std::move(expr);
            binary->op = (op == "==") ? BinaryOperator::Equal : BinaryOperator::NotEqual;
            binary->right = ParseRelationalExpression();
            expr = std::move(binary);
        }
        return expr;
    }

    ASTNodePtr Parser::ParseRelationalExpression()
    {
        auto expr = ParseAdditiveExpression();
        // Use lookahead to check if the next token is a relational operator.
        if (CheckType(TokenType::Operator))
        {
            std::string op = std::string(Peek().lexeme);
            if (op == "<" || op == ">" || op == "<=" || op == ">=")
            {
                Advance(); // consume the relational operator
                auto binary = std::make_unique<BinaryExprNode>();
                binary->left = std::move(expr);
                if (op == "<")
                    binary->op = BinaryOperator::Less;
                else if (op == ">")
                    binary->op = BinaryOperator::Greater;
                else if (op == "<=")
                    binary->op = BinaryOperator::LessEqual;
                else
                    binary->op = BinaryOperator::GreaterEqual;
                binary->right = ParseAdditiveExpression();
                expr = std::move(binary);
            }
        }
        return expr;
    }

    ASTNodePtr Parser::ParseAdditiveExpression()
    {
        auto expr = ParseMultiplicativeExpression();
        while (CheckType(TokenType::Operator) &&
            (std::string(Peek().lexeme) == "+" || std::string(Peek().lexeme) == "-"))
        {
            // Only then consume the operator.
            std::string op = std::string(Advance().lexeme);
            auto binary = std::make_unique<BinaryExprNode>();
            binary->left = std::move(expr);
            binary->op = (op == "+") ? BinaryOperator::Add : BinaryOperator::Subtract;
            binary->right = ParseMultiplicativeExpression();
            expr = std::move(binary);
        }
        return expr;
    }

    ASTNodePtr Parser::ParseMultiplicativeExpression()
    {
        auto expr = ParseUnaryExpression();
        while (CheckType(TokenType::Operator) &&
            (std::string(Peek().lexeme) == "*" ||
                std::string(Peek().lexeme) == "/" ||
                std::string(Peek().lexeme) == "%"))
        {
            // Now that we know the operator is one we want, consume it.
            std::string op = std::string(Advance().lexeme);
            auto binary = std::make_unique<BinaryExprNode>();
            binary->left = std::move(expr);
            if (op == "*")
                binary->op = BinaryOperator::Multiply;
            else if (op == "/")
                binary->op = BinaryOperator::Divide;
            else // op == "%"
                binary->op = BinaryOperator::Modulo;
            binary->right = ParseUnaryExpression();
            expr = std::move(binary);
        }
        return expr;
    }

    ASTNodePtr Parser::ParseUnaryExpression()
    {
        if (Match({ TokenType::Operator }) &&
            (std::string(Previous().lexeme) == "-" || std::string(Previous().lexeme) == "!"))
        {
            auto unary = std::make_unique<UnaryExprNode>();
            unary->op = std::string(Previous().lexeme);
            unary->operand = ParseUnaryExpression();
            return unary;
        }
        return ParsePrimaryExpression();
    }

    std::vector<ASTNodePtr> Parser::ParseArgumentList()
    {
        std::vector<ASTNodePtr> arguments;
        if (!CheckType(TokenType::Punctuation) || std::string(Peek().lexeme) != ")") //check it's not an empty list.
        {
            do
            {
                arguments.push_back(ParseExpression());
            } while (Match({ TokenType::Punctuation }) && std::string(Previous().lexeme) == ","); //consume ","
        }
        return arguments;
    }

    ASTNodePtr Parser::ParsePrimaryExpression()
    {
        if (Match({ TokenType::Identifier }))
        {
            std::string name = std::string(Previous().lexeme);
            // Look ahead to see if the identifier is followed by an opening parenthesis,
            // indicating a function call or object instantiation.
            if (CheckType(TokenType::Punctuation) && std::string(Peek().lexeme) == "(")
            {
                Advance(); // consume the "("
                auto args = ParseArgumentList(); //Parse arguments.
                ConsumePunctuation(")", "Expected ')' after arguments.");

                // TODO: Distinguish between FunctionCall and ObjectInstantiation
                // during semantic analysis.  For now, we'll create a
                // FunctionCallNode.  Later, we'll add a lookup in the
                // symbol table.
                auto functionCallNode = std::make_unique<FunctionCallNode>();
                functionCallNode->name = name;
                functionCallNode->arguments = std::move(args);
                return functionCallNode;
            }
            auto idNode = std::make_unique<IdentifierNode>();
            idNode->name = name;
            return idNode;
        }
        if (Match({ TokenType::Number, TokenType::String, TokenType::Keyword }))
        {
            auto lit = std::make_unique<LiteralNode>();
            lit->value = std::string(Previous().lexeme);
            return lit;
        }
        if (Match({ TokenType::Punctuation }) && std::string(Previous().lexeme) == "(")
        {
            auto expr = ParseExpression();
            ConsumePunctuation(")", "Expected ')' after expression.");
            return expr;
        }
        if (Match({ TokenType::Punctuation }) && std::string(Previous().lexeme) == "[") //array literal
        {
            auto arrayLiteral = std::make_unique<ArrayLiteralNode>();
            if (CheckType(TokenType::Punctuation) && std::string(Peek().lexeme) == "]")
            {
                Advance(); // Consume the ']' of an empty array.
                return arrayLiteral; // Return empty array literal.
            }
            arrayLiteral->elements = ParseExpressionList();
            ConsumePunctuation("]", "Expected ']' after array elements.");

            return arrayLiteral;
        }
        ErrorAtCurrent("Expected expression");
        return nullptr;
    }

    std::vector<ASTNodePtr> Parser::ParseExpressionList()
    {
        std::vector<ASTNodePtr> expressions;
        expressions.push_back(ParseExpression()); // Parse the first expression
        while (Match({ TokenType::Punctuation }) && std::string(Previous().lexeme) == ",") //consume ","
        {
            expressions.push_back(ParseExpression()); //Parse subsequent expressions.
        }
        return expressions;
    }
} // namespace Arcanelab::Mano
