#ifndef ZOE_BYTECODE_H_
#define ZOE_BYTECODE_H_

// TODO - move these tests somewhere else
#if __BYTE_ORDER__ != __ORDER_LITTLE_ENDIAN__
#error Sorry, Zoe is not yet implemented for big endian machines.
#endif
#if __SIZEOF_DOUBLE__ != 8
#error Sorry, Zoe is not yet implemented for machines where sizeof(double) != 8.
#endif

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "lib/global.h"

#define ZB_VERSION_MINOR 1
#define ZB_VERSION_MAJOR 0

struct B_Priv;

typedef struct Bytecode {
    uint8_t        version_minor;
    uint8_t        version_major;
    uint8_t*       code;
    size_t         code_sz;
    struct B_Priv* _;
} Bytecode;

typedef size_t Label;  // TODO

//
// CONSTRUCTOR/DESTRUCTOR
//
Bytecode* bytecode_new(ERROR errorf);
Bytecode* bytecode_newfromzb(uint8_t* data, size_t sz, ERROR errorf);
Bytecode* bytecode_newfromcode(const char* code, ERROR errorf);   // here is where the magic happens
void      bytecode_free(Bytecode* bc);

//
// ADD CODE
//
void      bytecode_addcode(Bytecode* bc, uint8_t code);
void      bytecode_addcodef64(Bytecode* bc, double code);
void      bytecode_addcodestr(Bytecode* bc, const char* str);

//
// LABELS
//
Label     bytecode_createlabel(Bytecode* bc);
void      bytecode_setlabel(Bytecode* bc, Label lbl);
void      bytecode_addcodelabel(Bytecode* bc, Label lbl);

//
// GENERATE ZB FILE
//
size_t    bytecode_generatezb(Bytecode* bc, uint8_t** data);

#endif

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=c
