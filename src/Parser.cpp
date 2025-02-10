#include "Parser.h"
#include <cassert>
#include <iostream>

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
        if (!IsAtEnd()) m_current++;
        return Previous();
    }

    bool Parser::CheckType(TokenType type) const
    {
        if (IsAtEnd()) return false;
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

    const Token& Parser::Consume(TokenType type, const std::string& message)
    {
        if (CheckType(type))
            return Advance();
        ErrorAtCurrent(message);
        return Peek(); // Will not reach due to exit in ErrorAtCurrent.
    }

    void Parser::ErrorAtCurrent(const std::string& message)
    {
        const Token& token = Peek();
        std::cerr << "[Line " << token.line << ", Column " << token.column
            << "] Error: " << message << "\n";
        exit(1);
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
        if (Match({ TokenType::Keyword }))
        {
            std::string kw(Previous().lexeme);
            if (kw == "let")
                return ParseConstantDeclaration();
            if (kw == "var")
                return ParseVariableDeclaration();
            if (kw == "fun")
                return ParseFunctionDeclaration();
            if (kw == "class")
                return ParseClassDeclaration();
            if (kw == "enum")
                return ParseEnumDeclaration();

            // For statements like if, for, while, etc.
            // we roll back if not a declaration.
            m_current--;
        }
        return ParseStatement();
    }

    // New constant declaration parser:
    // Grammar: let Identifier ":" PrimitiveType "=" Expression ";"
    ASTNodePtr Parser::ParseConstantDeclaration()
    {
        auto constDecl = std::make_unique<ConstDeclNode>();
        constDecl->name = Consume(TokenType::Identifier, "Expected constant name.").lexeme.data();
        Consume(TokenType::Punctuation, "Expected ':' after constant name.");
        // Use ParseType() restricting to primitive types—assuming the grammar only allows those.
        constDecl->typeName = ParseType();
        Consume(TokenType::Operator, "Expected '=' after constant type.");
        constDecl->initializer = ParseExpression();
        Consume(TokenType::Punctuation, "Expected ';' after constant declaration.");
        return constDecl;
    }

    ASTNodePtr Parser::ParseVariableDeclaration()
    {
        auto varDecl = std::make_unique<VarDeclNode>();
        varDecl->name = Consume(TokenType::Identifier, "Expected variable name.").lexeme.data();
        Consume(TokenType::Punctuation, "Expected ':' after variable name.");
        varDecl->typeName = ParseType();
        if (Match({ TokenType::Operator }) && Previous().lexeme == "=")
        {
            varDecl->initializer = ParseExpression();
        }
        Consume(TokenType::Punctuation, "Expected ';' after variable declaration.");
        return varDecl;
    }

    ASTNodePtr Parser::ParseFunctionDeclaration()
    {
        auto funDecl = std::make_unique<FunDeclNode>();
        funDecl->name = Consume(TokenType::Identifier, "Expected function name.").lexeme.data();
        Consume(TokenType::Punctuation, "Expected '(' after function name.");
        if (!CheckType(TokenType::Punctuation) || Peek().lexeme != ")")
        {
            ParseParameterList(funDecl->parameters);
        }
        Consume(TokenType::Punctuation, "Expected ')' after parameters.");
        if (Match({ TokenType::Punctuation }) && Previous().lexeme == ":")
        {
            funDecl->returnType = ParseType();
        }
        funDecl->body = ParseBlock();
        return funDecl;
    }

    void Parser::ParseParameterList(std::vector<std::pair<std::string, std::string>>& parameters)
    {
        do
        {
            std::string paramName = Consume(TokenType::Identifier, "Expected parameter name.").lexeme.data();
            Consume(TokenType::Punctuation, "Expected ':' after parameter name.");
            std::string paramType = ParseType();
            parameters.push_back({ paramName, paramType });
        } while (Match({ TokenType::Punctuation }) && Previous().lexeme == ",");
    }

    ASTNodePtr Parser::ParseClassDeclaration()
    {
        auto classDecl = std::make_unique<ClassDeclNode>();
        classDecl->name = Consume(TokenType::Identifier, "Expected class name.").lexeme.data();
        classDecl->body = ParseBlock();
        return classDecl;
    }

    ASTNodePtr Parser::ParseEnumDeclaration()
    {
        auto enumDecl = std::make_unique<EnumDeclNode>();
        enumDecl->name = Consume(TokenType::Identifier, "Expected enum name.").lexeme.data();
        enumDecl->body = ParseBlock();
        return enumDecl;
    }

    ASTNodePtr Parser::ParseBlock()
    {
        Consume(TokenType::Punctuation, "Expected '{' to start a block.");
        auto block = std::make_unique<BlockNode>();
        while (!CheckType(TokenType::Punctuation) || Peek().lexeme != "}")
        {
            block->statements.push_back(ParseDeclaration());
        }
        Consume(TokenType::Punctuation, "Expected '}' to close block.");
        return block;
    }

    ASTNodePtr Parser::ParseStatement()
    {
        if (Match({ TokenType::Keyword }))
        {
            std::string kw(Previous().lexeme);
            if (kw == "if")
                return ParseIfStatement();
            else if (kw == "for")
                return ParseForStatement();
            else if (kw == "while")
                return ParseWhileStatement();
            else if (kw == "return")
                return ParseReturnStatement();
            else
            {
                m_current--; // roll back, not a statement keyword we handle.
            }
        }
        auto expr = ParseExpression();
        Consume(TokenType::Punctuation, "Expected ';' after expression statement.");
        auto exprStmt = std::make_unique<ExprStmtNode>();
        exprStmt->expression = std::move(expr);
        return exprStmt;
    }

    ASTNodePtr Parser::ParseIfStatement()
    {
        Consume(TokenType::Punctuation, "Expected '(' after 'if'.");
        auto condition = ParseExpression();
        Consume(TokenType::Punctuation, "Expected ')' after if condition.");
        auto thenBranch = ParseBlock();
        ASTNodePtr elseBranch = nullptr;
        if (Match({ TokenType::Keyword }) && Previous().lexeme == "else")
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
        Consume(TokenType::Punctuation, "Expected '(' after 'for'.");
        ASTNodePtr init = nullptr;
        if (!CheckType(TokenType::Punctuation) || Peek().lexeme != ";")
            init = ParseDeclaration();
        Consume(TokenType::Punctuation, "Expected ';' after for initializer.");
        auto condition = ParseExpression();
        Consume(TokenType::Punctuation, "Expected ';' after for condition.");
        auto increment = ParseExpression();
        Consume(TokenType::Punctuation, "Expected ')' after for clauses.");
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
        Consume(TokenType::Punctuation, "Expected '(' after 'while'.");
        auto condition = ParseExpression();
        Consume(TokenType::Punctuation, "Expected ')' after while condition.");
        auto body = ParseBlock();
        auto whileStmt = std::make_unique<WhileStmtNode>();
        whileStmt->condition = std::move(condition);
        whileStmt->body = std::move(body);
        return whileStmt;
    }

    ASTNodePtr Parser::ParseReturnStatement()
    {
        auto retStmt = std::make_unique<ReturnStmtNode>();
        if (!CheckType(TokenType::Punctuation) || Peek().lexeme != ";")
        {
            retStmt->expression = ParseExpression();
        }
        Consume(TokenType::Punctuation, "Expected ';' after return statement.");
        return retStmt;
    }

    ASTNodePtr Parser::ParseExpression()
    {
        return ParseAssignmentExpression();
    }

    ASTNodePtr Parser::ParseAssignmentExpression()
    {
        auto left = ParseLogicalOrExpression();
        if (Match({ TokenType::Operator }) && Previous().lexeme == "=")
        {
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
        while (Match({ TokenType::Operator }) && Previous().lexeme == "||")
        {
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
        while (Match({ TokenType::Operator }) && Previous().lexeme == "&&")
        {
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
        while (Match({ TokenType::Operator }) &&
            (Previous().lexeme == "==" || Previous().lexeme == "!="))
        {
            auto binary = std::make_unique<BinaryExprNode>();
            binary->left = std::move(expr);
            binary->op = (Previous().lexeme == "==") ? BinaryOperator::Equal : BinaryOperator::NotEqual;
            binary->right = ParseRelationalExpression();
            expr = std::move(binary);
        }
        return expr;
    }

    ASTNodePtr Parser::ParseRelationalExpression()
    {
        auto expr = ParseAdditiveExpression();
        if (Match({ TokenType::Operator }))
        {
            std::string op = Previous().lexeme.data();
            if (op == "<" || op == ">" || op == "<=" || op == ">=")
            {
                auto binary = std::make_unique<BinaryExprNode>();
                binary->left = std::move(expr);
                if (op == "<") binary->op = BinaryOperator::Less;
                else if (op == ">") binary->op = BinaryOperator::Greater;
                else if (op == "<=") binary->op = BinaryOperator::LessEqual;
                else binary->op = BinaryOperator::GreaterEqual;
                binary->right = ParseAdditiveExpression();
                expr = std::move(binary);
            }
            else
            {
                m_current--;
            }
        }
        return expr;
    }

    ASTNodePtr Parser::ParseAdditiveExpression()
    {
        auto expr = ParseMultiplicativeExpression();
        while (Match({ TokenType::Operator }) &&
            (Previous().lexeme == "+" || Previous().lexeme == "-"))
        {
            auto binary = std::make_unique<BinaryExprNode>();
            binary->left = std::move(expr);
            binary->op = (Previous().lexeme == "+") ? BinaryOperator::Add : BinaryOperator::Subtract;
            binary->right = ParseMultiplicativeExpression();
            expr = std::move(binary);
        }
        return expr;
    }

    ASTNodePtr Parser::ParseMultiplicativeExpression()
    {
        auto expr = ParseUnaryExpression();
        while (Match({ TokenType::Operator }) &&
            (Previous().lexeme == "*" || Previous().lexeme == "/" || Previous().lexeme == "%"))
        {
            auto binary = std::make_unique<BinaryExprNode>();
            binary->left = std::move(expr);
            if (Previous().lexeme == "*")
                binary->op = BinaryOperator::Multiply;
            else if (Previous().lexeme == "/")
                binary->op = BinaryOperator::Divide;
            else
                binary->op = BinaryOperator::Modulo;
            binary->right = ParseUnaryExpression();
            expr = std::move(binary);
        }
        return expr;
    }

    ASTNodePtr Parser::ParseUnaryExpression()
    {
        if (Match({ TokenType::Operator }) &&
            (Previous().lexeme == "-" || Previous().lexeme == "!"))
        {
            auto unary = std::make_unique<UnaryExprNode>();
            unary->op = Previous().lexeme.data();
            unary->operand = ParseUnaryExpression();
            return unary;
        }
        return ParsePrimaryExpression();
    }

    ASTNodePtr Parser::ParsePrimaryExpression()
    {
        if (Match({ TokenType::Identifier }))
        {
            std::string name = Previous().lexeme.data();
            // Check for a function call or object instantiation
            if (Match({ TokenType::Punctuation }) && Previous().lexeme == "(")
            {
                // For simplicity, skip arguments and consume until ')'
                while (!CheckType(TokenType::Punctuation) || Peek().lexeme != ")")
                    Advance();
                Consume(TokenType::Punctuation, "Expected ')' after arguments.");
            }
            auto idNode = std::make_unique<IdentifierNode>();
            idNode->name = name;
            return idNode;
        }
        if (Match({ TokenType::Number, TokenType::String, TokenType::Keyword }))
        {
            auto lit = std::make_unique<LiteralNode>();
            lit->value = Previous().lexeme.data();
            return lit;
        }
        if (Match({ TokenType::Punctuation }) && Previous().lexeme == "(")
        {
            auto expr = ParseExpression();
            Consume(TokenType::Punctuation, "Expected ')' after expression.");
            return expr;
        }
        ErrorAtCurrent("Expected expression");
        return nullptr;
    }

    std::string Parser::ParseType()
    {
        // For an array type, expect a '[' token.
        if (Match({ TokenType::Punctuation }) && Previous().lexeme == "[")
        {
            std::string innerType = ParseType();
            Consume(TokenType::Punctuation, "Expected ']' after array type.");
            return "[" + innerType + "]";
        }
        // Otherwise, accept either a keyword (primitive type) or an identifier (user-defined type)
        if (CheckType(TokenType::Keyword) || CheckType(TokenType::Identifier))
        {
            return Advance().lexeme.data();
        }
        ErrorAtCurrent("Expected type");
        return "";
    }
} // namespace Arcanelab::Mano
