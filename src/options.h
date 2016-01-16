#ifndef SRC_OPTIONS_H_
#define SRC_OPTIONS_H_

#include <stdbool.h>

typedef enum { REPL } OperationMode;

typedef struct {
    bool disassemble;
} REPL_Options;

typedef struct {
    OperationMode mode;
    REPL_Options  repl_options;
} Options;

Options* options_parse(int argc, char* argv[]);
void     options_free(Options* opt);

#endif

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=c
