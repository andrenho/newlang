#ifndef VM_ZSTRING_H_
#define VM_ZSTRING_H_

#include <string>
using namespace std;

#include "vm/zvalue.hh"

class ZString : public ZValue {
public:
    explicit ZString(string const& value) : ZString(value, 0) {}
    ZString(string const& value, size_t hsh) : ZValue(StaticType()), _value(value), _hash(hsh)  {}

    string const& Value() const { return _value; }
    uint64_t Hash() const override;

    bool OpEq(shared_ptr<ZValue> other) const override;
    string Inspect() const override;

    static ZType StaticType() { return STRING; }

private:
    string _value;
    mutable size_t _hash;
};

template<> struct cpp_type<string> { typedef ZString type; };

#endif

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=cpp
