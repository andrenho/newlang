#ifndef VM_ZARRAY_H_
#define VM_ZARRAY_H_

#include "vm/zvalue.h"

class ZArray : public ZValue {
public:
    ZArray() : ZValue(ARRAY) {}
};

#endif

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=cpp
