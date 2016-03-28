#include "lib/bytecode.h"

#include <inttypes.h>
#include <string.h>

#include "opcode.h"

extern int parse(Bytecode* b, const char* code);

// {{{ CONSTRUCTOR/DESTRUCTOR

typedef struct Bytecode {
    uint8_t*       data;
    size_t         sz;
    size_t         allocd_sz;
    UserFunctions* uf;
} Bytecode;

Bytecode* 
bytecode_new(UserFunctions *uf)
{
    Bytecode* bc = uf->realloc(NULL, sizeof(Bytecode));
    memset(bc, 0, sizeof(Bytecode));
    bc->uf = uf;
    return bc;
}


Bytecode* 
bytecode_newfromcode(UserFunctions *uf, const char* code)
{
    Bytecode* bc = bytecode_new(uf);
    parse(bc, code);
    return bc;
}


void
bytecode_free(Bytecode* bc)
{
    if(bc->data) {
        free(bc->data);
    }
    free(bc);
}

// }}}

// {{{ ADD DATA

void
bytecode_add(Bytecode* bc, uint8_t data)
{
    if(bc->sz >= bc->allocd_sz) {
        bc->allocd_sz = (bc->allocd_sz+1) * 2;
        bc->data = bc->uf->realloc(bc->data, bc->allocd_sz);
    }
    bc->data[bc->sz++] = data;
}


void
bytecode_addf64(Bytecode* bc, double data)
{
    union {
        double a;
        uint8_t b[8];
    } c = { .a = data };
    
    for(int i=0; i<8; ++i) {
        bytecode_add(bc, c.b[i]);
    }
}

// }}}

// {{{ READ DATA

size_t
bytecode_data(Bytecode* bc, uint8_t** data)
{
    if(data) {
        *data = bc->data;
    }
    return bc->sz;
}

size_t
bytecode_copy_data(Bytecode* bc, uint8_t** data)
{
    *data = bc->uf->realloc(NULL, bc->sz);
    memcpy(*data, bc->data, bc->sz);
    return bc->sz;
}

// }}}

// {{{ DEBUGGING

void
bytecode_disassemble(FILE* f, uint8_t* data, size_t sz)
{
    uint64_t p = 4;
    int ns;

#define next(sz) {                          \
    fprintf(f, "%*s", 28-ns, " ");          \
    for(uint8_t i=0; i<sz; ++i) {           \
        fprintf(f, "%02X ", data[p+i]);     \
    }                                       \
    fprintf(f, "\n");                       \
    p += sz;                                \
}

    fprintf(f, "\033[1;30m");  // gray font
    while(p < sz) {
        fprintf(f, "%08" PRIx64 ":\t", p);
        switch((Opcode)data[p]) {
            case PUSH_Nil:
                ns = fprintf(f, "PUSH_Nil") - 1; next(1); break;
            case PUSH_Bt:
                ns = fprintf(f, "PUSH_Bt") - 1; next(1); break;
            case PUSH_Bf:
                ns = fprintf(f, "PUSH_Bf") - 1; next(1); break;
            case PUSH_N: {
                    ns = fprintf(f, "PUSH_N\t");
                    /* TODO
                    double _value;
                    int64_t m = (int64_t)p;
                    copy(begin(data)+m, begin(data)+m+8, reinterpret_cast<uint8_t*>(&_value));
                    ns += fprintf(f, "%g", _value);
                    next(9);
                    */
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
                fprintf(f, "\033[0m");  // TTY normal
                return;
        }
    }
    fprintf(f, "\033[0m");  // TTY normal
}

// }}}

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=c
