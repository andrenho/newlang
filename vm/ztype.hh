#ifndef VM_ZTYPE_H_
#define VM_ZTYPE_H_

#include <string>
using namespace std;

enum ZType {
    NIL, BOOL, NUMBER, STRING, ARRAY, TABLE, FUNCTION
};

// This template is the basis for converting C++ types to the C++ 
// class implementation of Zoe types. The definition below is the
// base for the implementations found the ZNil, ZBool, ZNumber and
// ZString files. To translate a C++ to a Zoe type, do this:
//
// typename cpp_type<double>::type   -->   ZNumber
//
template <typename T> struct cpp_type { typedef T type; };

string Typename(ZType type);

#endif

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=cpp
