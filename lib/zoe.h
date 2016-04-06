#ifndef ZOE_ZOE_H_
#define ZOE_ZOE_H_

#include <stdbool.h>
#include <stdint.h>

#include "lib/global.h"
#include "lib/zvalue.h"

typedef struct Zoe Zoe;

typedef int8_t STPOS;

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
void        zoe_pushbfunction(Zoe* Z, uint8_t* data, size_t sz);
void        zoe_pushstring(Zoe* Z, char* s);

void        zoe_pop(Zoe* Z, size_t count);

ZType       zoe_gettype(Zoe* Z, STPOS n);
void        zoe_getnil(Zoe* Z, STPOS i);
bool        zoe_getboolean(Zoe* Z, STPOS i);
double      zoe_getnumber(Zoe* Z, STPOS i);
char const* zoe_getstring(Zoe* Z, STPOS i);    // DO NOT FREE the result

void        zoe_popnil(Zoe* Z);
bool        zoe_popboolean(Zoe* Z);
double      zoe_popnumber(Zoe* Z);
char*       zoe_popstring(Zoe* Z);             // FREE the result

ZType       zoe_peektype(Zoe* Z);
void        zoe_peeknil(Zoe* Z);
bool        zoe_peekboolean(Zoe* Z);
double      zoe_peeknumber(Zoe* Z);
char const* zoe_peekstring(Zoe* Z);

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
STPOS zoe_absindex(Zoe* Z, STPOS pos);

//
// CODE EXECUTION
//
typedef enum {
    ZOE_ADD, ZOE_SUB, ZOE_MUL, ZOE_DIV, ZOE_IDIV, ZOE_MOD, ZOE_POW, ZOE_NEG,    // arithmetic
    ZOE_AND, ZOE_XOR, ZOE_OR, ZOE_SHL, ZOE_SHR, ZOE_NOT,                        // bitwise
    ZOE_LT, ZOE_LTE, ZOE_GT, ZOE_GTE, ZOE_EQ,                                   // relational
    ZOE_CAT,                                                                    // string
} Operator;
void zoe_len(Zoe* Z);
void zoe_lookup(Zoe* Z);
void zoe_oper(Zoe* Z, Operator oper);
void zoe_eval(Zoe* Z, const char* code);
void zoe_call(Zoe* Z, int n_args);

//
// DEBUGGING
//
void zoe_inspect(Zoe* Z, STPOS i);
#ifdef DEBUG
void zoe_disassemble(Zoe* Z);
void zoe_asmdebugger(Zoe* Z, bool value);
void zoe_gcdebugger(Zoe* Z, bool value);
#endif

#endif

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=c
