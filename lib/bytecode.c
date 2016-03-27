#include "lib/bytecode.h"

// {{{ CONSTRUCTOR/DESTRUCTOR

typedef struct Bytecode {
    uint8_t* data;
    size_t   sz;
} Bytecode;

Bytecode* 
bytecode_new()
{
}


Bytecode* 
bytecode_newfromcode()
{
}


void
bytecode_free(Bytecode* bc)
{
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
