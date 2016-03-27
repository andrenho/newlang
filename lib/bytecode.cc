#include "bytecode.h"

#include <inttypes.h>

#include "opcode.h"

extern int parse(Zoe::Bytecode& bc, string const& code);  // defined in lib/parser.y


namespace Zoe {


void Bytecode::Add_f64(double value)
{
    uint8_t const* ptr = reinterpret_cast<uint8_t const*>(&value);
    for(int i=0; i<8; ++i) {
        _data.push_back(ptr[i]);
    }
}


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
            case PUSH_N: {
                    ns = fprintf(f, "PUSH_N\t");
                    double _value;
                    int64_t m = static_cast<int64_t>(p);
                    copy(begin(data)+m, begin(data)+m+8, reinterpret_cast<uint8_t*>(&_value));
                    ns += fprintf(f, "%g", _value);
                    next(9);
                }
                break;
            case ADD:
                ns = fprintf(f, "ADD") - 1; next(1); break;
            case SUB:
                ns = fprintf(f, "SUB") - 1; next(1); break;
            case MUL:
                ns = fprintf(f, "MUL") - 1; next(1); break;
            case DIV:
                ns = fprintf(f, "DIV") - 1; next(1); break;
            case IDIV:
                ns = fprintf(f, "IDIV") - 1; next(1); break;
            case MOD:
                ns = fprintf(f, "MOD") - 1; next(1); break;
            case POW:
                ns = fprintf(f, "POW") - 1; next(1); break;
            case NEG:
                ns = fprintf(f, "NEG") - 1; next(1); break;
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
