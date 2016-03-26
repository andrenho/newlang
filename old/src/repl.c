#include "repl.h"

#include <stdio.h>
#include <string.h>

#include "bytecode.h"
#include "zoe.h"

#define BUFSZ 1024


void 
repl_exec(Options* opt)
{
    (void) opt;

    Zoe* zoe = zoe_new();

    char buf[BUFSZ];
    while(1) {
        // get input
        printf("> ");
        memset(buf, 0, BUFSZ);
        fgets(buf, BUFSZ, stdin);
        if(strcmp(buf, ".q") == 0 || buf[0] == 0) {
            break;
        }

        // run code
        zoe_load_buffer(zoe, (uint8_t *)buf, strlen(buf));
        if(opt->repl_options.disassemble) {
            uint8_t* bc_dump;
            size_t sz;
            if(zoe_dump(zoe, &bc_dump, &sz)) {
                bc_disassemble(stdout, bc_dump, sz);
                free(bc_dump);
            }
        }
        zoe_call(zoe, 0);

        // inspect return value
        /*
        char* insp;
        zoe_inspect(zoe, 0, &insp);
        printf("%s\n", insp);
        free(insp);
        */
    }

    zoe_terminate(&zoe);
}

// vim: ts=4:sw=4:sts=4:expandtab
