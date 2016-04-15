#include "src/repl.h"

#include <readline/readline.h>
#include <readline/history.h>

#include <iostream>

#include "src/options.h"
#include "lib/zoe.h"

#define DIMMAGENTA "\033[2;34m"
#define GREEN      "\033[1;32m"
#define RED        "\033[1;31m"
#define GRAY       "\033[1;30m"
#define NORMAL     "\033[0m"

void REPL::Execute(class Options const& opt)
{
    Zoe Z;
    char* buf;

#ifdef DEBUG
    if(opt.trace) {
        Z.Tracer = true;
    }
#endif
    
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
        try {
            cout << DIMMAGENTA << flush;
            Z.Eval(string(buf)); free(buf);
            cout << NORMAL << flush;
#ifdef DEBUG
            if(opt.repl_disassemble) {
                cout << GRAY << Z.Disassemble(-1) << NORMAL;
            }
#endif
            Z.Call(0);

            // display result
            Z.Inspect(-1);
            cout << GREEN << Z.Pop<string>() << NORMAL << endl;

        // catch errors
        } catch(string& e) {
            cout << RED << e << NORMAL << endl;
        } catch(const char* e) {
            cout << RED << e << NORMAL << endl;
        }
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
    
}


// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=cpp
