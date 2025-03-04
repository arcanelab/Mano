#pragma once

#include <AST.h>

#include <sstream>

namespace Arcanelab::Mano
{
    struct Scope
    {
        std::unordered_map<std::string, std::unique_ptr<Symbol>> symbols;
        Scope* parent = nullptr;
        Symbol* Lookup(const std::string& name) const;
    };

    struct Symbol
    {
        enum class Kind { Variable, Function, Class, Enum, Type };
        Kind kind;
        std::string name;
        TypeNodePtr type;
        Scope* scope = nullptr;
        ASTNode* declarationSite = nullptr;
        bool isInitialized = false;
    };

    class SemanticAnalyzer
    {
    public:
        explicit SemanticAnalyzer(ASTNodePtr& root);
        bool Analyze();
        const std::vector<std::string>& GetErrors() const;

    private:
        ASTNodePtr& root;
        std::vector<std::unique_ptr<Scope>> scopeStack;
        std::vector<std::string> errors;
        FunctionDeclarationNode* currentFunction = nullptr;
        int loopDepth = 0;

        // Pass handlers
        void DeclarationPass(ASTNode* node);
        void TypeResolutionPass(ASTNode* node);
        void ValidationPass(ASTNode* node);

        // Declaration pass implementations
        void HandleProgramDeclaration(ProgramNode* program);
        void HandleFunctionDeclaration(FunctionDeclarationNode* func);
        void HandleClassDeclaration(ClassDeclarationNode* cls);
        void HandleVariableDeclaration(VariableDeclarationNode* var);
        void AddParameter(const std::string& name, TypeNode* type);

        // Type resolution implementations
        void ResolveVariableType(VariableDeclarationNode* var);
        void ResolveIdentifier(IdentifierNode* id);
        void ResolveBinaryExpression(BinaryExpressionNode* expr);
        void ResolveFunctionType(FunctionDeclarationNode* func);

        // Validation implementations
        void ValidateReturn(ReturnStatementNode* ret);
        void ValidateLoopControl(ASTNode* node);
        void ValidateFunction(FunctionDeclarationNode* func);
        void CheckForReturns(ASTNode* node, bool& hasReturn);

        // Helper methods
        void PushScope();
        void PopScope();
        Scope* currentScope() const;
        TypeNodePtr CloneType(TypeNode* type);
        bool CheckTypeCompatibility(const TypeNode& t1, const TypeNode& t2);
        bool IsArrayType(const TypeNode& type);
        bool CheckArrayCompatibility(const TypeNode& t1, const TypeNode& t2);
        Symbol* GetClassSymbol(const TypeNode& type);
        TypeNodePtr GetExpressionType(ASTNode* expr);
        TypeNodePtr GetLiteralType(LiteralNode* lit);

        // Loop handlers
        void HandleWhileLoop(WhileStatementNode* node);
        void HandleForLoop(ForStatementNode* node);

        template<typename T>
        void Error(const std::string& format, const T& arg)
        {
            std::ostringstream ss;
            ss << arg;
            errors.push_back(format + ss.str());
        }
        void Error(const std::string& message);

        struct IHasDeclarations
        {
            virtual std::vector<ASTNodePtr>& GetDeclarations() = 0;
            virtual ~IHasDeclarations() = default;
        };

        struct IHasChildren
        {
            virtual std::vector<ASTNodePtr>& GetChildren() = 0;
            virtual ~IHasChildren() = default;
        };
    };
} // namespace Arcanelab::Mano
