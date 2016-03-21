#ifndef OPTIONS_H_
#define OPTIONS_H_

typedef enum {
    REPL,
} OperationMode;

typedef struct Options {
    OperationMode operation_mode;
} Options;

Options parse_args(int argc, char** argv);

#endif

// vim: ts=4:sw=4:sts=4:expandtab
