#include "options.h"
#include "repl.h"
#include "zoe.h"

extern void yyparse(void);

int main(int argc, char** argv) { 
    Options* opt = parse_args(argc, argv);
    Zoe* zoe = zoe_init();

    switch(opt->operation_mode) {
        case REPL:
            repl_exec(opt, zoe);
            break;
    }

    zoe_free(&zoe);
    options_free(&opt);
    return 0; 
}


// vim: ts=4:sw=4:sts=4:expandtab
