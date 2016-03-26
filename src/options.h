#ifndef OPTIONS_H_
#define OPTIONS_H_

#include <cstdio>

class Options {
public:
    Options(int argc, char* argv[]);

    enum OperationMode { REPL };
    const Options::OperationMode operation_mode;

    struct ReplOptions {
        bool disassemble;
    } repl_options;

private:
    void PrintHelp(FILE* f, int exit_status) const __attribute__((noreturn));
};

#endif

// vim: ts=4:sw=4:sts=4:expandtab
