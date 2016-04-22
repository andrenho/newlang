#ifndef VM_ZNUMBER_H_
#define VM_ZNUMBER_H_

#include "vm/zvalue.h"

class ZNumber : public ZValue {
public:
    explicit ZNumber(double value) : Value(value) {}

    const double value;
};

#endif

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=cpp
