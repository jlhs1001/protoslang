# Protoslang

#### The beauty of Protoslang is found in is maintenance of simplicity as the guiding principle, without neglecting the importance of performance, expressiveness, and safety.

Protoslang is a dynamically typed language with a syntax reminiscent of Rust and Go.
It is designed to be a simple language that anyone can pick up with relative ease,
provided they have some experience with other syntactically Rust-like languages.
It is a general purpose language that is designed to be fast and safe.
Notably, Protoslang has a garbage collector, which makes it easier to use than languages like C.

## Basic Language Constructs

### Basic Operators

Protoslang supports the following basic operators:

- Arithmetic
  - +, -, *, /, %
- Comparison
  - <, >, <=, >=, ==, !=
- Logical
  - &&, ||, !
- Assignment
  - =, +=, -=, *=, /=, %=

### Literal Values

Protoslang supports the following literal values:

- Integers
- Floats
- Strings
- Booleans

```Protoslang    
let a = 5

a = 5.0
a = "Hello, World!"
a = true
```

### Variables

Variables are declared using the `let` keyword. Variables are dynamically typed and can be assigned any value.

```Protoslang
let x = 5
```

### Functions

Functions are declared using the `fn` keyword. Functions can take any number of arguments and return any value.

```Protoslang
fn add(a, b) {
    return a + b
}
```

### Control Flow

Protoslang supports the following control flow constructs:

- If-else
- While
- For

```Protoslang
if a > 5 {
    println("a is greater than 5")
} else {
    println("a is less than or equal to 5")
}

while a > 0 {
    a = a - 1
}

for i in 0..10 {
    print(i)
}
```

### Comments

Protoslang supports single-line and multi-line comments.

```Protoslang
// This is a single-line comment

/*
This is a multi-line comment
*/
```

[//]: # (## V2 Constructs)

[//]: # ()
[//]: # (### Structs)

[//]: # ()
[//]: # (Structs are declared using the `struct` keyword. Structs can contain any number of fields.)

[//]: # ()
[//]: # (```Protoslang)

[//]: # (struct Point {)

[//]: # (    x: int;)

[//]: # (    y: int;)

[//]: # (})

[//]: # (```)

[//]: # ()
[//]: # (### Advanced Type System)
