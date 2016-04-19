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
  +------------+  parsing  +----------------+  execution   +--------+
  |  ZOE CODE  |---------->|  ZOE BYTECODE  |------------> | ZOE VM |
  +------------+           +----------------+              +--------+
~~~~~~~~~~~

The ZoeVM never interprets Zoe code directly - the code is read 


Basic operation
===============

Types
-----



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
