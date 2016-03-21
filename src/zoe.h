#ifndef VM_H_
#define VM_H_

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#define STACK_SIZE 256

typedef struct ZValue {
    enum { NIL, INTEGER } type;
    union {
        int64_t integer;
    };
} ZValue;

typedef struct Zoe {
    ZValue   stack[STACK_SIZE];
    uint16_t stack_top;
} Zoe;

Zoe* zoe_init(void);
void zoe_free(Zoe** zoe);

bool zoe_disassemble(Zoe* zoe, uint8_t* data, size_t sz);
void zoe_run_bytecode(Zoe* zoe, uint8_t* data, size_t sz);

#endif

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker
