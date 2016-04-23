#ifndef VM_ZTABLE_H_
#define VM_ZTABLE_H_

#include "vm/zvalue.h"

class ZTable : public ZValue {
public:
    ZTable() : ZValue(TABLE) {}
};

#endif

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=cpp
