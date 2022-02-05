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

### Scratch
```
type tree = [a] native(a data, tree[a] left, tree[a] right) | nil

let s = fn (integer x) -> integer
    x + 1
let q = fn () -> (fn (integer) -> integer)
    s
    
   

q()(0)
        
.native(-1,
      .native(-2, .nil, .nil),
      .native(-3, .nil, .nil))

type vector = {float x, float y}

let add_vectors = fn(vector v, vector w) -> vector
    vector(v.x + w.x, v.y + w.y)

type full_name = {name first, name last}
full_name('Andrei', 'Ilin')
full_name.first

type maybe = [a] nothing | just {a some}

.nothing
x = .just(some = -1)
case x of
    .nothing -> 0
    .just(some) -> some

type either = [a, b] this (a value) | that (b value)
x = .this(-1)
case x of
    .this(value) -> value
    .that(value) -> value

let pi = 3.14
let sgn = fn (float x) -> integer
        let
            y = -x
        in
            | y > 0     -> -1
            | y < 0     ->  1
            | ...       ->  0

let sort = fn [a] (list[a] x) -> list[a]
        
    
let inc_list = fn (list[integer] v) -> list[integer]
    transform[integer](v, map (integer z) -> integer z + 1)
    
let sum = fn [a] (list[a] x) -> a
    fold[T](x, add, 0)
    
let product = fn [a] (list[a] x) -> a
    fold[a](x, multiply, 0)
    
let plus = fn [a] (a x, a y) -> a
    x + y
    
let max = fn [a] (a x, a y) -> a
    | greater(x, y)    -> x
    | ...              -> y
    
let empty = fn [a] (list[a] x) -> boolean
    | size(x) > 0         -> true
    | ...                 -> false
    
let max = fn [a] (list[a] x) -> maybe[a]
    fold[a](x, max)
```
