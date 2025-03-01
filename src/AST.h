#pragma once

#include <memory>
#include <string>
#include <vector>

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

    struct VariableDeclarationNode : public ASTNode
    {
        std::string name;
        TypeNodePtr type;
        ASTNodePtr initializer;
        // This node is used for both constants (let) and variables (var).
    };

    struct FunctionDeclarationNode : public ASTNode
    {
        std::string name;
        std::vector<std::pair<std::string, TypeNodePtr>> parameters;
        TypeNodePtr returnType;
        ASTNodePtr body;
    };

    struct ClassDeclarationNode : public ASTNode
    {
        std::string name;
        ASTNodePtr body;
    };

    struct EnumDeclarationNode : public ASTNode
    {
        std::string name;
        std::vector<std::string> values;
    };

    struct BlockNode : public ASTNode
    {
        std::vector<ASTNodePtr> statements;
    };

    struct ClassBlockNode : public ASTNode
    {
        std::vector<ASTNodePtr> declarations;
    };

    struct ExpressionStatementNode : public ASTNode
    {
        ASTNodePtr expression;
    };

    struct ReturnStatementNode : public ASTNode
    {
        ASTNodePtr expression;
    };

    struct IfStatementNode : public ASTNode
    {
        ASTNodePtr condition;
        ASTNodePtr thenBranch;
        ASTNodePtr elseBranch;
    };

    struct ForStatementNode : public ASTNode
    {
        ASTNodePtr init;
        ASTNodePtr condition;
        ASTNodePtr increment;
        ASTNodePtr body;
    };

    struct WhileStatementNode : public ASTNode
    {
        ASTNodePtr condition;
        ASTNodePtr body;
    };

    struct SwitchStatementNode : public ASTNode
    {
        ASTNodePtr expression;
        std::vector<std::pair<ASTNodePtr, ASTNodePtr>> cases; // (case_expr, block)
        ASTNodePtr defaultCase; // Optional default block
    };

    struct MemberAccessNode : public ASTNode
    {
        ASTNodePtr object;
        std::string memberName;
    };

    struct IndexAccessNode : public ASTNode
    {
        ASTNodePtr object;
        ASTNodePtr index;
    };

    enum class BinaryOperator
    {
        Assign,
        LogicalOr,
        LogicalAnd,
        BitwiseOr,
        BitwiseXor,
        BitwiseAnd,
        Equal,
        NotEqual,
        Less,
        Greater,
        LessEqual,
        GreaterEqual,
        LeftShift,
        RightShift,
        Add,
        Subtract,
        Multiply,
        Divide,
        Modulo
    };

    struct BinaryExpressionNode : public ASTNode
    {
        ASTNodePtr left;
        BinaryOperator op;
        ASTNodePtr right;
    };

    struct UnaryExpressionNode : public ASTNode
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

    struct BreakStatementNode : public ASTNode
    {
    };

    struct ContinueStatementNode : public ASTNode
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
        ASTNodePtr callTarget;
    };

    struct ObjectInstantiationNode : public ASTNode
    {
        std::string name;
        std::vector<ASTNodePtr> arguments;
    };
} // namespace
