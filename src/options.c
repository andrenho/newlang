#include "options.h"

#include <stdlib.h>

Options* 
parse_args(int argc, char** argv)
{
    (void) argc;
    (void) argv;

    Options* opt = calloc(sizeof(Options), 1);
    opt->operation_mode = REPL;
    return opt;
}


void 
options_free(Options** opt)
{
    free(*opt);
    *opt = NULL;
}


// vim: ts=4:sw=4:sts=4:expandtab
