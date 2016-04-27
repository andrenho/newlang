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
    X(PNIL), X(PBF), X(PBT), X(PN8), X(PNUM), X(PSTR), X(PARY), X(PTBL), X(PENV),   \
    X(POP),                                                                         \
    /* branches */                                                                  \
    X(JMP), X(BT), X(CALL),                                                         \
    /* scopes */                                                                    \
    X(PSHS), X(POPS),                                                               \
    /* operators */                                                                 \
    X(UNM), X(ADD), X(SUB), X(MUL), X(DIV), X(IDIV), X(MOD), X(POW), X(SHL),        \
    X(SHR), X(BNOT), X(AND), X(OR), X(XOR), X(NOT), X(EQ), X(PART), X(LT), X(LTE),  \
    X(LEN), X(GET), X(SET), X(DEL), X(INSP), X(PTR), X(ISNIL)

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

enum TableConfig : uint8_t { PUB = 0b01, MUT = 0b10 };

#endif

// vim: ts=4:sw=4:sts=4:expandtab:syntax=cpp
