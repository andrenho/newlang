#include "options.h"
#include "repl.h"

int main(int argc, char* argv[])
{
    Options opt(argc, argv);

    switch(opt.operation_mode) {
        case Options::OperationMode::REPL:
            REPL().Execute(opt);
            break;
    }
}

// vim: ts=4:sw=4:sts=4:expandtab
