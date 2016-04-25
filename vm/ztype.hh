#ifndef VM_ZTYPE_H_
#define VM_ZTYPE_H_

#include <string>
using namespace std;

enum ZType {
    NIL, BOOL, NUMBER, STRING, ARRAY, TABLE
};

string Typename(ZType type);

#endif

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=cpp
