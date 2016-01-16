#include "lib/bytecode.h"

#include <inttypes.h>
#include <string.h>

#include "opcode.h"

// {{{ CONSTRUCTOR/DESTRUCTOR

struct B_Priv {
    UserFunctions *uf;
    size_t code_alloc;
};

Bytecode* 
bytecode_new(UserFunctions *uf)
{
    Bytecode* bc = uf->realloc(NULL, sizeof(Bytecode));
    memset(bc, 0, sizeof(Bytecode));
    bc->_ = uf->realloc(NULL, sizeof(struct B_Priv));
    bc->_->uf = uf;
    bc->_->code_alloc = 0;
    return bc;
}


extern int parse(Bytecode* b, const char* code);  // defined in parser.y

Bytecode* 
bytecode_newfromcode(UserFunctions *uf, const char* code)
{
    Bytecode* bc = bytecode_new(uf);
    parse(bc, code);
    return bc;
}


Bytecode* 
bytecode_newfromzb(UserFunctions *uf, uint8_t* data, size_t sz)
{
    // validate magic code
    static uint8_t magic[] = { 0x90, 0x6F, 0x65, 0x20, 0xEB, 0x00 };
    if(sz < 0x38 || memcmp(magic, data, sizeof magic) != 0) {
        uf->error("Not a ZB file.");
        return NULL;
    }

    // validate version
    if(data[6] != 1 || data[7] != 0) {
        uf->error("ZB version unsupported");
        return NULL;
    }

    // load public fields
    Bytecode* bc = bytecode_new(uf);
    bc->version_minor = data[6];
    bc->version_major = data[7];
    memcpy(&bc->code_sz, &data[0x10], 8);
    bc->code = uf->realloc(NULL, bc->code_sz);
    memcpy(bc->code, &data[0x38], bc->code_sz);
    
    // load private fields
    bc->_->code_alloc = bc->code_sz;

    return bc;
}


void
bytecode_free(Bytecode* bc)
{
    if(bc->code) {
        free(bc->code);
    }
    if(bc->_) {
        free(bc->_);
    }
    free(bc);
}

// }}}

// {{{ ADD CODE

void
bytecode_addcode(Bytecode* bc, uint8_t code)
{
    if(bc->code_sz >= bc->_->code_alloc) {
        bc->_->code_alloc = (bc->_->code_alloc+1) * 2;
        bc->code = bc->_->uf->realloc(bc->code, bc->_->code_alloc);
    }
    bc->code[bc->code_sz++] = code;
}


void
bytecode_addcodef64(Bytecode* bc, double code)
{
    union {
        double a;
        uint8_t b[8];
    } c = { .a = code };
    
    for(int i=0; i<8; ++i) {
        bytecode_addcode(bc, c.b[i]);
    }
}

// }}}

// {{{ GENERATE ZB FILE

size_t
bytecode_generatezb(Bytecode* bc, uint8_t** data)
{
    size_t sz = 0x38 + bc->code_sz;  // TODO
    *data = bc->_->uf->realloc(NULL, sz);

    // headers
    static uint8_t header[] = { 
        0x90, 0x6F, 0x65, 0x20, 0xEB, 0x00, ZB_VERSION_MINOR, ZB_VERSION_MAJOR,
    };
    memcpy(*data, &header, sizeof header);

    // indices
    uint64_t idx[] = {
        0x38,           // code_pos
        bc->code_sz,    // code_sz
        0x0,            // data_pos  TODO
        0x0,            // data_sz   TODO
        0x0,            // debug_pos TODO
        0x0,            // debug_sz  TODO
    };
    for(int i=0; i<6; ++i) {
        memcpy(&(*data)[0x8 + (i*8)], &idx[i], 8);
    }

    // data
    memcpy(&(*data)[0x38], bc->code, bc->code_sz);
    // TODO - others

    return sz;
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
