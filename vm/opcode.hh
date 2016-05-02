#ifndef VM_OPCODE_H_
#define VM_OPCODE_H_

/* This file uses a technique called X-macros to simultaneously define a enum
 * and a string array with the same names, to be able to print the enum 
 * description. See here: <http://www.drdobbs.com/the-new-c-x-macros/184401387> */

#include <cstdint>
#include <string>
#include <vector>
using namespace std;

#define OPCODE_TABLE                                                                \
    /* stack management */                                                          \
    X(PNIL, 0), X(PBF, 0), X(PBT, 0), X(PN8, 1), X(PNUM, d), X(PSTR, s), X(PARY, 2),\
    X(PTBL, 2), X(PTBX, 2),                                                         \
    X(POP, 0),                                                                      \
    /* variables */                                                                 \
    X(CVAR, 0), X(CMVAR, 2), X(SVAR, 4), X(GVAR, 4),                                \
    /* branches */                                                                  \
    X(JMP, 4), X(BT, 4), X(CALL, 1),                                                \
    /* scopes */                                                                    \
    X(PSHS, 0), X(POPS, 0),                                                         \
    /* operators */                                                                 \
    X(UNM, 0), X(ADD, 0), X(SUB, 0),  X(MUL, 0),  X(DIV, 0), X(IDIV, 0), X(MOD, 0), \
    X(POW, 0), X(SHL, 0), X(SHR, 0),  X(BNOT, 0), X(AND, 0), X(OR, 0),   X(XOR, 0), \
    X(NOT, 0), X(EQ, 0),  X(PART, 0), X(LT, 0),   X(LTE, 0), X(LEN, 0),  X(GET, p), \
    X(SET, 1), X(DEL, 0), X(INSP, 0), X(PTR, 0),  X(ISNIL, 0)

#define X(a, b) a
enum Opcode : uint8_t {
    OPCODE_TABLE
};
#undef X

#define X(a, b) #a
const vector<string> opcode_names = {
    OPCODE_TABLE
};
#undef X

#define X(a, b) ((#b)[0])
const char opcode_pars[] = {
    OPCODE_TABLE
};
#undef X

enum TableConfig : uint8_t { PUB = 0b01, MUT = 0b10 };

#endif

// vim: ts=4:sw=4:sts=4:expandtab:syntax=cpp
