#ifndef VM_ZNUMBER_H_
#define VM_ZNUMBER_H_

#include <cmath>
#include <limits>
#include <functional>
using namespace std;

#include "vm/zvalue.h"

class ZNumber : public ZValue {
public:
    explicit ZNumber(double value) : ZValue(StaticType()), Value(value) {}

    uint64_t Hash() { return hash<double>()(Value); }

    virtual bool OpEq(shared_ptr<ZValue> other) const {
        if(Type() != other->Type()) {
            return false;
        } else {
            return abs(Value - dynamic_pointer_cast<ZNumber>(other)->Value) < numeric_limits<double>::epsilon();
        }
    }
    
    static ZType StaticType() { return NUMBER; }

    const double Value;
};

#endif

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=cpp
