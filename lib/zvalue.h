#ifndef ZOE_ZVALUE_H_
#define ZOE_ZVALUE_H_

#include <stdbool.h>
#include <stdint.h>

typedef enum { INVALID, NIL, BOOLEAN, NUMBER, FUNCTION } ZType;

typedef enum { BYTECODE=1 } ZFunctionType;

typedef struct {
    ZFunctionType type;
    int8_t n_args;
    union {
        uint8_t* bytecode;
    };
} ZFunction;

typedef struct {
    ZType type;
    union {
        bool        boolean;
        double      number;
        ZFunction   function;
    };
} ZValue;

#endif

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=c
