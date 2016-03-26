#ifndef ZOE_ZVALUE_H_
#define ZOE_ZVALUE_H_

#include <cstdint>
#include <vector>
using namespace std;

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
    explicit ZInteger(vector<uint8_t> const& data, size_t pos=0);

    int64_t Value() const { return _value; }
    bool operator==(ZInteger const& other) { return Value() == other.Value(); }
    void InsertIntoVector(vector<uint8_t>& vec) const;

    const int64_t _value;
};


struct ZBytecodeFunction : public ZValue {
    ZBytecodeFunction(uint8_t n_args, vector<uint8_t> const& bytecode) : _n_args(n_args), _bytecode(bytecode) {}
    uint8_t NArgs() const { return _n_args; }
    vector<uint8_t> const& Bytecode() const { return _bytecode; }

    const uint8_t _n_args;
    const vector<uint8_t> _bytecode;
};


}

#endif

// vim: ts=4:sw=4:sts=4:expandtab
