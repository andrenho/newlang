#include "exe/exec.hh"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <readline/readline.h>
#include <readline/history.h>

#include <fstream>
#include <iostream>
#include <sstream>

#include "exe/options.hh"
#include "compiler/bytecode.hh"
#include "vm/zoevm.hh"

// ANSI colors for console output
#define DIMMAGENTA "\033[2;34m"
#define GREEN      "\033[1;32m"
#define RED        "\033[1;31m"
#define GRAY       "\033[1;30m"
#define NORMAL     "\033[0m"


void execute_repl(class Options const& opt)
{{{
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
            if(opt.disassemble) {
                cout << GRAY << b.Disassemble() << NORMAL;
            }

            // execute
            Z.ExecuteBytecode(b.GenerateZB());

            // display result
            cout << GREEN << Z.GetPtr()->Inspect() << NORMAL << "\n";

        // catch errors
        } catch(exception const& e) {
            cout << RED << "error: " << e.what() << NORMAL << "\n";
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
}}}


void execute_files(vector<string> const& files, class Options const& opt)
{{{
    ZoeVM Z;
    if(opt.trace) {
        Z.Tracer = true;
    }

    for(auto const& file: files) {

        // load file
        ifstream f(file);
        if(f.fail()) {
            cerr << RED << "Error opening file '" << file << "': " << strerror(errno) << "\n" << NORMAL;
            exit(EXIT_FAILURE);
        }
        stringstream buffer;
        buffer << f.rdbuf();

        try {
            // parse code
            Bytecode b(buffer.str());
            if(opt.disassemble) {
                cout << GRAY << b.Disassemble() << NORMAL;
            }

            // run code
            Z.ExecuteBytecode(b.GenerateZB());
        } catch(exception const& e) {
            cerr << RED << "error: " << e.what() << NORMAL << "\n";
            exit(EXIT_FAILURE);
        }
    }
}}}


// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=cpp
