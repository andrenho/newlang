#include "bytecode.h"

#include <inttypes.h>

#include "opcode.h"

extern int parse(Zoe::Bytecode& bc, string const& code);  // defined in lib/parser.y


namespace Zoe {


Bytecode Bytecode::FromCode(string const& code)
{
    Bytecode bc;
    parse(bc, code);  // magic happens
    return bc;
}


void Bytecode::Disassemble(FILE* f, vector<uint8_t> const& data)
{
    uint64_t p = 4;
    int ns;

    auto next = [&](uint64_t sz) {  // {{{
        fprintf(f, "%*s", 28-ns, " ");
        for(uint8_t i=0; i<sz; ++i) {
            fprintf(f, "%02X ", data[p+i]);
        }
        fprintf(f, "\n");
        p += sz;
    }; /// }}}

    fprintf(f, TTY_GRAY);
    while(p < data.size()) {
        fprintf(f, "%08" PRIx64 ":\t", p);
        Opcode op = static_cast<Opcode>(data[p]);  // this is for receiving compiler warnings when something is missing
        switch(op) {
            case PUSH_Nil:
                ns = fprintf(f, "PUSH_Nil") - 1; next(1); break;
            case PUSH_Bt:
                ns = fprintf(f, "PUSH_Bt") - 1; next(1); break;
            case PUSH_Bf:
                ns = fprintf(f, "PUSH_Bf") - 1; next(1); break;
            case PUSH_N:
                ns = fprintf(f, "PUSH_N\t");
                ns += fprintf(f, "%g", ZNumber(data, p+1).Value());
                next(9);
                break;
            case SUM:
                ns = fprintf(f, "SUM") - 1; next(1); break;
            default:
                fprintf(stderr, "Invalid opcode %02X\n", data[p]);
                fprintf(f, TTY_NORMAL);
                return;
        }
    }
    fprintf(f, TTY_NORMAL);
}


}

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker
