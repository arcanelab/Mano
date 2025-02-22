#pragma once

#include <Lexer.h>
#include <memory>
#include <string>
#include <vector>
#include <initializer_list>

namespace Arcanelab::Mano
{
    // --- AST hierarchy ---
    struct ASTNode
    {
        virtual ~ASTNode() = default;
    };

    using ASTNodePtr = std::unique_ptr<ASTNode>;

    struct ProgramNode : public ASTNode
    {
        std::vector<ASTNodePtr> declarations;
    };

    struct TypeNode : public ASTNode
    {
        std::string name;      // "int", "string", "[float]", etc.
        bool isConst;
    };

    using TypeNodePtr = std::unique_ptr<TypeNode>;

    struct VarDeclNode : public ASTNode
    {
        std::string name;
        TypeNodePtr type;
        ASTNodePtr initializer;
        // This node is used for both constants (let) and variables (var).
    };

    struct FunDeclNode : public ASTNode
    {
        std::string name;
        std::vector<std::pair<std::string, TypeNodePtr>> parameters;
        TypeNodePtr returnType;
        ASTNodePtr body;
    };

    struct ClassDeclNode : public ASTNode
    {
        std::string name;
        ASTNodePtr body;
    };

    struct EnumDeclNode : public ASTNode
    {
        std::string name;
        std::vector<std::string> values;
    };

    struct BlockNode : public ASTNode
    {
        std::vector<ASTNodePtr> statements;
    };

    struct ExprStmtNode : public ASTNode
    {
        ASTNodePtr expression;
    };

    struct ReturnStmtNode : public ASTNode
    {
        ASTNodePtr expression;
    };

    struct IfStmtNode : public ASTNode
    {
        ASTNodePtr condition;
        ASTNodePtr thenBranch;
        ASTNodePtr elseBranch;
    };

    struct ForStmtNode : public ASTNode
    {
        ASTNodePtr init;
        ASTNodePtr condition;
        ASTNodePtr increment;
        ASTNodePtr body;
    };

    struct WhileStmtNode : public ASTNode
    {
        ASTNodePtr condition;
        ASTNodePtr body;
    };

    enum class BinaryOperator
    {
        Assign,
        LogicalOr,
        LogicalAnd,
        Equal,
        NotEqual,
        Less,
        Greater,
        LessEqual,
        GreaterEqual,
        Add,
        Subtract,
        Multiply,
        Divide,
        Modulo
    };

    struct BinaryExprNode : public ASTNode
    {
        ASTNodePtr left;
        BinaryOperator op;
        ASTNodePtr right;
    };

    struct UnaryExprNode : public ASTNode
    {
        std::string op;
        ASTNodePtr operand;
    };

    struct LiteralNode : public ASTNode
    {
        std::string value;
    };

    struct IdentifierNode : public ASTNode
    {
        std::string name;
    };

    struct BreakStmtNode : public ASTNode
    {
    };

    struct ContinueStmtNode : public ASTNode
    {
    };

    struct ArrayLiteralNode : public ASTNode
    {
        std::vector<ASTNodePtr> elements;
    };

    struct FunctionCallNode : public ASTNode
    {
        std::string name;
        std::vector<ASTNodePtr> arguments;
    };

    struct ObjectInstantiationNode : public ASTNode
    {
        std::string name;
        std::vector<ASTNodePtr> arguments;
    };

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

        TypeNodePtr ParseType(const bool isConst);
        ASTNodePtr ParseDeclaration();
        ASTNodePtr ParseVariableDeclaration(const bool isConst);
        ASTNodePtr ParseFunctionDeclaration();
        void ParseParameterList(std::vector<std::pair<std::string, TypeNodePtr>>& parameters);
        ASTNodePtr ParseClassDeclaration();
        std::vector<std::string> ParseEnumBody();
        ASTNodePtr ParseEnumDeclaration();
        ASTNodePtr ParseBlock();
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
        ASTNodePtr ParseEqualityExpression();
        ASTNodePtr ParseRelationalExpression();
        ASTNodePtr ParseAdditiveExpression();
        ASTNodePtr ParseMultiplicativeExpression();
        ASTNodePtr ParseUnaryExpression();
        ASTNodePtr ParsePrimaryExpression();
        std::vector<ASTNodePtr> ParseArgumentList();
        std::vector<ASTNodePtr> ParseExpressionList();
    };

} // namespace Arcanelab::Mano
