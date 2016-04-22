#include "vm/zoevm.h"

#include <cassert>

#include "vm/znil.h"

ZoeVM::ZoeVM()
{
    _stack.push_back(make_shared<ZNil>());
}


// {{{ STACK MANAGEMENT

ssize_t ZoeVM::StackAbs(ssize_t pos) const
{
    ssize_t i = (pos >= 0) ? pos : StackSize() + pos;
    assert(i >= 0);
    return i;
}


ssize_t ZoeVM::StackSize() const
{
    return static_cast<ssize_t>(_stack.size());
}


ZValue const& ZoeVM::Push(shared_ptr<ZValue> value)
{
    _stack.push_back(value);
    return *_stack.back().get();
}


shared_ptr<ZValue> ZoeVM::Pop()
{
    if(_stack.empty()) {
        throw underflow_error("Stack underflow");
    }
    auto last = _stack.back();
    _stack.pop_back();
    return last;
}


ZValue const* ZoeVM::GetPtr(ssize_t pos) const
{
    ssize_t i = StackAbs(pos);
    if(i >= StackSize()) {
        throw out_of_range("Stack access out of range");
    }
    return _stack[i].get();
}


shared_ptr<ZValue> ZoeVM::GetCopy(ssize_t pos) const
{
    ssize_t i = StackAbs(pos);
    if(i >= StackSize()) {
        throw out_of_range("Stack access out of range");
    }
    return _stack[i];
}

// }}}

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=cpp
