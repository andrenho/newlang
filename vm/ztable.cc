#include "vm/ztable.hh"

#include "vm/zstring.hh"

bool ZTable::OpEq(shared_ptr<ZValue> other) const 
{
    (void) other;
    return false;  // TODO
}


void ZTable::OpSet(shared_ptr<ZValue> key, shared_ptr<ZValue> value, TableConfig tc)
{
    _items.emplace(make_pair(key, ZTableValue { value, tc }));
}


shared_ptr<ZValue> ZTable::OpGet(shared_ptr<ZValue> key) const
{
    return _items.at(key).value;
}


string ZTable::Inspect() const 
{
    string s = _pubmut ? "&{" : "%{";
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
        s.append(": " + kv.second.value->Inspect());
    }
    s.append("}");
    return s;
}
    
// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=cpp
