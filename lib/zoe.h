#ifndef ZOE_ZOE_H_
#define ZOE_ZOE_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "stack.h"

typedef struct Zoe {
    ZStack* stack;
} Zoe;

typedef enum { ZOE_OK, ZOE_ERRSYNTAX } ZoeLoadStatus;

Zoe*            zoe_new(void);
void            zoe_terminate(Zoe** zoe);

ZoeLoadStatus   zoe_load_buffer(Zoe* zoe, uint8_t* data, size_t sz);
bool            zoe_dump(Zoe* zoe, uint8_t** data, size_t* sz);

void            zoe_error(Zoe* zoe, const char* s);

void            zoe_call(Zoe* zoe, int args);  // in call.c

#endif

// vim: ts=4:sw=4:sts=4:expandtab
