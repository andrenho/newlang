#include "lib/bytecode.h"

#include <inttypes.h>
#include <string.h>

#include "lib/opcode.h"

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

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=c
