#include "vm/zstring.hh"

#include <functional>

uint64_t ZString::Hash() const
{
    if(_hash == 0) {
        _hash = hash<string>()(_value);
    }
    return _hash;
}


bool ZString::OpEq(shared_ptr<ZValue> other) const 
{
    if(Type() != other->Type()) {
        return false;
    } else {
        return Value() == dynamic_pointer_cast<ZString>(other)->Value();
    }
}


string ZString::Inspect() const 
{ 
    return string("'") + _value + "'";
}

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=cpp
