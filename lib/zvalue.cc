#include "lib/zvalue.h"

#include <cmath>
#include <limits>

#include <iostream>  // TODO

// {{{ ZVALUE CONSTRUCTOR/DESTRUCTOR

/* A brief explanation about this constructor and destructor:
 *
 * When initializing a C++ anonymous union, the compiler doesn't know which one
 * of the union members will be the one to be used, so it won't initialize
 * them. Is up to us to do it, so the constructor below initialize the complex
 * types.
 *
 * The same is for the destructor, where the destructor for each one of the
 * objects is called.
 */

ZValue::ZValue(ZType type)
    : type(type)
{
    if(type == ARRAY) {
        new(&ary) ZArray();
    } else if(type == TABLE) {
        new(&ary) ZTable();
    } else {
        throw "Use the other constructors";
    }
}

ZValue::~ZValue() { 
    if(type == FUNCTION) {
        func.~ZFunction();
    } else if(type == STRING) {
        str.~string();
    } else if(type == ARRAY) {
        ary.~ZArray();
    } else if(type == TABLE) {
        table.~ZTable();
    }
}

// }}}

// {{{ ZVALUE METHODS

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
        case ARRAY: {
                string s = "[";
                for(size_t i=0; i<ary.size(); ++i) {
                    if(i != 0) {
                        s.append(", ");
                    }
                    s.append(ary[i]->Inspect());
                }
                s.append("]");
                return s;
            }
        case TABLE: {
                string s = "%{";
                bool fst = true;
                for(auto const& kv: table.items) {
                    if(!fst) {
                        s.append(", ");
                    } else {
                        fst = false;
                    }
                    if(kv.first->type == STRING) {
                        s.append(kv.first->str);
                    } else {
                        s.append("[" + kv.first->Inspect() + "]");
                    }
                    s.append(": " + kv.second->Inspect());
                }
                s.append("}");
                return s;
            }
        case FUNCTION:
            return "[function]";  // TODO
    }
    return "[invalid value]";
}


uint64_t ZValue::Len() const
{
    if(type == STRING) {
        return str.size();
    } else if(type == ARRAY) {
        return ary.size();
    } else {
        throw "Expected 'string' or 'array', found '" + Typename(type) + "'.";
    }
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
        case ARRAY:
            if(ary.size() != other.ary.size()) {
                return false;
            }
            for(size_t i=0; i<ary.size(); ++i) {
                if(!(*ary[i] == *other.ary[i])) {
                    return false;
                }
            }
            return true;
        case TABLE:
            if(table.items.size() != other.table.items.size()) {
                return false;
            }
            for(auto const& kv: table.items) {
                auto it = other.table.items.find(kv.first);
                if(it == other.table.items.end() || !(*it->second == *kv.second)) {
                    return false;
                }
            }
            return true;
        case FUNCTION:
            throw "Functions can't be compared.";  // TODO
    }
    
    return false;
}

// }}}

// {{{ HASH

size_t ZValueHash::operator()(shared_ptr<ZValue> const& k) const 
{
    switch(k->type) {
        case NIL:
            return 0;
        case BOOLEAN:
            return hash<bool>()(k->boolean);
        case NUMBER:
            return hash<double>()(k->number);
        case STRING:
            return hash<string>()(k->str);
        case TABLE:
            // TODO - look for hash function
            throw "No hash function for this value.";
        case FUNCTION:
        case ARRAY:
            throw "No hash function for this value.";
        default:
            abort();
    }
}


bool ZValueHash::operator()(shared_ptr<ZValue> const& a, shared_ptr<ZValue> const& b) const 
{
    return *a == *b;
}

// }}}

// {{{ LOOSE FUNCTIONS (Typename)

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

// }}}

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=cpp
