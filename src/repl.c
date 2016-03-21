#include "repl.h"

#include <stdio.h>
#include <stdlib.h>

#include <readline/readline.h>
#include <readline/history.h>

void 
repl_exec(Options opt)
{
    (void) opt;

    char* buf;

    rl_bind_key('\t', rl_abort);

    while((buf = readline(">")) != NULL) {
        if(strcmp(buf, ".q") == 0) {
            free(buf);
            break;
        }
        
        printf("[%s]\n", buf);

        if(buf[0] != '\0') {
            add_history(buf);
        }
        free(buf);
    }


    clear_history();
}

// vim: ts=4:sw=4:sts=4:expandtab
