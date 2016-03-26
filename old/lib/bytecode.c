#include "bytecode.h"

#include <inttypes.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

Bytecode* 
bc_new(void)
{
    Bytecode* bc = calloc(sizeof(Bytecode), 1);
    bc->data = NULL;
    bc->sz = 0;
    bc->alloc = 0;
    return bc;
}


void 
bc_free(Bytecode** bc)
{
    if((*bc)->data) {
        free((*bc)->data);
    }
    free(*bc);
    *bc = NULL;
}


inline static void
bc_push_byte(Bytecode* bc, uint8_t value) 
{
    if(bc->sz >= bc->alloc) {
        bc->data = realloc(bc->data, bc->alloc + STEPSZ);
        bc->alloc += STEPSZ;
    }
    bc->data[bc->sz++] = value;
}


static void
bc_push_integer(Bytecode* bc, int64_t value) 
{
    bc_push_byte(bc, (uint8_t)value);
    bc_push_byte(bc, (uint8_t)(value >> 8));
    bc_push_byte(bc, (uint8_t)(value >> 16));
    bc_push_byte(bc, (uint8_t)(value >> 24));
    bc_push_byte(bc, (uint8_t)(value >> 32));
    bc_push_byte(bc, (uint8_t)(value >> 40));
    bc_push_byte(bc, (uint8_t)(value >> 48));
    bc_push_byte(bc, (uint8_t)(value >> 56));
}


void 
bc_push(Bytecode* bc, Opcode opcode, ...)
{
    va_list(ap);
    va_start(ap, opcode);

    bc_push_byte(bc, opcode);

    switch(opcode) {
        case PUSH_I: 
            bc_push_integer(bc, va_arg(ap, int64_t));
            break;
    }

    va_end(ap);
}


size_t 
bc_bytecode(Bytecode* bc, uint8_t** buffer)
{
    size_t sz = bc->sz + 4;
    *buffer = calloc(sz, 1);
    (*buffer)[0] = 0xB4;
    (*buffer)[1] = 0x7E;
    (*buffer)[2] = 0xC0;
    (*buffer)[3] = 0xDE;
    memcpy(&(*buffer)[4], bc->data, bc->sz);
    return sz;
}


//
// STATIC
//

void 
bc_disassemble(FILE* f, uint8_t* buffer, size_t sz)
{
#define NXT(sz) {                           \
    for(int8_t i=0; i<sz; ++i) {            \
        fprintf(f, "%02X ", buffer[p+i]); \
    }                                       \
    fprintf(f, "\n");                       \
    p += sz;                                \
}

#define INTEGER(n) \
    ((int64_t)(buffer[n]) | \
     ((int64_t)(buffer[n+1]) << 8)  | \
     ((int64_t)(buffer[n+2]) << 16) | \
     ((int64_t)(buffer[n+3]) << 24) | \
     ((int64_t)(buffer[n+4]) << 32) | \
     ((int64_t)(buffer[n+5]) << 40) | \
     ((int64_t)(buffer[n+6]) << 48) | \
     ((int64_t)(buffer[n+7]) << 56))

    uint64_t p = 4;
    uint8_t ns;
    while(p < sz) {
        fprintf(f, "%08" PRIx64 ":\t", p);
        switch(buffer[p]) {
            case PUSH_I:
                fprintf(f, "push_i\t");
                ns = fprintf(f, "%" PRId64, INTEGER(p+1));
                fprintf(f, "%*s", 23-ns, " ");
                NXT(9);
                break;
            default:
                fprintf(f, "Invalid opcode %02X\n", buffer[p]);
                return;
        }
    }

#undef INTEGER
#undef NXT
}


bool 
bc_is_bytecode(uint8_t* buffer, size_t sz)
{
    (void) buffer;
    (void) sz;

    if(sz < 4) {
        return false;
    }

    return buffer[0] == 0xB4 && buffer[1] == 0x7E && buffer[2] == 0xC0 && buffer[3] == 0xDE;
}


// vim: ts=4:sw=4:sts=4:expandtab
