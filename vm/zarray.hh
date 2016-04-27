#ifndef VM_ZARRAY_H_
#define VM_ZARRAY_H_

#include <vector>
using namespace std;

#include "vm/zvalue.hh"

class ZArray : public ZValue {
public:
    template<typename It>
    ZArray(It const& _begin, It const& _end) : ZValue(StaticType()), _items(_begin, _end) {}

    vector<shared_ptr<ZValue>> const& Value() const { return _items; }

    bool OpEq(shared_ptr<ZValue> other) const;
    string Inspect() const;
    
    static ZType StaticType() { return ARRAY; }

private:
    vector<shared_ptr<ZValue>> _items;
};

template<> struct cpp_type<vector<shared_ptr<ZValue>>> { typedef ZArray type; };

#endif

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=cpp
