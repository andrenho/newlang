#include "lib/zworld.h"

#include <assert.h>
#include <stdlib.h>

// {{{ CONSTRUCTOR/DESTRUCTOR

typedef struct ZValueRef {
    ZValue* ptr;
    struct ZValueRef* next;
} ZValueRef;

typedef struct ZWorld {
    ZValueRef* refs;   
    ZValueRef* last_ref;
    ERROR      errorf;
} ZWorld;


ZWorld*
zworld_new(ERROR errorf)
{
    ZWorld* w = calloc(sizeof(ZWorld), 1);
    assert(errorf);
    w->errorf = errorf;
    return w;
}


void
zworld_free(ZWorld* w)
{
    assert(w);
    free(w);
}

// }}}

// {{{ ALLOCATION/DEALLOCATION

ZValue*
zworld_alloc(ZWorld* w)
{
    // create value
    ZValue* value = calloc(sizeof(ZValue), 1);

    // create reference
    ZValueRef* ref = calloc(sizeof(ZValueRef), 1);
    ref->ptr = value;

    // add reference to world
    if(!w->last_ref) {
        w->refs = ref;
        w->last_ref = ref;
    } else {
        w->last_ref->next = ref;
        w->last_ref = ref;
    }

    return value;
}


void
zworld_release(ZWorld* w, ZValue* value)
{
    // find reference
    // (TODO) - this can be much faster if we keep an ordered list of pointers
    ZValueRef *ref = w->refs,
              *prev = NULL;
    while(ref) {
        if(ref->ptr == value) {
            goto found;
        }
        prev = ref;
        ref = ref->next;
    }
    w->errorf("Fatal error: trying to release a ZValue that was not allocated.");
found:

    // remove reference
    if(!prev) {  // it's the first reference
        w->refs = ref->next;
    } else {
        prev->next = ref->next;
    }

    if(ref == w->last_ref) {
        w->last_ref = prev;
    }
    free(ref);

    // free value structure
    zvalue_free_structure(value);

    // free value
    free(value);    // malloc'd in `zworld_alloc`
}

// }}}

// {{{ MEMORY MANAGEMENT

void zworld_inc(ZWorld* w, ZValue* value)
{
    (void) w;
    zvalue_incref(value);
}


void zworld_dec(ZWorld* w, ZValue* value)
{
    // decrement reference of the children
    if(value->type == ARRAY) {
        for(size_t i=0; i < value->array.n; ++i) {
            zworld_dec(w, value->array.items[i]);
        }
    }

    // decrement reference and possibly collect it
    zvalue_decref(value);
    if(value->ref_count <= 0) {
        zworld_gc(w, value);
    }
}


void zworld_gc(ZWorld* w, ZValue* value)
{
    if(value == NULL) {
        ZValueRef *ref = w->refs;
        while(ref) {
            ZValueRef *next = ref->next;
            if(ref->ptr->ref_count <= 0) {
                zworld_release(w, ref->ptr);
            }
            ref = next;
        }
    } else {
        zworld_release(w, value);
    }
}

// }}}

// {{{ INFORMATION

size_t zworld_ref_count(ZWorld* w)
{
    size_t i = 0;

    ZValueRef *ref = w->refs;
    while(ref) {
        ref = ref->next;
        ++i;
    }

    return i;
}

// }}}

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=c
