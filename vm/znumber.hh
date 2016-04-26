#ifndef VM_ZNUMBER_H_
#define VM_ZNUMBER_H_

#include <cmath>
#include <limits>
#include <functional>
using namespace std;

#include "vm/zvalue.hh"

class ZNumber : public ZValue {
public:
    explicit ZNumber(double value) : ZValue(StaticType()), _value(value) {}

    double   Value() const { return _value; }
    uint64_t Hash() { return hash<double>()(_value); }

    bool OpEq(shared_ptr<ZValue> other) const {
        if(Type() != other->Type()) {
            return false;
        } else {
            return abs(_value - dynamic_pointer_cast<ZNumber>(other)->Value()) < numeric_limits<double>::epsilon();
        }
    }

    string Inspect() const { return "nil"; }
    
    static ZType StaticType() { return NUMBER; }

private:
    const double _value;
};

template<> struct cpp_type<double>       { typedef ZNumber type; };
template<> struct cpp_type<int>          { typedef ZNumber type; };
template<> struct cpp_type<unsigned int> { typedef ZNumber type; };

#endif

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=cpp
