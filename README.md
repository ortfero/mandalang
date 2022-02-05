# mandalang

Simple calculation language

## Snippets

```
let fib = fn(integer n) -> integer if n < 2 then n else self(n - 2) + self(n - 1)
fib(40)
```

## Grammar
```
program -> { definition | expression }*
```

### Definitions
```
definition        -> value_definition
                     | type_definition ;
value_definition  -> 'let' name '=' expression ;
type_definition   -> 'type' name '=' type ;
type              -> atom_type
                     | function_type
                     | vector_type
                     | '(' type ')';
atom_type         -> 'boolean'
                     | 'integer'
                     | 'double' ;
function_type     -> 'fn' '(' [types] ')' '->' type ;
vector_type       -> 'vector' '[' type ']' ;
types             -> type { ',' type }*
```

### Expressions
```
expression          -> comparison | lambda | conditional;
comparison          -> boolean_term [ '==' | '!=' | '>' | '<' | '>=' | '<=' boolean_term ] ;
boolean_term        -> boolean_factor { '||' boolean_factor } ;
boolean_factor      -> term { '&&' term } ;
term                -> factor { ('+' | '-') factor }* ;
lambda              -> 'fn' '(' [parameters] ')' '->' type expression ;
conditional         -> 'if' expression 'then' expression 'else' expression ;
factor              -> unary { ('*' | '/') unary }* ;
unary               -> ['+' | '-' | '!'] primary | call ;
parameters          -> parameter {',' parameter}* ;
primary             -> FLOATING_POINT_NUMBER
                        | INTEGER_NUMBER
                        | NAME
                        | '(' expression ')'
                        | vector;
call                -> primary { '(' [expressions] ')' }+ ;
parameter           -> type NAME ;
vector              -> '[' [expressions] ']' ; 
expressions         -> expression { ',' expression }*            
```
