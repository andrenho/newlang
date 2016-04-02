#ifndef LIB_ZVALUE_H_
#define LIB_ZVALUE_H_

#include <stdlib.h>

typedef enum ZType { NIL, NUMBER } ZType;

typedef struct ZValue {
    ZType   type;
    ssize_t ref_count;
    union {
        double number;
    };
} ZValue;

// MEMORY MANAGEMENT
void zvalue_free_structure(ZValue* value);

// REFERENCE COUNTING MANAGEMENT
void zvalue_incref(ZValue* value);
void zvalue_decref(ZValue* value);
void zvalue_children(ZValue* value, ZValue** children);

#endif

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=c
