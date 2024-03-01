# Protoslang

Protoslang is still in active development. The README will temporarily hold the feature set completion status.

## Quick Start

### Clone the repository

```bash
git clone https://github.com/jlhs1001/protoslang.git
```

### Build the project

```bash
cd protoslang
cmake .
cmake --build .
```

### Run the REPL

```bash
./slang_prototype
```

### Input a file

```bash
./slang_prototype <path-to-file>
```

## Expressions

- [x] Arithmetic
  - [x] `+`
  - [x] `-`
  - [x] `*`
  - [x] `/`
  - [x] `%`
- [x] Comparison
  - [x] `<`
  - [x] `>`
  - [x] `<=`
  - [x] `>=`
  - [x] `==`
  - [x] `!=`
- [x] Logical
  - [x] `&&`
  - [x] `||`
  - [x] `!`
- [x] Assignment
  - [x] `=`

[//]: # (  - [ ] `+=`)

[//]: # (  - [ ] `-=`)

[//]: # (  - [ ] `*=`)

[//]: # (  - [ ] `/=`)

[//]: # (  - [ ] `%=`)

## Literal Values

- [x] Integers
- [x] Floats
- [x] Strings
- [x] Booleans

## Variables

- [x] Declaration
- [x] Assignment

## Functions

- [x] Declaration
- [x] Arguments
- [ ] Return
- [x] Call

## Control Flow

- [x] If-else
- [x] For-in (iterable)
- [x] For-range
