#include "zoe.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "bytecode.h"

extern int parse(Bytecode* bc, const char* code);

// {{{ INITIALIZATION

Zoe* 
zoe_new(void)
{
    Zoe* zoe = calloc(sizeof(Zoe), 1);
    zoe->stack = stack_new(20);
    return zoe;
}


void 
zoe_terminate(Zoe** zoe)
{
    stack_free(&(*zoe)->stack);

    free(*zoe);
    *zoe = NULL;
}

// }}}

// {{{  BYTECODE

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
    stack_pushbfunction(zoe->stack, 0, n_data, n_sz);

    free(n_data);

    return ZOE_OK;
}


bool 
zoe_dump(Zoe* zoe, uint8_t** data, size_t* sz)
{
    ZValue* rec = stack_peek(zoe->stack);

    if(rec->type != BFUNCTION) {
        zoe_error(zoe, "zoe: expected function at the top of the stack");
    }

    *sz = rec->bfunction.sz;
    *data = malloc(*sz);
    memcpy(*data, rec->bfunction.bytecode, *sz);

    return true;
}

// }}}

// {{{  ERROR MANAGEMENT

void 
zoe_error(Zoe* zoe, const char* s)
{
    (void) zoe;

    // TODO - this function needs to be improved
    printf("error: %s\n", s);
    abort();
}

// }}}

// {{{  CODE EXECUTION

extern void zoe_exec_bytecode(Zoe* zoe, uint8_t* bc, size_t sz);

void 
zoe_call(Zoe* zoe, int args)
{
    int8_t initial_stack_size = stack_size(zoe->stack);

    ZValue* f = stack_peek(zoe->stack);

    // is it a function?
    if(f->type != BFUNCTION) {
        zoe_error(zoe, "Element being called is not a function.\n");
    }

    // is the number of arguments correct?
    if(f->bfunction.n_args != args) {
        zoe_error(zoe, "Wrong number of arguments.\n");
    }

    // remove function from stack
    uint8_t* bc = malloc(f->bfunction.sz);
    size_t sz = f->bfunction.sz;
    memcpy(bc, f->bfunction.bytecode, f->bfunction.sz);
    stack_remove(zoe->stack, stack_size(zoe->stack) - args - 1);

    // execute
    zoe_exec_bytecode(zoe, bc, sz);
    free(bc);

    // verify if the stack has now is the same size as the beginning
    // (-1 function +1 return argument)
    if(stack_size(zoe->stack) != initial_stack_size) {
        fprintf(stderr, "Function should have returned exaclty one argument.\n");
    }
    
    // remove arguments from stack
    // TODO
}

// }}}

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker
