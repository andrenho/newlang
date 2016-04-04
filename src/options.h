#ifndef SRC_OPTIONS_H_
#define SRC_OPTIONS_H_

#include <stdbool.h>

typedef enum { REPL } OperationMode;

typedef struct {
#ifdef DEBUG
    bool disassemble;
#endif
} REPL_Options;

typedef struct {
    OperationMode mode;
    REPL_Options  repl_options;
#ifdef DEBUG
    bool          debug_asm;
    bool          debug_gc;
#endif
} Options;

Options* options_parse(int argc, char* argv[]);
void     options_free(Options* opt);

#endif

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=c
