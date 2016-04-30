#include "exe/options.hh"
#include "exe/repl.hh"

int main(int argc, char* argv[])
{
    Options const opt(argc, argv);

    switch(opt.mode) {
        case Options::REPL:
            repl_execute(opt);
            break;
    }
}

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=cpp
