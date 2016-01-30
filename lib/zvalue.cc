#include "lib/zvalue.h"

string Typename(ZType type)
{
    switch(type) {
        case NIL:      return "nil";
        case BOOLEAN:  return "boolean";
        case NUMBER:   return "number";
        case FUNCTION: return "function";
        case STRING:   return "string";
        case ARRAY:    return "array";
        case TABLE:    return "table";
        default:       return "undefined (?)";
    }
}

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=cpp
