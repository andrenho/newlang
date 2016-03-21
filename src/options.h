#ifndef OPTIONS_H_
#define OPTIONS_H_

#include <stdbool.h>

typedef enum {
    REPL,
} OperationMode;

typedef struct Options {
    OperationMode operation_mode;
    struct { 
        bool show_opcodes;
    } repl_options;
} Options;

Options* parse_args(int argc, char** argv);
void options_free(Options** opt);

#endif

// vim: ts=4:sw=4:sts=4:expandtab
