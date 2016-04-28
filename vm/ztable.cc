#include "vm/ztable.hh"

#include "vm/zstring.hh"

bool ZTable::OpEq(shared_ptr<ZValue> other) const 
{
    (void) other;
    return false;  // TODO
}


void ZTable::OpSet(shared_ptr<ZValue> key, shared_ptr<ZValue> value, TableConfig tc)
{
    // find attribute configuration (PUB|MUT if no element)
    TableConfig t = static_cast<TableConfig>(PUB|MUT);
    auto search = _items.find(key);
    if(search != _items.end()) {

        // verify configuration
        t = search->second.config;
        if(!(t & MUT)) {
            throw zoe_runtime_error("Property " + key->Inspect() + " is not mutable.");
        }
        if(!(t & PUB)) {
            // TODO - fix this for @this
            throw zoe_runtime_error("Property " + key->Inspect() + " is private.");
        }

        // remove existing item
        _items.erase(search);
    }

    // add to table
    _items.emplace(make_pair(key, ZTableValue { value, tc }));
}


shared_ptr<ZValue> ZTable::OpGet(shared_ptr<ZValue> key) const
{
    auto search = _items.find(key);
    if(search == _items.end()) {
        throw zoe_runtime_error("Property " + key->Inspect() + " not found.");
    } 

    ZTableValue const& v = search->second;
    if(!(v.config & PUB)) {
        // TODO - fix this for @this
        throw zoe_runtime_error("Property " + key->Inspect() + " is private.");
    }
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
