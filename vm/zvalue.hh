#ifndef VM_ZVALUE_H_
#define VM_ZVALUE_H_

#include <memory>
using namespace std;

#include "vm/ztype.hh"

class ZValue {
public:
    virtual ~ZValue() {}

    ZType Type() const { return _type; }
    
    virtual uint64_t Hash() const {
        throw invalid_argument("Values of type " + Typename(_type) + " can't be used as table key.");
    }

    virtual string Inspect() const = 0;
    virtual bool   OpEq(shared_ptr<ZValue> other) const = 0;

protected:
    explicit ZValue(ZType type) : _type(type) {}
    const ZType _type;
};

#endif

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=cpp
