#include "bytecode.h"

extern int parse(Zoe::Bytecode* bc, const char* code);


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
    parse(&bc, code.c_str());
    return bc;
}


}

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker
