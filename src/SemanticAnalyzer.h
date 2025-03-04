#pragma once

#include <AST.h>

#include <memory>
#include <sstream>
#include <stdexcept>
#include <unordered_map>
#include <vector>

namespace Arcanelab::Mano
{
    struct Scope
    {
        std::unordered_map<std::string, std::unique_ptr<Symbol>> symbols;
        Scope* parent = nullptr;

        Symbol* Lookup(const std::string& name) const
        {
            if (auto it = symbols.find(name); it != symbols.end())
            {
                return it->second.get();
            }
            return parent ? parent->Lookup(name) : nullptr;
        }
    };

    struct Symbol
    {
        enum class Kind { Variable, Function, Class, Enum, Type };

        // Common fields
        Kind kind;
        std::string name;
        TypeNodePtr type;
        Scope* scope = nullptr;

        // Variable-specific
        ASTNode* declarationSite = nullptr;  // Back pointer to AST node
        bool isInitialized = false;
    };

    class SemanticAnalyzer
    {
    public:
        explicit SemanticAnalyzer(ASTNodePtr& root) : root(root) {}

        bool Analyze()
        {
            try
            {
                DeclarationPass(root.get());
                TypeResolutionPass(root.get());
                ValidationPass(root.get());
                return errors.empty();
            }
            catch (const std::exception& err)
            {
                errors.push_back(err.what());
                return false;
            }
        }

        const std::vector<std::string>& GetErrors() const { return errors; }

    private:
        ASTNodePtr& root;
        std::vector<std::unique_ptr<Scope>> scopeStack;
        std::vector<std::string> errors;
        FunctionDeclarationNode* currentFunction = nullptr;
        int loopDepth = 0;

        // Pass 1: Declaration collection
        void DeclarationPass(ASTNode* node)
        {
            switch (node->nodeType)
            {
                case ASTType::Program:
                    HandleProgramDeclaration(static_cast<ProgramNode*>(node));
                    break;
                case ASTType::FunctionDeclaration:
                    HandleFunctionDeclaration(static_cast<FunctionDeclarationNode*>(node));
                    break;
                case ASTType::ClassDeclaration:
                    HandleClassDeclaration(static_cast<ClassDeclarationNode*>(node));
                    break;
                case ASTType::VariableDeclaration:
                    HandleVariableDeclaration(static_cast<VariableDeclarationNode*>(node));
                    break;
                default:
                    if (auto* container = dynamic_cast<IHasDeclarations*>(node))
                    {
                        for (auto& child : container->GetDeclarations())
                        {
                            DeclarationPass(child.get());
                        }
                    }
            }
        }

        // Pass 2: Type resolution and checking
        void TypeResolutionPass(ASTNode* node)
        {
            switch (node->nodeType)
            {
                case ASTType::VariableDeclaration:
                    ResolveVariableType(static_cast<VariableDeclarationNode*>(node));
                    break;
                case ASTType::FunctionDeclaration:
                    ResolveFunctionType(static_cast<FunctionDeclarationNode*>(node));
                    break;
                case ASTType::BinaryExpression:
                    ResolveBinaryExpression(static_cast<BinaryExpressionNode*>(node));
                    break;
                case ASTType::Identifier:
                    ResolveIdentifier(static_cast<IdentifierNode*>(node));
                    break;
                case ASTType::WhileStatement:
                    HandleWhileLoop(static_cast<WhileStatementNode*>(node));
                    break;
                case ASTType::ForStatement:
                    HandleForLoop(static_cast<ForStatementNode*>(node));
                    break;
                default:
                    if (auto* container = dynamic_cast<IHasChildren*>(node))
                    {
                        for (auto& child : container->GetChildren())
                        {
                            TypeResolutionPass(child.get());
                        }
                    }
            }
        }

        // Pass 3: Semantic validation
        void ValidationPass(ASTNode* node)
        {
            switch (node->nodeType)
            {
                case ASTType::ReturnStatement:
                    ValidateReturn(static_cast<ReturnStatementNode*>(node));
                    break;
                case ASTType::BreakStatement:
                case ASTType::ContinueStatement:
                    ValidateLoopControl(node);
                    break;
                case ASTType::FunctionDeclaration:
                    ValidateFunction(static_cast<FunctionDeclarationNode*>(node));
                    break;
                default:
                    if (auto* container = dynamic_cast<IHasChildren*>(node))
                    {
                        for (auto& child : container->GetChildren())
                        {
                            ValidationPass(child.get());
                        }
                    }
            }
        }

        // Declaration pass handlers
        void HandleProgramDeclaration(ProgramNode* program)
        {
            PushScope();
            for (auto& decl : program->declarations)
            {
                DeclarationPass(decl.get());
            }
            PopScope();
        }

        void HandleFunctionDeclaration(FunctionDeclarationNode* function)
        {
            // Add function to current scope
            auto symbol = std::make_unique<Symbol>();
            symbol->kind = Symbol::Kind::Function;
            symbol->name = function->name;
            symbol->type = CloneType(function->returnType.get());
            function->symbol = symbol.get();
            currentScope()->symbols[function->name] = std::move(symbol);

            // Create parameter scope (immediately nested in function scope)
            PushScope();
            for (auto& [name, type] : function->parameters)
            {
                AddParameter(name, type.get());
            }

            // Create body scope (nested in parameter scope)
            PushScope();
        }

        void AddParameter(const std::string& name, TypeNode* type)
        {
            auto symbol = std::make_unique<Symbol>();
            symbol->kind = Symbol::Kind::Variable;
            symbol->name = name;
            symbol->type = CloneType(type);
            currentScope()->symbols[name] = std::move(symbol);
        }

        void ResolveFunctionType(FunctionDeclarationNode* function)
        {
            // Process body in existing scope
            if (auto* body = dynamic_cast<BlockNode*>(function->body.get()))
            {
                for (auto& statement : body->statements)
                {
                    TypeResolutionPass(statement.get());
                }
            }

            PopScope();  // Body scope
            PopScope();  // Parameter scope
        }


        void HandleClassDeclaration(ClassDeclarationNode* classDeclaration)
        {
            auto sym = std::make_unique<Symbol>();
            sym->kind = Symbol::Kind::Class;
            sym->name = classDeclaration->name;
            classDeclaration->symbol = sym.get();
            currentScope()->symbols[classDeclaration->name] = std::move(sym);

            PushScope();
            if (auto* classBlock = dynamic_cast<ClassBlockNode*>(classDeclaration->body.get()))
            {
                for (auto& declaration : classBlock->declarations)
                {
                    DeclarationPass(declaration.get());
                }
            }
            PopScope();
        }

        // Type resolution handlers
        void ResolveVariableType(VariableDeclarationNode* variable)
        {
            // Ensure type annotation exists
            if (!variable->declaredType)
            {
                Error("Missing type annotation for variable: " + variable->name);
                return;
            }

            // Resolve initializer type if present
            if (variable->initializer)
            {
                TypeNodePtr initType = GetExpressionType(variable->initializer.get());

                // Check compatibility with declared type
                if (!CheckTypeCompatibility(*variable->declaredType, *initType))
                {
                    std::stringstream ss;
                    ss << "Type mismatch in variable '" << variable->name << "'. "
                        << "Declared: " << variable->declaredType->name
                        << ", Inferred: " << initType->name;
                    Error(ss.str());
                }
            }

            // Finalize resolved type
            variable->resolvedType = CloneType(variable->declaredType.get());
        }

        void ResolveIdentifier(IdentifierNode* identifier)
        {
            if (auto* symbol = currentScope()->Lookup(identifier->name))
            {
                identifier->resolvedSymbol = symbol;
                identifier->evaluatedType = CloneType(symbol->type.get());
            }
            else
            {
                Error("Undefined identifier: " + identifier->name);
            }
        }

        // Validation handlers
        void ValidateReturn(ReturnStatementNode* returnStatement)
        {
            if (!currentFunction)
            {
                Error("Return statement outside function");
                return;
            }

            TypeNodePtr returnType;
            if (returnStatement->expression)
            {
                returnType = GetExpressionType(returnStatement->expression.get());
            }
            else
            {
                returnType = std::make_unique<TypeNode>("void", false);
            }

            if (!CheckTypeCompatibility(*currentFunction->returnType, *returnType))
            {
                Error("Return type mismatch in function " + currentFunction->name);
            }
        }

        // Helper methods
        void Error(const std::string& message)
        {
            errors.push_back(message);
        }

        template<typename T>
        void Error(const std::string& format, const T& arg)
        {
            std::ostringstream ss;
            ss << arg;
            errors.push_back(format + ss.str());
        }

        void PushScope()
        {
            auto scope = std::make_unique<Scope>();
            scope->parent = currentScope();
            scopeStack.push_back(std::move(scope));
        }

        void PopScope()
        {
            if (!scopeStack.empty())
            {
                scopeStack.pop_back();
            }
        }

        Scope* currentScope() const
        {
            return scopeStack.empty() ? nullptr : scopeStack.back().get();
        }

        TypeNodePtr CloneType(TypeNode* type)
        {
            return std::make_unique<TypeNode>(*type);
        }

        bool CheckTypeCompatibility(const TypeNode& t1, const TypeNode& t2)
        {
            if (t1.name == t2.name) return true;

            // Handle array types
            if (IsArrayType(t1) && IsArrayType(t2))
            {
                return CheckArrayCompatibility(t1, t2);
            }

            // Handle class inheritance
            // if (auto* c1 = GetClassSymbol(t1), *c2 = GetClassSymbol(t2))
            // {
            //     return IsDerivedFrom(c1, c2);
            // }

            return false;
        }

        void HandleVariableDeclaration(VariableDeclarationNode* variable)
        {
            // Check for duplicates first
            if (currentScope()->symbols.contains(variable->name))
            {
                Error("Duplicate variable declaration: " + variable->name);
                return;
            }

            // Validate type annotation exists
            if (!variable->declaredType)
            {
                Error("Missing type annotation for variable: " + variable->name);
                return;
            }

            // Create and link symbol
            auto symbol = std::make_unique<Symbol>();
            symbol->kind = Symbol::Kind::Variable;
            symbol->name = variable->name;
            symbol->type = CloneType(variable->declaredType.get());
            symbol->declarationSite = variable;
            symbol->scope = currentScope();  // Track declaration scope
            symbol->isInitialized = variable->initializer != nullptr;  // Set initialization status

            variable->symbol = symbol.get();
            currentScope()->symbols[variable->name] = std::move(symbol);
        }

        void ResolveBinaryExpression(BinaryExpressionNode* expression)
        {
            TypeResolutionPass(expression->left.get());
            TypeResolutionPass(expression->right.get());

            auto leftType = GetExpressionType(expression->left.get());
            auto rightType = GetExpressionType(expression->right.get());

            // Handle assignment separately
            if (expression->op == BinaryOperator::Assign)
            {
                if (!CheckTypeCompatibility(*leftType, *rightType))
                {
                    Error("Assignment type mismatch");
                }
                expression->evaluatedType = CloneType(leftType.get());
                return;
            }

            // Check operand compatibility
            if (!CheckTypeCompatibility(*leftType, *rightType))
            {
                Error("Operand type mismatch in binary expression");
            }

            // Determine result type
            switch (expression->op)
            {
                case BinaryOperator::LogicalAnd:
                case BinaryOperator::LogicalOr:
                    expression->evaluatedType = std::make_unique<TypeNode>("bool", false);
                    break;
                case BinaryOperator::Equal:
                case BinaryOperator::NotEqual:
                case BinaryOperator::Less:
                case BinaryOperator::Greater:
                case BinaryOperator::LessEqual:
                case BinaryOperator::GreaterEqual:
                    expression->evaluatedType = std::make_unique<TypeNode>("bool", false);
                    break;
                default:  // Arithmetic operations
                    expression->evaluatedType = CloneType(leftType.get());
            }
        }

        TypeNodePtr GetExpressionType(ASTNode* expression)
        {
            switch (expression->nodeType)
            {
                case ASTType::Identifier:
                    return CloneType(static_cast<IdentifierNode*>(expression)->evaluatedType.get());
                case ASTType::Literal:
                    return GetLiteralType(static_cast<LiteralNode*>(expression));
                case ASTType::BinaryExpression:
                    return CloneType(static_cast<BinaryExpressionNode*>(expression)->evaluatedType.get());
                case ASTType::FunctionCall:
                    return CloneType(static_cast<FunctionCallNode*>(expression)->resolvedFunction->type.get());
                case ASTType::ArrayLiteral:
                    return CloneType(static_cast<ArrayLiteralNode*>(expression)->evaluatedType.get());
                default:
                    throw std::runtime_error("Unsupported expression type");
            }
        }

        TypeNodePtr GetLiteralType(LiteralNode* literal)
        {
            auto type = std::make_unique<TypeNode>();
            if (literal->value.find('.') != std::string::npos)
            {
                type->name = "float";
            }
            else if (literal->value == "true" || literal->value == "false")
            {
                type->name = "bool";
            }
            else if (literal->value.front() == '"' && literal->value.back() == '"')
            {
                type->name = "string";
            }
            else
            {
                type->name = "int";
            }
            return type;
        }

        void ValidateLoopControl(ASTNode* node)
        {
            if (loopDepth == 0)
            {
                const char* msg = (node->nodeType == ASTType::BreakStatement)
                    ? "Break statement outside loop"
                    : "Continue statement outside loop";
                Error(msg);
            }
        }

        void ValidateFunction(FunctionDeclarationNode* function)
        {
            TypeNode* returnType = function->symbol->type.get();
            if (returnType->name == "void") return;

            bool hasReturn = false;
            CheckForReturns(function->body.get(), hasReturn);

            if (!hasReturn)
            {
                Error("Function '" + function->name + "' with return type '" +
                    returnType->name + "' lacks return statement");
            }
        }

        void CheckForReturns(ASTNode* node, bool& hasReturn)
        {
            if (hasReturn) return;  // Early exit if already found

            switch (node->nodeType)
            {
                case ASTType::ReturnStatement:
                    hasReturn = true;
                    break;

                case ASTType::IfStatement: {
                    auto* ifstatement = static_cast<IfStatementNode*>(node);
                    CheckForReturns(ifstatement->thenBranch.get(), hasReturn);
                    if (ifstatement->elseBranch)
                    {
                        CheckForReturns(ifstatement->elseBranch.get(), hasReturn);
                    }
                    break;
                }

                default:
                    if (auto* container = dynamic_cast<IHasChildren*>(node))
                    {
                        for (auto& child : container->GetChildren())
                        {
                            CheckForReturns(child.get(), hasReturn);
                            if (hasReturn) break;  // Stop checking siblings
                        }
                    }
            }
        }

        bool IsArrayType(const TypeNode& type)
        {
            return type.name.size() > 2 &&
                type.name.front() == '[' &&
                type.name.back() == ']';
        }

        bool CheckArrayCompatibility(const TypeNode& t1, const TypeNode& t2)
        {
            // Extract element types (assuming format "[ElementType]")
            std::string elem1 = t1.name.substr(1, t1.name.size() - 2);
            std::string elem2 = t2.name.substr(1, t2.name.size() - 2);

            // Create temporary TypeNodes with proper initialization
            TypeNode tn1{ elem1, false, false };  // name, isArray=false, isConst=false
            TypeNode tn2{ elem2, false, false };

            return CheckTypeCompatibility(tn1, tn2);
        }

        Symbol* GetClassSymbol(const TypeNode& type)
        {
            if (auto* sym = currentScope()->Lookup(type.name))
            {
                return sym->kind == Symbol::Kind::Class ? sym : nullptr;
            }
            return nullptr;
        }

        void HandleWhileLoop(WhileStatementNode* node)
        {
            // Check condition type
            TypeResolutionPass(node->condition.get());
            TypeNodePtr condType = GetExpressionType(node->condition.get());
            if (condType->name != "bool")
            {
                Error("While condition must be boolean");
            }

            // Process loop body
            loopDepth++;
            TypeResolutionPass(node->body.get());
            loopDepth--;
        }

        void HandleForLoop(ForStatementNode* node)
        {
            // Process initializer
            if (node->init)
            {
                TypeResolutionPass(node->init.get());
            }

            // Check condition
            if (node->condition)
            {
                TypeNodePtr condType = GetExpressionType(node->condition.get());
                if (condType->name != "bool")
                {
                    Error("For loop condition must be boolean");
                }
            }

            // Process increment
            if (node->update)
            {
                TypeResolutionPass(node->update.get());
            }

            // Process body
            loopDepth++;
            TypeResolutionPass(node->body.get());
            loopDepth--;
        }

        // Interface helpers
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
