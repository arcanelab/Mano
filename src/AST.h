#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace Arcanelab::Mano
{
    // Forward declarations
    struct Symbol;
    struct Scope;

    enum class ASTType
    {
        Program,
        Type,
        VariableDeclaration,
        FunctionDeclaration,
        ClassDeclaration,
        EnumDeclaration,
        Block,
        ClassBlock,
        ExpressionStatement,
        ReturnStatement,
        IfStatement,
        ForStatement,
        WhileStatement,
        SwitchStatement,
        MemberAccess,
        IndexAccess,
        BinaryExpression,
        UnaryExpression,
        Literal,
        Identifier,
        BreakStatement,
        ContinueStatement,
        ArrayLiteral,
        FunctionCall,
        ObjectInstantiation
    };

    struct ASTNode
    {
        explicit ASTNode(ASTType type) : nodeType(type) {}
        virtual ~ASTNode() = default;
        ASTType nodeType;
    };

    using ASTNodePtr = std::unique_ptr<ASTNode>;

    struct ProgramNode : public ASTNode
    {
        ProgramNode() : ASTNode(ASTType::Program) {}
        std::vector<ASTNodePtr> declarations;
    };

    struct TypeNode : public ASTNode
    {
        TypeNode() : ASTNode(ASTType::Type) {}
        TypeNode(std::string name, bool isArray = false, bool isConst = false)
            : ASTNode(ASTType::Type),
            name(std::move(name)),
            array(isArray),
            isConst(isConst)
        {
        }

        std::string name;
        bool array = false;
        bool isConst = false;
    };

    using TypeNodePtr = std::unique_ptr<TypeNode>;

    struct VariableDeclarationNode : public ASTNode
    {
        VariableDeclarationNode()
            : ASTNode(ASTType::VariableDeclaration),
            symbol(nullptr)
        {
        }

        std::string name;         // From source code
        TypeNodePtr declaredType; // Type annotation
        TypeNodePtr resolvedType;
        ASTNodePtr initializer;   // Initial value
        Symbol* symbol;           // Semantic link

        // SourceLocation nameLocation; // Line/column info
    };

    struct FunctionDeclarationNode : public ASTNode
    {
        FunctionDeclarationNode()
            : ASTNode(ASTType::FunctionDeclaration),
            symbol(nullptr),
            functionScope(nullptr)
        {
        }

        std::string name;
        std::vector<std::pair<std::string, TypeNodePtr>> parameters;
        TypeNodePtr returnType;
        ASTNodePtr body;
        Symbol* symbol;
        Scope* functionScope;
    };

    struct ClassDeclarationNode : public ASTNode
    {
        ClassDeclarationNode()
            : ASTNode(ASTType::ClassDeclaration),
            symbol(nullptr),
            classScope(nullptr)
        {
        }

        std::string name;
        ASTNodePtr body;
        Symbol* symbol;
        Scope* classScope;
        std::unordered_map<std::string, Symbol*> methods;
    };

    struct EnumDeclarationNode : public ASTNode
    {
        EnumDeclarationNode() : ASTNode(ASTType::EnumDeclaration) {}
        std::string name;
        std::vector<std::string> values;
    };

    struct BlockNode : public ASTNode
    {
        BlockNode()
            : ASTNode(ASTType::Block),
            blockScope(nullptr),
            symbolsCollected(false)
        {
        }

        std::vector<ASTNodePtr> statements;
        Scope* blockScope;
        bool symbolsCollected;
    };

    struct ClassBlockNode : public ASTNode
    {
        ClassBlockNode()
            : ASTNode(ASTType::ClassBlock),
            classScope(nullptr),
            membersProcessed(false)
        {
        }

        std::vector<ASTNodePtr> declarations;
        Scope* classScope;
        bool membersProcessed;
    };

    struct ExpressionStatementNode : public ASTNode
    {
        ExpressionStatementNode() : ASTNode(ASTType::ExpressionStatement) {}
        ASTNodePtr expression;
    };

    struct ReturnStatementNode : public ASTNode
    {
        ReturnStatementNode()
            : ASTNode(ASTType::ReturnStatement),
            enclosingFunction(nullptr),
            hasValidReturnType(false)
        {
        }

        ASTNodePtr expression;
        FunctionDeclarationNode* enclosingFunction;
        bool hasValidReturnType;
    };

    struct IfStatementNode : public ASTNode
    {
        IfStatementNode() : ASTNode(ASTType::IfStatement) {}
        ASTNodePtr condition;
        ASTNodePtr thenBranch;
        ASTNodePtr elseBranch;
    };

    struct ForStatementNode : public ASTNode
    {
        // Add explicit constructor for base class
        ForStatementNode() : ASTNode(ASTType::ForStatement) {}

        ASTNodePtr init;       // Initializer (var decl or expr)
        ASTNodePtr condition;  // Loop condition
        ASTNodePtr update;     // Update expression (not 'increment')
        ASTNodePtr body;       // Loop body
    };

    struct WhileStatementNode : public ASTNode
    {
        WhileStatementNode() : ASTNode(ASTType::WhileStatement) {}
        ASTNodePtr condition;
        ASTNodePtr body;
    };

    struct SwitchStatementNode : public ASTNode
    {
        SwitchStatementNode() : ASTNode(ASTType::SwitchStatement) {}
        ASTNodePtr expression;
        std::vector<std::pair<ASTNodePtr, ASTNodePtr>> cases;
        ASTNodePtr defaultCase;
    };

    struct MemberAccessNode : public ASTNode
    {
        MemberAccessNode()
            : ASTNode(ASTType::MemberAccess),
            memberSymbol(nullptr)
        {
        }

        ASTNodePtr object;
        std::string memberName;
        Symbol* memberSymbol;
        TypeNodePtr objectType;
    };

    struct IndexAccessNode : public ASTNode
    {
        IndexAccessNode() : ASTNode(ASTType::IndexAccess) {}
        ASTNodePtr object;
        ASTNodePtr index;
    };

    enum class BinaryOperator
    {
        Assign, LogicalOr, LogicalAnd, BitwiseOr, BitwiseXor, BitwiseAnd,
        Equal, NotEqual, Less, Greater, LessEqual, GreaterEqual,
        LeftShift, RightShift, Add, Subtract, Multiply, Divide, Modulo
    };

    struct BinaryExpressionNode : public ASTNode
    {
        BinaryExpressionNode()
            : ASTNode(ASTType::BinaryExpression)
        {
        }

        ASTNodePtr left;
        BinaryOperator op;
        ASTNodePtr right;
        TypeNodePtr evaluatedType;
    };

    struct UnaryExpressionNode : public ASTNode
    {
        UnaryExpressionNode() : ASTNode(ASTType::UnaryExpression) {}
        std::string op;
        ASTNodePtr operand;
    };

    struct LiteralNode : public ASTNode
    {
        LiteralNode() : ASTNode(ASTType::Literal) {}
        std::string value;
    };

    struct IdentifierNode : public ASTNode
    {
        IdentifierNode()
            : ASTNode(ASTType::Identifier),
            resolvedSymbol(nullptr),
            evaluatedType(nullptr)
        {
        }

        std::string name;
        Symbol* resolvedSymbol;
        TypeNodePtr evaluatedType;
    };

    struct BreakStatementNode : public ASTNode
    {
        BreakStatementNode()
            : ASTNode(ASTType::BreakStatement),
            isInsideLoop(false)
        {
        }

        bool isInsideLoop;
    };

    struct ContinueStatementNode : public ASTNode
    {
        ContinueStatementNode()
            : ASTNode(ASTType::ContinueStatement),
            isInsideLoop(false)
        {
        }

        bool isInsideLoop;
    };

    struct ArrayLiteralNode : public ASTNode
    {
        ArrayLiteralNode()
            : ASTNode(ASTType::ArrayLiteral)
        {
        }

        std::vector<ASTNodePtr> elements;
        TypeNodePtr evaluatedType;
    };

    struct FunctionCallNode : public ASTNode
    {
        FunctionCallNode()
            : ASTNode(ASTType::FunctionCall),
            resolvedFunction(nullptr)
        {
        }

        std::string name;
        std::vector<ASTNodePtr> arguments;
        ASTNodePtr callTarget;
        Symbol* resolvedFunction;
        std::vector<TypeNodePtr> argumentTypes;
    };

    struct ObjectInstantiationNode : public ASTNode
    {
        ObjectInstantiationNode()
            : ASTNode(ASTType::ObjectInstantiation)
        {
        }

        std::string name;
        std::vector<ASTNodePtr> arguments;
    };
} // namespace Arcanelab::Mano
