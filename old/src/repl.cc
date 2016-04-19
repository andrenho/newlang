#include "src/repl.h"

#include <cstdio>
#include <readline/readline.h>
#include <readline/history.h>

#include <iostream>

#include "src/options.h"
#include "lib/zoe.h"

// ANSI colors for console output
#define DIMMAGENTA "\033[2;34m"
#define GREEN      "\033[1;32m"
#define RED        "\033[1;31m"
#define GRAY       "\033[1;30m"
#define NORMAL     "\033[0m"


void repl_execute(class Options const& opt)
{
    (void) opt;

    char* buf;
    rl_bind_key('\t', rl_abort);     // disable TAB in readline

    // initialize VM
    Zoe Z;
#ifdef DEBUG
    if(opt.trace) {
        Z.Tracer = true;
    }
#endif
    
    // read input
    while((buf = readline("(zoe) ")) != NULL) {

        if(!buf) {
            continue;
        }

        if(strcmp(buf, ".q") == 0) {
            free(buf);
            break;
        }
        add_history(buf);

#ifndef DEBUG
        // we catch errors only if not in development mode
        try {
#endif
            // evaluate expression
            cout << DIMMAGENTA << flush;
            Z.Eval(string(buf)); free(buf);
            cout << NORMAL << flush;

            // disassemble expression
#ifdef DEBUG
            if(opt.repl_disassemble) {
                cout << GRAY << Z.Disassemble(-1) << NORMAL;
            }
#endif

            // execute expression
            Z.Call(0);

            // verify execution

            // display result
            Z.Inspect(-1);
            cout << GREEN << Z.Pop<string>() << NORMAL << endl;

#ifndef DEBUG
        // catch errors
        } catch(exception const& e) {
            cout << RED << "error: " << e.what() << NORMAL << endl;
            exit(EXIT_FAILURE);
        }
#endif
    }

    // free history list (for a "almost" clear valgrind output)
    if(where_history() > 0) {
        HIST_ENTRY* const* h = history_list();
        size_t i = 0;
        while(h[i]) {
            free_history_entry(h[i]);
            ++i;
        }
    }
    
}


// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=cpp
