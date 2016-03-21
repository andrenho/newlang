#ifndef VM_H_
#define VM_H_

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

typedef struct VM {
    int TODO; // TODO
} VM;

VM* vm_init(void);
void vm_free(VM** vm);

bool vm_disassemble(VM* vm, uint8_t* data, size_t sz);
void vm_run(VM* vm, uint8_t* data, size_t sz);

#endif

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker
