#include "zoe.h"

#include <stdint.h>

#include "opcode.h"
#include "stack.h"

void
zoe_exec_bytecode(Zoe* zoe, uint8_t* bc, size_t sz)
{
    uint64_t p = 4;
    while(p < sz) {
        Opcode op = bc[p];
        switch(op) {
            case PUSH_I:
                stack_pushinteger(zoe->stack, 0);
                p += 9;
                break;
            default:
                zoe_error(zoe, "Invalid opcode.\n");
        }
    }
}

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker
