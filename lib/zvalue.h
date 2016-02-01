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

    explicit ZFunction(vector<uint8_t> const& bytecode) : type(BYTECODE), bytecode(bytecode) {}

    // NOTE: this is required for using in a anonymous union
    ~ZFunction() {
        bytecode.~vector<uint8_t>();
    }

    // }}}
};

// 
// ZARRAY
// 
struct ZValue;
typedef vector<shared_ptr<ZValue>> ZArray;

//
// ZTABLE
//
struct ZValueHash {
    size_t operator()(shared_ptr<ZValue> const& k) const;
    bool operator()(shared_ptr<ZValue> const& a, shared_ptr<ZValue> const& b) const;
};

struct ZTable {
    unordered_map<shared_ptr<ZValue>, shared_ptr<ZValue>, ZValueHash, ZValueHash> items = {};
};

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

    explicit ZValue(nullptr_t const&) : type(NIL) {}
    explicit ZValue(bool const& b)    : type(BOOLEAN), boolean(b) {}
    explicit ZValue(double const& d)  : type(NUMBER), number(d) {}
    explicit ZValue(string const& s)  : type(STRING), str(s) {}
    explicit ZValue(vector<uint8_t> const& data) : type(FUNCTION), func(data) {}

    // helper constructors
    template<class T, class = typename enable_if<is_scalar<T>::value, T>::type> explicit ZValue(T const& i) : ZValue(static_cast<double>(i)) {}
    explicit ZValue(const char s[]) : ZValue(string(s)) {}

    // NOTE: see an important observation for this constructor/destructor in `zvalue.cc`
    explicit ZValue(ZType type);
    ~ZValue();

    // }}}

    /* 
     * Methods:
     *   ValueRef  -> returns a reference to the value in C++ type
     *   ValueCopy -> returns a copy of the value in C++ type
     */
    // {{{

#define VALUE(cpp_type, zoe_type, member) \
    template<typename T> typename enable_if<is_same<T, cpp_type>::value, T>::type const& ValueRef() const { \
        ExpectType(zoe_type);                                                                                      \
        return member;                                                                                             \
    }                                                                                                              \
    template<typename T> typename enable_if<is_same<T, cpp_type>::value, T>::type ValueCopy() const {       \
        ExpectType(zoe_type);                                                                                      \
        return member;                                                                                             \
    }
    VALUE(nullptr_t, NIL,     nullptr);
    VALUE(bool,      BOOLEAN, boolean);
    VALUE(double,    NUMBER,  number);
    VALUE(string,    STRING,  str);

    // helper types
    template<typename T> typename enable_if<is_same<T, int>::value, T>::type ValueCopy() const {
        ExpectType(NUMBER);
        return static_cast<int>(number);
    }

    // }}}

    bool     operator==(ZValue const& other) const;
    string   Inspect() const;
    void     ExpectType(ZType expect) const;
    uint64_t Len() const;
};

#endif

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=cpp
