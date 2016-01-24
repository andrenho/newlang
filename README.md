# zoe

_zoe_ is a programming language that is productive, fun to use, and feels 
familiar to the experienced programmer.

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
