#ifndef VM_ZNIL_H_
#define VM_ZNIL_H_

#include "vm/zvalue.hh"

class ZNil : public ZValue {
public:
    ZNil() : ZValue(StaticType()) {}

    uint64_t Hash() { return 0; }

    virtual bool OpEq(shared_ptr<ZValue> other) const {
        if(Type() != other->Type()) {
            return false;
        } else {
            return true;
        }
    }
    
    static ZType StaticType() { return NIL; }
};

#endif

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=cpp
