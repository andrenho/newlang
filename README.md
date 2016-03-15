# newlang
A new provisional language (name pending)

## Characteristics

This language is a derivative of Lua, and tries to improve on it. From Lua, it maintains:

- embeddability
- VM
- small footprint
- a single executable/DLL

But, contrary to Lua, it also features:

- more features
- shorter syntax (less typing)
- a more extensive standard library
- a JIT engine
- ability to distribute binaries

Example syntax:

```
fn fib(n: int) {
   case(n) {
     n < 2: ret n;
     n:     ret fib(n-1) + fin(n-2);
   }
}
```
