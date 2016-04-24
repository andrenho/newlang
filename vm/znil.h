#ifndef VM_ZNIL_H_
#define VM_ZNIL_H_

#include "vm/zvalue.h"

class ZNil : public ZValue {
public:
    ZNil() : ZValue(StaticType()) {}

    static ZType StaticType() { return NIL; }
};

#endif

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=cpp
