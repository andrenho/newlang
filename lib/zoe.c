#include "zoe.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "bytecode.h"

extern int parse(Bytecode* bc, const char* code);

Zoe* 
zoe_new(void)
{
    Zoe* zoe = calloc(sizeof(Zoe), 1);
    memset(&zoe->stack, 0, sizeof zoe->stack);
    zoe->stack_sz = 0;
    return zoe;
}


void 
zoe_terminate(Zoe** zoe)
{
    free(*zoe);
    *zoe = NULL;
}


ZoeLoadStatus 
zoe_load_buffer(Zoe* zoe, uint8_t* data, size_t sz)
{
    (void) zoe;

    uint8_t *n_data;
    size_t n_sz;

    if(bc_is_bytecode(data, sz)) {
        n_data = data;
        n_sz = sz;
    } else {
        Bytecode* bc = bc_new();
        parse(bc, (char*)data);
        n_sz = bc_bytecode(bc, &n_data);
        bc_free(&bc);
    }

    // push into stack
    zoe_pushbfunction(zoe, 0, n_data, n_sz);

    return ZOE_OK;
}


bool 
zoe_dump(Zoe* zoe, uint8_t** data, size_t* sz)
{
    if(zoe->stack[zoe->stack_sz-1].type != FUNCTION) {
        zoe_error(zoe, "Expected function at the top of the stack.");
    }

    *sz = zoe->stack[zoe->stack_sz-1].function.sz;
    *data = calloc(*sz, 1);
    memcpy(*data, zoe->stack[zoe->stack_sz-1].function.bytecode, *sz);

    return true;
}


void 
zoe_call(Zoe* zoe, int args)
{
    (void) zoe;
    (void) args;

    if(zoe->stack[zoe->stack_sz-1].type != FUNCTION) {
        zoe_error(zoe, "Expected function at the top of the stack.");
    }

    // TODO
}


void 
zoe_error(Zoe* zoe, const char* s)
{
    (void) zoe;

    // TODO - this function needs to be improved
    printf("error: %s\n", s);
    exit(EXIT_FAILURE);
}


void 
zoe_pushbfunction(Zoe* zoe, int nargs, uint8_t* data, size_t sz)
{
    zoe->stack[zoe->stack_sz].type = FUNCTION;
    zoe->stack[zoe->stack_sz].function.nargs = nargs;
    zoe->stack[zoe->stack_sz].function.bytecode = calloc(sz, 1);
    memcpy(zoe->stack[zoe->stack_sz].function.bytecode, data, sz);
    zoe->stack[zoe->stack_sz].function.sz = sz;
    ++zoe->stack_sz;
}


// vim: ts=4:sw=4:sts=4:expandtab
