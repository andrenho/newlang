#include "options.h"

Options 
parse_args(int argc, char** argv)
{
    (void) argc;
    (void) argv;

    return (Options){
        .operation_mode = REPL,
    };
}


// vim: ts=4:sw=4:sts=4:expandtab
