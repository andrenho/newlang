#ifndef EXE_REPL_H_
#define EXE_REPL_H_

#include <string>
#include <vector>
using namespace std;

void execute_repl(class Options const& opt);
void execute_files(vector<string> const& files, class Options const& opt);

#endif

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=cpp
