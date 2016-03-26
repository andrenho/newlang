#ifndef ZOE_ZVALUE_H_
#define ZOE_ZVALUE_H_

#include <cstdint>
#include <vector>
using namespace std;

namespace Zoe {

enum ZType { NIL, BOOLEAN, INTEGER, FLOAT, BFUNCTION };

struct ZValue {
protected:
    ZValue(ZType type) : Type(type) {}
public:
    virtual ~ZValue() {}
    const ZType Type;

    // prevent copy
    ZValue(ZValue const&) = delete;
    ZValue& operator=(ZValue const&) = delete;
};


struct ZNil : public ZValue {
    ZNil() : ZValue(ZType::NIL) {}
};


struct ZBoolean : public ZValue {
    explicit ZBoolean(bool value) : ZValue(ZType::BOOLEAN), _value(value) {}

    bool Value() const { return _value; }

private:
    bool _value;
};


struct ZInteger : public ZValue {
    explicit ZInteger(int64_t value) : ZValue(ZType::INTEGER), _value(value) {}
    explicit ZInteger(vector<uint8_t> const& data, size_t pos=0);

    int64_t Value() const { return _value; }
    bool operator==(ZInteger const& other) const { return Value() == other.Value(); }
    void InsertIntoVector(vector<uint8_t>& vec) const;

private:
    const int64_t _value;
};


struct ZFloat : public ZValue {
    explicit ZFloat(double value) : ZValue(ZType::FLOAT), _value(value) {}
    explicit ZFloat(vector<uint8_t> const& data, size_t pos=0);

    double Value() const { return _value; }
    bool operator==(ZFloat const& other) const;
    void InsertIntoVector(vector<uint8_t>& vec) const;

private:
    double _value;
};


struct ZBytecodeFunction : public ZValue {
    ZBytecodeFunction(uint8_t n_args, vector<uint8_t> const& bytecode) 
        : ZValue(ZType::BFUNCTION), _n_args(n_args), _bytecode(bytecode) {}
    uint8_t NArgs() const { return _n_args; }
    vector<uint8_t> const& Bytecode() const { return _bytecode; }

private:
    const uint8_t _n_args;
    const vector<uint8_t> _bytecode;
};


}

#endif

// vim: ts=4:sw=4:sts=4:expandtab
