#ifndef VM_ZSTRING_H_
#define VM_ZSTRING_H_

#include <functional>
#include <string>
using namespace std;

#include "vm/zvalue.h"

class ZString : public ZValue {
public:
    explicit ZString(string const& value) : ZString(value, 0) {}
    ZString(string const& value, size_t hsh) : ZValue(StaticType()), _value(value), _hash(hsh)  {}

    uint64_t Hash() {
        if(_hash == 0) {
            _hash = hash<string>()(_value);
        }
        return _hash;
    }

    virtual bool OpEq(shared_ptr<ZValue> other) const {
        if(Type() != other->Type()) {
            return false;
        } else {
            return Value() == dynamic_pointer_cast<ZString>(other)->Value();
        }
    }

    static ZType StaticType() { return STRING; }

    string const& Value() const { return _value; }

private:
    string _value;
    size_t _hash;
};

#endif

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=cpp
