#include "vm/znumber.hh"

#include <cmath>
#include <limits>
#include <functional>

uint64_t ZNumber::Hash() const
{ 
    return hash<double>()(_value); 
}


bool ZNumber::OpEq(shared_ptr<ZValue> other) const 
{
    if(Type() != other->Type()) {
        return false;
    }
    return abs(_value - dynamic_pointer_cast<ZNumber>(other)->Value()) < numeric_limits<double>::epsilon();
}


string ZNumber::Inspect() const 
{
    string s = to_string(_value);
    s.erase(s.find_last_not_of("0")+1);
    s.erase(s.find_last_not_of(".")+1);
    return s;
}

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=cpp
