#include "vm/ztable.hh"

bool ZTable::OpEq(shared_ptr<ZValue> other) const 
{
    (void) other;
    return false;  // TODO
}


string ZTable::Inspect() const 
{
    return "nil"; 
}
    

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=cpp
