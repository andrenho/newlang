#include "lib/zvalue.h"

#include "lib/hash.h"
#include "lib/global.h"

// {{{ MEMORY MANAGEMENT

void zvalue_free_structure(ZValue* value)
{
    switch(value->type) {
        case NIL:     // makes GCC happy
        case BOOLEAN:
        case NUMBER:
            break;
        case STRING:
            free(value->string);
            break;
        case ARRAY:
            if(value->array.items) {
                free(value->array.items);
            }
            break;
        case FUNCTION:
            switch(value->function.type) {
                case BYTECODE:
                    free(value->function.bytecode.data);
                    break;
            }
            break;
        case TABLE:
            hash_free(value->table);
            break;
    }
}

// }}}

// {{{ REFERENCE COUNTING MANAGEMENT

inline void 
zvalue_incref(ZValue* value)
{
    ++value->ref_count;
}


inline void
zvalue_decref(ZValue* value)
{
    --value->ref_count;
}

// }}}

// {{{ INFORMATION

char* zvalue_typename(ZType type)
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

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=c
