# Infix to Postfix Calculator in C

## Minimal Language Core Inspired by Retro Interpreters 

---

This is a simple C-based calculator that converts infix expressions (e.g., `3 + 4 * a`) to postfix notation (`3 4 a * +`) and evaluates them. It allows:

+ One-digit numbers (`0`–`9`)
+ Single-letter variables (`a`–`z`) input at runtime
+ Basic arithmetic: `+`, `-`, `*`, `/`
+ Round brackets: `(` and `)` 

The program provides an experiment in language design fundamentals, inspired by early BASIC interpreters from the late 1970s or early 1980s.

---

## Features

+ Converts infix expressions to postfix notation (Reverse Polish Notation)

+ Evaluates the resulting postfix expression using a stack
+ Requests values for variables (`a`–`z`) during runtime

---

## Limitations

- Only single-digit numbers and one-letter variables
- Variables are stored as `char`, so numeric values are offset as ASCII characters
- Uses a char stack, so max value per digit is 9
- Lacks support for negative numbers, multi-digit numbers, floating-point math, exponentiation (`^`)
- No error handling beyond division by zero, no real syntax validation

Variables are stored using a static array with flags, which is as simple as it is memory-heavy. The code has some issues still -- it is more abandoned than completed. :)

---

I know this is standard code, but it might be combined with the stub C16 graphics command implementation to simulate a core part of a BASIC implementation. As such, it was written as part of the DFG-funded project _Cultures of Home Computer Music_ at the University of Music and Theatre Leipzig, Germany.

---

christoph.hust@hmt-leipzig.de