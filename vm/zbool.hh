#ifndef VM_ZBOOL_H_
#define VM_ZBOOL_H_

#include "vm/zvalue.hh"

class ZBool : public ZValue {
public:
    explicit ZBool(bool value) : ZValue(StaticType()), _value(value) {}

    bool     Value() const { return _value; }
    uint64_t Hash() const override { return _value ? 1 : 2; }

    bool     OpEq(shared_ptr<ZValue> other) const override;
    string   Inspect() const override;
    
    static ZType StaticType() { return BOOL; }

private:
    const bool _value;
};

template<> struct cpp_type<bool> { typedef ZBool type; };

#endif

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=cpp
