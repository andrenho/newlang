#ifndef ZOE_OPCODE_H_
#define ZOE_OPCODE_H_

typedef enum Opcode {
    // stack
    PUSH_Nil = 0x01,
    PUSH_Bt  = 0x02,
    PUSH_Bf  = 0x03,
    PUSH_N   = 0x04,
    PUSH_S   = 0x05,
    POP      = 0x0F,

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
    EQ       = 0x27,

    // string / array
    CAT      = 0x28,
    LEN      = 0x29,
    LOOKUP   = 0x2A,
    SLICE    = 0x2B,

    // binary
    BNOT     = 0x2F,

    // branches
    JMP      = 0x30,
    Bfalse   = 0x31,
    Btrue    = 0x32,

    // arrays
    PUSHARY  = 0x40,
    APPEND   = 0x41,

    // others
    END      = 0xFF,

} Opcode;

#endif

// vim: ts=4:sw=4:sts=4:expandtab
