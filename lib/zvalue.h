#ifndef LIB_ZVALUE_H_
#define LIB_ZVALUE_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

// TYPES
typedef enum ZType { NIL, BOOLEAN, NUMBER, STRING, FUNCTION, ARRAY } ZType;

// TYPES SPECIFICATION
typedef struct ZArray {
    uint64_t n;
    struct ZValue** items;
} ZArray;

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

// VALUES
typedef struct ZValue {
    ZType   type;
    ssize_t ref_count;
    union {
        bool      boolean;
        double    number;
        char*     string;
        ZFunction function;
        ZArray    array;
    };
} ZValue;

// MEMORY MANAGEMENT
void zvalue_free_structure(ZValue* value);

// REFERENCE COUNTING MANAGEMENT
void zvalue_incref(ZValue* value);
void zvalue_decref(ZValue* value);

// INFORMATION
char* zvalue_typename(ZType type);  // DO NOT FREE the returning string

#endif

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=c
