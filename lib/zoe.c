#include "zoe.h"

#include <stdlib.h>
#include <stdio.h>

#include "bytecode.h"

extern int parse(Bytecode* bc, const char* code);

Zoe* 
zoe_new(void)
{
    Zoe* zoe = calloc(sizeof(Zoe), 1);
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

    // TODO - push into stack

    return ZOE_OK;
}


bool 
zoe_dump(Zoe* zoe, uint8_t** data, size_t* sz)
{
    (void) zoe;
    (void) data;
    (void) sz;

    // TODO

    return true;
}


void 
zoe_call(Zoe* zoe, int args)
{
    (void) zoe;
    (void) args;

    // TODO
}


// vim: ts=4:sw=4:sts=4:expandtab
