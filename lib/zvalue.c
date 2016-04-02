#include "lib/zvalue.h"

#include "lib/global.h"

// {{{ MEMORY MANAGEMENT

void zvalue_free_structure(ZValue* value)
{
    switch(value->type) {
        case NIL:     // makes GCC happy
        case NUMBER:
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
        case NUMBER:
            *children = NULL;
            break;
    }
}

// }}}

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=c
