#include "src/repl.h"

#include <stdio.h>
#include <string.h>
#include <readline/history.h>
#include <readline/readline.h>

#include "lib/zoe.h"

void repl_execute(Options* opt)
{
    Zoe* Z = zoe_createvm(NULL);
#ifdef DEBUG
    if(opt->debug_asm) {
        zoe_asmdebugger(Z, true);
    }
#endif

    char* buf;
    
    rl_bind_key('\t', rl_abort);

    while((buf = readline("(zoe) ")) != NULL) {

        if(!buf) {
            continue;
        }

        // read input
        if(strcmp(buf, ".q") == 0) {
            free(buf);
            break;
        }
        add_history(buf);

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

    // free history list
    if(where_history() > 0) {
        HIST_ENTRY** h = history_list();
        size_t i = 0;
        while(h[i]) {
            free_history_entry(h[i]);
            ++i;
        }
    }

    zoe_free(Z);
}

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=c
