#include "vm/zbool.hh"

bool ZBool::OpEq(shared_ptr<ZValue> other) const 
{
    if(Type() != other->Type()) {
        return false;
    }
    return _value == dynamic_pointer_cast<ZBool>(other)->Value();
}

string ZBool::Inspect() const 
{ 
    return _value ? "true" : "false"; 
}

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=cpp
