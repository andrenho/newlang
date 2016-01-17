#ifndef ZOE_OPCODE_H_
#define ZOE_OPCODE_H_

typedef enum Opcode {
    // push
    PUSH_Nil = 0x01,
    PUSH_Bt  = 0x02,
    PUSH_Bf  = 0x03,
    PUSH_N   = 0x04,

    // arithmetic
    ADD      = 0x10,
    SUB      = 0x11,
    MUL      = 0x12,
    DIV      = 0x13,
    IDIV     = 0x14,
    MOD      = 0x15,
    POW      = 0x16,
    NEG      = 0x17,

    // bitwise
    AND      = 0x18,
    XOR      = 0x19,
    OR       = 0x1F,
    SHL      = 0x20,
    SHR      = 0x21,
    NOT      = 0x22,

    // relational
    LT       = 0x23,
    LTE      = 0x24,
    GT       = 0x25,
    GTE      = 0x26,
} Opcode;

#endif

// vim: ts=4:sw=4:sts=4:expandtab
