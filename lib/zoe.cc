#include "zoe.h"

#include <cstdio>
#include <cstdarg>
#include <memory>
using namespace std;

namespace Zoe {

// {{{ CONSTRUCTOR/DESTRUCTOR

Zoe::Zoe()
    : stack()
{
    stack.reserve(MaxStackSize);
}

// }}}

// {{{ ERROR MANAGEMENT

void Zoe::Error(string s, ...) const
{
    va_list ap;
    va_start(ap, s);
    fprintf(stderr, "zoe: ");
    vfprintf(stderr, s.c_str(), ap);
    fprintf(stderr, "\n");
    abort();
}

// }}}

// {{{ STACK OPERATIONS

int8_t Zoe::StackSize() const 
{
    return static_cast<int8_t>(stack.size());
}


void Zoe::Push(unique_ptr<ZValue>&& value)
{
    if(stack.size() == MaxStackSize) {
        Error("Stack overflow");  // TODO - improve message
    }
    stack.push_back(move(value));
}


void Zoe::Insert(unique_ptr<ZValue>&& value, int8_t pos)
{
    if(stack.size() == MaxStackSize) {
        Error("Stack overflow");  // TODO - improve message
    }
    stack.insert(stack.begin() + pos, move(value));
}


void Zoe::Pop(int8_t count)
{
    if(stack.size() == 0) {
        Error("Popping empty stack");
    }
    for(int8_t i=0; i<count; ++i) {
        Remove(static_cast<int8_t>(StackSize()-1));
    }
}


void Zoe::Remove(int8_t pos)
{
    if(S(pos) >= stack.size()) {
        Error("Removing index %d when the list has only %d items", S(pos), StackSize());
    }
    stack.erase(stack.begin() + S(pos));
}


// }}}

}  // namespace Zoe

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker
