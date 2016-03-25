#include "bytecode.h"

namespace Zoe {

void
Bytecode::Add(int32_t n)
{
    for(int i=0; i<4; ++i) {
        Add(static_cast<uint8_t>((n >> (i*8)) & 0xFF));
    }
}

void
Bytecode::Add(int64_t n)
{
    for(int i=0; i<8; ++i) {
        Add(static_cast<uint8_t>((n >> (i*8)) & 0xFF));
    }
}

}

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker
