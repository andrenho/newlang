#ifndef ZOE_ZVALUE_H_
#define ZOE_ZVALUE_H_

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


}

#endif

// vim: ts=4:sw=4:sts=4:expandtab
