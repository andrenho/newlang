#include "exe/options.hh"
#include "exe/exec.hh"

#include <cstdlib>

int main(int argc, char* argv[])
{
    Options const opt(argc, argv);

    switch(opt.mode) {
        case Options::REPL:
            execute_repl(opt);
            break;
        case Options::NONINTERACTIVE:
            execute_files(opt.scripts_filename, opt);
            break;
        default:
            abort();
    }
}

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=cpp
