# Mini C Grammar

This project implements a practical subset of C using recursive-descent parsing.

```text
program           -> external_decl*
external_decl     -> type identifier function_or_global
function_or_global-> "(" parameters? ")" block
                  | declarator_tail ("," identifier declarator_tail)* ";"

parameters        -> parameter ("," parameter)*
parameter         -> type identifier array_suffix?

type              -> "int" | "float" | "char" | "bool" | "void"
array_suffix      -> "[" int_literal "]"

block             -> "{" statement* "}"
statement         -> block
                  | declaration
                  | if_statement
                  | while_statement
                  | for_statement
                  | return_statement
                  | expression ";"
                  | ";"

declaration       -> type identifier array_suffix? ("=" expression)?
                     ("," identifier array_suffix? ("=" expression)?)* ";"

if_statement      -> "if" "(" expression ")" statement ("else" statement)?
while_statement   -> "while" "(" expression ")" statement
for_statement     -> "for" "(" for_init? ";" expression? ";" expression? ")" statement
return_statement  -> "return" expression? ";"

expression        -> assignment
assignment        -> logical_or ("=" assignment)?
logical_or        -> logical_and ("||" logical_and)*
logical_and       -> equality ("&&" equality)*
equality          -> relational (("==" | "!=") relational)*
relational        -> additive (("<" | "<=" | ">" | ">=") additive)*
additive          -> multiplicative (("+" | "-") multiplicative)*
multiplicative    -> unary (("*" | "/" | "%") unary)*
unary             -> ("!" | "-" | "++" | "--") unary | postfix
postfix           -> primary ("[" expression "]" | "++" | "--")*
primary           -> literal | identifier | function_call | "(" expression ")"
function_call     -> identifier "(" arguments? ")"
arguments         -> expression ("," expression)*
```

The parser performs semantic checks while parsing and emits Three Address Code
for declarations, assignments, function calls, conditions, loops, and returns.
After TAC generation, a dead-code elimination pass removes unreachable
instructions after unconditional `return` and `goto` statements until the next
label or function boundary.
