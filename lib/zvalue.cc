#include "lib/zvalue.h"

#include <cmath>
#include <limits>

void ZValue::ExpectType(ZType expect) const
{
    if(type != expect) {
        throw "Expected '" + Typename(expect) + "', found '" + Typename(type) + "'.";
    }
}


string ZValue::Inspect() const
{
    switch(type) {
        case NIL:
            return "nil";
        case BOOLEAN:
            return boolean ? "true" : "false";
        case NUMBER: {
                string s = to_string(number);
                s.erase(s.find_last_not_of("0")+1);
                s.erase(s.find_last_not_of(".")+1);
                return s;
            }
        case STRING:
            return "'" + str + "'";
    }
    return "[invalid value]";
}


bool ZValue::operator==(ZValue const& other) const
{
    if(type != other.type) {
        return false;
    }

    switch(type) {
        case NIL:
            return true;
        case BOOLEAN:
            return boolean == other.boolean;
        case NUMBER:
            return fabs(number - other.number) < numeric_limits<double>::epsilon();
        case STRING:
            return str == other.str;
    }
    
    return false;
}


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
