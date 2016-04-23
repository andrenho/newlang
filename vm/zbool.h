#ifndef VM_ZBOOL_H_
#define VM_ZBOOL_H_

#include "vm/zvalue.h"

class ZBool : public ZValue {
public:
    explicit ZBool(bool value) : ZValue(BOOL), Value(value) {}

    const bool Value;
};

#endif

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=cpp
