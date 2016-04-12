#include "lib/zoe.h"

// {{{ CONSTRUCTOR/DESTRUCTOR

Zoe::Zoe()
{
    Push(nullptr);
}

// }}}

// {{{ STACK ACCESS

void Zoe::Pop()
{
    if(stack.empty()) {
        throw "Stack underflow.";
    }
    stack.pop_back();
}

// }}

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=cpp
