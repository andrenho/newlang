#ifndef VM_ZVALUE_H_
#define VM_ZVALUE_H_

#include "vm/ztype.h"

class ZValue {
public:
    virtual ~ZValue() {}

    ZType Type() const { return _type; }

protected:
    explicit ZValue(ZType type) : _type(type) {}
    const ZType _type;
};

#endif

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=cpp
