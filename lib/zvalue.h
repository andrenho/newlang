#ifndef LIB_ZVALUE_H_
#define LIB_ZVALUE_H_

#include <cstddef>
#include <memory>
#include <string>
#include <unordered_map>
#include <type_traits>
#include <vector>
using namespace std;

// 
// TYPES
//
enum ZType { 
    NIL, BOOLEAN, NUMBER, STRING, FUNCTION, ARRAY, TABLE,
};

string Typename(ZType type);

//
// ZFUNCTION
//
enum ZFunctionType { BYTECODE };

struct ZFunction {
    ZFunctionType type;
    union {
        vector<uint8_t> bytecode;
    };
};

// 
// ZARRAY / ZTABLE
// 
struct ZValue;
typedef vector<shared_ptr<ZValue>> ZArray;
typedef unordered_map<shared_ptr<ZValue>,shared_ptr<ZValue>> ZTable;

//
// ZVALUE
//
struct ZValue {
    ZType type;
    union {
        bool      boolean;
        double    number;
        string    str;
        ZFunction func;
        ZArray    ary;
        ZTable    table;
    };

    ~ZValue() {} // TODO - why?

    ZValue(nullptr_t const&) : type(NIL) {}
    ZValue(double const& d) : type(NUMBER), number(d) {}

    template<class T> inline typename enable_if<is_same<T, nullptr_t>::value, T>::type const& Value() const { 
        ExpectType(NIL);
        return nullptr; 
    }
    template<class T> inline typename enable_if<is_same<T, bool>::value, T>::type const& Value() const { 
        ExpectType(BOOLEAN);
        return boolean; 
    }
    template<class T> inline typename enable_if<is_same<T, double>::value, T>::type const& Value() const { 
        ExpectType(NUMBER);
        return number; 
    }

private:
    void ExpectType(ZType expect) const;
};

#endif

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=cpp
