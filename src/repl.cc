#include "repl.h"

#include <cstdio>
#include <iostream>
#include <string>
using namespace std;

#include "options.h"
#include "zoe.h"
#include "bytecode.h"
using namespace Zoe;

void REPL::Execute(Options const& opt) const
{
    Zoe::Zoe zoe;

    while(1) {
        string buf;

        // read input
        cout << "> ";
        getline(cin, buf);
        if(buf == ".q" || cin.eof()) {
            break;
        }

        // run code
        zoe.LoadCode(buf);
        if(opt.repl_options.disassemble) {
            Bytecode::Disassemble(cout, zoe.Dump(0));
        }
        /*
        zoe.Call(0);

        // inspect
        cout << zoe.Inspect(0) << endl;
        */
    }
}

// vim: ts=4:sw=4:sts=4:expandtab
