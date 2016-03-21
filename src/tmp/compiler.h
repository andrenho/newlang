#ifndef COMPILER_H_
#define COMPILER_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#define ALLOC_STEP 1024

typedef enum Command {
    PUSH_I = 0x01,
} Command;

bool compile(char* code, uint8_t** buf, size_t* sz);
void cp_add(Command cmd, ...);

#endif

// vim: ts=4:sw=4:sts=4:expandtab
