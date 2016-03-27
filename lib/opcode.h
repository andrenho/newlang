#ifndef ZOE_OPCODE_H_
#define ZOE_OPCODE_H_

typedef enum Opcode {
    PUSH_Nil = 0x01,
    PUSH_Bt  = 0x02,
    PUSH_Bf  = 0x03,
    PUSH_N   = 0x04,

    ADD      = 0x10,
    SUB      = 0x11,
    MUL      = 0x12,
    DIV      = 0x13,
    IDIV     = 0x14,
    MOD      = 0x15,
    POW      = 0x16,
    NEG      = 0x17,
} Opcode;

#endif

// vim: ts=4:sw=4:sts=4:expandtab
