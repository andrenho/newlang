#ifndef VM_ZVALUE_H_
#define VM_ZVALUE_H_

#include <memory>
using namespace std;

#include "vm/ztype.hh"
#include "vm/opcode.hh"
#include "vm/exceptions.hh"

class ZValue {
public:
    virtual ~ZValue() {}

    ZType Type() const { return _type; }
    
    virtual uint64_t Hash() const {
        throw zoe_runtime_error("Values of type " + Typename(_type) + " can't be used as table key.");
    }

    virtual string Inspect() const = 0;
    virtual bool   OpEq(shared_ptr<ZValue> other) const = 0;
    virtual void   OpSet(shared_ptr<ZValue> /* key */, shared_ptr<ZValue> /* value */, TableConfig /* tc */) {
        throw zoe_runtime_error(Typename(Type()) + "s can't be set.");
    }
    virtual shared_ptr<ZValue> OpGet(shared_ptr<ZValue> /* key */) const {
        throw zoe_runtime_error(Typename(Type()) + "s can't be get.");
    }
    virtual void OpProto(shared_ptr<ZValue> /* proto */) {
        throw zoe_runtime_error(Typename(Type()) + "s can't have prototypes.");
    }

protected:
    explicit ZValue(ZType type) : _type(type) {}
    const ZType _type;
};

#endif

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=cpp
