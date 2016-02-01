#include "lib/zoe.h"

#include "src/options.h"
#include "src/repl.h"

int main(int argc, char* argv[])
{
    Options opt(argc, argv);

    switch(opt.mode) {
        case Options::REPL:
            repl_execute(opt);
            break;
    }
}

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=cpp
