This document is more a "brainstorming sesssion" rather than a guide.


Philosophy
==========

* Programming should be fun and productive. The whole language must fit inside the programmer's head, even if the standard library doesn't. Every aspect of the language should be clear and have as few exceptions as possible.
* The core language must be as small as possible, and any program must be possible to write (even if unpratical) using only the core language.
* The environment should small, simple and self-contained.
* Zoe should be familiar enough that an experienced programmer can pick it up in a couple hours.

### The language

* Syntatic sugar will be used to make the programmer more productive and happier. Syntatic sugar can be hardcoded in the parser/interpreter to improve speed.
* Const correctness and type safety are very important.
* The language can be used both embedded and standalone.
* Everything is an expression and every expression must return one single value.
* Every value is a table, though corners can be cut inside the parser/interpreter.


Execution flow
==============

~~~~~~~~~~~
  +------------+  parsing  +----------------+  execution  +--------+
  |  ZOE CODE  |---------->|  ZOE BYTECODE  |------------>| ZOE VM |
  +------------+           +----------------+             +--------+
~~~~~~~~~~~

The ZoeVM never interprets Zoe code directly - the code is read by a parser, that then generates a byte code that will be then executed by a Virtual Machine.


Core language
=============

The **core language** is the subset of the language on top of which the rest of the language sits. Everything that can be done with the rest of the language, can be done with only the core.

_(A possible implementation would be to translate the full syntax to the core language before generating the bytecode.)_


Values and Types
----------------

The following basic types are available from Zoe:

| Type    | Example    | _Internal C++ type_ |
| ------- | ---------- | ------------------- |
| nil     | nil        | _nullptr\_t_        |
| boolean | true       | _bool_              |
| number  | 42.3       | _double_ (64 bits)  |
| string  | 'hello'    | _string_            |
| array   | [1, 2, 3]  | _vector\<shared\_ptr\<ZValue>>_ |
| table   | %{ hello: 'world' } | _unordered\_map\<shared\_ptr\<ZValue>, shared\_ptr\<ZValue>>_ | _.

_Internally, each value is represented by the type `shared\_ptr\<ZValue>`. `ZValue` is a struct containing a union of all types. This structure is wrappen in a `shared\_ptr` so that we can get the reference counting memory management from C++ for free._


Strings
-------

Strings are mutable in Zoe. They are always represented using single quotes. If a quote is open, the string can span multiple lines.

It is possible to add expressions inside a string using the following syntax: `'2 + 2 = ${ '%d' % (2+2) }'`.

_Internally, a string is a structure containing a `std::string` and the precalculated hash of that string. This makes looking up methods much faster._


Functions
---------

Functions in Zoe are first-class functions. They are defined using the `fn` keyword. They have the following characteristics:

* Zoe will verify the number of arguments, and reject calls that don't match the number of arguments of the function definition.
* There can be optional arguments: `fn(a, b=42)`. Optional arguments must always be at the end of the arguments. A function with several optional arguments can be called by passing the arguments names: `myfunction(42, b=12, c=14)`.
* The arguments can force specific types, for example: `fn(a: number, b: boolean)`. The types are, actually table prototypes (see section _Tables_), so they can be used to verify a class name.
* When a function is called, the `()` is optional if there are no arguments.
* Functions can be passed as arguments.
* Parameters are always passed as constants, except when they are indicated as `mut`.
* Nested functions are allowed.

A function can be of the type `dl`. This means that the function will call an external library. Example: `fn umount(target) dl-> "umount", c\_const\_char. _This syntax is pending._

Since functions are, by definition, anonymous, they can be called recursivelly using the keyword `self`. Like this: `fn fibonacci(a) { (n < 2) ? n : self(n-1) + self(n-2) }`.

There's no syntax to define a function directly. As such, it must be always liked to a constant or a table key:

```
let sum = fn(a, b) {
    a + b
}

math = %{}
math.sum = fn(a, b) { a + b }

math = %{
    sum = fn(a, b) { a + b }
}
```

### this (@)

Internally, when a function is created, an additional parameter called `this` is always added to the parameter list. This is hidden from the user (syntatic sugar).

The `@` symbol can be used as a shortcut. So `this.value` can also be called as `@value`.

### Pure and impure functions

A pure function is a function that can't touch any external data except for mutable parameters that are passed to it. An impure function can change external data (such as `this` and "globals").

Pure functions can only call other pure functions.

Functions are, by default, pure. Impure function can be defined as `fn(v) impure { @value = v }`.

### Closures

Any local variable created at the top scope of a function is stored in the function table. These variables can be accessed at the function value `f.__upvalues`. This makes these variables exist for as long as the variable object exists.

Example:

```
let init = fn() {
    let mut x = 0
    fn() { x += 1 }
}

let increment = init()
increment()             -> 1
increment()             -> 2
increment()             -> 3
```

Another example:

```
let mut var a = []
let x = 20
for mut i=0, i<10, ++i {
    let mut y = 0
    a.push(fn() { y += 1; x+y })
}
```

### Iterator

It is possible to create iterators by yielding the control from a function. Example:

```
let one\_to\_ten = fn() {
    for mut i=0, i<10, ++i {
        yield i
    }
}

for n: one_to_ten {
    dbg(n)
}

// Result: 0123456789
```

Tables
------

Tables are associative arrays, with a few additions.

Tables are the main type in Zoe. Tables are used as classes, objects, modules, etc. 

The basic syntax for tables is the following:

```
let mut tbl = %pub { hello: 'world', test: 42 }   // initialization
let mut tbl = %pub { [42]: 'hello'}               // using something other than a string as key
tbl['hello']                                      // fetching data
tbl['hello'] = 'world'                            // setting data
tbl.hello                                         // alternative way to fetch data (string keys only)
tbl.hello = 'world'                               // alternative way to set data (string keys only)
```

Tables can't be used as keys, unless they have the metamethods `__hash` and `__eq` implemented.

Tables are initialized with the following syntax:

```
%[prototype] [properties] {
    key [properties] : value,
    ...
}
```

Properties are `pub` and `mut`. When defined in the table header, this property becomes the default for the table.

The syntax `&{}` is a shortcut for `%pub mut {}`.

### $G, $ENV and local variables

There is one single variable in a enviroment: `$G`. `$G` is a table that contains all local variables. For example, if a variable called `var` is created, its real name is `$G.var`.

Initially, `$ENV` is points to the same table as `$G`. When a scope is pushed, however, `$ENV` points to a new table, and has `$ENV` (`$G`) as prototype. So, every time a new scope is pushed, a new `$ENV` is created, with the old `$ENV` as a prototype of the new `$ENV`. When a scope is popped, the topmost `$ENV` is destroyed, and `$ENV` now points to the old `$ENV`.

Any variables that are created are child of this variable. Example:

```
Zoe syntax                What actually happens internally
----------                --------------------------------
let a = 4                 $ENV.a = 4
let mut b = 5             mut $ENV.b = 5
let x = &{}               $ENV.x = &{}
x.hello = 42              $ENV.x.hello = 42
{                         $ENV = &$ENV {}
let a = 8                 $ENV.a = 8
dbg(a)                    dbg($ENV.a)   ->  8
dbg(b)                    dbg($ENV.b)   ->  5
}                         $ENV = $ENV.__proto
dbg(a)                    dbg($ENV.a)
```


### Assignment

The syntax `[]=` (which is, in fact, an operator) is used for assignment. When a value is assigned, if the value is a nil, boolean, number or string, it is copied. If it's an array or table, only the reference is copied.


### Metadata

There are special methods and attributes that operate differenty when they are called: the **metamethods**. Metamethods are identified by a double underscore before the name. Most metamethods are called in case of a special syntax. For example, `2 + 3` is translated internally to `2.__add(3)`.

| Method        | Example syntax | Observation |
| ------------- | -------------- | ----------- |
| `__uminus`    | `-3`           | |
| `__add(x)`    | `3 + 4`        | |
| `__sub(x)`    | `3 - 4`        | |
| `__mul(x)`    | `3 * 4`        | |
| `__div(x)`    | `3 / 4`        | |
| `__idiv(x)`   | `3 ~/ 4`       | |
| `__mod(x)`    | `3 % 4`        | |
| `__pow(x)`    | `3 ** 4`       | |
| `__shl(x)`    | `3 << 4`       | |
| `__shr(x)`    | `3 >> 4`       | |
| `__bnot`      | `~3`           | |
| `__and(x)`    | `3 % 4`        | |
| `__or(x)`     | `3 | 4`        | |
| `__xor(x)`    | `3 ^ 4`        | |
| `__not`       | `!true`        | |
| `__eq(x)`     | `3 == 4`       | Inverts the result for `!=` |
| `__part(x)`   | `3 === 4`      | Partial match. Inverts the result for `!==` |
| `__lt(x)`     | `3 < 4`        | Inverts the result for `>=`|
| `__lte(x)`    | `3 <= 4`       | Inverts the result for `>`. If not present, then `__lt` and `__eq` are used. |
| `__len`       | `#value`       | |
| `__concat(x)` | `'a' .. 'b'`   | |
| `__get(x)`    | `tbl[x]` or `tbl.x`          | When `x` is not a key in table |
| `__set(x, y)` | `tbl[x] = y` or `tbl.x = y`  | When `x` is not a key in table |
| `__del(x)`    | `del tbl[x]`   | When the used deletes a value in the table |
| `__call(...)` | `tbl(...)` or `tbl` | |
| `__hash`      |                | Returns a number to be used as a hash |
| `__dbg`       | `dbg(tbl)`     | Return the table description for debugging pourposes. |

These meta-attributes define special configurations of the table:

| Method        | Description |
| ------------- | ----------- |
| `__proto`     | Prototype table (a table or array of tables) |
| `__upvalues`  | Upvalues (variables for use in closures) |

These meta-attributes are read-only and can't be changed by the programmer:

| Method        | Description |
| ------------- | ----------- |
| `__ptr`       | Return the internal C pointer of this value |

These metamethods are called is special occasions:

| Method        | Description |
| ------------- | ----------- |
| `__init(...)` | Called when a table is created. |
| `__gc`        | Called when the table is destroyed. |

Additionally, the "is nil" expression (`?exp`) doesn't have a metamethod and always returns false, except for nil values.


### Privacy

Attributes and methods defined as private can only be seen, read or written when they are part of `this` or one of its prototypes. 

All keys are, by default, private. Example:

```
let A = %{
    b pub: function() {}
    c: function() {}
}

let a = %A {}
a.b()          -> ok
a.c()          -> error: c is private
```

If a table is defined as `pub`, all its methods and attributes are public by default.

```
let A = %pub {
    x: 4
}

let a = %A {}
a.x             -> 4
```

### Const 

Fields in tables are constant by default. This means that they can't be changed. A single field can be defined as mutable (by using the keyword `mut`). The whole table can also be defined as `mut`.

```
let mut a = %pub {
    x: 4,
    y mut: 5,
}

let a = %A {}
a.y = 5         -> ok
a.x = 5         -> error, x is const
```

Notice that the table being mutable is not enough - its reference must also be set as mutable. So, let's say that we have `a.b.c.d.e = 4`, all of them must be set as mutable. If `c` is constant, everything below `c` is also a constant.

### Abstract functions

Abstract functions are defined by the property `abstract_fn`. If an function is abstract, it must be implemeneted by any table that inherits from it. Example:

```
let A = {
    x: abstract_fn(a),
}

let B = %A {}      -> error, 'x' is not implemented
```

### Weak tables

_Define this._


### "Everything is a table"

Every value in Zoe is a table. As such, if a method is called in a primary type, it will actually be called from a internal table. For example, `2.my_method` will be the same as `number.my_method(2)`.


Expressions
-----------

In Zoe, everything is an expression. Every expression must return one single value. For example: `let mut a; if x > y { a = 3 } else { a = 4 }` is the same as `let a = if x > y { 3 } else { 4 }`.

### Control flow

Flow of the operation can be controlled with the following commands:

```
if BOOLEAN_EXPRESSION { RETURN_IF_TRUE } [ else { RETURN_IF_FALSE } ]
for A, B, C {}     // C style (notice the commas instead of semicolons)
for A: B {}
while X {}
do {} while X
== X ==; goto X
```

The keywords `break` and `continue` can be used, just like in C.

### Matches

A special, more complete way of controlling flow is the `match` keyword. It works like rust.

```
match x {
  1              => do_thing1(),
  [1, _]         => do_thing2(),
  [1, 2, _]: r   => do_thing3(r),
  { abc:_ }      => do_thing4(),
  _              => do_something_else(),
}
```

Error management
----------------

Errors are controlled via exceptions:

```
let a = try {
            1/0
        } except(...) {
            0
        }

try {
    1/0
} excepct(e: DivByZeroError) {
    // do something
}

raise TypeError('error description')
```

Exception objects contains the error message and the stack trace.


Modules
-------

Modules are tables, imported via the keyword `import`. Since every expression returns one value, the execution of one file will return one single value -- this is the module.

`import` will search the path defined in `ZOE_PATH`.

```
let math = import 'math'
```

Standard library
================

_TO BE DEFINED_

## nil

## boolean

## number

## array

## table

## string

## regex

## posix

## serial


Advanced topics
===============

_TO BE DEFINED_

Tail calls
----------

External libraries
------------------

### Unsafe calls

Threads
-------

Unicode support
---------------


Ecosystem
=========

The ecosystem is composed by two programs:

* An executable, that runs the Zoe programs and also serves as a REPL,
* and a library, used to embed Zoe into other languages.

## Debugging

_Decision pending: a debugger will be writted, or are we going to communicate with GDB?_


Virtual machine reference
=========================

## Internal memory

The virtual machine contains two pieces of internal memory:

* a **operational stack**, where values are stored for common operations;
* a **function stack**, used to return to the previous function;
* a **heap**, where variables are stored.

Both the operational stack and the heap increment and decrement references.

## Opcodes

The opcodes are:

| Mnemonic | Parameters | Stack change | Description |
| -------- | ---------- | ------------ | ------------------------------------------------------ |
| `pnil`   |            | +1           | Push a nil value into the stack                        |
| `pbf`    |            | +1           | Push a boolean false into the stack                    |
| `pbt`    |            | +1           | Push a boolean true into the stack                     |
| `pn8`    | `u8`       | +1           | Push a 8-bit unsigned integer number into the stack    |
| `pnum`   | `f64`      | +1           | Push a 64-bit floating point number into the stack     |
| `pstr`   | `u32`      | +1           | Push a string from the 32-bit index into the stack     |
| `pary`   | `u16`      | -n +1        | Push an array with _n_ stacked items into it           |
| `ptbl`   | `u16` `u8` | -(n\*2) +1   | Push a table with _n_ stacked pair of key/values, with these characteristics |
| `penv`   |            | +1           | Push $ENV into the stack                               |
| `pop`    |            | -1           | Pop one item from the stack                            |
| `jmp`    | `u32`      |              | Unconditionally branch (jump) to address               |
| `bt`     | `u32`      | -1           | Branch to address if value in stack is true            |
| `pshs`   |            |              | Push a new scope                                       |
| `pops`   |            |              | Pop scope                                              |
| `call`   | `u8`       | -n           | Call function at the top of the stack with _n_ parameters |
| `umn`    |            | -1 +1        | Operator unary minus                                   |
| `add`    |            | -2 +1        | Operator addition                                      |
| `sub`    |            | -2 +1        | Operator subtraction                                   |
| `mul`    |            | -2 +1        | Operator multiplication                                |
| `div`    |            | -2 +1        | Operator division                                      |
| `idiv`   |            | -2 +1        | Operator integer division                              |
| `mod`    |            | -2 +1        | Operator modulo                                        |
| `pow`    |            | -2 +1        | Operator power                                         |
| `shl`    |            | -2 +1        | Operator shift left                                    |
| `shr`    |            | -2 +1        | Operator shift right                                   |
| `bnot`   |            | -1 +1        | Operator binary not                                    |
| `and`    |            | -2 +1        | Operator binary and                                    |
| `or`     |            | -2 +1        | Operator binary or                                     |
| `xor`    |            | -2 +1        | Operator binary xor                                    |
| `not`    |            | -1 +1        | Operator boolean not                                   |
| `eq`     |            | -2 +1        | Operator equals                                        |
| `part`   |            | -2 +1        | Operator partial equals                                |
| `lt`     |            | -2 +1        | Operator less than                                     |
| `lte`    |            | -2 +1        | Operator less than or equal                            |
| `len`    |            | -1 +1        | Operator length                                        |
| `get`    |            | -2 +1        | Operator get                                           |
| `set`    | `u8`       | -3 +1        | Operator set (possible mutable)                        |
| `del`    |            | -2 +1        | Operator delete                                        |
| `insp`   |            | -1 +1        | Generate a inspection string                           |
| `ptr`    |            | -1 +1        | Get internal pointer                                   |
| `isnil`  |            | -1 +1        | Return if value in stack is nil                        |


## Bytecode format

| Position | # bytes | Description          |
| -------- | ------- | -------------------- |
| `00`     |       4 | Magic code           |
| `04`     |       4 | Version              |
| `08`     |       8 | String list position |
| `10`     |     _n_ | Code                 |
| _n_      |     _n_ | Strings              |

Development order
=================

## Version 0.3.0

1. ~~Create build and test environment~~
1. ~~Create ZValue~~
1. ~~Create bytecode generator~~
1. ~~Create VM, stack, and basic execution~~
1. ~~Create basic arrays/tables~~
1. ~~Basic parsing~~
1. ~~Expression inside strings~~
1. ~~$ENV and local variables~~
1. Getting/setting table key/values
1. Table initialization: mut and pub
1. Table prototypes
1. Scopes
1. Some sort of basic documentation
1. Create REPL
1. Parser locations
1. Run in Windows

## Version 0.3.1

1. Basic function calls
1. Function calls with arguments
1. Function calls with optional and named arguments
1. Syntax for running function calls without ()
1. Function calls with type checking
1. Functions as arguments
1. Function parameters passed as contants/mutable
1. Nested functions
1. Recursion + self
1. Function as a table key
1. this (@)
1. Pure/impure functions
1. Closures, upvalues
1. Iterators

## Version 0.3.2

1. Value assignment, local variable assignment
1. Getting/setting array
1. Metadata & operator syntax
1. Table and array equality
1. Constructor/destructor
1. Operator assignment
1. Privacy
1. Constness
1. Abstract functions

## Version 0.3.3

1. Short circuit operators
1. Control flow
1. Matches
1. Error management
1. Modules
1. Partial interpretation (for REPL)

## Version 0.3.4

1. Define standard library
1. Write standard library

## Version 0.3.5

1. Weak tables
1. Tail calls
1. Using external libraries
1. Threads
1. Unicode support
1. Debugging

## Version 0.3.6

1. Slices and negative subscripts
1. Complete documentation
1. ...


Release checklist
=================
* `make lint`
* `make check`
* `make check-valgrind`
* `./configure --enable-coverage && make check && make coverage`
* `make distcheck`
* Compile with clang++
* Compile in mingw

<!--
vim: wrap lbr nolist expandtab
-->
