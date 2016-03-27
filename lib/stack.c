#include "lib/stack.h"

#include <stdlib.h>

// {{{ CONSTRUCTOR/DESTRUCTOR

typedef struct Stack {
    uint8_t sz;
    ZValue  items[STACK_MAX];
} Stack;


Stack* stack_new(void)
{
    Stack* st = calloc(sizeof(Stack), 1);
    return st;
}

void stack_free(Stack* st)
{
    // TODO - clear stack
    free(st);
}

// }}}

// {{{ STACK INFORMATION

uint8_t stack_size(Stack* st)
{
    return st->sz;
}

// }}}

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=c
