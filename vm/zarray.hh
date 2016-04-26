#ifndef VM_ZARRAY_H_
#define VM_ZARRAY_H_

#include "vm/zvalue.hh"

class ZArray : public ZValue {
public:
    template<typename It>
    ZArray(It _begin, It _end) : ZValue(StaticType()), _items(_begin, _end) {}

    static ZType StaticType() { return ARRAY; }

    bool OpEq(shared_ptr<ZValue> other) const {
        (void) other;
        return false;  // TODO
    }

    string Inspect() const { return "nil"; }
    
    vector<shared_ptr<ZValue>> const& Value() const { return _items; }

private:
    vector<shared_ptr<ZValue>> _items;
};

template<> struct cpp_type<vector<shared_ptr<ZValue>>> { typedef ZArray type; };

#endif

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=cpp
