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

### Constant and mutable functions

A constant (or pure) function is a function that can't touch any external data except for mutable parameters that are passed to it. A mutable function can change external data (such as `this` and "globals").

Constant functions can only call other constant functions.

Functions are, by default, constant. Mutable function can be defined as `fn(v) mut { @value = v }`.

### Closures

Any local variable created at the top scope of a function is stored in the function table. These variables can be accessed at the function value `f.\_\_upvalues`. This makes these variables exist for as long as the variable object exists.

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
for mut i=0; i<10; ++i {
    let mut y = 0
    a.push(fn() { y += 1; x+y })
}
```

### Iterator

It is possible to create iterators by yielding the control from a function. Example:

```
let one_to_ten = fn() {
    for mut i=0; i<10; ++i {
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
let mut tbl = %{ hello: 'world', test: 42 }      // initialization
let mut tbl = %{ [42]: 'hello'}                  // using something other than a string as key
tbl['hello']                                     // fetching data
tbl['hello'] = 'world'                           // setting data
tbl.hello                                        // alternative way to fetch data (string keys only)
tbl.hello = 'world'                              // alternative way to set data (string keys only)
```

Tables can't be used as keys, unless they have the metamethods `__hash` and `__==` implemented.

### $ENV and local variables



Any variables that are created are child of this variable. Example:

```
Zoe syntax                What actually happens internally
----------                --------------------------------
let a = 4                 $ENV.a = 4
let mut b = 5             mut $ENV.b = 5
let x = %{}               $ENV.x = %{}
x.hello = 42              $ENV.x.hello = 42
{                         let mut new_scope = %$ENV {}
let a = 8                 


### Assignment

### Metamethods

### Visibility

### Const 

### Weak tables

### "Everything is a table"

Every value in Zoe is a table. 


Expressions
-----------

### Control flow
_GOTO_


Syntatic sugar
==============


Local variables
---------------

### Implementation


Operators
---------


Matches
-------


Object orientation
------------------


Error management
----------------


Modules
-------


C interface
-----------


Standard library
================

## nil

## boolean

## number

## array

## table

## string

## regex

## posix


Advanced topics
===============

Garbage collection
------------------

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

## REPL / executable

## Debugger


Virtual machine reference
=========================


Development order
=================


<!--
vim: wrap lbr nolist
-->
