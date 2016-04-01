#include "lib/stack.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

// {{{ CONSTRUCTOR/DESTRUCTOR

typedef struct Stack {
    int8_t sz;
    ZValue items[STACK_MAX];
    ERROR  errorf;
} Stack;


Stack* stack_new(ERROR errorf)
{
    Stack* st = calloc(sizeof(Stack), 1);
    st->errorf = errorf;
    return st;
}

void stack_free(Stack* st)
{
    while(st->sz) {
        stack_popfree(st);
    }
    free(st);
}

// }}}

// {{{ STACK INFORMATION

STPOS
stack_abs(Stack* st, STPOS pos)
{
    return (pos >= 0) ? pos : (st->sz + pos);
}


ZValue
stack_peek(Stack* st, STPOS pos)
{
    if(pos >= st->sz) {
        st->errorf("__func__: accessing data out of stack\n");
        return (ZValue){0};
    }

    return st->items[stack_abs(st, pos)];
}


ZValue* 
stack_peek_ptr(Stack* st, STPOS pos)
{
    if(pos >= st->sz) {
        st->errorf("__func__: accessing data out of stack\n");
        return NULL;
    }

    return &st->items[stack_abs(st, pos)];
}

uint8_t
stack_size(Stack* st)
{
    return st->sz;
}

// }}}

// {{{ STACK CHANGES

void
stack_push(Stack* st, ZValue zvalue)
{
    if(st->sz == (STACK_MAX-1)) {
        st->errorf("stack_push: stack overflow.");
        return;
    }

    memmove(&st->items[st->sz], &zvalue, sizeof(ZValue));
    ++st->sz;
    assert(st->sz < STACK_MAX);
}


void
stack_pop(Stack* st)
{
    if(st->sz == 0) {
        st->errorf("stack_pop: stack undeflow.");
        return;
    }

    --st->sz;
    assert(st->sz >= 0);
}


void stack_popfree(Stack* st)
{
    if(st->sz == 0) {
        st->errorf("stack_popfree: stack undeflow.");
        return;
    }

    STPOS p = stack_abs(st, -1);
    zvalue_free_data(st->items[p], st->errorf);

    --st->sz;
    assert(st->sz >= 0);
}


// }}}

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=c
