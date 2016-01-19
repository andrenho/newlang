#ifndef ZOE_ZOE_H_
#define ZOE_ZOE_H_

#include <stdbool.h>
#include <stdint.h>

#include "lib/global.h"
#include "lib/stack.h"
#include "lib/zvalue.h"

typedef struct Zoe Zoe;

//
// CONSTRUCTOR/DESTRUCTOR
//
Zoe* zoe_createvm(ERROR errorf);
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

ZType       zoe_gettype(Zoe* Z, int n);

void        zoe_getnil(Zoe* Z, int i);
bool        zoe_getboolean(Zoe* Z, int i);
double      zoe_getnumber(Zoe* Z, int );
ZFunction   zoe_getfunction(Zoe* Z, int i);
char const* zoe_getstring(Zoe* Z, int i);

void        zoe_popnil(Zoe* Z);
bool        zoe_popboolean(Zoe* Z);
double      zoe_popnumber(Zoe* Z);
ZFunction   zoe_popfunction(Zoe* Z);
char*       zoe_popstring(Zoe* Z);

//
// ARRAY MANAGEMENT
// 
void zoe_pusharray(Zoe* Z);
void zoe_arrayappend(Zoe* Z);

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
typedef enum {
    ZOE_ADD, ZOE_SUB, ZOE_MUL, ZOE_DIV, ZOE_IDIV, ZOE_MOD, ZOE_POW, ZOE_NEG,    // arithmetic
    ZOE_AND, ZOE_XOR, ZOE_OR, ZOE_SHL, ZOE_SHR, ZOE_NOT,                        // bitwise
    ZOE_LT, ZOE_LTE, ZOE_GT, ZOE_GTE, ZOE_EQ,                                   // relational
    ZOE_CAT,                                                                    // string
} Operator;
void zoe_oper(Zoe* Z, Operator oper);
void zoe_eval(Zoe* Z, const char* code);
void zoe_call(Zoe* Z, int n_args);

//
// DEBUGGING
//
void zoe_inspect(Zoe* Z, int i);
#ifdef DEBUG
void zoe_disassemble(Zoe* Z);
void zoe_asmdebugger(Zoe* Z, bool value);
#endif

#endif

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=c
