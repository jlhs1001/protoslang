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
./protoslang
```

### Input a file

```bash
./protoslang <path-to-file>
```

## Expressions

- [ ] Arithmetic
  - [x] `+`
  - [x] `-`
  - [x] `*`
  - [x] `/`
  - [ ] `%`
- [ ] Comparison
  - [x] `<`
  - [x] `>`
  - [x] `<=`
  - [x] `>=`
  - [x] `==`
  - [ ] `!=`
- [ ] Logical
  - [ ] `&&`
  - [ ] `||`
  - [x] `!`
- [ ] Assignment
  - [x] `=`
  - [ ] `+=`
  - [ ] `-=`
  - [ ] `*=`
  - [ ] `/=`
  - [ ] `%=`

## Literal Values

- [x] Integers
- [x] Floats
- [x] Strings
- [x] Booleans

## Variables

- [x] Declaration
- [x] Assignment

## Functions

- [ ] Declaration
- [ ] Arguments
- [ ] Return
- [ ] Call

## Control Flow

- [x] If-else
- [ ] For
