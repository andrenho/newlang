#include "compiler.h"

#include <stdbool.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct yy_buffer_state* YY_BUFFER_STATE;
extern int yyparse(void);
extern YY_BUFFER_STATE yy_scan_string(char* str);
extern void yy_delete_buffer(YY_BUFFER_STATE buffer);

struct Context {
    bool     active;
    uint8_t* buffer;
    size_t   size;
    size_t   allocd;
} ctx = {
    .active = false,
    .buffer = NULL,
    .size   = 0,
    .allocd = 0,
};


bool
compile(char* code, uint8_t** buf, size_t* sz)
{
    (void) code;
    (void) buf;
    (void) sz;

    ctx.active = true;
    ctx.size = 0;
    ctx.allocd = 0;
    ctx.buffer = *buf;

    YY_BUFFER_STATE state = yy_scan_string(code);
    int r = yyparse();
    yy_delete_buffer(state);

    if(r == 1) {
        fprintf(stderr, "Parsing failed due to a syntax error.\n");
        return false;
    } else if(r == 2) {
        fprintf(stderr, "Parsing failed due to memory exausting.\n");
        return false;
    }

    *buf = ctx.buffer;
    *sz = ctx.size;
    ctx.active = false;

    return true;
}


static void cp_push(uint8_t byte) {
    if(ctx.size + 1 > ctx.allocd) {
        ctx.allocd += ALLOC_STEP;
        ctx.buffer = realloc(ctx.buffer, ctx.allocd);
    }
    ctx.buffer[ctx.size] = byte;
    ++ctx.size;
}


static void cp_push_integer(uint64_t value) {
    cp_push((uint8_t)value);
    cp_push((uint8_t)(value >> 8));
    cp_push((uint8_t)(value >> 16));
    cp_push((uint8_t)(value >> 24));
    cp_push((uint8_t)(value >> 32));
    cp_push((uint8_t)(value >> 40));
    cp_push((uint8_t)(value >> 48));
    cp_push((uint8_t)(value >> 56));
}


void cp_add(Command cmd, ...)
{
    va_list ap;

    va_start(ap, cmd);
    
    cp_push(cmd);
    switch(cmd) {
        case PUSH_I:
            cp_push_integer(va_arg(ap, int64_t));
            break;
        default:
            abort();
    }

    va_end(ap);
}


// {{{  DISASSEMBLY

bool 
zoe_disassemble(uint8_t* data, size_t sz)
{
    (void) vm;

    size_t p = 0;
    while(p < sz) {
        switch(data[p]) {
            case PUSH_I:
                printf("\tpush_i\t%" PRId64 "\n", get_integer(data, p+1));
                p += 9;
                break;
        }
    }

    return true;
}

// }}}

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker
