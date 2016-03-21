#ifndef COMPILER_H_
#define COMPILER_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

bool cp_compile(const char* code, uint8_t** buf, size_t* sz);

#endif

// vim: ts=4:sw=4:sts=4:expandtab
