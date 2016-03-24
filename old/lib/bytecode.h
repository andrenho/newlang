#ifndef BYTECODE_H_
#define BYTECODE_H_

#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "opcode.h"

#define STEPSZ 512

typedef struct Bytecode {
    uint8_t* data;
    size_t sz;
    size_t alloc;
} Bytecode;

Bytecode* bc_new(void);
void bc_free(Bytecode** bc);
void bc_push(Bytecode* bc, Opcode opcode, ...);
size_t bc_bytecode(Bytecode* bc, uint8_t** buffer);

// static
void bc_disassemble(FILE* f, uint8_t* buffer, size_t sz);
bool bc_is_bytecode(uint8_t* buffer, size_t sz);

#endif

// vim: ts=4:sw=4:sts=4:expandtab
