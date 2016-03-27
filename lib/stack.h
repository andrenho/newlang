#ifndef ZOE_STACK_H_
#define ZOE_STACK_H_

#include "zvalue.h"

#define STACK_MAX 20

typedef struct Stack Stack;

//
// CONSTRUCTOR/DESTRUCTOR
//
Stack* stack_new(void);
void stack_free(Stack* st);

//
// STACK INFORMATION
//
uint8_t stack_size(Stack* st);

#endif

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=c
