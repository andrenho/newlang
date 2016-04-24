#ifndef VM_ZBOOL_H_
#define VM_ZBOOL_H_

#include "vm/zvalue.h"

class ZBool : public ZValue {
public:
    explicit ZBool(bool value) : ZValue(StaticType()), Value(value) {}

    uint64_t Hash() { return Value ? 1 : 2; }

    virtual bool OpEq(shared_ptr<ZValue> other) const {
        if(Type() != other->Type()) {
            return false;
        } else {
            return Value == dynamic_pointer_cast<ZBool>(other)->Value;
        }
    }
    
    static ZType StaticType() { return BOOL; }

    const bool Value;
};

#endif

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=cpp
