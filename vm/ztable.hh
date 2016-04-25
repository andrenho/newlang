#ifndef VM_ZTABLE_H_
#define VM_ZTABLE_H_

#include <unordered_map>
using namespace std;

#include "vm/zvalue.hh"
#include "vm/opcode.hh"  // TODO - for TableConfig

struct ZTableHash {
    
    size_t operator()(shared_ptr<ZValue> const& k) const {
        return k->Hash();
    }

    bool operator()(shared_ptr<ZValue> const& a, shared_ptr<ZValue> const& b) const {
        return a->OpEq(b);
    }

};


typedef unordered_map<shared_ptr<ZValue>, shared_ptr<ZValue>, ZTableHash, ZTableHash> ZTableHashMap;

class ZTable : public ZValue {
public:
    template<typename It>
    ZTable(It _begin, It _end, TableConfig tc) 
            : ZValue(StaticType()), _config(tc) {
        for(auto& t=_begin; t < _end; ++t) {
            auto key = *t++;
            _items[key] = *t;
        }
    }

    virtual bool OpEq(shared_ptr<ZValue> other) const {
        (void) other;
        return false;  // TODO
    }
    
    static ZType StaticType() { return TABLE; }
    ZTableHashMap const& Items() const { return _items; }

private:
    ZTableHashMap _items = {};
    TableConfig _config;
};

#endif

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=cpp
