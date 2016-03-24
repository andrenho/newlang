#include "options.h"

Options::Options(int argc, char* argv[])
    : operation_mode(OperationMode::REPL),
      repl_options({ true })
{
    (void) argc;
    (void) argv;

    // TODO
}


// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=syntax
