#ifndef VM_ZFUNCTION_H_
#define VM_ZFUNCTION_H_

#include "vm/zvalue.hh"

enum ZFunctionType { POINTER };

class ZFunction : public ZValue {
public:
    ZFunction() : ZValue(StaticType()) {}

    virtual ZFunctionType FunctionType() const = 0;

    static ZType StaticType() { return FUNCTION; }
};


class ZFunctionPointer : public ZFunction {
public:
    ZFunctionPointer(uint64_t ptr, uint8_t n_args) : 
        ZFunction(), _ptr(ptr), _nargs(n_args) {}

    ZFunctionType FunctionType() const { return POINTER; }

    uint64_t Value() const;
    uint64_t Hash() const override;

    bool     OpEq(shared_ptr<ZValue> other) const override;
    string   Inspect() const override;

private:
    uint64_t _ptr;
    uint8_t _nargs;
};

#endif

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=cpp
