#ifndef ZOE_ZOE_H_
#define ZOE_ZOE_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#define STACKSZ 20

typedef enum { NIL, INTEGER, FUNCTION } ZType;

typedef struct {
    int nargs;
    uint8_t* bytecode;
    size_t sz;
} ZFunction;

typedef struct {
    ZType type;
    union {
        int64_t integer;
        ZFunction function;
    };
} ZValue;

typedef struct Zoe {
    ZValue stack[STACKSZ];
    int stack_sz;
} Zoe;

Zoe* zoe_new(void);
void zoe_terminate(Zoe** zoe);

typedef enum { ZOE_OK, ZOE_ERRSYNTAX } ZoeLoadStatus;
ZoeLoadStatus zoe_load_buffer(Zoe* zoe, uint8_t* data, size_t sz);

bool zoe_dump(Zoe* zoe, uint8_t** data, size_t* sz);
void zoe_call(Zoe* zoe, int args);

void zoe_error(Zoe* zoe, const char* s);

void zoe_pushbfunction(Zoe* zoe, int nargs, uint8_t* data, size_t sz);

#endif

// vim: ts=4:sw=4:sts=4:expandtab
