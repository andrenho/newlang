#ifndef VM_ZNIL_H_
#define VM_ZNIL_H_

#include "vm/zvalue.hh"

class ZNil : public ZValue {
public:
    ZNil() : ZValue(StaticType()) {}

    nullptr_t Value() const { return nullptr; }
    uint64_t  Hash() { return 0; }

    bool OpEq(shared_ptr<ZValue> other) const {
        if(Type() != other->Type()) {
            return false;
        } else {
            return true;
        }
    }

    string Inspect() const { return "nil"; }
    
    static ZType StaticType() { return NIL; }
};

template<> struct cpp_type<nullptr_t> { typedef ZNil type; };

#endif

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=cpp
