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
STPOS     zoe_stacksize(Zoe* Z);

void      zoe_pushnil(Zoe* Z);
void      zoe_pushboolean(Zoe* Z, bool b);
void      zoe_pushnumber(Zoe* Z, double n);
void      zoe_pushfunction(Zoe* Z, ZFunction f);

void      zoe_pop(Zoe* Z, int count);

void      zoe_peeknil(Zoe* Z);
bool      zoe_peekboolean(Zoe* Z);
double    zoe_peeknumber(Zoe* Z);
ZFunction zoe_peekfunction(Zoe* Z);

void      zoe_popnil(Zoe* Z);
bool      zoe_popboolean(Zoe* Z);
double    zoe_popnumber(Zoe* Z);
ZFunction zoe_popfunction(Zoe* Z);

//
// ERROR MANAGEMENT
//
void zoe_error(Zoe* Z, char* fmt, ...);

// 
// INFORMATION
// 
char* zoe_typename(ZType type);  // DO NOT FREE the returning string

#endif

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=c
