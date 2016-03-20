# zoe
A new provisional language (name pending)


## Characteristics

This language is a derivative of Lua, and tries to improve on it. From Lua, it maintains:

- embeddability
- VM
- small footprint
- a single executable/DLL
- supports tail calls
- allows manipulating environments (\_ENV)

It copies from other languages:

- more features (like Python / Ruby)
- shorter syntax (less typing - like C)
- a more extensive standard library (like Python / Ruby)
- a JIT engine (like LuaJIT)
- ability to distribute binaries (like Java)
- allow unsafe block, if explicit (like Rust)
- encompassing case (like Rust)
- const variables (like ES6)
- advanced class management, with final methods, forced overwrite, etc (like C++)
- a good REPL (like Ruby's IRB)
- a good debugger (like gdb, or maybe accepting connections from gdb)
- garbage collected (like most languages)
- native POSIX support
- generate documentation from code
- native threading
- unit testing

It also innovates in that:

- generates executables


## Syntax

Basic example:

```
fn fib(n: int) {
   match n {
     n < 2: n
     _:     fib(n-1) + fib(n-2)
   }
}
```

Syntax characteristics:
- everything is an object
- everything is a expression
- 
