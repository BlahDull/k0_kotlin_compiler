# k0
>Final Project for CSE 4023
# Description
A compiler for the K0 language, supporting lexical analysis, parsing, syntax tree generation, symbol table construction, intermediate code generation, assembly output, object file creation, and executable linking.

The compiler is built using Flex, Bison, and GCC, and follows a traditional multi-stage compilation pipeline.

# Project Overview

The compiler performs the following stages:

Lexical Analysis – Tokenizes K0 source code

Syntax Analysis – Builds an Abstract Syntax Tree (AST)

Semantic Analysis – Constructs symbol tables and checks for errors

Intermediate Code Generation – Generates .ic files

Assembly Generation – Converts intermediate code to assembly (.s)

Object Code Generation – Produces .o files

Assembler - Converts assembly into final executable

# Flags

| Option    | Description                                      |
| --------- | ------------------------------------------------ |
| *(none)*  | Compile source file all the way to an executable |
| `-s`      | Generate assembly file (`.s`)                    |
| `-c`      | Generate object file (`.o`)                      |
| `-ic`     | Generate intermediate code file (`.ic`)          |
| `-symtab` | Print symbol tables                              |
| `-tree`   | Print the syntax tree to stdout                  |
| `-dot`    | Generate a DOT file and PNG of the syntax tree   |
| `-lexer`  | Interactive lexer mode (token testing)           |
| `-h`      | Display help message                             |

# Example
> ./k0 input.kt
