#include "lib/bytecode.h"

#include <string.h>

// {{{ CONSTRUCTOR/DESTRUCTOR

typedef struct Bytecode {
    uint8_t*       data;
    size_t         sz;
    UserFunctions* uf;
} Bytecode;

Bytecode* 
bytecode_new(UserFunctions *uf)
{
    Bytecode* bc = uf->realloc(NULL, sizeof(Bytecode));
    memset(bc, 0, sizeof(Bytecode));
    bc->uf = uf;
    return bc;
}


Bytecode* 
bytecode_newfromcode(UserFunctions *uf, const char* code)
{
    Bytecode* bc = bytecode_new(uf);
    // TODO parse(bc, code);
    return bc;
}


void
bytecode_free(Bytecode* bc)
{
    if(bc->data) {
        free(bc->data);
    }
    free(bc);
}

// }}}

// {{{ ADD DATA

void
bytecode_add(Bytecode* bc, uint8_t data)
{
}


void
bytecode_addf64(Bytecode* bc, double data)
{
}

// }}}

// {{{ READ DATA

size_t
bytecode_data(Bytecode* bc, uint8_t** data)
{
}

// }}}

// {{{ DEBUGGING

void
bytecode_disassemble(uint8_t* data, size_t sz)
{
}

// }}}

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=c
