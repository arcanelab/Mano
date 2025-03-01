#pragma once
#include <AST.h>
#include <Lexer.h>

#include <string>

namespace Arcanelab::Mano
{
    class Parser
    {
    public:
        Parser(const std::vector<Token>& tokens);
        ASTNodePtr ParseProgram();

    private:
        const std::vector<Token>& m_tokens;
        size_t m_current;

        bool IsAtEnd() const;
        const Token& Peek() const;
        const Token& Previous() const;
        const Token& Advance();
        bool CheckType(TokenType type) const;
        bool Match(const std::initializer_list<TokenType>& types);
        bool MatchKeyword(const std::string& expected);
        bool MatchPunctuation(const std::string& expected);
        const Token& Consume(TokenType type, const std::string& message);
        const Token& ConsumePunctuation(const std::string& expected, const std::string& message);
        void ErrorAtCurrent(const std::string& message);

        TypeNodePtr ParseType(const bool isConst, const bool allowArrayType);
        ASTNodePtr ParseDeclaration();
        ASTNodePtr ParseVariableDeclaration(const bool isConst);
        ASTNodePtr ParseFunctionDeclaration();
        void ParseParameterList(std::vector<std::pair<std::string, TypeNodePtr>>& parameters);
        ASTNodePtr ParseClassDeclaration();
        std::vector<std::string> ParseEnumBlock();
        ASTNodePtr ParseEnumDeclaration();
        ASTNodePtr ParseBlock();
        ASTNodePtr ParseClassBlock();
        ASTNodePtr ParseStatement();
        ASTNodePtr ParseBreakStatement();
        ASTNodePtr ParseContinueStatement();
        ASTNodePtr ParseIfStatement();
        ASTNodePtr ParseForStatement();
        ASTNodePtr ParseWhileStatement();
        ASTNodePtr ParseReturnStatement();
        ASTNodePtr ParseExpression();
        ASTNodePtr ParseAssignmentExpression();
        ASTNodePtr ParseLogicalOrExpression();
        ASTNodePtr ParseLogicalAndExpression();
        ASTNodePtr ParseShiftExpression();
        ASTNodePtr ParseBitwiseOrExpression();
        ASTNodePtr ParseBitwiseXorExpression();
        ASTNodePtr ParseBitwiseAndExpression();
        ASTNodePtr ParseEqualityExpression();
        ASTNodePtr ParseRelationalExpression();
        ASTNodePtr ParseAdditiveExpression();
        ASTNodePtr ParseMultiplicativeExpression();
        ASTNodePtr ParseUnaryExpression();
        ASTNodePtr ParsePrimaryExpression();
        std::vector<ASTNodePtr> ParseArgumentList();
        std::vector<ASTNodePtr> ParseExpressionList();
        ASTNodePtr ParseSwitchStatement();
    };

} // namespace Arcanelab::Mano
