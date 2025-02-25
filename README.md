# Mano - Simple Embeddable Scripting Language

**Mano** is a statically typed scripting language designed for simplicity and embeddability. It combines modern syntax inspired by Swift with deterministic memory management, making it suitable for performance-sensitive environments.  

### Key Features  
- **Statically Typed**: Compile-time type checks ensure safety without sacrificing expressiveness.  
- **Automatic Memory Management**: Uses reference counting (no garbage collector) with zero runtime overhead.  
- **Embedded Focus**: Designed as a lightweight alternative to Lua for integration into C/C++ hosts.  
- **Minimal Syntax**: Familiar constructs with minimal boilerplate.  

---

## Language Elements  

### 1. Primitive Types  
```swift  
let pi: float = 3.14159;  
var score: uint = 100;  
var isActive: bool = true;  
var name: string = "Mano";  
```  
Supported types: **```int```**, **```uint```**, **```float```**, **```bool```**, **```string```**.  
- **Passed by Copy**: Primitives are copied when assigned or passed to functions.  
- **No Automatic Conversion**: Explicit casting or library functions required for type conversions.  

---

### 2. Variables and Constants  
- **Variables**: Mutable, explicitly typed.  
  ```swift  
  var counter: int = 0;  
  counter = 10;  
  ```  

- **Constants**: Immutable after initialization.  
  ```swift  
  let maxItems: int = 100;  
  maxItems = 64; // maxItems is immutable, yields a compilation error
  ```  

---

### 3. Functions  
```swift  
fun Add(a: int, b: int): int  
{  
    return a + b;  
}  

fun Greet(name: const string): string  
{  
    return "Hello, " + name + "!";  
}

fun Log(message: string)  // no return value
{
    System.Log(message);
}
```  
- **Parameters**: Can be marked as **```const```** to prevent modification.  
- **Return Types**: Optional if omitted (void).  

---

### 4. Classes  
```swift  
class Person  
{  
    var name: string = "";  
    var age: int = 0;  

    // Constructor: method matching class name  
    fun Person(newName: string, newAge: int)  
    {  
        name = newName;  
        age = newAge;  
    }  

    fun Describe(): string  
    {  
        return name + " is " + age + " years old.";  
    }  
}  
```  
- **Constructors**: Defined as methods with the same name as the class.  
- **No ```this``` Keyword**: Use distinct parameter names to avoid shadowing.  
- **Passed by Reference**: Objects are reference types.  

---

### 5. Enums  
```swift  
enum Direction  
{  
    North,  
    East,  
    South,  
    West,  
}  
```  
- **Members**: Fixed set of named constants.  
- **Distinct Types**: Not convertible to integers.  

---

### 6. Control Flow  
#### If-Else  
```swift  
if (score > 50)  
{  
    Print("Pass");  
}  
else  
{  
    Print("Fail");  
}  
```  

#### Loops  
```swift  
for (var i: uint = 0; i < 10; i = i + 1)  
{  
    // Loop body  
}  

var count: uint = 5;  
while (count > 0)  
{  
    count = count - 1;  
}  
```  

#### Switch  
```swift  
switch(dir)  
{  
    case Direction.North:  
    {  
        MoveNorth();  
    }  
    case Direction.East:  
    {  
        // Empty block allowed (no fallthrough)  
    }  
    default:  
    {  
        Stop();  
    }  
}  
```  

---

### 7. Arrays  
```swift  
var numbers: [int] = [1, 2, 3];  
var empty: [float] = [];  
```  
- **No Nested Arrays**: Only single-dimension arrays are supported.  
- **Immutability**: Arrays declared with **```let```** are fixed in size and elements.  

---

### 8. Operators  
```swift  
var a: int = 10 * 2 + 5;  // Arithmetic  
var b: bool = (a == 25) && (a < 30);  // Logical  
```  
Supported operators:  
- Arithmetic: ```+```, ```-```, ```*```, ```/```, ```%```, unary ```-```  
- Logical: ```&&```, ```||```, ```!```  
- Relational: ```==```, ```!=```, ```<```, ```>```, ```<=```, ```>=```  

---

### 9. Memory Management  
- **Automatic Reference Counting**: Objects are deallocated when references drop to zero.  
- **Object References**: Class instances are passed by reference; primitives are copied.  

---

### 10. Scoping  
Variables are block-scoped. A variable declared within **```{ }```** is accessible only within that block.  
```swift  
fun DemoScope()  
{  
    var x: int = 10;  
    {  
        var y: int = 20;  
        Print(x + y);  // Valid  
    }  
    // y is not accessible here  
}  
```  

---

### 11. Integration with C/C++  
Mano is designed for seamless embedding in C/C++ applications, supporting bidirectional function calls:  
- Invoke Mano functions from host code.  
- Mano functions can call registered host APIs.  

---

## Example Program  
```swift  
enum Status  
{  
    Active,  
    Inactive,  
    Suspended,  
}  

class User  
{  
    var id: uint = 0;  
    var name: string = "";  
    var status: Status = Status.Inactive;  

    fun User(newId: uint, newName: string)  
    {  
        id = newId;  
        name = newName;  
        status = Status.Active;  
    }  

    fun UpdateStatus(newStatus: Status): bool  
    {  
        if (status == newStatus)  
        {  
            return false;  
        }  
        status = newStatus;  
        return true;  
    }  

    fun GetInfo(): string  
    {  
        return "User #" + id + ": " + name + " (" + status + ")";  
    }  
}  

fun ProcessUsers(users: [User])  
{  
    for (var i: uint = 0; i < users.size(); i = i + 1)  
    {  
        var user: User = users[i];  
        if (user.status == Status.Active)  
        {  
            Print(user.GetInfo());  
        }  
        else  
        {  
            user.UpdateStatus(Status.Suspended);  
        }  
    }  
}  

fun Main()  
{  
    let users: [User] = [  
        User(1, "Alice"),  
        User(2, "Bob"),  
        User(3, "Charlie")  
    ];  

    users[1].UpdateStatus(Status.Inactive);  
    ProcessUsers(users);  

    var counter: uint = 0;  
    while (counter < 5)  
    {  
        switch(counter % 3)  
        {  
            case 0:  
            {  
                Print("Processing batch " + counter);  
            }  
            case 1:  
            {  
                // Placeholder for future logic  
            }  
            default:  
            {  
                Print("Default action");  
            }  
        }  
        counter = counter + 1;  
    }  

    let colors: [string] = ["red", "green", "blue"];  
    for (var color: string in colors)  
    {  
        Print("Color: " + color);  
    }  
}  
```  

--- 

This manual reflects Mano’s core features as of February 2025. Syntax and semantics are subject to refinement.
