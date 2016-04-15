#ifndef EXE_OPTIONS_H_
#define EXE_OPTIONS_H_

#include <ostream>
using namespace std;

class Options {
public:
    Options(int argc, char* argv[]);

    enum OperationMode { REPL };
    OperationMode mode = OperationMode::REPL;

    bool repl_disassemble = false;
    bool trace = false;
    bool debug_bison = false;

private:
    [[noreturn]] void PrintHelp(ostream& ss, int status) const;
};

#endif

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=cpp
