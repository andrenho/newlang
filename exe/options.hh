#ifndef EXE_OPTIONS_HH_
#define EXE_OPTIONS_HH_

#include <ostream>
#include <string>
#include <vector>
using namespace std;

class Options {
public:
    Options(int argc, char* argv[]);

    enum OperationMode { REPL, NONINTERACTIVE };
    OperationMode mode = OperationMode::REPL;

    bool disassemble = false;
    bool trace = false;
    bool debug_bison = false;

    vector<string> scripts_filename = {};

private:
    [[noreturn]] void PrintHelp(ostream& ss, int status) const;
};

#endif

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=cpp
