#include "src/repl.h"

#include <stdio.h>
#include <string.h>

#include "lib/zoe.h"

void repl_execute(Options* opt)
{
    Zoe* Z = zoe_createvm(NULL);

    while(1) {
        char* buf = NULL;
        size_t sz = 0;

        // read input
        printf("> ");
        ssize_t r = getline(&buf, &sz, stdin);
        if(r == -1) {
            free(buf);
            break;
        } else {
            buf[strlen(buf)-1] = 0;  // remove enter
        }
        if(strcmp(buf, ".q") == 0) {
            free(buf);
            break;
        }

        // run code
        /* TODO
        zoe.LoadCode(buf);
        if(opt.repl_options.disassemble) {
            Bytecode::Disassemble(stderr, zoe.Dump(0));
        }
        zoe.Call(0);

        // inspect
        cout << zoe.Inspect() << endl;
        zoe.Pop();
        */

        free(buf);
    }

    zoe_free(Z);
}

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=c
