#include "vm/ztype.hh"

string Typename(ZType type) {
    switch(type) {
        case NIL:    return "nil";
        case BOOL:   return "bool";
        case NUMBER: return "number";
        case STRING: return "string";
        case ARRAY:  return "array";
        case TABLE:  return "table";
        default:     return "(undefined = " + to_string(type) + ")";
    }
}

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=cpp
