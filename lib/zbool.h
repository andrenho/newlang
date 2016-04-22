#ifndef LIB_ZBOOL_H_
#define LIB_ZBOOL_H_

#include "lib/zvalue.h"

class ZBool : public ZValue {
public:
    explicit ZBool(bool value) : Value(value) {}

    const bool Value;
};

#endif

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=cpp
