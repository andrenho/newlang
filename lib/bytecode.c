#include "lib/bytecode.h"

#include <inttypes.h>
#include <string.h>

#include "lib/opcode.h"

#define NO_ADDRESS ((uint64_t)(~0))

typedef struct LabelRef {
    uint64_t  address;
    size_t    n_refs;
    uint64_t* refs;
} LabelRef;

struct B_Priv {
    ERROR     errorf;
    size_t    code_alloc;
    LabelRef* labels;
    size_t    labels_sz;
};

// {{{ CONSTRUCTOR/DESTRUCTOR

Bytecode*
bytecode_new(ERROR errorf)
{
    Bytecode* bc = calloc(sizeof(Bytecode), 1);
    bc->_ = calloc(sizeof(struct B_Priv), 1);
    bc->_->errorf = errorf;
    return bc;
}


extern int parse(Bytecode* b, const char* code);  // defined in parser.y

Bytecode*
bytecode_newfromcode(const char* code, ERROR errorf)
{
    Bytecode* bc = bytecode_new(errorf);
    parse(bc, code);
    return bc;
}


Bytecode*
bytecode_newfromzb(uint8_t* data, size_t sz, ERROR errorf)
{
    // validate magic code
    static uint8_t magic[] = { 0x90, 0x6F, 0x65, 0x20, 0xEB, 0x00 };
    if(sz < 0x38 || memcmp(magic, data, sizeof magic) != 0) {
        errorf("Not a ZB file.");
        return NULL;
    }

    // validate version
    if(data[6] != 1 || data[7] != 0) {
        errorf("ZB version unsupported");
        return NULL;
    }

    // load public fields
    Bytecode* bc = bytecode_new(errorf);
    bc->version_minor = data[6];
    bc->version_major = data[7];
    memcpy(&bc->code_sz, &data[0x10], 8);
    bc->code = calloc(bc->code_sz, 1);
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
    for(size_t i=0; i<bc->_->labels_sz; ++i) {
        free(bc->_->labels[i].refs);
    }
    free(bc->_->labels);
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
        bc->code = realloc(bc->code, bc->_->code_alloc);
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


void
bytecode_addcodestr(Bytecode* bc, const char* str)
{
    int c = 0;
    while(str[c]) {
        bytecode_addcode(bc, str[c++]);
    }
    bytecode_addcode(bc, 0);
}

// }}}

// {{{ LABELS

Label
bytecode_createlabel(Bytecode* bc)
{
    bc->_->labels = realloc(bc->_->labels, (bc->_->labels_sz+1) * sizeof(LabelRef));
    bc->_->labels[bc->_->labels_sz] = (LabelRef) {
        .address = NO_ADDRESS,
        .n_refs = 0,
        .refs = NULL,
    };

    ++bc->_->labels_sz;
    return bc->_->labels_sz - 1;
}


void
bytecode_addcodelabel(Bytecode* bc, Label lbl)
{
    LabelRef* lb = &bc->_->labels[lbl];

    // add reference to label
    lb->refs = realloc(lb->refs, (lb->n_refs+1) * sizeof(uint64_t));
    lb->refs[lb->n_refs] = bc->code_sz;
    ++lb->n_refs;

    // add filler bytes, that will later be replaced by the address
    for(int i=0; i<8; ++i) {
        bytecode_addcode(bc, 0);
    }
}


void
bytecode_setlabel(Bytecode* bc, Label lbl)
{
    bc->_->labels[lbl].address = bc->code_sz;
}


// }}}

// {{{ GENERATE ZB FILE

static void
bytecode_adjust_labels(Bytecode* bc)
{
    LabelRef* lbs = bc->_->labels;
    for(size_t i=0; i < bc->_->labels_sz; ++i) {
        for(size_t j=0; j < lbs[i].n_refs; ++j) {
            if(lbs[i].address == NO_ADDRESS) {
                bc->_->errorf("Label without a corresponding address.");
            }
            if(lbs[i].address > bc->code_sz+8) {
                bc->_->errorf("Label beyond code size.");
            }
            memcpy(&bc->code[lbs[i].refs[j]], &lbs[i].address, __SIZEOF_DOUBLE__);
        }
    }
}


size_t
bytecode_generatezb(Bytecode* bc, uint8_t** data)
{
    size_t sz = 0x38 + bc->code_sz;  // TODO
    *data = calloc(sz, 1);

    // adjust labels
    bytecode_adjust_labels(bc);

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
