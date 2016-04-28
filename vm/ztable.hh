#ifndef VM_ZTABLE_H_
#define VM_ZTABLE_H_

#include <unordered_map>
using namespace std;

#include "vm/zvalue.hh"
#include "vm/znumber.hh"
#include "vm/opcode.hh"  // TODO - for TableConfig

struct ZTableHash {
    
    size_t operator()(shared_ptr<ZValue> const& k) const {
        return k->Hash();
    }

    bool operator()(shared_ptr<ZValue> const& a, shared_ptr<ZValue> const& b) const {
        return a->OpEq(b);
    }

};


struct ZTableValue {
    shared_ptr<ZValue> value;
    TableConfig        config;
    ZTableValue(shared_ptr<ZValue> v, TableConfig c) : value(v), config(c) {}
};


typedef unordered_map<shared_ptr<ZValue>, ZTableValue, ZTableHash, ZTableHash> ZTableHashMap;

class ZTable : public ZValue {
public:
    explicit ZTable(bool pubmut) : ZValue(StaticType()), _pubmut(pubmut) {}

    template<typename Iter> ZTable(Iter const& _begin, Iter const& _end, bool pubmut) : ZTable(pubmut) {{{
    
        // find prototypes
        auto t = _begin;
        OpProto(*t++);

        // load data
        while(t != _end) {
            auto key = *t++;

            /* If using PTBL opcode, then pubmut = false, else if using PTBX, pubmut = true.
             * If pubmut = false, we have 3 parameters stacked in the iterator (key, property, value), 
             *   else we have two (key, property). Thus we advance the iterator depending on pubmut. */
            TableConfig n = static_cast<TableConfig>(PUB|MUT);
            if(!pubmut) {
                n = static_cast<TableConfig>(dynamic_pointer_cast<ZNumber>(*t++)->Value());
            }

            _items.emplace(make_pair(key, ZTableValue { *t++, n }));
        }
    }}}

    string Inspect() const override;
    void   Clear() { _items.clear(); }

    bool OpEq(shared_ptr<ZValue> other) const override;
    void OpSet(shared_ptr<ZValue> key, shared_ptr<ZValue> value, TableConfig tc) override;
    void OpProto(shared_ptr<ZValue> proto) override;
    shared_ptr<ZValue> OpGet(shared_ptr<ZValue> key) const override;

    static ZType StaticType() { return TABLE; }
    ZTableHashMap const& Value() const { return _items; }

private:
    ZTableHashMap _items = {};
    bool _pubmut;                   // fields are public and mutable by default
    shared_ptr<ZValue> _prototype = nullptr;
};

#endif

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=cpp
