Philosophy
==========

* Programming should be fun and productive. The whole language must fit inside the programmer's head, even if the standard library doesn't. Every aspect of the language should be clear and have as few exceptions as possible.
* The core language must be as small as possible, and any program must be possible to write (even if unpratical) using only the core language.
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

Ideally, the Zoe code is translated to the core language before generating the Bytecode. _In practice, the whole parsing happens at once. This way, better and faster bytecode can be produced._

Types
-----

The following basic types are available from Zoe:

| Type    | Example    | _Internal C++ type_ |
| ------- | ---------- | ------------------- |
| nil     | nil        | _nullptr\_t_        |
| boolean | true       | _bool_              |
| number  | 42.3       | _double_ (64 bits)  |
| string  | 'hello'    | _string_            |
| array   | [1, 2, 3]  | _vectori\<shared\_ptr\<ZValue>>_ |
| table   | %{ hello: 'world' } | _unordered\_map\<shared\_ptr\<ZValue>, shared\_ptr\<ZValue>>_ |

Tables
------

### Metamethods

### Visibility

### Const 

### $ENV

### Weak tables


Functions
---------

### Const functions

### this (@)

### Closures

### Yields


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
