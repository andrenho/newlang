#include <stdlib.h>

#include "src/options.h"
#include "src/repl.h"

int main(int argc, char* argv[])
{
    Options* opt = options_parse(argc, argv);

    switch(opt->mode) {
        case REPL:
            repl_execute(opt);
            break;
    }

    options_free(opt);
    return EXIT_SUCCESS;
}

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=c
