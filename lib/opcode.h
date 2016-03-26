#ifndef ZOE_OPCODE_H_
#define ZOE_OPCODE_H_

#include "global.h"

typedef enum Opcode : uint8_t {
    PUSH_Nil = 0x01_u8,
    PUSH_Bt  = 0x02_u8,
    PUSH_Bf  = 0x03_u8,
    PUSH_I   = 0x04_u8,
    PUSH_F   = 0x05_u8,
} Opcode;

#endif

// vim: ts=4:sw=4:sts=4:expandtab
