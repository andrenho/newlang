#ifndef ZOE_ZOE_H_
#define ZOE_ZOE_H_

#include <stdbool.h>
#include <stdint.h>

#include "lib/stack.h"
#include "lib/userfunctions.h"
#include "lib/zvalue.h"

typedef struct Zoe Zoe;

//
// CONSTRUCTOR/DESTRUCTOR
//
Zoe* zoe_createvm(UserFunctions* uf);
void zoe_free(Zoe* Z);

//
// HIGH LEVEL STACK ACCESS
//
STPOS       zoe_stacksize(Zoe* Z);

void        zoe_pushnil(Zoe* Z);
void        zoe_pushboolean(Zoe* Z, bool b);
void        zoe_pushnumber(Zoe* Z, double n);
void        zoe_pushfunction(Zoe* Z, ZFunction f);
void        zoe_pushstring(Zoe* Z, char* s);

void        zoe_pop(Zoe* Z, int count);

ZType       zoe_peektype(Zoe* Z);

void        zoe_peeknil(Zoe* Z);
bool        zoe_peekboolean(Zoe* Z);
double      zoe_peeknumber(Zoe* Z);
ZFunction   zoe_peekfunction(Zoe* Z);
char const* zoe_peekstring(Zoe* Z);

void        zoe_popnil(Zoe* Z);
bool        zoe_popboolean(Zoe* Z);
double      zoe_popnumber(Zoe* Z);
ZFunction   zoe_popfunction(Zoe* Z);
char*       zoe_popstring(Zoe* Z);

//
// ERROR MANAGEMENT
//
void zoe_error(Zoe* Z, char* fmt, ...) __attribute__ ((format (printf, 2, 3)));

//
// INFORMATION
//
char* zoe_typename(ZType type);  // DO NOT FREE the returning string

//
// CODE EXECUTION
//
void zoe_eval(Zoe* Z, const char* code);
void zoe_call(Zoe* Z, int n_args);

//
// DEBUGGING
//
void zoe_disassemble(Zoe* Z);
void zoe_inspect(Zoe* Z);

#endif

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=c
