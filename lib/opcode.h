#ifndef ZOE_OPCODE_H_
#define ZOE_OPCODE_H_

typedef enum Opcode {
    // stack
    PUSH_Nil, PUSH_Bt, PUSH_Bf, PUSH_N, PUSH_S, POP,

    // arithmetic
    ADD, SUB, MUL, DIV, IDIV, MOD, POW, NEG,

    // bitwise
    AND, XOR, OR, SHL, SHR, NOT,

    // relational
    LT, LTE, GT, GTE, EQ,

    // string / array
    CAT, LEN, LOOKUP, SLICE,

    // binary
    BNOT,

    // branches
    JMP, Bfalse, Btrue,

    // arrays
    PUSHARY, APPEND,

    // tables
    PUSHTBL, TBLSET,

    // local variables
    ADDCNST, ADDMCNST, GETLOCAL,

    // others
    END,

} Opcode;

#endif

// vim: ts=4:sw=4:sts=4:expandtab
