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
            else
                ErrorAtCurrent("Declaration expected");
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
        return ParseStatement();
    }

    ASTNodePtr Parser::ParseConstantDeclaration()
    {
        // "let" has already been consumed.
        auto constDecl = std::make_unique<VarDeclNode>(); // Also used for constants.
        constDecl->name = std::string(Consume(TokenType::Identifier, "Expected constant name.").lexeme);
        ConsumePunctuation(":", "Expected ':' after constant name.");
        constDecl->typeName = std::string(Consume(TokenType::Keyword, "Expected type after ':'.").lexeme);
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
        varDecl->typeName = std::string(Consume(TokenType::Keyword, "Expected type name.").lexeme);
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
        if (!(CheckType(TokenType::Punctuation) && std::string(Peek().lexeme) == ")"))
        {
            ParseParameterList(funDecl->parameters);
        }
        ConsumePunctuation(")", "Expected ')' after parameters.");
        // Check for an optional return type.
        if (CheckType(TokenType::Punctuation) && std::string(Peek().lexeme) == ":")
        {
            Advance(); // Consume the colon.
            funDecl->returnType = std::string(Consume(TokenType::Keyword, "Expected return type.").lexeme);
        }
        funDecl->body = ParseBlock();
        return funDecl;
    }

    // Updated ParseParameterList so we don't consume the closing ')'
    void Parser::ParseParameterList(std::vector<std::pair<std::string, std::string>>& parameters)
    {
        // At least one parameter is expected.
        {
            std::string paramName = std::string(Consume(TokenType::Identifier, "Expected parameter name.").lexeme);
            ConsumePunctuation(":", "Expected ':' after parameter name.");
            std::string paramType = std::string(Consume(TokenType::Keyword, "Expected parameter type.").lexeme);
            parameters.push_back({ paramName, paramType });
        }
        // Now, while the next token is a comma, consume it and parse another parameter.
        while (CheckType(TokenType::Punctuation) && std::string(Peek().lexeme) == ",")
        {
            Advance(); // Consume the comma.
            std::string paramName = std::string(Consume(TokenType::Identifier, "Expected parameter name after comma.").lexeme);
            ConsumePunctuation(":", "Expected ':' after parameter name.");
            std::string paramType = std::string(Consume(TokenType::Keyword, "Expected parameter type.").lexeme);
            parameters.push_back({ paramName, paramType });
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
            block->statements.push_back(ParseDeclaration());
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
        auto expressionNode = std::make_unique<ExprStmtNode>();
        expressionNode->expression = std::move(expr);
        return expressionNode;
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
        if (!CheckType(TokenType::Punctuation) || std::string(Peek().lexeme) != ";")
            init = ParseDeclaration();
        // ConsumePunctuation(";", "Expected ';' after for initializer.");
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
                // Consume all tokens until we find the matching ")".
                while (!CheckType(TokenType::Punctuation) || std::string(Peek().lexeme) != ")")
                {
                    Advance();
                }
                ConsumePunctuation(")", "Expected ')' after arguments.");
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
        ErrorAtCurrent("Expected expression");
        return nullptr;
    }
} // namespace Arcanelab::Mano
