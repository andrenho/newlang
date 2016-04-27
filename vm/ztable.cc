#include "vm/ztable.hh"

#include "vm/zstring.hh"

bool ZTable::OpEq(shared_ptr<ZValue> other) const 
{
    (void) other;
    return false;  // TODO
}


string ZTable::Inspect() const 
{
    string s = (_config == (PUB|MUT)) ? "&{" : "%" + InspectProperties(_config) + "{";
    bool fst = true;
    for(auto const& kv: _items) {
        if(!fst) {
            s.append(", ");
        } else {
            fst = false;
        }
        if(kv.first->Type() == STRING) {
            s.append(static_pointer_cast<ZString>(kv.first)->Value());
        } else {
            s.append("[" + kv.first->Inspect() + "]");
        }
        s.append(": " + kv.second->Inspect());
    }
    s.append("}");
    return s;
}
    

string ZTable::InspectProperties(TableConfig tc)
{
    string s;
    if(tc & PUB) {
        s += "pub ";
    }
    if(tc & MUT) {
        s += "mut ";
    }
    return s;
}


// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=cpp
