# Mano Language Specification

Mano is designed to be simple and C-like, supporting class-based object orientation with a minimal set of primitive types. It is intended for embedding in game engines, and its code is compiled into bytecode that runs on a custom virtual machine.

---

## 1. Primitive Types

The core primitive types are:

- **int**  
- **uint**  
- **float**  
- **bool**

### Variable Declaration

Variables are declared with the ```var``` keyword and a type annotation. Literal values are assigned using the ```=``` operator. Note that numeric literals are non-negative by default; to represent a negative number, apply the unary minus operator.

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

---

## 2. Operators

The following operators are supported:

### Arithmetic Operators

- **Addition:** ```+```
- **Subtraction:** ```-```
- **Multiplication:** ```*```
- **Division:** ```/```
- **Modulo:** ```%``` (applicable only to integer types)

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

- **Unary Negative (```-```):** Negates its operand.  
- **Logical Not (```!```):** Applies Boolean negation.

The unary ```-``` has a higher precedence than multiplication. For instance:

```
var value: int = 5;
var negated: int = -value * 2;  // Evaluates as (-value) * 2, not -(value * 2)
```

### Boolean Operators

- **Logical AND:** ```&&```
- **Logical OR:** ```||```

For ```&&``` and ```||```, evaluation is short-circuited; the right-hand side is only evaluated if required by the left-hand operand. They are left-associative.

### Assignment and Comparison

- **Assignment:** ```=``` (right-associative; e.g., ```a = b = c``` means ```a = (b = c)```)
- **Equality Comparison:** ```==```
- **Inequality Comparison:** ```!=```

Chained comparisons like ```a == b == c``` are not supported; use explicit grouping with parentheses if needed.

---

## 3. Functions

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

**Parameter Passing Semantics:**

- **Primitives:** Copied when passed.
- **Objects:** Passed as references. Their lifetimes are managed via reference counting. When passing an object to a function, the reference count is increased and later decreased when the function’s execution concludes.

---

## 4. Classes

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

## 5. Control Flow

### 5.1 Conditionals

Conditional constructs utilize ```if``` and ```else``` blocks. In this example, we check voting and driving license eligibility while avoiding the use of "else if" syntax:

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

### 5.2 For Loop

The ```for``` loop consists of three expressions (initialization, condition, and iteration) separated by semicolons within parentheses.

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

### 5.3 While Loop

The ```while``` loop repeatedly executes a block as long as its condition remains true.

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

## 6. Scoping

Variables are block-scoped. A variable declared within a block (delimited by ```{ }```) is accessible only within that block and any nested blocks.

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

## 7. Comments

Mano supports single-line comments. Everything following ```//``` on a given line is ignored by the compiler. Block comments (e.g., ```/* ... */```) are not supported.

**Example:**

```
// This is a single-line comment
var example: int = 5;  // Inline comment after code
```

---

## 8. Enums

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
