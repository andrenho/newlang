#include "lib/zoe.h"

// {{{ CONSTRUCTOR/DESTRUCTOR

typedef struct Zoe {
    Stack*         stack;
    UserFunctions* uf;
} Zoe;

Zoe* 
zoe_createvm(UserFunctions* uf)
{
    if(!uf) {
        uf = &default_userfunctions;
    }
    Zoe* Z = uf->realloc(NULL, sizeof(Zoe));
    Z->stack = stack_new(uf);
    Z->uf = uf;
    return Z;
}


void 
zoe_free(Zoe* Z)
{
    stack_free(Z->stack);
    Z->uf->free(Z);
}

// }}}


// {{{ HIGH LEVEL STACK ACCESS

STPOS 
zoe_stacksize(Zoe* Z)
{
    return stack_size(Z->stack);
}


void 
zoe_pushnil(Zoe* Z)
{
    stack_push(Z->stack, (ZValue){ .type=NIL });
}


void 
zoe_pushboolean(Zoe* Z, bool b)
{
    stack_push(Z->stack, (ZValue){ .type=BOOLEAN, .boolean=b });
}


void 
zoe_pushnumber(Zoe* Z, double n)
{
    stack_push(Z->stack, (ZValue){ .type=NUMBER, .number=n });
}


void 
zoe_pushfunction(Zoe* Z, ZFunction f)
{
    stack_push(Z->stack, (ZValue){ .type=FUNCTION, .function=f });
}


void 
zoe_pop(Zoe* Z, int count)
{
    for(int i=0; i<count; ++i) {
        stack_popfree(Z->stack);
    }
}


void      
zoe_peeknil(Zoe* Z)
{
}


bool      
zoe_peekboolean(Zoe* Z)
{
}


double    
zoe_peeknumber(Zoe* Z)
{
}


ZFunction 
zoe_peekfunction(Zoe* Z)
{
}


void      
zoe_popnil(Zoe* Z)
{
}


bool
zoe_popboolean(Zoe* Z)
{
}


double
zoe_popnumber(Zoe* Z)
{
}


ZFunction
zoe_popfunction(Zoe* Z)
{
}

// }}}

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=c
