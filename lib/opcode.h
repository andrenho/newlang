#ifndef ZOE_OPCODE_H_
#define ZOE_OPCODE_H_

#include "global.h"

typedef enum Opcode : uint8_t {
    PUSH_Nil = 0x01,
    PUSH_Bt  = 0x02,
    PUSH_Bf  = 0x03,
    PUSH_N   = 0x04,

    SUM      = 0x10,
} Opcode;

#endif

// vim: ts=4:sw=4:sts=4:expandtab
