#include "lib/zoe.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

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
    memset(Z, 0, sizeof(Zoe));
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


static ZValue
zoe_checktype(Zoe* Z, ZType type_expected)
{
    ZValue value = stack_peek(Z->stack, -1);
    if(value.type != type_expected) {
        zoe_error(Z, "Expected %s, found %s\n", zoe_typename(type_expected), zoe_typename(value.type));
    }
    return value;
}


void      
zoe_peeknil(Zoe* Z)
{
    zoe_checktype(Z, NIL);
}


bool      
zoe_peekboolean(Zoe* Z)
{
    return zoe_checktype(Z, BOOLEAN).boolean;
}


double    
zoe_peeknumber(Zoe* Z)
{
    return zoe_checktype(Z, NUMBER).number;
}


ZFunction 
zoe_peekfunction(Zoe* Z)
{
    return zoe_checktype(Z, FUNCTION).function;
}


void      
zoe_popnil(Zoe* Z)
{
    zoe_checktype(Z, NIL);
    stack_pop(Z->stack);
}


bool
zoe_popboolean(Zoe* Z)
{
    bool b = zoe_checktype(Z, BOOLEAN).boolean;
    stack_pop(Z->stack);
    return b;
}


double
zoe_popnumber(Zoe* Z)
{
    double n = zoe_checktype(Z, NUMBER).number;
    stack_pop(Z->stack);
    return n;
}


ZFunction
zoe_popfunction(Zoe* Z)
{
    ZFunction f = zoe_checktype(Z, FUNCTION).function;
    stack_pop(Z->stack);
    return f;
}

// }}}

// {{{ ERROR MANAGEMENT

void zoe_error(Zoe* Z, char* fmt, ...)
{
    static char buf[1024];
    va_list a;

    va_start(a, fmt);
    vsnprintf(buf, sizeof buf, fmt, a);
    va_end(a);

    Z->uf->error(buf);
}

// }}}

// {{{ INFORMATION

char* zoe_typename(ZType type)
{
    switch(type) {
        case INVALID:  return "invalid";
        case NIL:      return "nil";
        case BOOLEAN:  return "boolean";
        case NUMBER:   return "number";
        case FUNCTION: return "function";
        default:       return "undefined (?)";
    }
}

// }}}

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=c
