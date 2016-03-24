#include "stack.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// {{{  INITIALIZATION

ZStack* 
stack_new(uint8_t sz)
{
    ZStack* stack = calloc(sizeof(ZStack), 1);
    stack->records = calloc(sizeof(ZValue), sz);
    stack->sz = 0;
    stack->max_size = sz;
    return stack;
}


void 
stack_free(ZStack** stack)
{
    uint8_t n = (*stack)->sz;
    for(uint8_t i=0; i<n; ++i) {
        stack_pop(*stack);
    }

    free((*stack)->records);
    free(*stack);
    *stack = NULL;
}

// }}}

// {{{ INSERTION

static void stack_prepare_insertion(ZStack* stack)
{
    if(stack->sz >= stack->max_size) {
        fprintf(stderr, "zoe: no room left in stack\n");
        abort();
    }
}


void 
stack_pushbfunction(ZStack* stack, uint8_t n_args, uint8_t* bytecode, size_t sz)
{
    stack_prepare_insertion(stack);

    ZValue* rec = &stack->records[stack->sz];
    rec->type = BFUNCTION;
    rec->bfunction.n_args = n_args;
    rec->bfunction.bytecode = malloc(sz);
    memcpy(rec->bfunction.bytecode, bytecode, sz);
    rec->bfunction.sz = sz;
    
    ++stack->sz;
}


void
stack_pushinteger(ZStack* stack, int64_t integer)
{
    stack_prepare_insertion(stack);

    ZValue* rec = &stack->records[stack->sz];
    rec->type = INTEGER;
    rec->integer = integer;

    ++stack->sz;
}

// }}}

// {{{  PEEK

int8_t
stack_size(ZStack* stack)
{
    return stack->sz;
}


ZValue* 
stack_peek(ZStack* stack)
{
    if(stack->sz == 0) {
        return NULL;
    } else {
        return &stack->records[stack->sz-1];
    }
}

// }}}

// {{{ REMOVAL

static void
stack_free_element(ZValue* rec) 
{
    if(rec->type == BFUNCTION) {
        free(rec->bfunction.bytecode);
    }

    memset(rec, 0, sizeof(ZValue));  // clear memory, just to be sure
}


void 
stack_pop(ZStack* stack)
{
    if(stack->sz == 0) {
        fprintf(stderr, "zoe: can't pop from empty stack\n");
        abort();
    }

    stack_remove(stack, stack->sz-1);
}


void 
stack_remove(ZStack* stack, int8_t pos)
{
    if(pos >= stack->sz) {
        fprintf(stderr, "zoe: removing element above stack size\n");
        abort();
    }

    stack_free_element(&stack->records[pos]);
    for(int8_t i=pos; i < (stack->sz - 2); ++i) {
        memcpy(&stack->records[i], &stack->records[i+1], sizeof(ZValue));
    }

    --stack->sz;
}

// }}}

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker
