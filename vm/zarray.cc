#include "vm/zarray.hh"


bool ZArray::OpEq(shared_ptr<ZValue> other) const 
{
    (void) other;
    return false;  // TODO
}


string ZArray::Inspect() const 
{ 
    string s = "[";
    for(size_t i=0; i<_items.size(); ++i) {
        if(i != 0) {
            s.append(", ");
        }
        s.append(_items[i]->Inspect());
    }
    s.append("]");
    return s;
}

    
// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=cpp
