#ifndef VM_ZTABLE_H_
#define VM_ZTABLE_H_

#include <unordered_map>
using namespace std;

#include "vm/zvalue.h"

class ZTable : public ZValue {
public:
    template<typename It>
    ZTable(It _begin, It _end, TableConfig tc) 
            : ZValue(StaticType()), _config(tc) {
        // TODO
    }

    static ZType StaticType() { return TABLE; }

private:
    struct ZTableHash {
        size_t operator()(shared_ptr<ZValue> const& k) const;
        bool   operator()(shared_ptr<ZValue> const& a, shared_ptr<ZValue> const& b) const;
    };

    unordered_map<shared_ptr<ZValue>, shared_ptr<ZValue>, ZTableHash, ZTableHash> _items = {};
    TableConfig _config;
};

#endif

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=cpp
