#include "repl.h"

#include <stdio.h>
#include <stdlib.h>

#include <readline/readline.h>
#include <readline/history.h>


static void 
repl_run_line(const char* code, Options* opt, VM* vm)
{
    (void) opt;

    // compile
    uint8_t** buf = NULL;
    size_t buf_sz;
    if(!cp_compile(code, &buf, &buf_sz)) {
        fprintf(stderr, "repl: %s\n", buf);
        free(buf);
    }

    // debug
    if(opt->repl_options.show_opcodes) {
        vm_disassemble(vm, buf, buf_sz);
    }

    vm_run(buf, buf_sz);

    /*
    char* insp_buf;
    cp_inspect_stack_pos(cp, -1, &insp_buf);
    printf("%s\n", insp_buf);
    free(insp_buf);
    */
}


void 
repl_exec(Options* opt, VM* vm)
{
    (void) opt;

    char* buf;

    rl_bind_key('\t', rl_abort);

    while((buf = readline(">")) != NULL) {
        if(strcmp(buf, ".q") == 0) {
            free(buf);
            break;
        }
        
        repl_run_line(buf, opt, vm);

        if(buf[0] != '\0') {
            add_history(buf);
        }
        free(buf);
    }


    // free history
    HIST_ENTRY** hl = history_list();
    if(hl) {
        for(int i=0; hl[i]; ++i) {
            free_history_entry(hl[i]);
        }
    }
}

// vim: ts=4:sw=4:sts=4:expandtab
