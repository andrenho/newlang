#ifndef EXE_REPL_H_
#define EXE_REPL_H_

#include <string>
using namespace std;

class REPL {
public:
    void Execute(class Options const& opt);

private:
    string ReadLine(bool& alive);
};

#endif

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=cpp
