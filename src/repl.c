#include "src/repl.h"

#include <stdio.h>
#include <string.h>

#include "lib/zoe.h"

void repl_execute(Options* opt)
{
    Zoe* Z = zoe_createvm(NULL);
#ifdef DEBUG
    if(opt->debug_asm) {
        zoe_asmdebugger(Z, true);
    }
#endif

    while(1) {
        char* buf = NULL;
        size_t sz = 0;

        // read input
        printf("> ");
        ssize_t r = getline(&buf, &sz, stdin);
        if(r == -1) {
            free(buf);
            break;
        }
        if(strcmp(buf, ".q\n") == 0) {
            free(buf);
            break;
        }

        // run code
        zoe_eval(Z, buf);
#ifdef DEBUG
        if(opt->repl_options.disassemble) {
            zoe_disassemble(Z);
            printf("\033[1;30m");  // gray font
            char* txt = zoe_popstring(Z);
            printf("%s", txt);
            free(txt);
            printf("\033[0m");  // TTY normal
        }
#endif
        zoe_call(Z, 0);

        // display result
        zoe_inspect(Z, -1);
        char* result = zoe_popstring(Z);
        printf("%s\n", result);
        free(result);

        // pop the result
        zoe_pop(Z, 1);

        free(buf);
    }

    zoe_free(Z);
}

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=c
