# Mano Language Specification

Mano is a simple, statically typed, embeddable scripting language designed with ease of use in mind. With a syntax similar to Swift, it is both familiar and accessible for new users. Developers can extend the language with custom classes, functions, and constants. Mano supports bidirectional integration: you can invoke Nano functions from your host application, and Nano functions can also call any registered host functions.

(See [test.mano](bin/test.mano) for a sneak-peak of the syntax.)

---

## 1. Primitive Types

The core primitive types are:

- **int**  
- **uint**  
- **float**  
- **bool**

### Variable Declaration

Variables are declared with the `var` keyword and a type annotation. Literal values are assigned using the `=` operator. Note that numeric literals are non-negative by default; to represent a negative number, apply the unary minus operator.

**Examples:**

```
// Declaration without initialization
var score: uint;

// Declaration with initialization
var score: uint = 100;
var temperature: float = 36.6;
var balance: int = -42;
var isActive: bool = true;
```

**Type Conversion:**  
Automatic conversion between types (e.g., from int to float) is not supported. If explicit transformation is needed, support must be implemented via library functions or explicit casting (if added later).

### Constants

Constants are declared with the `let` keyword and require an initializer. Once a constant is defined, its value cannot be changed.

**Example:**

```
let pi: float = 3.14159;
```

---

## 2. Arrays

Arrays allow you to store multiple values of the same type. They have the following characteristics:

- **Homogeneity:** All elements in an array must be of the same type.  
- **Declaration and Initialization:** Arrays are declared with a square-bracket type annotation. For example, 
```
var numbers: [int] = [1, 3, 5, 7, 9];
```
- **Dynamic Sizing:** When declared with `var`, arrays are dynamic in size.  
- **Immutability with Constants:** When declared with `let`, arrays are fixed in size and cannot be modified after initialization.  
- **Indexing:** Elements are accessed using square brackets with 0-based indexing. For example:  
```
var third: int = numbers[2];
```  
- **Mutability:** Arrays declared with `var` are mutable; their elements can be updated. Arrays declared with `let` are immutable.

---

## 3. Operators

The following operators are supported:

### Arithmetic Operators

- **Addition:** `+`
- **Subtraction:** `-`
- **Multiplication:** `*`
- **Division:** `/`
- **Modulo:** `%` (applicable only to integer types)

**Precedence and Associativity:**

- Multiplication, division, and modulo have higher precedence than addition and subtraction.
- All arithmetic operators are left-associative.

**Example:**

```
var a: int = 10;
var b: int = 3;
var result: int = a - b - 2;  // Interpreted as ((10 - 3) - 2)
```

### Unary Operators

- **Unary Negative (`-`):** Negates its operand.  
- **Logical Not (`!`):** Applies Boolean negation.

The unary `-` has a higher precedence than multiplication. For instance:

```
var value: int = 5;
var negated: int = -value * 2;  // Evaluates as (-value) * 2, not -(value * 2)
```

### Boolean Operators

- **Logical AND:** `&&`
- **Logical OR:** `||`

For `&&` and `||`, evaluation is short-circuited; the right-hand side is only evaluated if required by the left-hand operand. They are left-associative.

### Assignment and Comparison

- **Assignment:** `=` (right-associative; e.g., `a = b = c` means `a = (b = c)`)
- **Equality Comparison:** `==`
- **Inequality Comparison:** `!=`

Chained comparisons like `a == b == c` are not supported; use explicit grouping with parentheses if needed.

---

## 4. Functions

Functions in Mano have a C-like syntax with named parameters and an optional return type. Parameters for primitive types are passed by value, while class objects (instances) are passed by reference.

### Function Declaration Example

```
fun GetGreeting(name: string, count: int) : string
{
    // Constructs a greeting message using the provided name and count.
    var message: string = "Hello, " + name + "! You are visitor number " + count + ".";
    return message;
}
```

Returning a value is optional. If there's no return value, the return type can be omitted.

```
fun SimpleFunction()
{
    // No value returned
}
```

**Parameter Passing Semantics:**

- **Primitives:** Copied when passed.
- **Objects:** Passed as references. Their lifetimes are managed via reference counting. When passing an object to a function, the reference count is increased and later decreased when the function’s execution concludes.

---

## 5. Classes

Classes encapsulate properties (state) and methods (behavior). Objects are always instantiated on the heap and handled as reference types.

### Class Declaration Example

```
class Person
{
    var name: string;
    var age: int;

    // Constructor: Initializes a new Person instance.
    fun Person(newName: string, newAge: int)
    {
        name = newName;
        age = newAge;
    }

    // Method: Returns a short description of the person.
    fun Describe() : string
    {
        return name + " is " + age + " years old.";
    }
}
```

### Object Instantiation

Class instances are created using a constructor-like syntax. You can call the class name as if it were a function, with or without parameters, to create a new instance.

**Examples:**

```
var p1: Person = Person("Alice", 30);  // With constructor parameters.
var p2: Person = Person();             // Without parameters.
```

Since objects are passed by reference, when an instance is passed to a function or assigned to another variable, they refer to the same underlying object managed by reference counting.

---

## 6. Control Flow

### 6.1 Conditionals

Conditional constructs utilize `if` and `else` blocks. In this example, we check voting and driving license eligibility while avoiding the use of "else if" syntax:

```
fun CheckEligibility(age: uint) : string
{
    if (age >= 18)
    {
        // A person aged 18 or older is eligible for both voting and driving licenses.
        return "Eligible for both voting and driving licenses.";
    }
    else
    {
        // For ages below 18, nest another if statement to determine eligibility.
        if (age >= 16)
        {
            return "Eligible for a driving license only.";
        }
        else
        {
            return "Not eligible for voting or driving licenses.";
        }
    }
}
```

### 6.2 For Loop

The `for` loop consists of three expressions (initialization, condition, and iteration) separated by semicolons within parentheses.

**Example:**

```
for (var i: uint = 0; i < 10; i = i + 1)
{
    // Skip the iteration when i equals 3.
    if (i == 3)
    {
        continue;
    }
    
    // Exit the loop when i equals 7.
    if (i == 7)
    {
        break;
    }
    
    Print("Iteration number: " + i);
}
```

### 6.3 While Loop

The `while` loop repeatedly executes a block as long as its condition remains true.

**Example:**

```
var count: uint = 5;
while (count > 0)
{
    Print("Count: " + count);
    count = count - 1;
}
```

---

## 7. Scoping

Variables are block-scoped. A variable declared within a block (delimited by `{ }`) is accessible only within that block and any nested blocks.

**Example:**

```
fun DemoScope()
{
    var x: int = 10;
    
    {
        var y: int = 20;
        Print(x + y);  // Accessible: prints 30.
    }
    
    // y is not accessible here.
}
```

---

## 8. Comments

Mano supports single-line comments. Everything following `//` on a given line is ignored by the compiler. Block comments (e.g., `/* ... */`) are not supported.

**Example:**

```
// This is a single-line comment
var example: int = 5;  // Inline comment after code
```

---

## 9. Enums

Enums allow you to define a new type representing a fixed set of named values. These are distinct types not automatically convertible to integers.

### Enum Declaration Example

```
enum Direction
{
    North,
    East,
    South,
    West
}
```

### Enum Usage Example

```
var currentDirection: Direction = Direction.North;

if (currentDirection == Direction.North)
{
    Print("Heading North.");
}
```

---

## Contributors

Designed and written by Zoltán Majoros ([github.com/arcanelab](https://github.com/arcanelab/)) in 2025.
