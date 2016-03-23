#ifndef ZOE_ZOE_H_
#define ZOE_ZOE_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct Zoe {
} Zoe;

Zoe* zoe_new(void);
void zoe_terminate(Zoe** zoe);

typedef enum { ZOE_OK, ZOE_ERRSYNTAX } ZoeLoadStatus;
ZoeLoadStatus zoe_load_buffer(Zoe* zoe, uint8_t* data, size_t sz);

bool zoe_dump(Zoe* zoe, uint8_t** data, size_t* sz);
void zoe_call(Zoe* zoe, int args);

#endif

// vim: ts=4:sw=4:sts=4:expandtab
