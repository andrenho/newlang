#include "vm.h"

#include <stdlib.h>

VM* 
vm_init()
{
    VM* vm = calloc(sizeof(VM), 1);
    return vm;
}


void 
vm_free(VM** vm)
{
    free(*vm);
    *vm = NULL;
}


bool 
vm_disassemble(VM* vm, uint8_t* data, size_t sz)
{
}


void 
vm_run(VM* vm, uint8_t* data, size_t sz)
{
}


// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker
