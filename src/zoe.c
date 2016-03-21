#include "zoe.h"

#include "compiler.h"

#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>

// {{{  INITIALIZATION/DESTRUCTION

Zoe* 
zoe_init(void)
{
    Zoe* zoe = calloc(sizeof(VM), 1);

    // initialize stack
    for(uint16_t s=0; s < STACK_SIZE; ++s) {
        zoe->stack[s] = (ZValue) { .type=NIL };
    }
    zoe->stack_top = 0;

    return zoe;
}


void 
zoe_free(VM** zoe)
{
    free(*zoe);
    *zoe = NULL;
}

// }}}

// {{{  LOAD DATA

static int64_t get_integer(uint8_t* data, size_t p) {
    int64_t v = data[p];
    v |= (int64_t)data[p+1] << 8;
    v |= (int64_t)data[p+2] << 16;
    v |= (int64_t)data[p+3] << 24;
    v |= (int64_t)data[p+4] << 32;
    v |= (int64_t)data[p+5] << 40;
    v |= (int64_t)data[p+6] << 48;
    v |= (int64_t)data[p+7] << 56;
    return v;
}

// }}}

// {{{  EXECUTION

void 
zoe_run_bytecode(Zoe* zoe, uint8_t* data, size_t sz)
{
    (void) vm;
    (void) data;
    (void) sz;
}

// }}}


// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker
