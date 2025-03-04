# Semantic Analyzer

## Overview
The semantic analyzer performs static checks through 3 passes:
1. **Declaration Pass** - Builds symbol tables and checks declaration validity
2. **Type Resolution Pass** - Verifies type consistency across expressions
3. **Validation Pass** - Enforces language semantics and control flow rules

## 1. Declaration Pass Checks

### A. Variable Declarations
- **Duplicate declaration** detection in same scope
- **Mandatory type annotations** for all variables
- **Symbol registration** in current scope
- **Initialization tracking** (marks if variable has initializer)

### B. Function Declarations
- **Parameter registration** in dedicated parameter scope
- **Return type validation** (must be specified or default to void)
- **Body scope creation** nested within parameter scope
- **Symbol registration** with function signature

### C. Class Declarations
- **Member registration** in class-specific scope
- **Symbol creation** for class type
- **Nested declarations** processing (methods, properties)

### D. Program Structure
- **Global scope management** for top-level declarations
- **Hierarchical scoping** (global → function → block)
- **Cross-reference linking** between symbols and AST nodes

## 2. Type Resolution Pass Checks

### A. Variables & Expressions
- **Type annotation validation** (must resolve to known type)
- **Initializer compatibility**:
  ```cpp
  let x: int = 3.14; // Error: double → int mismatch
  ```
- **Literal type inference**:
  - ```42``` → int
  - ```"text"``` → string
  - ```true```/```false``` → bool
  - ```3.14``` → float

### B. Binary Operations
- **Operand compatibility** checks:
  ```cpp
  "text" + 5; // Error: string + int
  ```
- **Result type determination**:
  ```cpp
  int + int → int
  bool || bool → bool
  ```
- **Assignment validation**:
  ```cpp
  let x: float;
  x = 42; // Valid (int → float)
  ```

### C. Control Structures
- **Loop conditions** must be boolean:
  ```cpp
  while(1) {...} // Error: int → bool
  ```
- **Type resolution** for:
  - For-loop initializers
  - Loop update expressions
  - Conditional expressions

### D. Identifier Resolution
- **Existence checks** for all identifiers
- **Scope chain lookup** (current → parent scopes)
- **Type propagation** from symbols to usage sites

## 3. Validation Pass Checks

### A. Function Validation
- **Return type enforcement**:
  ```cpp
  fn foo(): int { return; } // Error: missing value
  ```
- **Path analysis** for return statements:
  ```cpp
  fn bar(): bool
  {
      if (cond) return true;
      // Error: missing return for false path
  }
  ```
- **Void return checks**:
  ```cpp
  fn baz()
  {
      return 42; // Error: void function returns value
  }
  ```

### B. Control Flow
- **Break/continue** only in loops:
  ```cpp
  fn error()
  {
      break; // Error: outside loop
  }
  ```
- **Loop depth tracking** for nested loops

### C. Type Compatibility
- **Array compatibility** (element-wise check):
  ```cpp
  let a: [int] = [1,2,3];
  let b: [float] = a; // Valid if int → float allowed
  ```
- **Primitive type checks** (planned for future):
  ```cpp
  class Animal {}
  class Dog: Animal {}
  let a: Animal = Dog(); // Valid (inheritance)
  ```

## Helper Systems

### Scope Management
- **Stack-based scoping** with parent links
- **Automatic scope creation** for:
  - Functions (parameters + body)
  - Classes
  - Control structures
- **Symbol lookup** with lexical scoping rules

### Error Handling
- **Error accumulation** with contextual messages
- **Fail-safe continuation** (multiple errors per pass)
- **Error types**:
  - Undeclared identifiers
  - Type mismatches
  - Invalid control flow
  - Declaration conflicts

### Type System
- **Type cloning** for independent type instances
- **Compatibility matrix**:
  ```cpp
  Same type → same name
  Arrays → same element type 
  Classes → inheritance hierarchy (future)
  ```
- **Const correctness** (tracked but not enforced yet)

## Key Features
1. **Three-phase analysis** for clear separation of concerns
2. **Comprehensive type tracking** throughout AST
3. **Context-sensitive error reporting**
4. **Symbol resolution** with scope hierarchy
5. **Flow-sensitive checks** for return statements
6. **Extensible architecture** for future type system improvements

## Future Considerations
- Class inheritance validation
- Generic type support
- Type coercion rules
- Const correctness enforcement
- Cross-module symbol resolution

This analyzer forms the foundation for static type safety and semantic correctness in the language implementation.

# Semantic Analyzer Technical Specification

## 1. Three-Phase Architecture

### Phase 1: Declaration Pass
**Purpose**: Build symbol tables and validate declarations  
**Key Methods**:
```cpp
void DeclarationPass(ASTNode* node);
void HandleProgramDeclaration(ProgramNode* program);
void HandleFunctionDeclaration(FunctionDeclarationNode* func);
void HandleClassDeclaration(ClassDeclarationNode* cls);
void HandleVariableDeclaration(VariableDeclarationNode* var);
```

**Key Rules**:
1. Creates 3-tier scope hierarchy:
   - Global → Function Parameters → Function Body
   - Class → Class Members
2. Prohibits duplicate symbols in same scope
3. Links symbols to declaration sites:
   ```cpp
   sym->declarationSite = var; // VariableDeclarationNode
   ```

### Phase 2: Type Resolution
**Purpose**: Establish and validate type relationships  
**Key Methods**:
```cpp
void TypeResolutionPass(ASTNode* node);
void ResolveVariableType(VariableDeclarationNode* var);
void ResolveIdentifier(IdentifierNode* id);
void ResolveBinaryExpression(BinaryExpressionNode* expr);
```

**Key Checks**:
1. Variable initializer compatibility:
   ```cpp
   if (!CheckTypeCompatibility(*var->declaredType, *initType))
   {
       Error("Type mismatch...");
   }
   ```
2. Identifier resolution through scope chain
3. Binary operation validation:
   - Assignment LHS/RHS compatibility
   - Operand type matching

### Phase 3: Validation Pass
**Purpose**: Enforce semantic constraints  
**Key Methods**:
```cpp
void ValidationPass(ASTNode* node);
void ValidateReturn(ReturnStatementNode* ret);
void ValidateLoopControl(ASTNode* node);
void ValidateFunction(FunctionDeclarationNode* func);
```

**Key Rules**:
1. Return type enforcement:
   ```cpp
   if (!CheckTypeCompatibility(*currentFunction->returnType, *returnType))
   {
       Error("Return type mismatch...");
   }
   ```
2. Break/continue only within loops
3. Functions with non-void returns must have at least one return

---

## 2. Scope Management Implementation

### Scope Stack Mechanics
```cpp
std::vector<std::unique_ptr<Scope>> scopeStack;

void PushScope(const std::string& name)
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
```

### Symbol Lookup Logic
```cpp
Symbol* Lookup(const std::string& name) const
{
    // Current scope first
    if (auto it = symbols.find(name); it != symbols.end())
    {
        return it->second.get();
    }
    // Then parent scopes recursively
    return parent ? parent->Lookup(name) : nullptr;
}
```

---

## 3. Type System Implementation

### Type Checking Rules
```cpp
bool CheckTypeCompatibility(const TypeNode& t1, const TypeNode& t2)
{
    // 1. Direct name match
    if (t1.name == t2.name) return true;
    
    // 2. Array element compatibility
    if (IsArrayType(t1) && IsArrayType(t2))
    {
        return CheckArrayCompatibility(t1, t2);
    }
    
    // 3. No inheritance support currently
    return false;
}
```

### Array Handling
```cpp
bool IsArrayType(const TypeNode& type)
{
    return type.name.size() > 2 &&
           type.name.front() == '[' &&
           type.name.back() == ']';
}

bool CheckArrayCompatibility(const TypeNode& t1, const TypeNode& t2)
{
    // Extract element types from "[ElementType]" format
    std::string elem1 = t1.name.substr(1, t1.name.size() - 2);
    std::string elem2 = t2.name.substr(1, t2.name.size() - 2);
    
    // Compare base element types
    return elem1 == elem2; // No nested arrays currently
}
```

---

## 4. Error Handling Implementation

### Error Collection
```cpp
std::vector<std::string> errors;

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
```

### Recovery Strategy
- Continue analysis after errors
- Maintain partial symbol tables
- Skip invalid subtrees using ```dynamic_cast``` checks:
  ```cpp
  if (auto* body = dynamic_cast<BlockNode*>(func->body.get()))
  {
      // Process valid blocks
  }
  ```

---

# Semantic Analyzer Description

## Overview

Starting with the DeclarationPass:
- The code processes Program, FunctionDeclaration, ClassDeclaration, and VariableDeclaration nodes.
- For functions, it creates scopes for parameters and the body. It adds symbols to the current scope, checking for duplicates.
- For classes, it creates a new scope for class members and processes declarations within the class block.
- Variables are checked for duplicate declarations in the same scope and whether they have type annotations.

Next, the TypeResolutionPass:
- Variables have their initializer types resolved and checked against their declared types.
- Identifiers are resolved by looking up symbols in the scope hierarchy.
- Binary expressions, especially assignments, are checked for type compatibility between left and right operands.
- Function return types and parameter types are resolved.
- Loop conditions are verified to be boolean.

The ValidationPass:
- Ensures return statements match the function's return type.
- Checks that break and continue statements are within loops.
- Validates that non-void functions have at least one return statement in all code paths.

Looking at helper methods:
- Error handling collects messages without stopping the analysis.
- Scopes are managed with a stack, allowing nested scopes for functions and classes.
- Type compatibility checks include direct name matching and array element checks, but not inheritance.

### 1. **Declaration Phase Verifications**
**1.1 Variable Declarations**  
```cpp
void HandleVariableDeclaration(VariableDeclarationNode* var)
```
- Ensures no duplicate variables in same scope:
  ```cpp
  if (currentScope()->symbols.contains(var->name))
  {
      Error("Duplicate variable...");
  }
  ```
- Requires explicit type annotations:
  ```cpp
  if (!var->declaredType)
  {
      Error("Missing type annotation...");
  }
  ```
- Tracks initialization status:
  ```cpp
  sym->isInitialized = var->initializer != nullptr;
  ```

**1.2 Function Declarations**  
```cpp
void HandleFunctionDeclaration(FunctionDeclarationNode* func)
```
- Creates 3-layer scope hierarchy:
  1. Function symbol in parent scope
  2. Parameter scope
  3. Body scope
- Parameters become symbols in parameter scope:
  ```cpp
  for (auto& [name, type] : func->parameters)
  {
      AddParameter(name, type.get());
  }
  ```

**1.3 Class Declarations**  
```cpp
void HandleClassDeclaration(ClassDeclarationNode* cls)
```
- Creates class-specific scope for members
- Processes nested declarations through ```ClassBlockNode```

### 2. **Type Resolution Verifications**
**2.1 Variable Type Checking**  
```cpp
void ResolveVariableType(VariableDeclarationNode* var)
```
- Validates initializer compatibility:
  ```cpp
  if (!CheckTypeCompatibility(*var->declaredType, *initType))
  {
      Error("Type mismatch...");
  }
  ```

**2.2 Expression Typing**  
```cpp
TypeNodePtr GetExpressionType(ASTNode* expr)
```
- Literal type inference:
  ```cpp
  if (lit->value.find('.') != ...) // → float
  if (value is "true"/"false")    // → bool
  if (quoted string)              // → string
  else                            // → int
  ```
- Array literal validation:
  ```cpp
  case ASTType::ArrayLiteral:
      return CloneType(...->evaluatedType);
  ```

**2.3 Binary Operations**  
```cpp
void ResolveBinaryExpression(BinaryExpressionNode* expr)
```
- Assignment validation:
  ```cpp
  if (expr->op == BinaryOperator::Assign) {
      if (!CheckTypeCompatibility(...))
  }
  ```
- Logical operator enforcement:
  ```cpp
  case LogicalAnd/LogicalOr:
      expr->evaluatedType = "bool"
  ```

### 3. **Validation Phase Checks**
**3.1 Control Flow**  
```cpp
void ValidateLoopControl(ASTNode* node)
```
- Ensures break/continue only in loops:
  ```cpp
  if (loopDepth == 0)
  {
      Error("Break/continue outside loop");
  }
  ```

**3.2 Function Validity**  
```cpp
void ValidateFunction(FunctionDeclarationNode* func)
```
- Non-void functions require returns:
  ```cpp
  CheckForReturns(func->body.get(), hasReturn);
  if (!hasReturn) { Error(...); }
  ```
- Return type compatibility:
  ```cpp
  if (!CheckTypeCompatibility(...))
  {
      Error("Return type mismatch");
  }
  ```

**3.3 Type System**  
```cpp
bool CheckTypeCompatibility(const TypeNode& t1, const TypeNode& t2)
```
- Name equivalence:
  ```cpp
  if (t1.name == t2.name) return true;
  ```
- Array compatibility:
  ```cpp
  if (IsArrayType(t1) && IsArrayType(t2))
  {
      return CheckArrayCompatibility(t1, t2);
  }
  ```

### 4. **Scope Management**
**4.1 Hierarchical Lookup**  
```cpp
Symbol* Scope::Lookup(const std::string& name) const
```
- Current scope → parent chain lookup
- Class scopes nest members properly

**4.2 Scope Stack**  
```cpp
std::vector<std::unique_ptr<Scope>> scopeStack;
```
- Global → Function → Parameters → Body
- Class → Member declarations

### 5. **Error Handling**
- Collects multiple errors:
  ```cpp
  std::vector<std::string> errors;
  ```
- Error recovery continues analysis:
  ```cpp
  catch (const std::exception& err)
  {
      errors.push_back(err.what());
      return false;  // After full analysis attempt
  }
  ```

### 6. **Future Ideas**
1. **Inheritance Support**  

2. **Additional Checks**  
   - Const correctness (```isConst``` exists but unused)
   - Function overloading
   - Variable shadowing prevention
   - Dead code detection
   - Generic types

3. **Array Limitations**  
   - No multidimensional arrays
   - No size tracking in type system
