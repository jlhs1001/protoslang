# Protoslanguage

Protoslanguage is a dynamically typed, interpreted programming language. It is designed to be simple and powerful. This
iteration is more so a syntactic proof of concept. Future versions will likely be more feature rich,
statically typed, and compiled.

## Quick Start

### Clone the repository

```bash
cd protoslang
cmake .
cmake --build .
```

### Input a file

```bash
./slang_prototype <path-to-file>
```

## Hello World

As it is the traditional introduction to any programming language, let's start with a simple "Hello, World!" program.

```rust
// The 'println' function is used to print to the console.
println("hello world");
```

## Variables and Data Types

Protoslanguage is dynamically typed. Variables are declared with the `let` keyword. The type of the variable is inferred
from the value assigned to it. Variables can be reassigned to values of different types.

```rust
// Variables are declared with the 'let' keyword.
let x = 5;
let y = x;

// Variables can be reassigned.
x = 10;

println(x + y);
```

### Operators and Expressions in Action

```rust
// This is a bit of an interesting expression.
// It combines all the supported operators.
let x = 30;
let y = x + 2 * 3 / (4 - 2);

println(y % 3);
```

## Functions

Functions are declared with the `fn` keyword. They can take any number of arguments and return a single value.

```rust
// Function argument can be of any type.
fn add(x, y) {
    return x + y;
}
```

Calling the function is as simple as expected.

```rust
let result = add(5, 10);
println(result);
```

## Aggregates

Protoslanguage supports lists of values.

### List indexing
```rust
let list = [1, 2, 3, 4, 5];

// Lists can be indexed.
println(list[0]);

// A list index may be assigned a new value.
list[0] = 10;

println(list[0]);
```

[//]: # (### List concatenation)
[//]: # (```rust)
[//]: # ()
[//]: # (let list1 = [1, 2, 3];)
[//]: # (let list2 = [4, 5, 6];)
[//]: # ()
[//]: # (// Lists can be concatenated.)
[//]: # (println&#40;list1 + list2&#41;;)
[//]: # (```)

Lists may contain any type of value.

```rust
let list = [1, "two", 3.0, true];

// Lists can conveniently be printed.
println(list);
```

## Control Flow

Protoslanguage supports if-else statements, range-based for loops, iterative loops, and while loops.

### If-else statement
```rust
let x = 5;

// A neat little syntax. Quite rust-like.
if x > 10 {
    println("x is greater than 10");
} else {
    println("x is less than 10");
}
```

### Range-based for loop

The range-based for loop is a simple way to iterate over a range of values.
A range in slang is a built-in type that represents a range of values.
It simply contains a lower and upper bound.
```rust

// A range-based for loop.
for i in 0..10 {
    println(i);
}

```

### Iterative loop
The iterative loop is a simple way to iterate over a list of values. It is similar to the range-based for loop, but it
grants the loop body access to the current value in the list. Quite convenient.
```rust
// An iterative loop.
let grocery_list = [
    "apples",
    "bananas",
    "oranges",
    "grapes",
    "pears",
];

for let item in list {
    println(item);
}
```

### While loop
The while loop is a simple way to iterate over a block of code until a condition is met.
```rust
let x = 0;

while x < 10 {
    println(x);
    x = x + 1;
}
```

That's slanguage for now. More language constructs and features will surely be added soon.
