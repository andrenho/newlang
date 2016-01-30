#ifndef LIB_ZVALUE_H_
#define LIB_ZVALUE_H_

#include <cstddef>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
using namespace std;

// 
// TYPES
//
enum ZType { 
    NIL, BOOLEAN, NUMBER, STRING, FUNCTION, ARRAY, TABLE,
};

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
};

//
// INFORMATION
//
string Typename(ZType type);

#endif

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=cpp
