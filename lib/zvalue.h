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

    // {{{ CONSTRUCTORS/DESTRUCTORS

    ZFunction(vector<uint8_t> const& bytecode) : type(BYTECODE), bytecode(bytecode) {}

    // NOTE: this is required for using in a anonymous union
    ~ZFunction() {
        bytecode.~vector<uint8_t>();
    }

    // }}}
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

    // {{{ CONSTRUCTORS/DESTRUCTORS

    ZValue(nullptr_t const&) : type(NIL) {}
    ZValue(bool const& b)    : type(BOOLEAN), boolean(b) {}
    ZValue(double const& d)  : type(NUMBER), number(d) {}
    ZValue(string const& s)  : type(STRING), str(s) {}
    ZValue(vector<uint8_t> const& data) : type(FUNCTION), func(data) {}

    // helper constructors
    ZValue(int const& i)      : ZValue(static_cast<double>(i)) {}
    ZValue(long int const& i) : ZValue(static_cast<double>(i)) {}
    ZValue(const char s[])    : ZValue(string(s)) {}

    // NOTE: this is required for using in a anonymous union
    ~ZValue() { 
        if(type == FUNCTION) {
            func.~ZFunction();
        }
    }

    // }}}

    /* 
     * Methods:
     *   ValueRef  -> returns a reference to the value in C++ type
     *   ValueCopy -> returns a copy of the value in C++ type
     */
    // {{{
#define VALUE(cpp_type, zoe_type, member) \
    template<typename T> inline typename enable_if<is_same<T, cpp_type>::value, T>::type const& ValueRef() const { \
        ExpectType(zoe_type);                                                                                      \
        return member;                                                                                             \
    }                                                                                                              \
    template<typename T> inline typename enable_if<is_same<T, cpp_type>::value, T>::type ValueCopy() const {       \
        ExpectType(zoe_type);                                                                                      \
        return member;                                                                                             \
    }
    VALUE(nullptr_t, NIL,     nullptr);
    VALUE(bool,      BOOLEAN, boolean);
    VALUE(double,    NUMBER,  number);
    VALUE(string,    STRING,  str);

    // helper types
    VALUE(int,       NUMBER,  number);
    // }}}

    bool   operator==(ZValue const& other) const;
    string Inspect() const;
    void   ExpectType(ZType expect) const;
};

#endif

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=cpp
