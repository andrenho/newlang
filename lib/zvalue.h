#ifndef ZOE_ZVALUE_H_
#define ZOE_ZVALUE_H_

#include <cstdint>

namespace Zoe {

struct ZValue {
protected:
    ZValue() {}
public:
    virtual ~ZValue() {}

    // prevent copy
    ZValue(ZValue const&) = delete;
    ZValue& operator=(ZValue const&) = delete;
};


struct ZInteger : public ZValue {
    explicit ZInteger(int64_t value) : _value(value) {}
    int64_t Value() const { return _value; }
    int64_t _value;

    bool operator==(ZInteger const& other) { return Value() == other.Value(); }
};


struct ZBytecodeFunction : public ZValue {
    ZBytecodeFunction(uint8_t n_args, vector<uint8_t> const& code) : _n_args(n_args), _code(code) {}
    uint8_t NArgs() const { return _n_args; }
    vector<uint8_t> const& Code() const { return _code; }

    uint8_t _n_args;
    vector<uint8_t> _code;
};


}

#endif

// vim: ts=4:sw=4:sts=4:expandtab
