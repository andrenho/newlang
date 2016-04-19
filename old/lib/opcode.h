#ifndef ZOE_OPCODE_H_
#define ZOE_OPCODE_H_

/* This file uses a technique called X-macros to simultaneously define a enum
 * and a string array with the same names, to be able to print the enum 
 * description. See here: <http://www.drdobbs.com/the-new-c-x-macros/184401387> */

#include <cstdint>
#include <string>
#include <vector>
using namespace std;

#define OPCODE_TABLE                                                              \
    /* stack */                                                                   \
    X(PUSH_Nil), X(PUSH_Bt), X(PUSH_Bf), X(PUSH_N), X(PUSH_S), X(POP),            \
                                                                                  \
    /* arithmetic */                                                              \
    X(ADD), X(SUB), X(MUL), X(DIV), X(IDIV), X(MOD), X(POW), X(NEG),              \
                                                                                  \
    /* bitwise */                                                                 \
    X(AND), X(XOR), X(OR), X(SHL), X(SHR), X(NOT),                                \
                                                                                  \
    /* relational */                                                              \
    X(LT), X(LTE), X(GT), X(GTE), X(EQ),                                          \
                                                                                  \
    /* binary */                                                                  \
    X(BNOT),                                                                      \
                                                                                  \
    /* branches */                                                                \
    X(JMP), X(Bfalse), X(Btrue),                                                  \
                                                                                  \
    /* string / array */                                                          \
    X(CAT), X(LEN), X(LOOKUP), X(SETUP), X(SLICE), X(SLICESET),                   \
                                                                                  \
    /* arrays */                                                                  \
    X(PUSHARY), X(APPEND),                                                        \
                                                                                  \
    /* tables */                                                                  \
    X(PUSHTBL), X(TBLSET),                                                        \
                                                                                  \
    /* local variables */                                                         \
    X(ADDCNST), X(ADDMCNST), X(GETLOCAL), X(SETLOCAL), X(ADDVAR), X(ADDMVAR),     \
                                                                                  \
    /* scopes */                                                                  \
    X(PUSH_Sc), X(POP_Sc),                                                        \
                                                                                  \
    /* others */                                                                  \
    X(END)

#define X(a) a
enum Opcode : uint8_t {
    OPCODE_TABLE
};
#undef X

#define X(a) #a
const vector<string> opcode_names = {
    OPCODE_TABLE
};
#undef X

#endif

// vim: ts=4:sw=4:sts=4:expandtab
