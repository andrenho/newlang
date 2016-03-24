#ifndef STACK_H_
#define STACK_H_

#include <stdint.h>
#include <stdlib.h>

typedef enum { NIL, INTEGER, BFUNCTION } ZType;

typedef enum { BYTECODE } FunctionType;

typedef struct {
    uint8_t n_args;
    uint8_t* bytecode;
    size_t sz;
} ZFunctionBytecode;

typedef struct {
    ZType type;
    union {
        int64_t integer;
        ZFunctionBytecode bfunction;
    };
} ZValue;

typedef struct {
    ZValue* records;
    uint8_t sz;
    uint8_t max_size;
} ZStack;

ZStack* stack_new(uint8_t sz);
void    stack_free(ZStack** stack);

void    stack_pushbfunction(ZStack* stack, uint8_t n_args, uint8_t* bytecode, size_t sz);
void    stack_pushinteger(ZStack* stack, int64_t integer);

ZValue* stack_peek(ZStack* stack);

void    stack_pop(ZStack* stack);
void    stack_remove(ZStack* stack, int8_t pos);

int8_t  stack_size(ZStack* stack);

#endif

// vim: ts=4:sw=4:sts=4:expandtab
