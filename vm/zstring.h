#ifndef VM_ZSTRING_H_
#define VM_ZSTRING_H_

#include <string>
using namespace;

#include "vm/zvalue.h"

class ZString : public ZValue {
public:
    explicit ZString(string const& value) : _value(value), _hash(0) {}
    ZString(string const& value, size_t hsh) : _value(value), _hash(hsh) {}
    
private:
    string _value;
    size_t _hash;
};

#endif

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=cpp
