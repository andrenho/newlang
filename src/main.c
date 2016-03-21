#include "options.h"
#include "repl.h"
#include "vm.h"

extern void yyparse(void);

int main(int argc, char** argv) { 
    Options* opt = parse_args(argc, argv);
    VM* vm = vm_init();

    switch(opt->operation_mode) {
        case REPL:
            repl_exec(opt, vm);
            break;
    }

    vm_free(&vm);
    options_free(&opt);
    return 0; 
}


// vim: ts=4:sw=4:sts=4:expandtab
