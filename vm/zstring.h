#ifndef VM_ZSTRING_H_
#define VM_ZSTRING_H_

#include <string>
using namespace std;

#include "vm/zvalue.h"

class ZString : public ZValue {
public:
    explicit ZString(string const& value) : ZString(value, 0) {}
    ZString(string const& value, size_t hsh) : ZValue(STRING), _value(value), _hash(hsh)  {}

    string const& Value() const { return _value; }

private:
    string _value;
    size_t _hash;
};

#endif

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=cpp
