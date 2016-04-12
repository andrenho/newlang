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


void 
ZValue::ExpectType(ZType expect) const
{
    if(type != expect) {
        throw "Expected '" + Typename(expect) + "', found '" + Typename(type) + "'.";
    }
}


// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=cpp
