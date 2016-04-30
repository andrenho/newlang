#include "exe/repl.hh"

#include <cstdio>
#include <cstring>
#include <readline/readline.h>
#include <readline/history.h>

#include <iostream>

#include "exe/options.hh"
#include "compiler/bytecode.hh"
#include "vm/zoevm.hh"

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
    ZoeVM Z;
    if(opt.trace) {
        Z.Tracer = true;
    }
    
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

        try {
            // evaluate expression
            cout << DIMMAGENTA << flush;
            Bytecode b(buf); free(buf);   // text is printed here if debug-bison is active
            cout << NORMAL << flush;

            // disassemble expression
            if(opt.repl_disassemble) {
                cout << GRAY << b.Disassemble() << NORMAL;
            }

            // execute
            Z.ExecuteBytecode(b.GenerateZB());

            // display result
            cout << GREEN << Z.GetPtr()->Inspect() << NORMAL << endl;

        // catch errors
        } catch(exception const& e) {
            cout << RED << "error: " << e.what() << NORMAL << endl;
            exit(EXIT_FAILURE);
        }
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
