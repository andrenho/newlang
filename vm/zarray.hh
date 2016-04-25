#ifndef VM_ZARRAY_H_
#define VM_ZARRAY_H_

#include "vm/zvalue.hh"

class ZArray : public ZValue {
public:
    template<typename It>
    ZArray(It _begin, It _end) : ZValue(StaticType()), _items(_begin, _end) {}

    static ZType StaticType() { return ARRAY; }

    virtual bool OpEq(shared_ptr<ZValue> other) const {
        (void) other;
        return false;  // TODO
    }
    
    vector<shared_ptr<ZValue>> const& Items() const { return _items; }

private:
    vector<shared_ptr<ZValue>> _items;
};

#endif

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=cpp
