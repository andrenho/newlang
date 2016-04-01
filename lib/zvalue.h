#ifndef ZOE_ZVALUE_H_
#define ZOE_ZVALUE_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

typedef enum { INVALID, NIL, BOOLEAN, NUMBER, FUNCTION, STRING } ZType;

typedef enum { BYTECODE=1 } ZFunctionType;

typedef struct {
    uint8_t* bytecode;
    size_t   sz;
} ZBytecodeFunction;

typedef struct {
    ZFunctionType type;
    int8_t n_args;
    union {
        ZBytecodeFunction bfunction;
    };
} ZFunction;

struct ZValue;

typedef struct {
    uint64_t n;
    struct ZValue* items;
} ZArray;

typedef struct ZValue {
    ZType type;
    union {
        bool        boolean;
        double      number;
        ZFunction   function;
        char*       string;
        ZArray      array;
    };
} ZValue;

#endif

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=c
