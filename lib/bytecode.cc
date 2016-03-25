#include "bytecode.h"

#include <inttypes.h>

#include "opcode.h"

extern int parse(Zoe::Bytecode& bc, string const& code);  // defined in lib/parser.y


namespace Zoe {

void Bytecode::Add_i64(int64_t n)
{
    for(int i=0; i<8; ++i) {
        Add_u8(static_cast<uint8_t>((n >> (i*8)) & 0xFF));
    }
}


Bytecode Bytecode::FromCode(string const& code)
{
    Bytecode bc;
    bc.Add_u8(0xB4);
    bc.Add_u8(0x7E);
    bc.Add_u8(0xC0);
    bc.Add_u8(0xDE);
    parse(bc, code);  // magic happens
    return bc;
}


void Bytecode::Disassemble(FILE* f, vector<uint8_t> const& data)
{
    uint64_t p = 4;

    auto next = [&](uint64_t sz) {
        for(uint8_t i=0; i<sz; ++i) {
            fprintf(f, "%02X ", data[p+i]);
        }
        fprintf(f, "\n");
        p += sz;
    };

    auto integer = [&](int64_t n) {
        return static_cast<int64_t>(data[n])         | \
               static_cast<int64_t>(data[n+1]) << 8  | \
               static_cast<int64_t>(data[n+1]) << 16 | \
               static_cast<int64_t>(data[n+1]) << 24 | \
               static_cast<int64_t>(data[n+1]) << 32 | \
               static_cast<int64_t>(data[n+1]) << 40 | \
               static_cast<int64_t>(data[n+1]) << 48 | \
               static_cast<int64_t>(data[n+1]) << 56;
    };

    uint8_t ns;
    while(p < data.size()) {
        fprintf(f, "%08" PRIx64 ":\t", p);
        switch(data[p]) {
            case PUSH_I:
                fprintf(f, "push_i\t");
                ns = fprintf(f, "%" PRId64, integer(p+1));
                fprintf(f, "%*s", 23-ns, " ");
                next(9);
                break;
            default:
                fprintf(stderr, "Invalid opcode %02X\n", data[p]);
                return;
        }
    }
}


}

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker
