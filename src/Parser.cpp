#include "Parser.h"
#include "Lexer.h"
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

    bool Parser::MatchKeyword(const std::string& expected)
    {
        if (CheckType(TokenType::Keyword) && Peek().lexeme == expected)
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

    bool Parser::MatchPunctuation(const std::string& expected)
    {
        if (CheckType(TokenType::Punctuation) && Peek().lexeme == expected)
        {
            Advance();
            return true;
        }
        return false;
    }

    const Token& Parser::ConsumePunctuation(const std::string& expected, const std::string& message)
    {
        if (!IsAtEnd() && Peek().type == TokenType::Punctuation && Peek().lexeme == expected)
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
            return ParseVariableDeclaration(true);
        if (MatchKeyword("var"))
            return ParseVariableDeclaration(false);
        if (MatchKeyword("fun"))
            return ParseFunctionDeclaration();
        if (MatchKeyword("class"))
            return ParseClassDeclaration();
        if (MatchKeyword("enum"))
            return ParseEnumDeclaration();

        ErrorAtCurrent("Expected declaration.");
        return nullptr;
    }

    TypeNodePtr Parser::ParseType(const bool isConst, const bool allowArrayType = true)
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
            if (!allowArrayType)
            {
                ErrorAtCurrent("Nested arrays not supported.");
            }
            //Handle array type.
            auto arrayType = ParseType(false, false); // Recursively parse element type to the 1st order, disallow array types.
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

    ASTNodePtr Parser::ParseVariableDeclaration(const bool isConst)
    {
        auto varDecl = std::make_unique<VariableDeclarationNode>();
        varDecl->name = std::string(Consume(TokenType::Identifier, "Expected variable name.").lexeme);
        ConsumePunctuation(":", "Expected ':' after variable name.");
        auto typeNode = ParseType(isConst);
        varDecl->type = std::move(typeNode);

        if (Match({ TokenType::Operator }) && std::string(Previous().lexeme) == "=")
        {
            varDecl->initializer = ParseExpression();
        }
        else
        {
            ErrorAtCurrent("Expected '=' after type for " + std::string(isConst ? "constant" : "variable") + " declaration.");
        }
        ConsumePunctuation(";", "Expected ';' after variable declaration.");
        return varDecl;
    }

    ASTNodePtr Parser::ParseFunctionDeclaration()
    {
        auto funDecl = std::make_unique<FunctionDeclarationNode>();
        funDecl->name = std::string(Consume(TokenType::Identifier, "Expected function name.").lexeme);
        ConsumePunctuation("(", "Expected '(' after function name.");
        // Only parse parameters if the next token is not a closing parenthesis.
        if (CheckType(TokenType::Identifier)) //we now check if it's an identifier
        {
            ParseParameterList(funDecl->parameters);
        }
        ConsumePunctuation(")", "Expected ')' after parameters.");
        // Check for an optional return type.
        if (CheckType(TokenType::Punctuation) && Peek().lexeme == ":")
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
            bool isConst = CheckType(TokenType::Keyword) && Peek().lexeme == "const";
            if (isConst) Advance();
            TypeNodePtr paramType = ParseType(isConst);
            parameters.push_back({ paramName, std::move(paramType) });
        }

        while (CheckType(TokenType::Punctuation) && Peek().lexeme == ",")
        {
            Advance(); // Consume the comma.
            std::string paramName = std::string(Consume(TokenType::Identifier, "Expected parameter name after comma.").lexeme);
            ConsumePunctuation(":", "Expected ':' after parameter name.");
            bool isConst = CheckType(TokenType::Keyword) && Peek().lexeme == "const";
            if (isConst) Advance();
            TypeNodePtr paramType = ParseType(isConst);
            parameters.push_back({ paramName, std::move(paramType) });
        }
    }

    ASTNodePtr Parser::ParseClassDeclaration()
    {
        auto classDecl = std::make_unique<ClassDeclarationNode>();
        classDecl->name = std::string(Consume(TokenType::Identifier, "Expected class name.").lexeme);
        classDecl->body = ParseClassBlock();
        return classDecl;
    }

    ASTNodePtr Parser::ParseEnumDeclaration()
    {
        auto enumDecl = std::make_unique<EnumDeclarationNode>();
        enumDecl->name = std::string(Consume(TokenType::Identifier, "Expected enum name.").lexeme);
        enumDecl->values = ParseEnumBlock();
        return enumDecl;
    }

    std::vector<std::string> Parser::ParseEnumBlock()
    {
        std::vector<std::string> values;
        ConsumePunctuation("{", "Expected '{' to start enum body.");

        // Empty enum.
        if (CheckType(TokenType::Punctuation) && Peek().lexeme == "}")
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
            if (CheckType(TokenType::Punctuation) && Peek().lexeme == ",")
            {
                Advance(); // consume comma
                if (CheckType(TokenType::Punctuation) && Peek().lexeme == "}") // optional last comma
                {
                    break;
                }
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
        while (!CheckType(TokenType::Punctuation) || Peek().lexeme != "}")
        {
            //Check for declaration keywords first.
            if (CheckType(TokenType::Keyword) &&
                (Peek().lexeme == "let" ||
                    Peek().lexeme == "var" ||
                    Peek().lexeme == "fun" ||
                    Peek().lexeme == "class" ||
                    Peek().lexeme == "enum"))
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

    ASTNodePtr Parser::ParseClassBlock()
    {
        ConsumePunctuation("{", "Expected '{' to start a class block.");
        auto block = std::make_unique<ClassBlockNode>();
        while (!CheckType(TokenType::Punctuation) || Peek().lexeme != "}")
        {
            if (CheckType(TokenType::Keyword) &&
                (Peek().lexeme == "let" ||
                    Peek().lexeme == "var" ||
                    Peek().lexeme == "fun" ||
                    Peek().lexeme == "class" ||
                    Peek().lexeme == "enum"))
            {
                block->declarations.push_back(ParseDeclaration());
            }
            else
            {
                ErrorAtCurrent("Expected declaration.");
                break;
            }
        }
        ConsumePunctuation("}", "Expected '}' to close class block.");
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
        if (MatchKeyword("switch")) return ParseSwitchStatement();

        // If none of the above, it must be an expression statement.
        auto expression = ParseExpression();

        bool isAssignmentNode = false;
        if (BinaryExpressionNode* binaryExpressionNode = dynamic_cast<BinaryExpressionNode*>(expression.get()))
        {
            isAssignmentNode = (binaryExpressionNode->op == BinaryOperator::Assign);
        }

        if (isAssignmentNode || dynamic_cast<FunctionCallNode*>(expression.get())) // assignment or function call
        {
            ConsumePunctuation(";", "Expected ';' after expression statement.");
            auto expressionNode = std::make_unique<ExpressionStatementNode>();
            expressionNode->expression = std::move(expression);
            return expressionNode;
        }
        else
        {
            ErrorAtCurrent("Expected statement.");
        }
        return nullptr;
    }

     ASTNodePtr Parser::ParseBreakStatement()
    {
        ConsumePunctuation(";", "Expected ';' after 'break'.");
        return std::make_unique<BreakStatementNode>();
    }

    ASTNodePtr Parser::ParseContinueStatement()
    {
        ConsumePunctuation(";", "Expected ';' after 'break'.");
        return std::make_unique<ContinueStatementNode>();
    }

    ASTNodePtr Parser::ParseIfStatement()
    {
        ConsumePunctuation("(", "Expected '(' after 'if'.");
        auto condition = ParseExpression();
        ConsumePunctuation(")", "Expected ')' after if condition.");
        auto thenBranch = ParseBlock();
        ASTNodePtr elseBranch = nullptr;
        if (MatchKeyword("else"))
        {
            elseBranch = ParseBlock();
        }
        auto ifStmt = std::make_unique<IfStatementNode>();
        ifStmt->condition = std::move(condition);
        ifStmt->thenBranch = std::move(thenBranch);
        ifStmt->elseBranch = std::move(elseBranch);
        return ifStmt;
    }

    ASTNodePtr Parser::ParseForStatement()
    {
        ConsumePunctuation("(", "Expected '(' after 'for'.");
        ASTNodePtr init = nullptr;

        if (MatchKeyword("var"))
            init = ParseVariableDeclaration(false);

        auto condition = ParseExpression();
        ConsumePunctuation(";", "Expected ';' after for condition.");
        auto increment = ParseExpression();
        ConsumePunctuation(")", "Expected ')' after for clauses.");
        auto body = ParseBlock();

        auto forStmt = std::make_unique<ForStatementNode>();
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
        auto whileStmt = std::make_unique<WhileStatementNode>();
        whileStmt->condition = std::move(condition);
        whileStmt->body = std::move(body);
        return whileStmt;
    }

    ASTNodePtr Parser::ParseReturnStatement()
    {
        auto retStmt = std::make_unique<ReturnStatementNode>();
        if (!CheckType(TokenType::Punctuation) || Peek().lexeme != ";")
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
        if (CheckType(TokenType::Operator) && Peek().lexeme == "=")
        {
            Advance(); // consume the "=" operator.
            auto binary = std::make_unique<BinaryExpressionNode>();
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
        while (CheckType(TokenType::Operator) && Peek().lexeme == "||")
        {
            Advance(); // consume the "||" token
            auto binary = std::make_unique<BinaryExpressionNode>();
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
        while (CheckType(TokenType::Operator) && Peek().lexeme == "&&")
        {
            Advance(); // consume the "&&" token
            auto binary = std::make_unique<BinaryExpressionNode>();
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
            (Peek().lexeme == "==" || Peek().lexeme == "!="))
        {
            // Now that we know the operator is one we want, consume it.
            std::string op = std::string(Advance().lexeme);
            auto binary = std::make_unique<BinaryExpressionNode>();
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
                auto binary = std::make_unique<BinaryExpressionNode>();
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
            (Peek().lexeme == "+" || Peek().lexeme == "-"))
        {
            std::string op = std::string(Advance().lexeme);
            auto binary = std::make_unique<BinaryExpressionNode>();
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
            (Peek().lexeme == "*" ||
                Peek().lexeme == "/" ||
                Peek().lexeme == "%"))
        {
            // Now that we know the operator is one we want, consume it.
            std::string op = std::string(Advance().lexeme);
            auto binary = std::make_unique<BinaryExpressionNode>();
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
            auto unary = std::make_unique<UnaryExpressionNode>();
            unary->op = std::string(Previous().lexeme);
            unary->operand = ParseUnaryExpression();
            return unary;
        }
        return ParsePrimaryExpression();
    }

    std::vector<ASTNodePtr> Parser::ParseArgumentList()
    {
        std::vector<ASTNodePtr> arguments;
        if (!CheckType(TokenType::Punctuation) || Peek().lexeme != ")") //check it's not an empty list.
        {
            arguments.push_back(ParseExpression());
            while (CheckType(TokenType::Punctuation) && Peek().lexeme == ",")
            {
                Advance(); // Consume ","
                arguments.push_back(ParseExpression());
            }
        }
        return arguments;
    }

    ASTNodePtr Parser::ParsePrimaryExpression()
    {
        if (Match({ TokenType::Identifier }))
        {
            std::string name = std::string(Previous().lexeme);
            if (CheckType(TokenType::Punctuation) && Peek().lexeme == "(")
            {
                Advance(); // consume "("
                auto args = ParseArgumentList();
                ConsumePunctuation(")", "Expected ')' after arguments");
                auto functionCallNode = std::make_unique<FunctionCallNode>();
                functionCallNode->name = name;
                functionCallNode->arguments = std::move(args);
                return functionCallNode;
            }

            ASTNodePtr expr = std::make_unique<IdentifierNode>();
            static_cast<IdentifierNode*>(expr.get())->name = name;

            while (true)
            {
                // Handle member accesses (x.y)
                if (MatchPunctuation("."))
                {
                    auto memberAccess = std::make_unique<MemberAccessNode>();
                    memberAccess->object = std::move(expr);
                    memberAccess->memberName = std::string(
                        Consume(TokenType::Identifier, "Expected member name after '.'").lexeme
                    );
                    expr = std::move(memberAccess);
                }
                // Handle method calls (x.y())
                else if (CheckType(TokenType::Punctuation) && Peek().lexeme == "(")
                {
                    Advance(); // consume "("
                    auto args = ParseArgumentList();
                    ConsumePunctuation(")", "Expected ')' after arguments");

                    auto methodCall = std::make_unique<FunctionCallNode>();
                    methodCall->callTarget = std::move(expr); // Store member access
                    methodCall->arguments = std::move(args);
                    expr = std::move(methodCall);
                }
                else
                {
                    break; // No more member accesses/method calls
                }
            }
            return expr;
        }

        if (Match({ TokenType::Number, TokenType::String, TokenType::Keyword }))
        {
            auto lit = std::make_unique<LiteralNode>();
            lit->value = std::string(Previous().lexeme);
            return lit;
        }

        if (MatchPunctuation("("))
        {
            auto expr = ParseExpression();
            ConsumePunctuation(")", "Expected ')' after expression.");
            return expr;
        }

        if (MatchPunctuation("["))
        {
            auto arrayLiteral = std::make_unique<ArrayLiteralNode>();
            if (CheckType(TokenType::Punctuation) && Peek().lexeme == "]")
            {
                Advance();
                return arrayLiteral;
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
        while (CheckType(TokenType::Punctuation) && Peek().lexeme == ",")
        {
            Advance();
            expressions.push_back(ParseExpression()); //Parse subsequent expressions.
        }
        return expressions;
    }

    ASTNodePtr Parser::ParseSwitchStatement()
    {
        ConsumePunctuation("(", "Expected '(' after 'switch'.");
        auto expr = ParseExpression();
        ConsumePunctuation(")", "Expected ')' after switch expression.");
        ConsumePunctuation("{", "Expected '{' to start switch body.");

        auto switchNode = std::make_unique<SwitchStatementNode>();
        switchNode->expression = std::move(expr);

        while (!CheckType(TokenType::Punctuation) || Peek().lexeme != "}")
        {
            if (MatchKeyword("case"))
            {
                auto caseExpr = ParseExpression();
                ConsumePunctuation(":", "Expected ':' after case expression.");
                auto caseBlock = ParseBlock();
                switchNode->cases.emplace_back(std::move(caseExpr), std::move(caseBlock));
            }
            else if (MatchKeyword("default"))
            {
                ConsumePunctuation(":", "Expected ':' after 'default'.");
                auto defaultBlock = ParseBlock();
                if (switchNode->defaultCase)
                {
                    ErrorAtCurrent("Multiple default clauses in switch statement.");
                }
                switchNode->defaultCase = std::move(defaultBlock);
            }
            else
            {
                ErrorAtCurrent("Expected 'case' or 'default' in switch statement.");
            }
        }

        ConsumePunctuation("}", "Expected '}' to close switch body.");
        return switchNode;
    }
} // namespace Arcanelab::Mano
