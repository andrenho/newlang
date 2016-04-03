#include "lib/zvalue.h"

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
        case FUNCTION:
            switch(value->function.type) {
                case BYTECODE:
                    free(value->function.bytecode.data);
                    break;
            }
            break;
    }
}

// }}}

// {{{ REFERENCE COUNTING MANAGEMENT

void zvalue_incref(ZValue* value)
{
    ++value->ref_count;
}


void zvalue_decref(ZValue* value)
{
    --value->ref_count;
}


void zvalue_children(ZValue* value, ZValue** children)
{
    switch(value->type) {
        case NIL:
        case BOOLEAN:
        case NUMBER:
        case STRING:
        case FUNCTION:
            *children = NULL;
            break;
    }
}

// }}}

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=c
