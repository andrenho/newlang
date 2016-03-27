#ifndef ZOE_STACK_H_
#define ZOE_STACK_H_

#include "lib/zvalue.h"
#include "lib/userfunctions.h"
#include "lib/global.h"

#define STACK_MAX 20

typedef struct Stack Stack;
typedef int8_t STPOS;

//
// CONSTRUCTOR/DESTRUCTOR
//
Stack* stack_new(UserFunctions* uf);
void stack_free(Stack* st);

//
// STACK INFORMATION
//
uint8_t stack_size(Stack* st);
STPOS  stack_abs(Stack* st, STPOS pos);
ZValue stack_peek(Stack* st, STPOS pos);

//
// STACK CHANGES
//
void stack_push(Stack* st, ZValue zvalue);
void stack_pop(Stack* st);
void stack_popfree(Stack* st);

#endif

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=c
