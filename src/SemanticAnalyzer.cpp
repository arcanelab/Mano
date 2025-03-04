#include "SemanticAnalyzer.h"

#include <memory>
#include <sstream>
#include <stdexcept>
#include <unordered_map>
#include <vector>

namespace Arcanelab::Mano
{
    // Scope implementation
    Symbol* Scope::Lookup(const std::string& name) const
    {
        if (auto it = symbols.find(name); it != symbols.end())
            return it->second.get();
        return parent ? parent->Lookup(name) : nullptr;
    }

    // SemanticAnalyzer implementation
    SemanticAnalyzer::SemanticAnalyzer(ASTNodePtr& root) : root(root) {}

    bool SemanticAnalyzer::Analyze()
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

    const std::vector<std::string>& SemanticAnalyzer::GetErrors() const
    {
        return errors;
    }

    void SemanticAnalyzer::DeclarationPass(ASTNode* node)
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

    void SemanticAnalyzer::TypeResolutionPass(ASTNode* node)
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

    void SemanticAnalyzer::ValidationPass(ASTNode* node)
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

    void SemanticAnalyzer::HandleProgramDeclaration(ProgramNode* program)
    {
        PushScope();
        for (auto& decl : program->declarations)
        {
            DeclarationPass(decl.get());
        }
        PopScope();
    }

    void SemanticAnalyzer::HandleFunctionDeclaration(FunctionDeclarationNode* function)
    {
        auto symbol = std::make_unique<Symbol>();
        symbol->kind = Symbol::Kind::Function;
        symbol->name = function->name;
        symbol->type = CloneType(function->returnType.get());
        function->symbol = symbol.get();
        currentScope()->symbols[function->name] = std::move(symbol);

        PushScope();
        for (auto& [name, type] : function->parameters)
        {
            AddParameter(name, type.get());
        }
        PushScope();
    }

    void SemanticAnalyzer::AddParameter(const std::string& name, TypeNode* type)
    {
        auto symbol = std::make_unique<Symbol>();
        symbol->kind = Symbol::Kind::Variable;
        symbol->name = name;
        symbol->type = CloneType(type);
        currentScope()->symbols[name] = std::move(symbol);
    }

    void SemanticAnalyzer::ResolveFunctionType(FunctionDeclarationNode* function)
    {
        if (auto* body = dynamic_cast<BlockNode*>(function->body.get()))
        {
            for (auto& statement : body->statements)
            {
                TypeResolutionPass(statement.get());
            }
        }
        PopScope();
        PopScope();
    }

    void SemanticAnalyzer::HandleClassDeclaration(ClassDeclarationNode* classDeclaration)
    {
        auto symbol = std::make_unique<Symbol>();
        symbol->kind = Symbol::Kind::Class;
        symbol->name = classDeclaration->name;
        classDeclaration->symbol = symbol.get();
        currentScope()->symbols[classDeclaration->name] = std::move(symbol);

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

    void SemanticAnalyzer::ResolveVariableType(VariableDeclarationNode* variable)
    {
        if (!variable->declaredType)
        {
            Error("Missing type annotation for variable: " + variable->name);
            return;
        }

        if (variable->initializer)
        {
            TypeNodePtr initType = GetExpressionType(variable->initializer.get());
            if (!CheckTypeCompatibility(*variable->declaredType, *initType))
            {
                std::stringstream ss;
                ss << "Type mismatch in variable '" << variable->name << "'. "
                    << "Declared: " << variable->declaredType->name
                    << ", Inferred: " << initType->name;
                Error(ss.str());
            }
        }
        variable->resolvedType = CloneType(variable->declaredType.get());
    }

    void SemanticAnalyzer::ResolveIdentifier(IdentifierNode* identifier)
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

    void SemanticAnalyzer::ValidateReturn(ReturnStatementNode* returnStatement)
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

    void SemanticAnalyzer::Error(const std::string& message)
    {
        errors.push_back(message);
    }

    void SemanticAnalyzer::PushScope()
    {
        auto scope = std::make_unique<Scope>();
        scope->parent = currentScope();
        scopeStack.push_back(std::move(scope));
    }

    void SemanticAnalyzer::PopScope()
    {
        if (!scopeStack.empty())
        {
            scopeStack.pop_back();
        }
    }

    Scope* SemanticAnalyzer::currentScope() const
    {
        return scopeStack.empty() ? nullptr : scopeStack.back().get();
    }

    TypeNodePtr SemanticAnalyzer::CloneType(TypeNode* type)
    {
        return std::make_unique<TypeNode>(*type);
    }

    bool SemanticAnalyzer::CheckTypeCompatibility(const TypeNode& t1, const TypeNode& t2)
    {
        if (t1.name == t2.name) return true;
        if (IsArrayType(t1) && IsArrayType(t2))
        {
            return CheckArrayCompatibility(t1, t2);
        }
        return false;
    }

    void SemanticAnalyzer::HandleVariableDeclaration(VariableDeclarationNode* variable)
    {
        if (currentScope()->symbols.contains(variable->name))
        {
            Error("Duplicate variable declaration: " + variable->name);
            return;
        }

        if (!variable->declaredType)
        {
            Error("Missing type annotation for variable: " + variable->name);
            return;
        }

        auto symbol = std::make_unique<Symbol>();
        symbol->kind = Symbol::Kind::Variable;
        symbol->name = variable->name;
        symbol->type = CloneType(variable->declaredType.get());
        symbol->declarationSite = variable;
        symbol->scope = currentScope();
        symbol->isInitialized = variable->initializer != nullptr;

        variable->symbol = symbol.get();
        currentScope()->symbols[variable->name] = std::move(symbol);
    }

    void SemanticAnalyzer::ResolveBinaryExpression(BinaryExpressionNode* expression)
    {
        TypeResolutionPass(expression->left.get());
        TypeResolutionPass(expression->right.get());

        auto leftType = GetExpressionType(expression->left.get());
        auto rightType = GetExpressionType(expression->right.get());

        if (expression->op == BinaryOperator::Assign)
        {
            if (!CheckTypeCompatibility(*leftType, *rightType))
            {
                Error("Assignment type mismatch");
            }
            expression->evaluatedType = CloneType(leftType.get());
            return;
        }

        if (!CheckTypeCompatibility(*leftType, *rightType))
        {
            Error("Operand type mismatch in binary expression");
        }

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
            default:
                expression->evaluatedType = CloneType(leftType.get());
        }
    }

    TypeNodePtr SemanticAnalyzer::GetExpressionType(ASTNode* expression)
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

    TypeNodePtr SemanticAnalyzer::GetLiteralType(LiteralNode* literal)
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

    void SemanticAnalyzer::ValidateLoopControl(ASTNode* node)
    {
        if (loopDepth == 0)
        {
            const char* msg = (node->nodeType == ASTType::BreakStatement)
                ? "Break statement outside loop"
                : "Continue statement outside loop";
            Error(msg);
        }
    }

    void SemanticAnalyzer::ValidateFunction(FunctionDeclarationNode* function)
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

    void SemanticAnalyzer::CheckForReturns(ASTNode* node, bool& hasReturn)
    {
        if (hasReturn) return;

        switch (node->nodeType)
        {
            case ASTType::ReturnStatement:
                hasReturn = true;
                break;
            case ASTType::IfStatement:
            {
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
                        if (hasReturn) break;
                    }
                }
        }
    }

    bool SemanticAnalyzer::IsArrayType(const TypeNode& type)
    {
        return type.name.size() > 2 &&
            type.name.front() == '[' &&
            type.name.back() == ']';
    }

    bool SemanticAnalyzer::CheckArrayCompatibility(const TypeNode& t1, const TypeNode& t2)
    {
        std::string elem1 = t1.name.substr(1, t1.name.size() - 2);
        std::string elem2 = t2.name.substr(1, t2.name.size() - 2);
        TypeNode tn1{ elem1, false, false };
        TypeNode tn2{ elem2, false, false };
        return CheckTypeCompatibility(tn1, tn2);
    }

    Symbol* SemanticAnalyzer::GetClassSymbol(const TypeNode& type)
    {
        if (auto* sym = currentScope()->Lookup(type.name))
        {
            return sym->kind == Symbol::Kind::Class ? sym : nullptr;
        }
        return nullptr;
    }

    void SemanticAnalyzer::HandleWhileLoop(WhileStatementNode* node)
    {
        TypeResolutionPass(node->condition.get());
        TypeNodePtr condType = GetExpressionType(node->condition.get());
        if (condType->name != "bool")
        {
            Error("While condition must be boolean");
        }
        loopDepth++;
        TypeResolutionPass(node->body.get());
        loopDepth--;
    }

    void SemanticAnalyzer::HandleForLoop(ForStatementNode* node)
    {
        if (node->init) TypeResolutionPass(node->init.get());
        if (node->condition)
        {
            TypeNodePtr condType = GetExpressionType(node->condition.get());
            if (condType->name != "bool")
            {
                Error("For loop condition must be boolean");
            }
        }
        if (node->update) TypeResolutionPass(node->update.get());
        loopDepth++;
        TypeResolutionPass(node->body.get());
        loopDepth--;
    }
} // namespace Arcanelab::Mano
