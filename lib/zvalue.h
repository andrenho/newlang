#ifndef LIB_ZVALUE_H_
#define LIB_ZVALUE_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

typedef enum ZType { NIL, BOOLEAN, NUMBER, STRING, FUNCTION, ARRAY } ZType;

typedef enum ZFunctionType { BYTECODE } ZFunctionType;

typedef struct ZFunctionBytecode {
    uint8_t* data;
    size_t   sz;
} ZFunctionBytecode;

typedef struct ZFunction {
    ZFunctionType type;
    union {
        ZFunctionBytecode bytecode;
    };
} ZFunction;

typedef struct ZValue {
    ZType   type;
    ssize_t ref_count;
    union {
        bool      boolean;
        double    number;
        char*     string;
        ZFunction function;
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
