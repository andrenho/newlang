#ifndef ZOE_BYTECODE_H_
#define ZOE_BYTECODE_H_

#include <stdint.h>
#include <stdlib.h>

#include "userfunctions.h"

typedef struct Bytecode Bytecode;

// 
// CONSTRUCTOR/DESTRUCTOR
//
Bytecode* bytecode_new(UserFunctions *uf);
Bytecode* bytecode_newfromcode(UserFunctions *uf, const char* code);
void      bytecode_free(Bytecode* bc);

//
// ADD DATA
//
void      bytecode_add(Bytecode* bc, uint8_t data);
void      bytecode_addf64(Bytecode* bc, double data);

// 
// READ DATA
//
size_t    bytecode_data(Bytecode* bc, uint8_t** data);

// 
// DEBUGGING
//
void      bytecode_disassemble(uint8_t* data, size_t sz);

#endif

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=c
