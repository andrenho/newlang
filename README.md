# newlang
A new provisional language (name pending)


## Characteristics

This language is a derivative of Lua, and tries to improve on it. From Lua, it maintains:

- embeddability
- VM
- small footprint
- a single executable/DLL

It copies from other languages:

- more features (like Python / Ruby)
- shorter syntax (less typing - like C)
- a more extensive standard library (like Python / Ruby)
- a JIT engine (like LuaJIT)
- ability to distribute binaries (like Java)
- Rust-like case and enum

It also innovates in that:

- generates executables


## Example syntax:

```
fn fib(n: int) {
   case(n) {
     n < 2: ret n;
     n:     ret fib(n-1) + fin(n-2);
   }
}
```
