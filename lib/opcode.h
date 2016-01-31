#ifndef ZOE_OPCODE_H_
#define ZOE_OPCODE_H_

#include <cstdint>

enum Opcode : uint8_t {
    // stack
    PUSH_Nil=0x0, PUSH_Bt, PUSH_Bf, PUSH_N, PUSH_S, POP,

    // arithmetic
    ADD=0x10, SUB, MUL, DIV, IDIV, MOD, POW, NEG,

    // bitwise
    AND=0x20, XOR, OR, SHL, SHR, NOT,

    // relational
    LT=0x30, LTE, GT, GTE, EQ,

    // string / array
    CAT=0x40, LEN, LOOKUP, SLICE,

    // binary
    BNOT=0x50,

    // branches
    JMP=0x60, Bfalse, Btrue,

    // arrays
    PUSHARY=0x70, APPEND,

    // tables
    PUSHTBL=0x80, TBLSET,

    // local variables
    ADDCNST=0x90, ADDMCNST, GETLOCAL,

    // scopes
    PUSH_Sc=0xA0, POP_Sc,

    // others
    END=0xFF,

};

#endif

// vim: ts=4:sw=4:sts=4:expandtab
